#ifndef __KMEANS_HH__

#define __KMEANS_HH__



#include <iostream>

#include <random>

#include <tuple>

#include <vector>



using std::cout;

using std::endl;

using std::ostream;

using std::pair;

using std::tie;

using std::vector;



using Point = vector<double>;

using Clustering = vector<size_t>;



pair<Clustering, vector<Point>> kmeans(const vector<Point>& dataset, size_t k,

                                       double epsilon, bool bootstrap);



ostream& operator<<(ostream& os, const Point& p) {

  os << "<";

  for (size_t i = 0; i < p.size(); i++) {

    os << " " << p[i];

  }

  os << ">";

  return os;

}



void printClustering(const vector<Point>& dataset, const Clustering& clustering,

                     const vector<Point>& centroids, size_t k) {

  /*

    for (size_t i = 0; i < dataset.size(); i++) {

      cout << dataset[i] << " -> " << clustering[i] << endl;

    }

    */

  size_t n = dataset.size();

  vector<size_t> count(k, 0);

  for (size_t i = 0; i < n; i++) {

    size_t ci = clustering[i];

    count[ci]++;

  }



  for (size_t i = 0; i < k; i++) {

    cout << " cluster " << i << ": " << count[i] << " center: " << centroids[i]

         << endl;

  }

}



/**

 * Runs kmeans on a subset of the dataset to select the centroids.

 */

vector<Point> kmeansPoints(size_t k, const vector<Point>& ds, double percent,

                           double epsilon) {

  std::random_device rd;

  std::mt19937 generator(rd());

  std::uniform_int_distribution<> unif(0, ds.size() - 1);



  size_t dim = ds[0].size();

  size_t n = floor(ds.size() * percent);

  vector<Point> subset(n, Point(dim, 0.0));



  for (size_t i = 0; i < n; i++) {

    size_t r = unif(rd);

    subset.push_back(ds[r]);

  }



  // Run kmeans on the subset

  Clustering clustering(n, 0);

  vector<Point> centroids(k, Point(dim, 0.0));



  tie(clustering, centroids) = kmeans(subset, k, epsilon, true);



  return centroids;

}



/**

 * Randomly selects k points from the dataset to initialize centroids.

 */

vector<Point> randomPoints(size_t k, const vector<Point>& ds) {

  std::random_device rd;

  std::mt19937 generator(rd());

  std::uniform_int_distribution<> unif(0, ds.size() - 1);



  size_t dim = ds[0].size();

  vector<Point> c(k, Point(dim, 0.0));

  for (size_t i = 0; i < k; i++) {

    // The following random generation is deterministic and for testing

    // purposes.

    size_t r = std::rand() % ds.size();

    // This is the real random number generation

    // size_t r = unif(rd);

    c[i] = ds[r];

  }

  return c;

}



/**

 * Euclidean distance (squared)

 */

double sqdistance(const Point& p, const Point& q) {

  double d = 0.0;

  for (size_t i = 0; i < p.size(); i++) {

    d += pow(p[i] - q[i], 2);

  }

  return d;

}



/**

 * Find the closest centroid to p an returns both the centroid and the distance.

 */

pair<size_t, double> closestCentroid(const Point& p,

                                     const vector<Point>& centroids) {

  double d = std::numeric_limits<double>::max();

  size_t c = 0;



  for (size_t i = 0; i < centroids.size(); i++) {

    double dt = sqdistance(p, centroids[i]);

    if (dt < d) {

      d = dt;

      c = i;

    }

  }



  return {c, d};

}



pair<Clustering, double> cluster(const vector<Point>& dataset,

                                 const vector<Point>& centroids) {

  size_t n = dataset.size();

  Clustering clustering(n, 0);

  double ssd = 0.0;

  for (size_t i = 0; i < n; i++) {

    size_t c;

    double d;

    tie(c, d) = closestCentroid(dataset[i], centroids);

    clustering[i] = c;

    ssd += d;

  }

  return {clustering, ssd};

}



vector<Point> newCentroids(const Clustering& clustering,

                           const vector<Point>& dataset,

                           vector<Point>& centroids) {

  size_t k = centroids.size();

  size_t dim = centroids[0].size();

  vector<Point> newCentroids(k, Point(dim, 0.0));

  vector<double> count(k, 0.0);

  for (size_t i = 0; i < dataset.size(); i++) {

    size_t ci = clustering[i];

    for (size_t j = 0; j < dim; j++) {

      newCentroids[ci][j] += dataset[i][j];

    }

    count[ci]++;

  }



  for (size_t i = 0; i < k; i++) {

    for (size_t j = 0; j < dim; j++) {

      newCentroids[i][j] /= count[i];

    }

  }

  return newCentroids;

}



pair<Clustering, vector<Point>> kmeans(const vector<Point>& dataset, size_t k,

                                       double epsilon, bool bootstrap = false) {



  size_t dim = dataset[0].size();

  size_t n = dataset.size();



  cout << "Kmeans on " << n << " points and " << dim << " dimensions" << endl;



  vector<Point> centroids;

  if (bootstrap) {

    centroids = kmeansPoints(k, dataset, 0.2, epsilon);

  } else {

    centroids = randomPoints(k, dataset);

  }

  Clustering clustering(n, 0);

  double ssd = 0.0;

  double ssdPrev = 0.0;

  double d;

  size_t iter = 0;



  do {

    ssdPrev = ssd;

    cout << "Iteration " << iter << endl;

    tie(clustering, ssd) = cluster(dataset, centroids);

    cout << "SSD: " << ssd << endl;

    // printClustering(dataset, clustering);

    centroids = newCentroids(clustering, dataset, centroids);

    iter++;

    d = abs(ssdPrev - ssd);

    cout << "----> " << d << endl;

  } while (d > epsilon);



  return {clustering, centroids};

}



#endif