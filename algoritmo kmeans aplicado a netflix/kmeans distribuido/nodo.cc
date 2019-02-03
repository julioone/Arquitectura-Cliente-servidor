// g++ -std=c++11 -fopenmp -o kmeans kmeans.cc
// ./kmeans [namearchivo]
#include <fstream>
#include <iostream>
#include <map>
#include <chrono>
#include <string>
#include <vector>
#include <random>
#include <omp.h>
#include <ctime>
#include <cmath>

#include <zmqpp/zmqpp.hpp>
//acomodar los pair y los tie en cluster y closecluster.

#include <cassert>
// como se debe hacer los centroides
// para calcular la similaridad angular que hay que hacer
//
using namespace std;
typedef string str;
str ip="localhost", port="5555";
using namespace zmqpp;
using SPoint = vector<pair<size_t, double>>;
using rate = vector<double>;

using Rates = map<pair<string, string>, double>; // (user,movie) -> rate
uint quantityThreads = 4;
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
      //cout << "Movie: " << currMovie << endl;
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
void printClustering(const vector<SPoint>& dataset,
                     const vector<size_t>& clustering, size_t k) {

  
  
  size_t n = dataset.size();
  vector<size_t> count(k, 0);
  for (size_t i = 1; i < n; i++) 
  {
    size_t ci = clustering[i];
    count[ci]++;
  }

  for(size_t i = 0; i < k; i++) 
  	cout << " cluster " << i << ": " << count[i] << endl;
}


vector<double> normcentroid(const vector<rate>& centroids){
	vector<double> normacentros(centroids.size(),0);
  //#pragma omp parallel for
	for(size_t i=0; i< centroids.size();i++){
		for(size_t j=1; j<=centroids[0].size() ;j++)
		{
			normacentros[i]+=pow(centroids[i][j],2);
		}
		normacentros[i]=sqrt(normacentros[i]);
	}
  cout <<"tamano columnas"<< centroids[0].size() <<endl;

  return normacentros;
}


vector<double> normuser(const vector<SPoint>& dataset){
	vector<double> norma(dataset.size(),0);

    for (size_t i=1; i < dataset.size();i++ ){
      for(size_t j=0; j < dataset[i].size();j++){

    	   norma[i]+= pow(dataset[i][j].second,2);

       }
      norma[i]= sqrt(norma[i]);
    }
  	
  	return norma;
}


void printnetflix(const vector<SPoint>& dataset){
  for (size_t i=0; i < dataset.size();i++ ){
    cout << "IDUSER: "<< i<< endl;
    for(size_t j=0;j< dataset[i].size();j++){
      cout<< "idpelicula: "<<dataset[i][j].first << " rate: "<<dataset[i][j].second <<endl;

    }
  }
}



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
tuple<size_t,double> closestCentroid(const SPoint& user, const vector<rate>& centroids,const double normauser,const  vector<double>& normacentroids) 
{
  double angulo_menor = numeric_limits<double>::max();
  size_t idcentroid = 0;

  
  //paara el punto compare con todos los centroides.
  
  for (size_t i = 0; i < centroids.size(); i++){

    double dt = Coseno_similarity(user, centroids[i],normauser, normacentroids[i]);//recibe un usuario un centroide y las normas.
    if (dt < angulo_menor) {//toma el menor angulo entre un usuario y un centroide.
      angulo_menor = dt;

      idcentroid = i;

    }
  }
  

  return make_tuple(idcentroid,angulo_menor);
}


//retorna una tupla. un vector y un double. recibe vector puntos data set y vector puntos centroides.
pair<vector<size_t>,double> cluster(const vector<SPoint>& dataset,const vector<rate>& centroids,const vector<double>& normauser,const vector<double>& normacentroids) 
{
  vector<size_t> clustering(dataset.size(), 0);//vector clusterin que guardar el id del centroide a que pertenece el dataset[i]
  double ssd = 0.0;
  

  double dchunk = (double)dataset.size()/quantityThreads;
  uint chunk = ceil(dchunk);


    #pragma omp parallel for schedule(dynamic,chunk) num_threads(4)
    for (size_t i = 1; i <  dataset.size(); i++) {//for que asocia un usuario al centroide mas cercano.
      size_t idcentroid;
      double angulo_menor;
      //tupla c y d guarda lo que retorne closestCentroid( y recibe el dataset en i "un punto." y el vector de puntos centroides.)
      tie(idcentroid,angulo_menor) = closestCentroid(dataset[i], centroids,normauser[i],normacentroids);
      //vector clustering en i igual a c
      clustering[i] = idcentroid;//centroide correspondiente para cada usuario.
      //cout << "angulo menor: " << angulo_menor<< endl;
      #pragma omp critical
      #pragma omp atomic
      ssd += angulo_menor;
    }
  

 	return {clustering,ssd};
}


