#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>
#include <zmqpp/zmqpp.hpp>
#include <sstream>
#include <list>
#include <algorithm>
#include <math.h>
#include <map>
#include <time.h> 
using namespace std;
using namespace zmqpp;

double distancia(vector<float> centroide,vector<int> nodo, int num_nodos);

class asignacion{

	private:
		vector<int> v;
		int id_centro;
		double dist; 
	public:
		asignacion(){}
		asignacion(const vector<int> &x,const int &id, const vector<float> &centro){
			v=x;
			id_centro=id;
			dist=distancia(centro,x,4040);
		}
		
		const vector<int> &Getv()const{
			return v;
		}

		int Getid()const{
			return id_centro;
		}
		double Getdist()const{
			return dist;
		}

};

class centro{

	private:	
		vector<float> v;
	public:
		centro(){}
		centro(const vector<float> &x){
			v=x;	
		}
		const vector<float> &Getv()const{
			return v;
		}
};


////////////////Funcion que toma la distancia Euclidiana de un nodo a un centro//////////////
/////////////////////////////////////////////////////////////////////////////////////////////
double distancia(vector<float> centroide,vector<int> nodo, int num_nodos){
	double aux=0;
	for (int l = 0; l < num_nodos; l++){
	    if(nodo[l]==0)
	    	aux+=pow((centroide[l]-0),2);
	    else
	    	aux+=pow((centroide[l]-nodo[l]),2);
	}
	return aux=sqrt(aux);
}

///////////Funcion que asigna cada nodo a el centroide mas cercano//////////////
////////////////////////////////////////////////////////////////////////////////


void asignar(map<int, vector<int>> matriz_grafo,vector<centro>centros, map<int, asignacion> &tabla_asignaciones,int id_nodo,int nodos,int num_nodos, int K){
	double aux=0;
	double  temp=0;
	int id_centro=0;
	int i=0;
	int id=id_nodo;
	int flag=nodos+id_nodo;
	vector<float> centroide;
	vector<int> nodo;
	for (int p = id_nodo; p < flag ; p++){
		nodo=matriz_grafo[p];
	    for (int j=0 ; j < K ; j++){		  	
	    	centroide=centros[j].Getv();
	    	aux=distancia(centroide,nodo,num_nodos);
	    	if(temp == 0){
	    		temp=aux;
	    	}
	    	if(aux < temp){
	    		temp=aux;
	    		id_centro=i;
	    	}
	    	i++;
	    }
	    asignacion asig(nodo,id_centro,centros[id_centro].Getv());
	    tabla_asignaciones[id]=asig;
	    id++;
	    i=0;
	    temp=0;   
	}
}



int main(int argc,char *argv[]){
	
	int block;
	int nodos;
	int num_nodos;
	int K;
  	int x;
  	int tam_relaciones;
  	int counter;
  	float y;
	cout<<"This is the worker\n";
	context ctx;
	//=======Crearcion de los Sockets======
	socket Socket_client(ctx, socket_type::xreq);
  	socket Socket_server(ctx, socket_type::xreq);
  	//==============Estableciendo conexion con el cliente  y el servidor================
  	Socket_client.connect("tcp://localhost:5554");
	Socket_server.connect("tcp://localhost:5558");
	//=======Creacion de Poller=============
  	poller Poller;
  	Poller.add(Socket_client,poller::poll_in);
  	message connection1;
  	message connection2;
  	connection1<<"online";
  	connection2<<"online";
  	
  	Socket_client.send(connection1);
  	Socket_server.send(connection2);
  	
  	while(true){
  		
		if(Poller.poll(500)){
			if(Poller.has_input(Socket_client)){
				map<int, asignacion> tabla_asignaciones;
				map<int, vector<int>> matriz_grafo;
				message entrada;
				Socket_client.receive(entrada);
				entrada >> block;
				entrada >> K;
				entrada >> num_nodos;
				nodos =ceil((int)num_nodos/K);
				//clock_t t3;
				//t3=clock();
				//=========Aqui se recibe la lista de adyacencia de cada nodo==========
				for (int y = 0; y < nodos ; y++){
		  			vector<int> v_aux (num_nodos,0);
		  			entrada >> counter;
		  			entrada >> tam_relaciones;
		  			for (int h = 0; h < tam_relaciones; h++){
		  				entrada >> x;
		  				v_aux.at(x)=1;
		  			}
		  			//===================Creo la matriz de adyacencia==================
		  			matriz_grafo[counter]=v_aux;	
		  		}
		  		//==================Aqui se reciben los centroides===========
		  		vector<centro> centros(K);
		  		for (int t=0 ; t < K ; t++){
		  			vector<float> vaux;
		  			for (int o = 0; o < num_nodos; o++){
		  				entrada >> y;
		  				vaux.push_back(y);
		  			}
		  			centro taux(vaux);
		  			centros.at(t)=vaux;
		  			
		  		}
		  		/*cout << "time receive data  " <<(clock()-t3)/(double)CLOCKS_PER_SEC<<endl;
		  		clock_t t2;
				t2=clock();*/
				//======Aqui se se asigna a cada centroide los nodo mas cercanos a el================
				asignar(matriz_grafo,centros,tabla_asignaciones,block*nodos,nodos,num_nodos,K);
				//cout << "time asignar data  " <<(clock()-t2)/(double)CLOCKS_PER_SEC<<endl;
				message to_server;
				to_server << "request";
				to_server << block;
				to_server << K;
				int size=tabla_asignaciones.size();
				to_server << size;
				cout << "tamaño tabla asignacion " <<size<<endl;
				to_server << num_nodos;
				//clock_t t1;
				//t1=clock();
				//=====================Envio de nodos y a que centro pertenece cada nodo====================
				for(map<int,asignacion>::iterator it=tabla_asignaciones.begin(); it!=tabla_asignaciones.end(); it++){
					vector<int> vaux2=it->second.Getv();
					to_server<<it->first;
					for (int i = 0; i < num_nodos; i++){
						to_server << vaux2[i];
					}
					to_server<<it->second.Getid();
					to_server << it->second.Getdist();
				}
		  		Socket_server.send(to_server);
		  		//cout << "time send data  " <<(clock()-t1)/(double)CLOCKS_PER_SEC<<endl;
			}
				
		}
		
	}
	return 0;
}
