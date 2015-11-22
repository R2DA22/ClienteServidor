#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>
#include <list>
#include <zmqpp/zmqpp.hpp>
#include <map>
#include <queue>
#include <algorithm>
#include <functional>

using namespace std;
using namespace zmqpp;


class server{

	private:
		string id;
		int carga;
	
	public:
		//void play(soundEngine e){ e.play(filename);}
		server(){}
		server(const string &Id,const int &Carga){
			id=Id;
			carga=Carga;	
		}
		
		const int &Getcarga()const{
			return carga;
		}

		const string &Getid()const{
			return id;
		}
		void Setcarga(int n){
			if(n==0)
				carga++;
			else
				carga--;
		}
};

bool heap_comp(server a, server b){
	return a.Getcarga() < b.Getcarga();
}

class client{
	private:
		string id;
		string ip;
		string port;
		int option;
		string song_name;
		int song_number;

	public:
		client(){};
		client(const string &ID,const string  &IP,const string &PORT,const int &OPTION, const string &SONG_NAME , const int &SONG_NUMBER){
			id=ID;
			ip=IP;
			port=PORT;
			option=OPTION;
			song_name=SONG_NAME;
			song_number=SONG_NUMBER;
		}
		const string &Getid()const{
			return id;
		}

		const string &Getip()const{
			return ip;
		}
		const string  &Getport()const{
			return port;
		}
		const int &Getoption()const{
			return option;
		}
		const string &Getsongname()const{
			return song_name;
		}
		const int &Getsongnumber()const{
			return song_number;
		}
		
};





int main(){
	string id_server;
	string id_client;
	string ip_server;
	string ip_client;
	string song_name;
	string action;
	string port_client;
	int option;
	int song_number;
	cout<<"This is the Broker\n";
	context ctx;
	socket Socket_servers(ctx, socket_type::xrep);
	cout<<"Binding socket to tcp port 5555\n";
	cout<<"Binding socket to tcp port 6666\n";
	socket Socket_clients(ctx, socket_type::xrep);
	Socket_servers.bind("tcp://*:5555");
	Socket_clients.bind("tcp://*:5556");
  	cout<<"Waiting for message to arrive!!\n";
  	
  	poller Poller;
  	Poller.add(Socket_servers,poller::poll_in);
  	Poller.add(Socket_clients,poller::poll_in);

  	queue<client> client_Queue;

  	vector<server> s;
  	
  	while(true){
  		if(!s.empty())
  			sort(s.begin(), s.end(), heap_comp);		
		if(Poller.poll(500)){
			if(Poller.has_input(Socket_servers)){
				message conection_server;
				Socket_servers.receive(conection_server);
				conection_server >> id_server;
				conection_server >> action;
				if(action == "online"){
					server a(id_server,0);
					s.push_back(a);
					cout <<"Server :" + s.back().Getid()<<" Online"<<endl;
				}
				if(action == "less"){
					for(int i=0; i < s.size(); i++){
						if(s.at(i).Getid() == id_server){
							s.at(i).Setcarga(1);
						}	
					}
					
					
				}
				
			}
			if(Poller.has_input(Socket_clients)){
				message peticion_client;
				Socket_clients.receive(peticion_client);
				
				peticion_client >>id_client;
				peticion_client >>ip_client;
				peticion_client >>port_client;
				peticion_client >>option;
				if(option==2)
					peticion_client >>song_name;
				else
					song_name="";

				if(option==3)
					peticion_client >>song_number;
				else
					song_number=0;
				message to_server;
				to_server << s.front().Getid();
				to_server << ip_client;
				to_server << port_client;
				to_server << option;
				to_server << song_name;
				to_server << song_number;
				s.front().Setcarga(0);
				Socket_servers.send(to_server);
				cout <<"Request of client : "+ip_client<<endl;
			}			
		}
		
	}
	return 0;
}