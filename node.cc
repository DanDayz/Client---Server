#include <bits/stdc++.h>
#include <czmq.h>

using namespace std;

vector< pair <zframe_t* , string > > allNodes; // IdNodo: ipNodo
vector< pair <zframe_t* , string > > misConexiones; //Idnodo: ipnodo
vector<string> canciones; //Las canciones que tiene el nodo
typedef unordered_map<string,vector<string>> DIC;
DIC partes; // song:<part1,part2...> BD local
unordered_map<string,DIC> song_nodes; //song:<parte:<nodo1,nodo2...>>
string myIP; //Ip del nodo para crear el registro

int N;
int ndescargas=0,nsubidas=0;
float reputacion=0.0;

//Me crea las partes y las carpetas de cada archivo.
void inicializarMusic(){
  cout<<"Lista de archivos"<<endl;
  DIR *dir;
  struct dirent *ent;
  dir = opendir ("./music/");
  if (dir == NULL)
   cout<<"No puedo abrir el directorio"<<endl;
  while ((ent = readdir (dir)) != NULL) {
      if ( (strcmp(ent->d_name, ".")!=0) && (strcmp(ent->d_name, "..")!=0) ){
        char* nombre=ent->d_name;
        canciones.push_back(nombre);
        //songSearch.insert(nombre);
        mkdir(nombre,0777);
        string comando="split -b 512k -d ./music/"+string(nombre)+" ./"+string(nombre)+"/s.";
        system(comando.c_str());
      }
  }
  closedir (dir);
}

//Creo un mapa con las canciones que yo tengo y sus respectivas partes
void crearmapa(){
	int l= canciones.size();
	for (int i = 0;i<l; ++i){
		DIR *dir;
		struct dirent *ent;
		string carpeta="./"+canciones[i];
		vector<string> parts;
		dir = opendir (carpeta.c_str());
		if (dir == NULL)
	   cout<<"No puedo abrir el directorio"<<endl;
		while ((ent = readdir (dir)) != NULL) {
		    if ( (strcmp(ent->d_name, ".")!=0) && (strcmp(ent->d_name, "..")!=0) ){
		        string part(ent->d_name);
		       	parts.push_back(part);
		    }
		}
		partes[canciones[i]]=parts;
  	closedir (dir);
	}
}

void buildConnectionMsg(void* context,string dir,zframe_t* &id){
  //Hago la solicitud a un nodo de que me envia su BD

  void* doRequestBD = zsocket_new(context, ZMQ_DEALER);
  int a = zsocket_connect(doRequestBD, dir.c_str());
  zmsg_t* msg= zmsg_new();
  zframe_t* copy2=zframe_dup(id);
  zmsg_prepend(msg,&copy2);
  zmsg_addstr(msg,"sendBD");
  zmsg_addstr(msg,dir.c_str());
  zmsg_send(&msg,doRequestBD);

  //|idnodo|sendBD|dirRespuesta|

}


void handleNodeMessage(zmsg_t* msg,zmsg_t* outmsg,void* Send,void* toCoor,void* context){
 zframe_t* idN=zmsg_pop(msg);
 string code(zmsg_popstr(msg));

 if(code.compare("register")==0){
   zframe_t* copy=zframe_dup(idN);
   string address(zmsg_popstr(msg));
   allNodes.push_back(make_pair( copy, address) );

   if(allNodes.size()==N){
     int steps=floor(log(N)); //Revisar
     //CONVERTIR ESTO EN UNA FUNCION
     //MsgConexiones: |idnodo|Conexiones|steps|ip1|ip2|..|ipn|

     for(int i=0;i<N;i++){
       zmsg_t* msg= zmsg_new();
       zframe_t* copy2=zframe_dup(nodos[i].first);
       zmsg_prepend(msg,&copy2);
       zmsg_addstr(msg,"Conexiones");
       zmsg_addstr(msg,to_string(steps).c_str());
       int p=1;
       for(int j=1;j<=steps;j++){
         if(p>=N)p=p%N;
         zmsg_addstr(msg,nodos[p].second.c_str());
         p=p<<1;
       }
       zmsg_send(&msg,toCoor);
       //zmsg_size (zmsg_t *self);
     }

   }
 }else if(code.compare("BD")==0){
   //Recibo la BD, la adiciono a mi arbol de busqueda.
   //Recino las canciones y necesito saber la direccion ip del nodo
   //YA TENGO LA IP EN misConexiones, solo necesito guardar las canciones

   //recibo: |idNodo|BD|nsongs|song1|nparts|part1|...|partn|song2|..|partn|
   int numNodes=atoi(zmsg_popstr(msg)); //nsongs
   for ( int it = 0; it<numNodes; it++ ){
     string song(zmsg_popstr(msg));
     DIR part_nodo;
     if(song_nodes.count(song)==0){
       //CREAR LA ESTRUCTURA Y GUARDAR LA PARTE
       //Dir=> misConexiones[idN]
       song_nodes[song]=part_nodo;;
     }else{
       part_nodo=song_nodes[song];
     }
     int len=atoi(zmsg_popstr(msg)); //nparts
     for(int i=0;i<len;i++){
       string part(zmsg_popstr(msg));
       if(part_nodo.count(part)==0){
         vector<string> vnodes;
         vnodes.push_back(misConexiones[idN]); //Ipnodo
         part_nodo[part]=vnodes;
       }else{
         part_nodo[part].push_back(misConexiones[idN]);
       }
     }
   }
   /*
   FALTA GUARDAR EN LA BASE DE DATOS MIS PROPIAS CANCIONES CON MYIP.
   HACER FUNCION QUE HAGA ESTO.
   */

 }else if(code.compare("Conexiones")==0){
   int len=atoi(zmsg_popstr(msg));
   for(int i=0;i<len;i++){
     string dirRec(zmsg_popstr(msg));
     misConexiones.push_back(make_pair(copy,dirRec));
     //Envio solicitud de BD
     buildConnectionMsg(context,dirRec,idN);
   }
 }else if(code.compare("sendBD")==0){
   //Enviar Base de datos de canciones y partes con las que se cuentan.

   //Leer de carpeta music y enviar las canciones con las partes.
   //POR AHORA NO TENER EN CUENTA EL SHA1

   //Mensaje: |idNodo|BD|nsongs|song1|nparts|part1|...|partn|song2|..|partn|
   //Enviarlo a la direccion de recibir.
   int l=partes.size();
   zframe_t* copy=zframe_dup(idN);
   zmsg_t* msg= zmsg_new();
   zmsg_append(msg, &copy); //idnode
   zmsg_addstr(msg,"BD");
   zmsg_addstr(msg,to_string(l).c_str());
   for ( auto it = partes.begin(); it != partes.end(); ++it ){
     string song=it->first;
     vector<string> parts=it->second;
     int len=parts.size();
     //Posible error por usar el mismo song
     zmsg_addstr(msg,song.c_str()); //song
     zmsg_addstr(msg,to_string(len).c_str()); //nparts
     for(int i=0;i<len;i++){
       //add part
       zmsg_addstr(msg,parts[i].c_str()); //nparts
     }
   }
 }
}


