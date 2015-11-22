#include <iostream>
#include <string>
#include <cassert>
#include <fstream>
#include <zmqpp/zmqpp.hpp>
#include <SFML/Audio.hpp>
#include <cctype>

using namespace std;
using namespace zmqpp;


int validar(){
	int number;
	unsigned n;
	bool found_nondigit, valid;
	valid = false;
	string option;
	string::size_type sz;
	while (!valid){ 
		cin>>option;
		found_nondigit = false;
		if (option.length() == 0) 
			found_nondigit = true; 

		for (n=0; n<option.length(); n++) 
			if (!isdigit(option[n]))
				found_nondigit = true; 	
		if (!found_nondigit) { 
			number = stoi(option,&sz); 
			valid = true; 
		}else{
			cout<< "Error: Invalid input\n"; 
		}
			
		
	}
	return number;
}

void Tocar(sf::Music &music,string filename,string bytes){
	fstream ifs(filename+"(1).ogg", ios::out);
	ifs << bytes;
	if (!music.openFromFile(filename+"(1).ogg"))
		   std::cout<<"ERROR!"<< std::endl;
	music.play();
	ifs.close();
}
void print_menu(){
	cout<< "_________MENU__________\n\n";
	cout<< "[1].Song's list"<<endl;
	cout<< "[2].Search songs by name\n"<<endl;
	cout<< "[3].Download song\n"<<endl;
	cout<< "[0].Exit\n"<<endl;
	cout<<"option: "<<endl;
}



void answer(socket_t &Socket, int flag,string &text,map<string,string> &dic,sf::Music &music){
	string action;
	string id;
	string bytes;
	string filename;
	string filesong="";
	string msn;
	map<string,string>::iterator it = dic.begin();
	message resp;
	Socket.receive(resp);
	resp >> id;
	resp >> action;
	if(action=="list"){
		resp >> text;
		cout<<"SONG'S LIST"<<endl;
		cout<<text<<endl;
	}
	if(action=="sublist"){
		cout<<"SONG'S LIST"<<endl;
		resp >> text;
		if(text!=""){
			cout<<text;
		}else{
			cout<<"No songs found.\n";
		}
	}
	if(action=="song"){
		resp >> bytes;
		resp >> filename;
		resp >> flag;   
		it = dic.find(filename);
		if(it == dic.end())
			dic.insert (pair<string,string>(filename,"") );
		if(bytes!="" or filename!=""){
			filesong=dic[filename]+bytes;
			dic.at(filename)=filesong; 
		    if(flag!=0){	            
		        Tocar(music,filename,dic[filename]);
		        dic.erase(filename);
		        filesong="";
		    }
		}else{
			cout<<"No songs found.\n";
		}	
								
	}
	if(action=="error"){
		resp >> msn;
		cout<< msn << endl;
	}
}

bool request(socket_t &Socket, string &text, string port){
	int option;
	int number;
	string name;
	option=validar();
	message op;
	op << "192.168.1.10";
	op << port;
	op << option;
	if(option >= 0 and option <= 3){
		if(option==1){
			Socket.send(op);
		}

		if(option==2){
			cout << "Search: ";
			cin >> name;
			op << name;				
			Socket.send(op);

		}
		if(option==3){
			cout<<"SONG'S LIST"<<endl;
			cout<<text;
			number=validar();
			op << number;
			Socket.send(op);

		}
		if(option==0){
			return true;
		}
	}else{
		cout << "Wrong option!"<< endl;
	}
	return false;

}

int main (int argc,char *argv[]){
	cout<<"This is the Cliente\n";
	string text;
    int flag=0;
	int STDIN=fileno(stdin);
	map<string,string> dic;
	context ctx;
	string s;
	s=argv[1];
	socket Socket_broker(ctx, socket_type::xreq);
	socket Socket_servers(ctx, socket_type::xrep);
	cout<<"Binding socket to tcp port "<<argv[1]<<endl;
	Socket_servers.bind("tcp://*:"+s);
	cout<<"Connecting to tcp port 5556\n";
	Socket_broker.connect("tcp://localhost:5556");

    sf::Music music;
	poller Poller;
	Poller.add(Socket_servers,poller::poll_in);
	Poller.add(STDIN,poller::poll_in);
	print_menu();
	while(true){

		if(Poller.poll()){
			if(Poller.has_input(STDIN)){
				system("clear");
				print_menu();
				if(request(Socket_broker,text,argv[1]))
					break;	
			}
			if(Poller.has_input(Socket_servers)){
				answer(Socket_servers,flag,text,dic,music);
			}
		}
	}
	return 0;
}
