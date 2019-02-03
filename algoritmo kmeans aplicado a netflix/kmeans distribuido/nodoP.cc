//g++ -std=c++11 -L/usr/local/lib -I/usr/local/include nodoP.cpp -o nodoPri.bin -lzmqpp -lzmq

#include <zmqpp/zmqpp.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <functional>
#include <queue>
#include <chrono>
#include <cmath>

using namespace std;
using namespace zmqpp;
typedef string str;

size_t numero_ks = 12;

//solo cambia cuando la prioridad es mayor.
struct Koptimo{
	size_t k;
	double prioridad;
//metodo para guardar lo que viene en sus atributos
public:
	Koptimo(double nueva_prioridad,size_t nuevo_k){
		k=nuevo_k;
		prioridad=nueva_prioridad;
	}

};
class mayor{
public:
	double operator()(const Koptimo &a,const Koptimo &b){
		return a.prioridad < b.prioridad;
	}
};
//cola de prioridad de c++ queue metodos push, pop
using Pcola = priority_queue<Koptimo,vector<Koptimo>,mayor>;


str IP="*",PORT="5555";

void inicializar(Pcola& cola_Ks,vector<str>& estados,size_t& numero_ks){
	size_t medio=floor((numero_ks-1)/2)+1;
	estados[0]="encolado";
	cola_Ks.push(Koptimo(100.0,1));
	estados[medio-1]="encolado";
	cola_Ks.push(Koptimo(100.0,medio));
	estados[numero_ks-1]="encolado";
	cola_Ks.push(Koptimo(100.0,numero_ks));

}

double calcular_pendiente(const size_t& x1,const size_t& x2,vector<double>& ssd_calculados){
	double y2 = ssd_calculados[x2-1];
	double y1 = ssd_calculados[x1-1];
	double pendiente = (y2-y1)/(x2-x1); //formula de la pendiente
	return pendiente;
}
double a_grados(double& radian){
	double angulo = -1.0;
	double constante_conversion = 57.2957;
	double rad = radian*constante_conversion;
	if(rad < 0)
		angulo =360+rad;
	if(rad > 0)
		angulo=rad;
	if(rad == 0)
		angulo=360;
	return angulo;
	
}
bool es_medio(const size_t& x,const size_t& x_med){
	if(x_med==x){
		return false;

	}
	else
		return true;
	

}
bool disponible(const size_t& new_k,vector<str>& estados){
	if(estados[new_k-1]=="disponible"){
		return true;
	}
	else
		return false;
	
}
double cambio_angular(const size_t& k_first,const size_t& k_second,const size_t& k_third, Pcola& cola_Ks, vector<double>& ssd_calculados,vector<str>& estados){

	double angulo_rec1=atan(calcular_pendiente(k_first,k_second,ssd_calculados));
	double angulo_rec2 =atan(calcular_pendiente(k_second,k_third,ssd_calculados));
	angulo_rec1= a_grados(angulo_rec1);
	angulo_rec1= a_grados(angulo_rec2);
	double cambio_pend= abs(angulo_rec1-angulo_rec2);
	//cout<< "prioridad actual de nuevos K"<<angulo_rec2 <<endl;
	size_t medio_rec1 = (floor((k_second-k_first)/2))+k_first;
	size_t medio_rec2 = (floor((k_third-k_second)/2))+ k_second;
	bool k_first_medio= es_medio(k_first,medio_rec1);
	bool k_second_medio= es_medio(k_second,medio_rec2);
	//condicionales para que se agreguen nuevos K a la cola de prioridad
	if(k_first_medio){
		//cout << "primerr medio de la recta obtenido."<< endl;
		if( disponible(medio_rec1,estados) ){
			estados[medio_rec1-1]="encolado";
			cola_Ks.push(Koptimo(cambio_pend,medio_rec1));
			cout << "se ingreso a la cola: " << medio_rec1 << " "<<cambio_pend<< endl;
		}
	}
	if( k_second_medio ){
		//cout << "segundo medio de la recta obtenido << endl"<< endl;
		if( disponible(medio_rec2,estados) ){
			estados[medio_rec2-1]="encolado";
			cola_Ks.push(Koptimo(cambio_pend,medio_rec2));
			cout << "se ingreso a la cola: " << medio_rec2 << " "<<cambio_pend<< endl;
			
		}


	}
	if( !k_first_medio && !k_second_medio ){
		return -1.1; // cuando no hay mas k en medio es porque estan juntos los tres puntos
	}
	else
		return cambio_pend;
	
	
}