void msgRegistrar(void* toCoor,string myRec){
  zmsg_t* msg= zmsg_new();
  zmsg_addstr(msg,"register");
  zmsg_addstr(msg,myRec.c_str());
  zmsg_send(&msg, toCoor);
}

int main(int argc, char** argv){

  if (argc != 3) {
    cerr << "Wrong call\n";
    return 1;
  }

  // [Dirtracker][PortTracker][NodoActualDir][NodoActualPort][DirFiles][Delay]
  // ./Client localhost 5555 localhost 6666 Temp 5
  // Anisho.
  //string IzqDir=argv[1];
  //string IzqPort=argv[2];
  string DerDir=argv[1];
  string DerPort=argv[2];
  string CoorDir=argv[3]; // el mancito principal
  string CoorPort=argv[4];
  string MayDir=argv[5]; // Mi diretión
  string MayPort=argv[6];
  N=atoi(argv[7]);


  //string IzqConnect="tcp://"+IzqDir+":"+IzqPort;
  string DerConnect="tcp://"+DerDir+":"+DerPort;
  string NodeListenerConnect="tcp://*:"+MayPort;
  string Coordinador="tcp://"+CoorDir+":"+CoorPort;

  zctx_t* context = zctx_new();

  void* Send = zsocket_new(context, ZMQ_DEALER);
  int a = zsocket_connect(Send, DerConnect.c_str());
  cout << "connecting to DerNode: "<<DerConnect<< (a == 0 ? " OK" : "ERROR") << endl;
  cout << "Listening! DerNode" << endl;

  void* Rec = zsocket_new(context, ZMQ_ROUTER);
  int b = zsocket_bind(Rec,NodeListenerConnect.c_str());
  cout << "Listening! Nodes at : "<<NodeListenerConnect << (b == 0 ? " OK" : "ERROR") << endl;

  void* toCoor = zsocket_new(context, ZMQ_DEALER);
  int c = zsocket_connect(toCoor, Coordinador.c_str());
  cout << "connecting to DerNode: "<<Coordinador<< (c == 0 ? " OK" : "ERROR") << endl;
  cout << "Listening! DerNode" << endl;


  //Enviar un mensaje Coordinador
  if(CoorDir.compare("localhost")!=0){
    msgRegistrar(toCoor,NodeListenerConnect);
  }else{
    zmsg_t* m=zmsg_new();
    zmsg_addstr(m,"BootPeer");

    zframe_t* id= zmsg_pop(m);
    nodos.push_back( make_pair(id , NodeListenerConnect));
  }


  zmq_pollitem_t items[] = {{Rec, 0, ZMQ_POLLIN, 0}};

  while (true) {
      zmq_poll(items, 1, 10 * ZMQ_POLL_MSEC);
      if (items[0].revents & ZMQ_POLLIN) {
        	cerr << "From NO se de quien\n";
        	zmsg_t* msg=zmsg_recv(Rec);
    			zmsg_print(msg);
        	zmsg_t* outmsg = zmsg_new();
        	handleNodeMessage(msg,outmsg,Send,toCoor,context);
      }
  	}

  zctx_destroy(&context);
  return 0;

}
