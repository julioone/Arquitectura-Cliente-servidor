// g++ -std=c++11 -fopenmp -o kmeans kmeans.cc
// ./kmeans [namearchivo]
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <random>
#include <omp.h>
#include <ctime>



#include <cassert>
// como se debe hacer los centroides
// para calcular la similaridad angular que hay que hacer
//
using namespace std;
using SPoint = vector<pair<size_t, double>>;
using rate = vector<double>;

using Rates = map<pair<string, string>, double>; // (user,movie) -> rate


void printClustering(const vector<SPoint>& dataset,
                     const vector<size_t>& clustering, size_t k) {

  
  
  size_t n = dataset.size();
  vector<size_t> count(k, 0);
  for (size_t i = 0; i < n; i++) {
    size_t ci = clustering[i];
    count[ci]++;
  }

  for(size_t i = 1; i <= k; i++) {
    cout << " cluster " << i << ": " << count[i] << endl;
  }
}
vector<double> normcentroid(const vector<rate>& centroids){
	vector<double> normacentros(centroids.size(),0);
	double dchunk = (double)centroids.size()/4;
	uint chunk = ceil(dchunk);
	
	for(size_t i=0;i< centroids.size();i++){
		for(size_t j=1;j<=centroids[0].size();j++){
			normacentros[i]+=pow(centroids[i][j],2);
		}
	}
	}	
	for(size_t i=0; i<normacentros.size();i++){
		normacentros[i]=sqrt(normacentros[i]);
		//cout << "normadelcentroide "<< i<< ": " << normacentros[i]<<endl;
	

	return normacentros;
}
vector<double> normuser(const vector<SPoint>& dataset){
	vector<double> norma(dataset.size(),0);



	for (size_t i=1; i<=dataset.size();i++ ){


    	for(size_t j=0;j < dataset[i].size();j++){

    	norma[i]+= pow(dataset[i][j].second,2);
      	

    	}

  	}
  	for (size_t i=1;i <= norma.size();i++){

  		norma[i]= sqrt(norma[i]);
  	}
  	return norma;
}
void printnetflix(const vector<SPoint>& dataset){
  for (size_t i=1; i<=dataset.size();i++ ){
    cout << "IDUSER: "<< i<< endl;
    for(size_t j=0;j< dataset[i].size();j++){
      cout<< "idpelicula: "<<dataset[i][j].first << " rate: "<<dataset[i][j].second <<endl;

    }
  }
}
/**
 * Reads the netflix dataset
 */

Rates readNetflix(const string& fname) {
  ifstream input(fname);
  string line;
  size_t lines = 0;

  Rates rates;

  string currMovie;
  size_t m = 0;
  while (getline(input, line)) {//linea por linea.
    if (line.back() == ':') {// si encuentra un ":" al fin de la linea 
      line.pop_back();
      currMovie = line;
      cout << "Movie: " << currMovie << endl;
      //if (max != 0 && m > max) break;
      	//m++;
    } else {
      size_t endUser = line.find_first_of(",");
      string currUser = line.substr(0, endUser);
      line.erase(0, endUser + 1);
      size_t endRate = line.find_first_of(",");
      string currRate = line.substr(0, endRate);
      rates[{currUser, currMovie}] = stoi(currRate);
    }
    lines++;
  }
  return rates;
}

pair<vector<SPoint>,size_t> createPoints(const Rates& rates) {
  map<string, size_t> normUsers;
  map<string, size_t> normMovies;
  for (const auto& e : rates) {
    const auto& user = e.first.first;
    const auto& movie = e.first.second;

    if (normUsers.count(user) == 0)
      normUsers[user] = normUsers.size();

    if (normMovies.count(movie) == 0)
      normMovies[movie] = normMovies.size();
  } 
  cout << "End of normalization " << normUsers.size() << " "
       << normMovies.size() << " "<< rates.size()<<endl;
       size_t num_movies= normMovies.size();

  vector<SPoint> users(normUsers.size());
  for (const auto& e : rates) {
    size_t user = normUsers[e.first.first];
    //cout << user << "u " << users.size() << endl;
    size_t movie = normMovies[e.first.second];
    double rate = e.second;
    //assert(user > 0 && user < users.size());
    users[user].push_back({movie, rate});
  }
  return {users,num_movies};
}
//hasta aqui para solo convertir el txt en un vector <SPoint>
/////////////////////////////////////////////////////


