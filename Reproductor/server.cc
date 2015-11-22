#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>
#include <list>
#include <zmqpp/zmqpp.hpp>
#include <map>
#include <queue>

using namespace std;
using namespace zmqpp;


class song{

	private:
		string name;
		string artist;
		int duration;
		string filename;
	
	public:
		//void play(soundEngine e){ e.play(filename);}
		song(){}
		song(const string &nam,const string &artis,const string &filenam){
			name=nam;
			artist=artis;
			filename=filenam;
		}
		
		const string &Getname()const{
			return name;
		}

		const string &Getfilename()const{
			return filename;
		}
};

class client{
	private:
		string ip_port;
		string filename;
		int number;
		//song s;
		int option;
		int block;
		int lastSizebytes;
	public:
		client(){};
		client(const string &Ip,const int &Option,const  string &Name,const int &Num,/*const song &SONG,*/const int &BLOCK, const int &lsb){
			ip_port=Ip;
			//port=Port;
			option= Option;
			filename=Name;
			number=Num;
			//s=SONG;
			block=BLOCK;
			lastSizebytes=lsb;
		}
		const string &Getip_port()const{
			return ip_port;
		}
		
		const string &Getfilename()const{
			return filename;
		}
		const int &Getoption()const{
			return option;
		}
		/*const song &Getsong()const{
			return s;
		}*/
		const int &Getnumber()const{
			return number;
		}
		const int &Getblock()const{
			return block;
		}
		const int &GetlastSize()const{
			return lastSizebytes;
		}
		void Setblock(){
			block++;
		}
		
};




vector<char> ReadAllBytes(const string &filename){

    ifstream ifs(filename, ios::binary|ios::ate);
    ifstream::pos_type pos = ifs.tellg();

    vector<char> result(pos);
    ifs.seekg(0, ios::beg);
    ifs.read(&result[0], pos);

    return result;
}



vector<char> ReadBlockBytes(const string &filename,int lastSizebytes,int Sizebytes, int block, int *end_song){
	ifstream ifs(filename, ios::binary);
	vector<char> result(Sizebytes);
	ifs.seekg(block*lastSizebytes,ios::beg);
	ifs.read(&result[0],Sizebytes);
	if(ifs.eof()){
		*end_song=1;
		cout<<"The file was read completely"<<endl;
	}
	//cout<<"se leyeron "<<ifs.gcount()<<" bytes"<<endl;
	return result;
}

void filetomessage(const string &filename, message &r,int block,int lastSizebytes, int Sizebytes,int *end_song){
	vector <char> bytes = ReadBlockBytes(filename,lastSizebytes,Sizebytes,block, end_song);
	r.add_raw(bytes.data(), bytes.size());
		
}
string listar(map<int, song> &music){
	string SongList;
	for(map<int,song>::iterator it=music.begin(); it!=music.end(); it++){
		SongList=SongList+"["+to_string(it->first)+"] "+it->second.Getname()+"\n";					
	}
	return SongList;
}

string find_song(string name, map<int, song> music){
			string result;
			for(map<int,song>::iterator it=music.begin(); it!=music.end(); it++){
				auto p = it->second.Getname().find(name);
				if(p != string::npos){
					result=result+"["+to_string(it->first)+"] "+it->second.Getname()+"\n";		
				}

			}
			return result;
}
void attend_queue(socket_t &Socket_broker, context &ctx,map<int, song> &music,queue<client> &MyQueue,int *end_song){

	if(!MyQueue.empty()){
		string SongList;
		song Song;
		client aux = MyQueue.front();
		
		message to_client;
		message to_broker;
		to_broker << "less";
		socket Socket_clients(ctx, socket_type::xreq);
		Socket_clients.connect(aux.Getip_port());
		if(aux.Getoption() == 1){
			cout<<aux.Getip_port()<<endl;		
			SongList=listar(music);
			to_client << "list";
			to_client << SongList;
			Socket_broker.send(to_broker);
		}
		if(aux.Getoption() == 2){
			cout<<aux.Getip_port()<<endl;
			SongList=find_song(aux.Getfilename(),music);
			to_client <<"sublist";
			to_client << SongList;
			Socket_broker.send(to_broker);			
		}
		if(aux.Getoption() == 3){
			if(music.count(aux.Getnumber()) > 0 && aux.Getnumber() <= music.size() && aux.Getnumber() > 0){
				Song=music[aux.Getnumber()];
				int numClients=MyQueue.size();
				int Kb=90000/numClients;
				to_client<<"song";
				filetomessage(Song.Getfilename(),to_client,aux.Getblock(),aux.GetlastSize(),Kb,end_song);
				aux.Setblock();
				to_client << Song.Getname();
				cout<<"Sent part "+to_string(aux.Getblock())+" of "<< Song.Getname()+"to client "+ aux.Getip_port()<<endl;
		        to_client << *end_song;
				client temp(aux.Getip_port(),aux.Getoption(),aux.Getfilename(),aux.Getnumber(),aux.Getblock(),Kb);
				if(*end_song == 0){
					MyQueue.push(temp);
				}else{
					*end_song=0;
					Socket_broker.send(to_broker);
				}			
			}else{	
				to_client << "Error";
				to_client << "the  request is wrong, please try again!";
			}
		}
		Socket_clients.send(to_client);
		
		MyQueue.pop();

		
	}
}



void build_list_songs( map<int, song> &music){
	song a("Quitate","piso21","Quitate.ogg");
	song b("DiamondEyes","ShineDown","DiamondEyes.ogg");
	song c("Elperdon","NickyJam","Elperdon.ogg");
	song d("corazonEncantado","DragonBall","corazonEncantado.ogg");
	song e("butterfly","Digimon","butterfly.ogg");
	song f("braveHeart","Digimon","braveHeart.ogg");
	music[1]=a;
	music[2]=b;
	music[3]=c;
	music[4]=d;
	music[5]=e;
	music[6]=f;
}


int main(){
	int number;
	int *end_song=(int*)malloc(sizeof(int));
	*end_song=0;
	string id;
	string ip;
	string filename;
	string username;
	string port;
	string ip_port;
	map<int, song> music;
	queue<client> MyQueue;
	
	build_list_songs(music);
	cout<<"This is the server\n";
	context ctx;
	socket Socket_broker(ctx, socket_type::xreq);
	
  	Socket_broker.connect("tcp://localhost:5555");
  	
  	poller Poller;
  	Poller.add(Socket_broker,poller::poll_in);

  	message connection;
  	connection<<"online";
  	Socket_broker.send(connection);
  	while(true){
  		attend_queue(Socket_broker,ctx,music,MyQueue,end_song);
		if(Poller.poll(500)){
				int option;
				message op;
				Socket_broker.receive(op);
				op >> ip;
				op >> port;
				op >> option;
				op >> filename;
				op >> number;
				ip_port="tcp://"+ip+":"+port;
				client c(ip_port,option,filename,number,0,0);
				MyQueue.push(c);
				//system("ls -laR | grep ogg");
		}
		
	}
	return 0;
}



