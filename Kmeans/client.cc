#include <iostream>
#include <fstream>
#include <string>
#include <zmqpp/zmqpp.hpp>
#include <sstream>
#include <vector>
#include <algorithm>//funcion find
#include <map>
#include <math.h>
#include <time.h> 
#include <stdlib.h>
#include <stdio.h> 

using namespace std;
using namespace zmqpp; 


class Grafo{
	int V; // número de nodos
	vector<int> *adj; // puntero a un array que contiene lista de adyacencia

	public:
	Grafo(int V); // construtor
	void adicionarRelacion(int  v1, int v2); // adiciona una arista un grafo

	int obtenerNumeroRelaciones(int v);

	vector<int> obtenerRelacion(int v);

};

Grafo::Grafo(int V){
	this->V = V; // agrega el numero de nodos al grafo
	adj = new vector<int>[V]; // crea las listas
}

void Grafo::adicionarRelacion(int v1, int v2){
	// adiciona nodo v2 a lista de nodos adyacentes de v1
	adj[v1].push_back(v2);
}

int Grafo::obtenerNumeroRelaciones(int v){
	//retorna el numero de relaciones del nodo.
	return adj[v].size();
}

vector<int> Grafo::obtenerRelacion(int v){
	//retorna un nu.
	return adj[v];
}



class worker{

	private:
		string id;
		int carga;
		int block;
		
	
	public:
		worker(){}
		worker(const string &Id,const int &Carga, const int &block){
			id=Id;
			carga=Carga;	
		}
		
		const int &Getcarga()const{
			return carga;
		}

		const string &Getid()const{
			return id;
		}
		const int &Getblock()const{
			return block;
		}
		void Setcarga(int n){
			if(n==0)
				carga++;
			else
				carga--;
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

///////////////Funcion de ordenamiento de monticulo///////////////////
//////////////////////////////////////////////////////////////////////
bool heap_comp(worker a, worker b){
	return a.Getcarga() < b.Getcarga();
}

/////////////Funcion que crea la lista de adyacencia/////////////////
////////////////////////////////////////////////////////////////

void CrearGrafo( Grafo &grafo){
	ifstream infile("facebook_combined.txt");
	string line;
	while (getline(infile, line)){
	    istringstream iss(line);
	    int a;
	    int b;
	    if (!(iss >> a >> b)){ 
	    	break; 
	    }else{
	    	grafo.adicionarRelacion(a, b);
	    }
	}
}

void init_centros(vector<centro> &centros,int K,int numero_nodos){
	int temporal=0;
	for (int t=0 ; t < K ; t++){
		vector<float> vaux;
		for (int i = 0; i < numero_nodos; i++){
		  	temporal=rand() % 2;
		  	vaux.push_back(temporal);

		}
		centros.push_back(vaux);
		  				
	}
}
int main(int argc,char *argv[]){
	int numero_nodos=4040;
	//=========Creacion del grafo=======
	Grafo grafo(numero_nodos);
	//=========Cargo los datos del archivo y creo la lista de adyacencia============
	CrearGrafo(grafo);
	string  temp=argv[1];
	int K=stoi( temp );
	int block=0;
	int i=0;
	int counter=0;
	float x;
	double alpha=1300;
	double d=0;
	double error=0;
    double dist=0;
	vector<centro> centros;
	vector<worker> s;
	srand (time(NULL));
	//=============inicializacion de centroides aleatorios==========
	init_centros(centros,K,numero_nodos);
	string action,id_worker;
	string id_server;
	context ctx;
	cout<<"This is the Client\n";
	cout<<"Binding socket to tcp port 5554\n";
	//=======Crearcion de los Sockets======
  	socket Socket_workers(ctx, socket_type::xrep);
  	socket Socket_server(ctx, socket_type::xrep);
	Socket_workers.bind("tcp://*:5554");
  	Socket_server.bind("tcp://*:5553");
  	//=======Creacion de Poller=============
  	poller Poller;
  	Poller.add(Socket_workers,poller::poll_in);
  	Poller.add(Socket_server,poller::poll_in);
  	while(true){
  		if(!s.empty()){
	  		if(block!=K){
	  			//=========Ordenamiento del Heap de forma ascendente
	  			sort(s.begin(), s.end(), heap_comp);
	  			if(s.front().Getcarga() == 0){
		  			
		  			message to_worker;
		  			to_worker <<s.front().Getid();
		  			to_worker << block;
		  			to_worker << K;
		  			to_worker << numero_nodos;
		  			//clock_t r;
					//r=clock();
					//===========Envio de la lista de adyacencia===============
		  			for (int y = 0; y < ceil((int)numero_nodos/K); y++){
		  				vector<int> v_aux=grafo.obtenerRelacion(counter);
		  				to_worker << counter;
		  				to_worker << grafo.obtenerNumeroRelaciones(counter);
		  				for (int j = 0; j < grafo.obtenerNumeroRelaciones(counter); j++)
		  					to_worker << v_aux[j];
		  				counter++;
		  			}
		  			//============Envio de los centroides ================
		  			for (int t=0 ; t < K ; t++){
		  				vector<float> aux=centros[t].Getv();
		  				for (int o = 0; o < numero_nodos; ++o)
		  					to_worker<<aux[o];
		  			}
		  			Socket_workers.send(to_worker);
		  			//cout << "time send data "<<(clock()-r)/(double)CLOCKS_PER_SEC<<endl;
		  			s.front().Setcarga(0);
		  			block++;
		  		}
	  		}
	  	}
  		
  		if(Poller.poll(500)){
  			//====Encolacion de workers cuando se conectan============
			if(Poller.has_input(Socket_workers)){
				message conection_worker;
				Socket_workers.receive(conection_worker);
				conection_worker >> id_worker;
				conection_worker >> action;
				cout<<action<<endl;
				
				if(action == "online"){
					worker a(id_worker,0,i);
					s.push_back(a);
					cout <<"Server :" + s.back().Getid()<<" Online"<<endl;
					i++;
				}
				
			}
			//============ Recibir datos del Server==================
			if(Poller.has_input(Socket_server)){
				message from_server;
				Socket_server.receive(from_server);
				from_server >> id_server;
				from_server>>action;
				if(action=="online"){
					cout << "server online"<<endl;
				}

				if(action=="data"){
					
					//clock_t r;
					//r=clock();
					//===============Aqui se reciben los centros nuevos==============
					vector<centro> new_centros(K);
					for (int t=0 ; t < K ; t++){
						vector<float> vaux;
						for (int o = 0; o < numero_nodos; o++){
							from_server >> x;
							vaux.push_back(x);
						}
						centro taux(vaux);
						new_centros[t]=taux;
						
					}
					
					from_server >> dist;
					//cout << "time recieve data "<<(clock()-r)/(double)CLOCKS_PER_SEC<<endl;
					//clock_t r1;
					//r1=clock();
					//===============calculando el Error restando la distancia anterios menos la distancia actual==============
					error=abs(dist-d);
					//cout << "distancia anterior "<< d << endl;
					cout <<"error: "<< error <<endl;
					
					//cout << "time distancias " <<(clock()-r1)/(double)CLOCKS_PER_SEC<<endl;
					if (error < alpha){
						break;
					}else{
						d=error;
					}
					//========Aqui se reemplazan los centros anteriors por los nuevo================
					for (int n = 0; n < K; ++n){
						centros[n]=new_centros[n];
					}
					block=0;
					counter=0;
					for (int i = 0; i < K; ++i){
						s[i].Setcarga(1);
					}
				}
			}
		}	
  	}
	return 0;
}