vector<rate> randomcentroids(size_t k, size_t num_movies) {
  random_device rd;
  mt19937 generator(rd());
  uniform_int_distribution<> unif(0, 5);
  vector<rate> c(k, rate(num_movies+1, 0.0));
  for (size_t i = 0; i < k; i++) {
    for (size_t j= 1;j <= num_movies;j++){
    	size_t r = unif(rd);
    	c[i][j] = r;
    }    
  }
  /*
  for(size_t i=0; i < 3;i++){
  	cout<< "fila"<<i<<endl;
  	for(size_t j=0;j<=num_movies;j++){

  		cout << " "<< c[i][j] << " ";

  	}
  }*/
  return c;
}



//recibe  un dos puntos uno que viene del centroide y uno del data set. 
//calcula la distancia etre dos puntos de n dimensiones.


double Coseno_similarity(const SPoint& user, const rate& rates_centroid_actual,const double normauser,const  double normacentroids) {
  // cout << p << endl;
  // cout << q << endl;

  double d = 0.0;
  //para cada componente del user (una calificacion)
  for (size_t i = 0; i < user.size(); i++) {
  	size_t indice_pelicula_actual = user[i].first;
  	//multiplico componente por componente osea un rate de una pelicula igual en usuario y del centroide.
    d += user[i].second * rates_centroid_actual[indice_pelicula_actual]; 
  }
  d= acos(d / (normauser*normacentroids)); //angulo entre un centroide y un usuario.


  return d;
}
//recibe un punto y vector de puntos centroides.
size_t closestCentroid(const SPoint& user, const vector<rate>& centroids,const double normauser,const  vector<double>& normacentroids) 
{
  double d = numeric_limits<double>::max();
  size_t c = 0;
  
  //paara el punto compare con todos los centroides.
  for (size_t i = 0; i < centroids.size(); i++) {

    double dt = Coseno_similarity(user, centroids[i],normauser, normacentroids[i]);//recibe un usuario un centroide y las normas.
    if (dt < d) {//toma el menor angulo entre un usuario y un centroide.
      d = dt;
      c = i;
    }
  }
  

  return c;
}

//retorna una tupla. un vector y un double. recibe vector puntos data set y vector puntos centroides.
vector<size_t> cluster(const vector<SPoint>& dataset,const vector<rate>& centroids,const vector<double>& normauser,const vector<double>& normacentroids) 
{

  size_t n = dataset.size(); //n tamaño de dataset
  vector<size_t> clustering(n, 0);//vector clusterin que guardar el id del centroide a que pertenece el dataset[i]
  //double ssd = 0.0;
  

  for (size_t i = 1; i <= n; i++) {//for que asocia un usuario al centroide mas cercano.
    size_t c;
    //double d;
    //tupla c y d guarda lo que retorne closestCentroid( y recibe el dataset en i "un punto." y el vector de puntos centroides.)
    c = closestCentroid(dataset[i], centroids,normauser[i],normacentroids);
    //vector clustering en i igual a c
    clustering[i] = c;
    //ssd += d;
    }
   
  return clustering;
}

//funcion que retornar un vector de puntos. 
//recibe clustering, data set(vector de puntos), vector de puntos centroides.
vector<rate> newCentroids(const vector<size_t>& clustering, const vector<SPoint>& dataset, vector<rate>& centroids)
{
  //el k es el tamaño de vector centroides. las filas.
  size_t k = centroids.size();
  size_t dim = centroids[0].size();//dimension tamaño columnas
  vector<rate> newCentroids(k, rate(dim+1, 0.0)); //nuevo vector de centroides con mismas dimensiones y filas.
  vector<double> count(k, 0.0);
  size_t indice_centroide;
  size_t indice_peli;

  //para el clustering obtenga el id del centroide actual 
  for (size_t i = 1; i <= dataset.size(); i++) {
    indice_centroide = clustering[i];//ci es el indice del centroide.


    

    //sume todas los rates de ese usuario de ese centroide, en las posiciones del centroide.
    for (size_t j = 0; j < dataset[i].size(); j++) {
    	indice_peli = dataset[i][j].first;
        newCentroids[indice_centroide][indice_peli] += dataset[i][j].second;
        //e
    }
    count[indice_centroide]++;// para saber cuantos puntos son del centroide.
  }
  cout << "salio del primer for anidado" << endl;

  //para cada centroide
  for (size_t i = 0; i < k; i++) {

    //para cada centroide actual 
    for (size_t j = 1; j <= dim; j++) {

      //divida lo que haya en todas sus dimensiones por la cantidad de puntos asociados al centroide.
      newCentroids[i][j] /= count[i];
    }
    //cout<< "salio del segundo for anidado"<<endl;
  }
  return newCentroids;//retorna un 
}
double diferencia_centroides(const vector<rate>& nuevo_centroids,const vector <rate>& centroids,const size_t){

	double similarity;
	double ssd =0.0;
	double cant = 0.0;
	cant = centroids.size();
	#
	for (size_t i=0;i<nuevo_centroids.size();i++){
		double prod_punto= 0.0, norma_A= 0.0, norma_B= 0.0;
		for(size_t j=1;j<=nuevo_centroids[i].size();j++){
			norma_A= pow(centroids[i][j],2);
			norma_B= pow(nuevo_centroids[i][j],2);

			prod_punto+= nuevo_centroids[i][j]*centroids[i][j];
		}
		similarity+= acos(prod_punto/(sqrt(norma_A)*sqrt(norma_B)));
	}

	ssd = similarity/cant ;
	return ssd;

}

