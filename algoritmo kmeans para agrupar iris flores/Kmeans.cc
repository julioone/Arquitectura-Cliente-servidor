// g++ -std=c++11 -fopenmp -o kmeans kmeans.cc

// ./kmeans iris.data

#include "kmeans.hh"

#include <fstream>

#include <iostream>

#include <omp.h>

#include <random>

#include <vector>



using namespace std;



/**

 * Reads the Iris dataset

 */

vector<Point> readIris(const string& fname) {

  vector<Point> ds;

  ifstream input(fname);

  double sl;

  double sw;

  double pl;

  double pw;

  string label;

  while (input >> sl >> sw >> pl >> pw >> label) {

    Point p{sl, sw, pl, pw};

    // cout << p << endl;

    ds.push_back(move(p));

  }

  cout << "Input has " << ds.size() << " points." << endl;

  return ds;

}



int main(int argc, char** argv) {

  if (argc != 2)

    return -1;

  string fname(argv[1]);

  vector<Point> ds = readIris(fname);

  size_t dim = ds[0].size();

  Clustering clustering(150, 0);

  vector<Point> centroids(3, Point(dim, 0.0));

  tie(clustering, centroids) = kmeans(ds, 3, 0.001);

  printClustering(ds, clustering, centroids, 3);

  return 0;

}