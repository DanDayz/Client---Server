#include <bits/stdc++.h>
#include <czmq.h>

using namespace std;

vector< pair <zframe_t* , string > > nodos;
vector< pair <zframe_t* , string > > misConexiones;
int N;



void buildConnectionMsg(void* context,string dir){
  // Crear mensaje con mi base de datos y enviarla a la direccion
  //NO enviar mensajes grandes. Por canciones.



}


void handleNodeMessage(zmsg_t* msg,zmsg_t* outmsg,void* Send,void* toCoor,void* context){
 zframe_t* idN=zmsg_pop(msg);
 zframe_t* copy=zframe_dup(idN);
 string code(zmsg_popstr(msg));

 if(code.compare("register")==0){
   string address(zmsg_popstr(msg));
   nodos.push_back(make_pair( copy, address) );

   if(nodos.size()==N){
     int steps=floor(log(N)); //Revisar

     for(int i=0;i<N;i++){
       zmsg_t* msg= zmsg_new();
       zframe_t* copy2=zframe_dup(nodos[i].first);
       zmsg_prepend(msg,&copy2);
       zmsg_addstr(msg,"Conexiones");
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
 }else if(code.compare("leave")==0){
   //enviar mensajes a mis vecinos
   //sacar la ip que me mandan y cerrar sockets
 }else if(code.compare("BD")==0){
   //Crear tablas hash con los vecinos del mensaje
   //DOS TIPOS DE MENSAJES
   //y base de datos de canciones
 }else if(code.compare("Conexiones")==0){
   int len=zmsg_size (msg);
   for(int i=0;i<len;i++){
     string dirRec(zmsg_popstr(msg));
     misConexiones.push_back(make_pair(copy,dirRec));
     buildConnectionMsg(context,dirRec);
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