//k means retorna un vector recibe un databaset (vector de puntos) el K un error epsilon y max interacciones.
vector<size_t> kmeans(const vector<SPoint>& dataset, size_t k,size_t dim_centroid, double epsilon,int max_iter) {
  //variable dimension que sera el tamaño de la primera columna
  //y n tamaño filas.
  size_t n = dataset.size();
  //crea un vector de puntos centroides y guarda lo que retorna la funcion randomPoints con K numero de centroides y el dataset
  vector<rate> centroids(k, rate(dim_centroid+1, 0.0));//matriz de filas k y columnas con numero de peliculas 4499
  centroids=randomcentroids(k,dim_centroid);
  //saca la norma de los usuarios y lo guarda en un vector double
  vector<double> normauser(n+1,0);
  normauser= normuser(dataset);
  //crea otro vector clustering con el tamaño de dataset.
  vector<size_t> clustering(n, 0); 
  //desviacion inicializada en 0.0 
  double ssd = 0.0;
  //desviacion previa tambien inicializada en 0.0
  double ssdPrev = 0.0;
  double d;//diferencia entre ssd y ssdprevio
  size_t iter = 0;
  clock_t cl = clock();


  //hacer , mientras
  do {
    ssdPrev = ssd;
    cout << "Iteration " << iter << endl;
    //para usar la norma para centroides. actuales
  	vector<double> normacentroides(k,0);
  	normacentroides=normcentroid(centroids);

    //primera parte del algoritmo asociar cada punto al cluster mas cercano 
    clustering = cluster(dataset, centroids,normauser,normacentroides);
    //printClustering(dataset,clustering,k);
    /*
    for(size_t i=1; i<= clustering.size();i++){
    	cout << "usuario"<< i<<"cluster: "<< clustering[i]<< endl;
    }
    */
    //cout << "SSD: " << ssd << endl;
    //printClustering(dataset, clustering);
    //segunda parte reubicar los centroides sacando el promedio de la ubicacion del grupo asociado al centroide.
    // el while hace que esto se repita hasta ajustar los cambios.
    vector<rate> nuevo_centroids(k, rate(dim_centroid+1, 0.0));
    centroids = newCentroids(clustering, dataset, centroids);//centroids

    //ssd = diferencia_centroides(nuevo_centroids,centroids,k);
    //cout << "ssd final: "<< ssd <<endl; // es un angulo entre centroides nuevos y viejos.
    //centroids = nuevo_centroids;
    d = abs(ssdPrev - ssd);
    iter++;
  	} while(iter < max_iter);//mientras que la diferencia sea menor que el error epsilon 0.001
  	cout << "tiempo de ejecucion algoritmo."<< (clock()-cl)*1000/CLOCKS_PER_SEC << "ms" << endl;


  return clustering;//retorna una lista del tamaño de filas del data set con el indice del centroide que le fue asignado al punto.
}

int main(int argc, char** argv) {
  if (argc != 2)
      return -1;
  
  string fname(argv[1]);
  Rates rates = readNetflix(fname);
  vector<SPoint> ds;
  size_t num_movies;
  

  tie(ds,num_movies) = createPoints(rates);
  //printnetflix(ds);
  
  vector<size_t> clustering(ds.size(),0);
  clustering = kmeans(ds,30,num_movies,0.087,1);
  






  //printnetflix(ds);

  

  return 0;
}
