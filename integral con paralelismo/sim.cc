/* Tarea semana santa cliente servidor Estudiante Julio Cesar ospina 
____________________________________________________________________
*/


#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <ctime>

using namespace std;
double funcion_a_integrar(double x)// en el cuerpo de esta funcion puede ir cualquier funcion que se quiera integrar
{
	return(4/(1+ (x*x)));// integral de pi, si se evalua esta integral entre 0 y 1 da aproximadamente 3.1416
}


int main(){
	double h,a,b;
	int n,i;
	printf("SIMPSON COMBINADO\n");
	printf("Ingrese el inicio del intervalo\n");
	scanf("%lf",&a);
	printf("Ingrese el fin del intervalo\n");
	
	scanf("%lf",&b);
	printf("Ingrese el valor de pasos\n");
	scanf("%d",&n);


	h=((b-a)/n);
	double f[n+1]; // arreglo de 7 double.
	clock_t cl = clock();
	#pragma omp parallel for//calcula pararlela mente en el arreglo la funcion evaluada en todos los puntos 
		for (i = 0; i <= n; ++i)
			f[i]= funcion_a_integrar(h*i);//el punto x que recibe la funcion es h*i 
		
	
	double sum=f[0]+f[n];//variable que se inicializa con los valores de la funcion calculada en x sub cero y x sub n
	#pragma omp parallel for// for en paralelo que suma los valores de la lista multiplicando en los coheficientes
	for (i=1; i<=n-2; i=i+2)
		#pragma omp critical
		#pragma omp atomic
		sum =sum + 4*f[i]+2*f[i+1];

	
	
	sum = sum +4*f[n-1];
	cout << (clock()-cl)*1000/CLOCKS_PER_SEC << "ms" << endl;
	cout << "el area bajo la curva es: " << endl; 
	printf("%lf",h*sum/3);
	cout << endl;
	return 0;
}