int Ksiguiente(size_t i, vector<double>& ssd_calculados){
	int K_siguiente=-1;
	for(int siguiente=i+1;siguiente <= ssd_calculados.size()-1;siguiente++){
		if(ssd_calculados[siguiente] != -1){
			K_siguiente=siguiente+1;
			break;
		}
	}
	return K_siguiente;
}
int Kanterior(size_t i,vector<double>& ssd_calculados){
	int K_anterior=-1;
	for(int anterior =i-1;anterior>=0;anterior--){
		if(ssd_calculados[anterior] != -1){
			K_anterior= anterior+1;
			break;
		}
	}
	return K_anterior;
}

int main(int argc, char const *argv[]){

	vector< double> ssd_calculados(numero_ks,-1.0);
	// cada K tiene estados asignado o no asignado y resuelto
	vector<str> estados(numero_ks,"disponible");
	Pcola cola_Ks;

	bool Koptimo_encontrado =false;
	size_t medio=floor((numero_ks-1)/2)+1;
	//inicializar cola con los tres primeros k
	inicializar(cola_Ks,estados,numero_ks);
	

	context ctx;
	socket nodoP(ctx,socket_type::rep);
	nodoP.bind("tcp://"+IP+":"+PORT);
	Koptimo encontrado = Koptimo(-1.0,0);
	bool encontrar_Koptimo= true;
	message solicitud_nodo, respuesta_k;
	while (encontrar_Koptimo){
		cout << "TIPO SOLICITUD: ";
		nodoP.receive(solicitud_nodo);

		str tipo_solicitud, k_string,ssd_string;

		solicitud_nodo >> tipo_solicitud;
		if(tipo_solicitud == "solicito K" && !cola_Ks.empty() && !Koptimo_encontrado){
			cout << tipo_solicitud<< endl;
			Koptimo primero =cola_Ks.top();
			cola_Ks.pop();
			estados[primero.k-1] = "asignado";
			size_t k= primero.k;
			respuesta_k << to_string(k);

		}
		else if(tipo_solicitud== "K solucionado"){
			cout << tipo_solicitud<< endl;

			solicitud_nodo >> k_string >> ssd_string;
			cout << "K: "<<k_string<< ", "<<"SSD: "<<ssd_string<<endl;
			size_t k = stoi(k_string);
			double ssd = stod(ssd_string);
			ssd_calculados[k-1]=ssd;
			estados[k-1]="resuelto";

			respuesta_k << "ssd guardado";
			size_t k_first =0, k_second=0,k_third=0;
			double prioridad_KS=0.0;
			for(size_t i=1; i<= ssd_calculados.size()-2;i++){
				if(ssd_calculados[i] !=-1){
					k_second = i+1;
					k_first= Kanterior(i,ssd_calculados);
					k_third= Ksiguiente(i,ssd_calculados);
					//si es diferente a -1 a estos k hay que darles la prioridad que se obtiene
					// a calcular el cambio angular en esos tres puntos.
  					if(k_first !=-1 && k_third !=-1){
  						//cout << "ks actuales: " << k_first <<" "<<k_second<<" "<<k_third<< endl;
  						//cambio angular entre dos rectas formados por tres puntos
  						// el cambio angular es el que me da la prioridad de esos K en ese intervalo
  						prioridad_KS = cambio_angular(k_first,k_second,k_third,cola_Ks,ssd_calculados,estados);
  						// cuando la prioridad sea -2.0 es porque ya se encontro
  						if(prioridad_KS == -1.1){
  						cout << "EL K OPTIMO ES: "<< encontrado.k << endl;
  						encontrar_Koptimo=false;
  						Koptimo_encontrado = true;
  						break;
  						}
  					//cuando la prioridad de el k actual es menor a una nueva prioridad
  					else if(encontrado.prioridad < prioridad_KS){
  						encontrado.k= k_second;
  						encontrado.prioridad = prioridad_KS;
						cout << "K optimo actual: "<<encontrado.k<< endl;
					}

  					}


  				}	
			}
		}
		else if (tipo_solicitud == "solicito K" && cola_Ks.empty() && !Koptimo_encontrado){
			respuesta_k << "solicite mas tarde";
		}
		else if(!cola_Ks.empty() && Koptimo_encontrado){
			respuesta_k << "Kencontrado";

		}
		nodoP.send(respuesta_k);


	}
	return 0;


}