//funcion que retornar un vector de puntos. 
//recibe clustering, data set(vector de puntos), vector de puntos centroides.
vector<rate> newCentroids(const vector<size_t>& clustering, const vector<SPoint>& dataset, vector<rate>& centroids)
{	
  //el k es el tamaño de vector centroides. las filas.
  size_t k = centroids.size();
  size_t dim = centroids[0].size();//dimension tamaño columnas
  vector<rate> newCentroids(k, rate(dim, 0.0)); //nuevo vector de centroides con mismas dimensiones y filas.
  vector<double> count(k, 0.0);
  size_t indice_centroide;
  size_t indice_peli;
  //cout << "entro antes del primer for" << endl;

  //para el clustering obtenga el id del centroide actual 
  for (size_t userid = 1; userid < dataset.size(); userid++) {
    indice_centroide = clustering[userid];//ci es el indice del centroide.


    

    //sume todas los rates de ese usuario de ese centroide, en las posiciones del centroide.
    for (size_t movieid_user_rate = 0; movieid_user_rate < dataset[userid].size(); movieid_user_rate++) {
    	indice_peli = dataset[userid][movieid_user_rate].first;
        newCentroids[indice_centroide][indice_peli] += dataset[userid][movieid_user_rate].second;
        
    }
    count[indice_centroide]++;// para saber cuantos usuarios son del centroide.
  }
  cout << "salio del primer for" << endl;

  //para cada centroide



  #pragma omp parallel for 
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
// cuanto cambio angular hay entre centroides en cada interaccion en el tiempo.
double dtcentrois(const vector<rate>& centroidspreev,const vector<rate>& centroids){
  double dtcentroids= 0.0;
  for(size_t i=0; i < centroidspreev.size();i++){

    double prod_x = 0.0,norm_centroidspreev=0.0,norm_centroids=0.0;

    for(size_t j=1; j < centroidspreev[0].size();j++){
        
        prod_x += centroidspreev[i][j]*centroids[i][j];
        norm_centroidspreev+= pow(centroidspreev[i][j],2);
        norm_centroids+= pow(centroids[i][j],2);
    }
    norm_centroidspreev=sqrt(norm_centroidspreev);
    norm_centroids=sqrt(norm_centroids);
    double similaridad = acos(prod_x/(norm_centroids*norm_centroidspreev));
    dtcentroids+=similaridad;
  }
  return dtcentroids;
}
//k means retorna un vector recibe un databaset (vector de puntos) el K un error epsilon y max interacciones.
double kmeans(const vector<SPoint>& dataset, uint k,size_t num_movies, double epsilon,const vector<double>& normauser) {
  //variable dimension que sera el tamaño de la primera columna
  //y n tamaño filas.

  //crea un vector de puntos centroides y guarda lo que retorna la funcion randomPoints con K numero de centroides y el dataset
  vector<rate> centroids;//matriz de filas k y columnas con numero de peliculas 4499
  centroids=randomcentroids(k,num_movies);
  //saca la norma de los usuarios y lo guarda en un vector double
  

  //crea otro vector clustering con el tamaño de dataset.
  vector<size_t> clustering(dataset.size(), 0); 
  //vector<double> datos_angleusers(n, 0);
  //vector<double> datos_angleusersprev(n, 0);
  //cout << "entrara..."<<endl;
  vector<rate> new_centroids;




  
  double ssd = 0.0; // distanncia de todos los puntos a su centroide.
  //desviacion previa tambien inicializada en 0.0
  double dtcentroids=0.0;
  double dtcentroidsPrev=0.0;

  double d;//diferencia entre ssd y ssdprevio
  size_t iter = 0;
  vector<double> normacentroides;


  //hacer , mientras
  do {

    dtcentroidsPrev=dtcentroids;
    
    cout << "Iteration " << iter << endl;
    //para usar la norma para centroides. actuales
  	
  	normacentroides=normcentroid(centroids);
    //cout << "normalizo centroides" << endl;

    //primera parte del algoritmo asociar cada punto al cluster mas cercano 
    tie(clustering,ssd) = cluster(dataset, centroids,normauser,normacentroides);
    cout << "SSD: " << ssd << endl;
    
    
    
    //segunda parte del algoritmo re acomodar los centroides.
    new_centroids = newCentroids(clustering, dataset, centroids);
    dtcentroids= dtcentrois(centroids,new_centroids);

    //centroides actuales ahora son los nuevos
    centroids=new_centroids;



    //d = abs(ssdPrev - ssd);
    d = abs(dtcentroidsPrev-dtcentroids);
    cout << "----> " << d << endl;
    iter++;
  	} while(d > epsilon);

  printClustering(dataset,clustering,k);
  	


  return ssd;//retorna una lista del tamaño de filas del data set con el indice del centroide que le fue asignado al punto.
}

int main(int argc, char** argv) {
  if (argc != 2) {
    cerr << "Modo de uso " << argv[0] << " dataset.txt\n";
    return -1;
  }
  
  string fname(argv[1]);
  Rates rates = readNetflix(fname);
  
  vector<SPoint> ds;
  size_t num_movies;
  tie(ds,num_movies) = createPoints(rates);
  vector<double> normauser;
  normauser= normuser(ds);
  cout << "normalizo user"<< endl;
  //double ssd = 0.0;
  context ctx;
  socket nodo(ctx,socket_type::req);
  nodo.connect("tcp://"+ip+":"+port);
  message solicitud, respuesta;
  cout <<"se conecto"<< endl;
  bool kencontrado =true;
  str orden="entre";
  while(kencontrado){

    cout << "solicitando K..."<< endl;
    solicitud << "solicito K";
    nodo.send(solicitud);
    nodo.receive(respuesta);
    respuesta >> orden;
    if(orden != "Kencontrado" && orden !="ssd guardado" && orden!= "solicite mas tarde"){
      uint k = stoi(orden);
      cout << "el K a ejecutar es: " << k << endl;
      clock_t cl = clock();
      double ssd = kmeans(ds,k,num_movies,0.08,normauser);
      //cout << "tiempo de ejecucion algoritmo."<< (clock()-cl)*1000/CLOCKS_PER_SEC << "ms" << endl;
      solicitud << "K solucionado" << to_string(k) << to_string(ssd);
      nodo.send(solicitud);
      nodo.receive(respuesta);
      respuesta >> orden;
    }
    else if(orden == "solicite mas tarde"){
      this_thread::sleep_for(chrono::seconds(10));
    }
    else if(orden == "kencontrado"){
      kencontrado =false;
    }
    cout << orden << endl;
    }


 return 0;

}
