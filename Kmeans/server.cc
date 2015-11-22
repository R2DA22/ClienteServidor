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

class asignacion{

	private:
		vector<int> v;
		int id_centro;
		
	public:
		asignacion(){}
		asignacion(const vector<int> &x,const int &id){
			v=x;
			id_centro=id;
		}
		
		const vector<int> &Getv()const{
			return v;
		}

		const int &Getid()const{
			return id_centro;
		}
		
};

class centro{

	private:	
		vector<float> v;
		int size=0;
	public:
		centro(){}
		centro(const vector<float> &x,int S){
			v=x;	
			size=S;
		}
		const vector<float> &Getv()const{
			return v;
		}
		void sum(int index,float n){
			float a;
			a=v.at(index)+n;
			v.at(index)=a;
		}
		int Getsize(){
			return size;
		}
		void setSize(){
			size++;
		}

		void Setpromedio(int n){
			for (int i = 0; i < n; i++){
				if(size!=0){
					v.at(i)=((float)v.at(i)/size);
				}else{
					v.at(i)=0;
				}
			}
		}

};

class worker{

	private:
		string id;	
	public:
		worker(){}
		worker(const string &Id){
			id=Id;
		}
		const string &Getid()const{
			return id;
		}
		

};

////////Funcion que hace una sumatoria de cada  dimensiones de cada nodo/////////
///////////////////////////////////////////////////////////////////////////////

void sumatoria(centro &C,int num_nodos,vector<int> v){
	for (int i = 0; i < num_nodos; i++){
		C.sum(i,v[i]);
	}
	C.setSize();
}

//////////Funcion que me busca en la tabla los nodos que estan asignados al mismo centroide /////////////////
//////////////////////////////////// Y genera los nuevos centroides//////////////////////////////////////////

vector<centro> clasificar(map<int,asignacion> tabla_asignaciones, int num_nodos,int K){
	vector<float> temp (num_nodos,0);
	vector<centro> v(K);
	for (int i=0; i < K ; i++) {
		centro C(temp,0);
		v[i]=C;
	}
	int id_centro;
	for (int j=0; j < K ; j++){
		for (map<int,asignacion>::iterator it=tabla_asignaciones.begin(); it!=tabla_asignaciones.end(); it++){
			vector<int> v_aux =it->second.Getv();
			id_centro=it->second.Getid();
			if(id_centro == j){
				sumatoria(v.at(j),num_nodos,v_aux);
			}		
		
		}
		v.at(j).Setpromedio(num_nodos);
	}
	for (int k=0; k < K; k++){
		cout << "Cluster" << ": "<< v[k].Getsize() <<endl;
		cout << "===============================" <<endl;
	}
	return v;
}

int main(int argc,char *argv[]){
	
	string num;
	string nodos="";
	string bytes;
	cout<<"This is the Server\n";
	context ctx;
	socket Socket_worker(ctx, socket_type::xrep);
	socket Socket_client(ctx, socket_type::xreq);
	int id_cluster;
  	Socket_worker.bind("tcp://*:5558");
  	Socket_client.connect("tcp://localhost:5553");
  	message connection;
  	connection << "online";
  	Socket_client.send(connection);
  	string action;
  	string id_worker;
  	int x;
  	int cluster;
  	int id_nodo;
  	int size;
  	int num_nodos;
	double dist;
	double sum_dist;
  	int K;
  	poller Poller;
  	Poller.add(Socket_worker,poller::poll_in);
  	map<int, asignacion> tabla_asignaciones;
  	vector<centro> new_centros;
  	vector<worker> s;
  	while(true){
		if(Poller.poll(500)){
			if(Poller.has_input(Socket_worker)){

				message entrada;
				Socket_worker.receive(entrada);
				entrada >> id_worker;
				entrada >> action;
				if(action == "online"){
					worker a(id_worker);
					s.push_back(a);
					cout <<"worker :" + s.back().Getid()<<" Online"<<endl;
				}
				
				if(action== "request"){
					entrada >> id_cluster;
					entrada >> K;
					entrada >> size;
					entrada >> num_nodos;
					
					//clock_t t4;
					//t4=clock();
					for(int z=0 ; z < size ; z++ ){
						entrada >> id_nodo;
						vector<int> vaux;
						for (int i = 0; i < num_nodos; ++i){
							entrada >> x;
							vaux.push_back(x);
						}
						entrada >> cluster;
						entrada >> dist;
						sum_dist+=dist;
						//cout << "distancia: "<<dist <<endl;
						asignacion aux(vaux,cluster);
						tabla_asignaciones[id_nodo]=aux;
					}
					//cout << "time receive data  " <<(clock()-t4)/(double)CLOCKS_PER_SEC<<endl;
					if(id_cluster==(K-1)){
						//clock_t t1;
						//t1=clock();
						vector<centro> new_centros=clasificar(tabla_asignaciones,num_nodos,K);
						//cout << "time clasificar data  " <<(clock()-t1)/(double)CLOCKS_PER_SEC<<endl;					
						message to_client;
						to_client << "data";
						//clock_t t2;
						//t2=clock();

						for (int t=0 ; t < K ; t++){
							vector<float> v_aux=new_centros[t].Getv();
							for (int o = 0; o < num_nodos; o++){
								to_client<<v_aux[o];
							}
						}
						to_client << sum_dist;
						Socket_client.send(to_client);
						sum_dist=0;
						//new_centros.clear();
					//	cout << "time send data  " <<(clock()-t2)/(double)CLOCKS_PER_SEC<<endl;
					}
				}
			}
		}
	}
	return 0;
}
