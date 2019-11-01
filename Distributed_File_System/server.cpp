//COMPILE USING MAKEFILE: make server
//Or make all: make

//To start this:
//./server <ThisMachines Port> <This Machines IP> <SuperNode IP> <SuperNode Port> <Coordinator Mode>
//ex 7 servers:
/*
./server 9001 localhost localhost 9090 1
./server 9002 localhost localhost 9090 0
./server 9003 localhost localhost 9090 0
./server 9004 localhost localhost 9090 0
./server 9005 localhost localhost 9090 0
./server 9006 localhost localhost 9090 0
./server 9007 localhost localhost 9090 0
*/
//NOTE: HAVE TO START THE COORDINATOR FIRST UNLESS WE IMPLEMENT A DIFFERENT JOIN MECHANISM(different than AnnounceLife())

#include "gen-cpp/Server.cpp"
#include "gen-cpp/Server.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/TSocket.h>

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <vector>
#include <dirent.h>
#include <thread>
#include <inttypes.h>
#include <string>
#include <sstream>
#include <fstream>
#include <random>
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <thread>         // std::thread
#include <mutex>          // std::mutex
#include <bits/stdc++.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <queue>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;

using boost::shared_ptr;

using namespace  ::PA3;

#define MAXFOLDERS 10
/******* Global Variables **********/
int coordinator = 0;
int N = -1;
int Nw = 0;
int Nr = 0;
char* CoordIP;
int CoordPort;
std::string myPORT;
std::mutex file_locks[MAXFOLDERS];
int writes = 0;
/****** End Global Variables *******/


/****** Structs ********************/
struct files {
  std::string filename;
  int version;
};


struct server_info {
  std::string PORT;
  std::string IP;
};


struct request {
  std::string _return;
  std::string filename;
  std::string contents;
  int64_t OPCODE;
};


std::vector <server_info> servers;
std::vector <files> filesMetaData;
std::queue <request> queue[MAXFOLDERS];
//std::vector<void (*)(std::string&, const std::string&, const std::string&, const int64_t)> requestQueue;  // SendRequest() function queue
/****** End Structs ****************/


/******* Function Prototypes *******/
void SetValuesN(int N, int& Nr, int& Nw);
void GetServersQuorum(std::vector<server_info>& serversNx, const std::vector<server_info> servers, int Nx);
void NonQuorumServers(std::vector<server_info>& non_quorum, const std::vector<server_info> serversNw);
void UpdateMetaData(files updateInfo);
int CoordWrite(std::string filename, std::string contents,  int version);
int FindMetaDataIndex(std::string filename);
std::string CoordRead(std::string filename);
void UpdateNonQuorum(const std::vector<server_info>& non_quorum, const std::string& filename, const std::string& contents, const int max, const int index);
int ProcessRequest(struct request info, int max, int index);
/***** End Function Prototypes *****/

class ServerHandler : virtual public ServerIf {
 public:
  ServerHandler() {
  }

  bool ping() {
    return true;
  }

  //This function does a lot of the heavy lifting. This handles the control logic of all requests.
  void SendRequest(std::string& _return, const std::string& filename, const std::string& contents, const int64_t OPCODE) {
    //If this machine isn't the coordinator, forward the request
    if(!coordinator){
      //Send to Coordinator
      std::string temp_return;
      shared_ptr<TTransport> socket(new TSocket(CoordIP, CoordPort));
      shared_ptr<TTransport> transport(new TBufferedTransport(socket));
      shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
      ServerClient client2(protocol);
      try {
        transport->open();
        printf("Sending Request: %s\tcontents:%s\topcode:%s to coordinator.\n\r", filename.c_str(), contents.c_str(), std::to_string(OPCODE).c_str());
        client2.SendRequest(_return, filename, contents, OPCODE);
        transport->close();
      }
      catch (TException& e) {
        printf("Error: %s\n", e.what());
      }
      return;
    }
    else{//This machine is the coordinator
      //OPCODE 0 => Write      1 => Read       2 => Print
      if(OPCODE == 0){

        //1st Check to see if the file is locked!!!
        //This is checking whether the files exists
        int index = FindMetaDataIndex(filename);
        if(index != -1){
            //Check the lock
            if(file_locks[index].try_lock()){
              //We have the lock
              std::cout << "Lock taken by " << filesMetaData.at(index).filename << " with contents \"" << contents << "\"";
              std::cout << "and with most current updated version " << filesMetaData.at(index).version << std::endl;
            }
            else{
              //Add to queue and return: "Queued"
              request temp;
              temp.filename = filename;
              temp.OPCODE = OPCODE;
              temp.contents = contents;
              std::cout << "Adding Queue:  " << filename << "\t" << contents << "\t" << filesMetaData.at(index).version << std::endl;
              queue[index].push(temp);
              _return = "Queued";
              //while(1);
              return;
            }
        }
        else{
            //create the index here, waiting till the end is causing multiple threads to push multiple copies into the metadata
            files current;
            current.filename = filename;
            current.version = 0;
            UpdateMetaData(current);
            //grab the index
            index = FindMetaDataIndex(filename);
            //lock
            if(file_locks[index].try_lock()){
              //We have the lock
              std::cout << "Lock taken by " << filesMetaData.at(index).filename << " in else (line ~218) with contents \"" << contents << "\"";
              std::cout << "and with most current updated version " << filesMetaData.at(index).version << std::endl;
            }
        }

        //If all good then continue here
        //1 Grab random servers until # = Nw
        std::vector<server_info> serversNw;
        GetServersQuorum(serversNw, servers, Nw);

        //get servers not in quorum to update later
        std::vector<server_info> non_quorum;
        NonQuorumServers(non_quorum, serversNw);

        //2 Check versions
        int max = -1;
        std::vector<server_info> current_versions_server;
        std::vector<server_info> noncurrent_versions_server;

        // find highest version value (max)
        for(int i = 0; (unsigned) i < serversNw.size(); i++) {

          //Check to see if the server is actually this one (can happen when the coordinator is part of the quorum).
          int temp;
          if(!strcmp(serversNw.at(i).IP.c_str(), "me")){
            for(int i = 0;(unsigned) i < filesMetaData.size(); i++){
              if(!strcmp(filesMetaData.at(i).filename.c_str(), filename.c_str())){
                max = filesMetaData.at(i).version;
              }
            }
          }
          else{
            shared_ptr<TTransport> socket(new TSocket(serversNw.at(i).IP, std::stoi(serversNw.at(i).PORT)));
            shared_ptr<TTransport> transport(new TBufferedTransport(socket));
            shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
            ServerClient coordFindVersion(protocol);
            try {
              transport->open();
              //printf("Server in Write Check Recieved Ping: %s\n", coordFindVersion.ping() ? "true" : "false");
              std::string full_path = "ServerFiles" + serversNw.at(i).PORT + "/" + filename;
              temp = coordFindVersion.CheckVersion(filename);
              if(max < temp) {
                max = temp;
              }
              transport->close();
            }
            catch (TException& e) {
               printf("Error: %s\n", e.what());
            }
          }
        }  // end for loop
        if(max == -1){
          //this file doesn't exist on any of these servers so this is the first version
          max = 1;
        }
        else{
          max++;
        }

        //3 Write to current versions and replace outdated ones in the Quorum
        //For version => take the highest found in step 2 and increment it by 1

        for(int i = 0; (unsigned) i < serversNw.size(); i++) {

          //Current server is this one (happens when the coordinator is part of the Quorum).
          if(!strcmp(serversNw.at(i).IP.c_str(), "me")){
            //write this server
            CoordWrite(filename, contents, max);
          }
          else{
            shared_ptr<TTransport> socket(new TSocket(serversNw.at(i).IP, std::stoi(serversNw.at(i).PORT)));
            shared_ptr<TTransport> transport(new TBufferedTransport(socket));
            shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
            ServerClient coordWrite(protocol);
            //Check versions
            try {
              transport->open();
              coordWrite.Write(filename, contents, max);
              transport->close();
            }
            catch (TException& e) {
               printf("Error: %s\n", e.what());
            }
          }
        }  // end for loop

        //4 Update rest in the background

        //If index = -1 that means this file didn't exist before this write
        if(index != -1){
          //Check whether the queue is empty
          if(!queue[index].empty()){
            //Empty the queue
            while(!queue[index].empty()) {
            	struct request next_request = queue[index].front();
              std::cout << "Queue: filename:\t" << next_request.filename << "\tcontents:\t" << next_request.contents << std::endl;
	          	queue[index].pop();
	            int success = ProcessRequest(next_request, max, index);
              if(success == 1) {
	              printf("Process Request successful\n");
	            }
	            else{
	              printf("Process Request failure\n");
	            }
              //if queue is finally empty, run background non quorum thread update
              if(queue[index].empty()) {
                std::thread{UpdateNonQuorum, non_quorum, next_request.filename, next_request.contents, max, index}.detach();
              }
	          }
          }
          else{
            //Spawn thread and update the servers outside of the quorum
            //UNLOCK THIS FILE FROM INSIDE THE THREAD SPAWNED!!!
	          printf("Empty queue onto thread\n");
            std::thread{UpdateNonQuorum, non_quorum, filename, contents, max, index}.detach();
          }
        }
        else{
          //spawn thread and update the servers outside of the quorum
	        printf("No Queue, Updating Non-Quorum\n");
          std::thread{UpdateNonQuorum, non_quorum, filename, contents, max, index}.detach();
        }
        //RETURN STATUS
        _return = "Write Complete.";
        return;

      }  // end coordinator write
      else if (OPCODE == 1) {
        int index = FindMetaDataIndex(filename);
        if(index != -1) {
          file_locks[index].lock();
          //printf("Read got the lock\n");
          file_locks[index].unlock();
        }
        //Grab random servers until # = Nr
        std::vector<server_info> serversNr;
        GetServersQuorum(serversNr, servers, Nr);

        // now contact Nr servers
        int max = -1;
        std::string maxIP = "";
        std::string maxPort = "";
        for(int i = 0;(unsigned) i < serversNr.size(); i++) {

          //This is the case that the coord is part of the read quorum
          if(!strcmp(serversNr.at(i).IP.c_str(), "me")){
            _return = CoordRead(filename);
            return;
          }
          else{
            shared_ptr<TTransport> socket(new TSocket(serversNr.at(i).IP, std::stoi(serversNr.at(i).PORT)));
            shared_ptr<TTransport> transport(new TBufferedTransport(socket));
            shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
            ServerClient coordFindVersion(protocol);
            //Check versions
            try {
              transport->open();
              //printf("Server in Read Recieved Ping: %s\n", coordFindVersion.ping() ? "true" : "false");
              std::string full_path = "ServerFiles" + serversNr.at(i).PORT + "/" + filename;
              int temp = coordFindVersion.CheckVersion(filename);
              if(max < temp) {
                max = temp;
                maxIP = serversNr.at(i).IP;
                maxPort = serversNr.at(i).PORT;
              }
              transport->close();
            }
            catch (TException& e) {
               printf("Error: %s\n", e.what());
            }
          }

          if(max == -1) {	// file does not exist
            _return = "DNE";
            return;
          }
        }  // end for loop
        //Select most up to date, If DNE => Respond likewise
        shared_ptr<TTransport> socket(new TSocket(maxIP, std::stoi(maxPort)));
        shared_ptr<TTransport> transport(new TBufferedTransport(socket));
        shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
        ServerClient coordRead(protocol);
        try {
          transport->open();
          coordRead.Read(_return, filename);
          transport->close();
        }
        catch (TException& e) {
          printf("Error: %s\n", e.what());
        }
      }  // end coordinator read
      else {
        //PRINT FUNCTION
        std::cout << "**********************PRINT*******************************\n";
        std::cout << "FILES" << ":\t\t" <<  "VERSION" << "\t\tCONTENTS" << std::endl;
        std::string dir = "ServerFiles" + myPORT + "/";
        for(int i = 0; (unsigned) i < filesMetaData.size(); i++) {
          std::string file = dir + filesMetaData.at(i).filename;
          //file = file + filesMetaData.at(i).filename;
          char line[30];
          std::string cont = "";
          std::ifstream rfile;
          rfile.open(file);
          if (rfile.is_open()) {
            while (rfile.getline(line, 30)) {
              cont = cont + line;
            }
            rfile.close();
          }
          std::cout << filesMetaData.at(i).filename << ":\t\t" <<  filesMetaData.at(i).version << "\t\t" << cont << std::endl;
        }
        std::cout << "*********************PRINT*************************************\n";
        _return = "Done Printing";
      } // end coordinator print
    }  // end coordinator
  }  // end SendRequest()

  //This function is used in processing a request to see what the file versions are for the write/read quorum
  int64_t CheckVersion(const std::string& filename) {
    // Your implementation goes here
    for(int i = 0;(unsigned) i < filesMetaData.size(); i++){
      //printf("comparing: %s\t%s\n",filesMetaData.at(i).filename.c_str(),  filename.c_str());
      if(!strcmp(filesMetaData.at(i).filename.c_str(), filename.c_str())){
        //printf("CheckVersion: %d\n", filesMetaData.at(i).version);
        return filesMetaData.at(i).version;
      }
    }
    //printf("CheckVersion: %d\n", -1);
    return -1;
  }

  void IncrementVersion(const std::string& filename, const int max) {
    printf("IncrementVersion\n");
    for(int i = 0;(unsigned) i < filesMetaData.size(); i++){
      if(!strcmp(filesMetaData.at(i).filename.c_str(), filename.c_str())){
        filesMetaData.at(i).version = max + 1;
      }
    }
    return;
  }

  //This function reads the file and returns the contents
  //Returns "DNE" if file is not found or file cant be open
  void Read(std::string& _return, const std::string& filename) {
    //printf("Read: %s\n", filename.c_str());
    std::string full_path = "ServerFiles" + myPORT + "/" + filename;

    for(int i = 0;(unsigned) i < filesMetaData.size(); i++){
      if(!strcmp(filesMetaData.at(i).filename.c_str(), filename.c_str())){
        _return = "";
        char line[30];
        std::ifstream rfile;
        rfile.open(full_path);
        if (rfile.is_open()) {
          while (rfile.getline(line, 30)) {
            _return = _return + line;
          }
          rfile.close();
        }
        else {
          printf("Could not open file %s in Read function!", filename.c_str());
          _return = "DNE";
        }
        return;
      }
    }
    _return = "DNE";
  }//End Read();

  //This function can be used in various places in this program, its function is self-explanatory
  //It writes the contents to the provided filename and updates to the given version.
  int64_t Write(const std::string& filename, const std::string& contents, const int64_t version) {
    //printf("Write file: %s version: %d\n", filename.c_str(), (int) version);
    writes++;
    //printf("writes: %d\n", writes);

    //Update metadata
    files current;
    current.filename = filename;
    current.version = version;
    UpdateMetaData(current);


    //Write to the file
    //Each server has its own subdirectory
    std::string full_path = "ServerFiles" + myPORT + "/" + filename;
    std::ofstream file (full_path, std::ofstream::out);
    //file.open (full_path);
    std::string line = contents;// + std::to_string(version), pretty sure the contents already include the version
    file << line;
    file.close();
    return 1;
  }

  //This is similar to Join() from PA2 where this just tells the coordinator that you exist
  void AnnounceLife(const std::string& IP, const std::string& Port) {
    server_info temp;
    temp.IP = IP;
    temp.PORT = Port;
    servers.push_back(temp);
    printf("There are now %d servers\n", (int) servers.size());
  }

};




/*
  TO RUN:
  >   ./server <ThisMachines Port> <This Machines IP> <CoordIP> <CoordPort> <Coordinator Mode>

  Coordinator Example:
  >  ./server 9000 localhost localhost 9000 1

  Non Coordinator Example:
  >  ./server 9001 localhost localhost 9000 0

*/

int main(int argc, char **argv) {
  int port;
  char* IP;

  //Gets Input for values and some light value verifying.

  if(argc == 6) {
    printf("Using user input for Server IP and Port\n");
    port = std::stoi(argv[1]);
    IP = argv[2];
    CoordIP = argv[3];
    CoordPort = std::stoi(argv[4]);
    coordinator = std::stoi(argv[5]);
  }
  else if(argc == 5) {
    printf("Using user input for Server IP and Port, DEFAULT value for server as NON COORDINATOR \n");
    port = std::stoi(argv[1]);
    IP = argv[2];
    CoordIP = argv[3];
    CoordPort = std::stoi(argv[4]);
    coordinator = 0;
  }
  else {
    printf("Invalid number of arguments\n");
    printf("Run ./server <ThisMachines Port> <This Machines IP> <CoordIP> <CoordPort> <Coordinator Mode>\n");
    return 1;
  }  // end check arguments
  //check if coordinator
  if(coordinator >= 1) {
    coordinator = 1;  // Keeps it binary
    while(N <= 0){	  // User inputs N total odd number of servers in file system
      std::cout << "Enter N positive, odd number of total servers in distributed file system:  ";
      std::cin >> N;
      if(N % 2 == 0) {
        N = 0;
      }
    }
    bool done = false;
    while(!done){
      std::cout << "Enter Nw, where Nw > N/2, and Nw <= N:  ";
      std::cin >> Nw;
      if(Nw > N/2 && Nw <= N) {
        done = true;
      }
    }
    done = false;
    while(!done){
      std::cout << "Enter Nr, where Nr + Nw > N, and Nr <= N:  ";
      std::cin >> Nr;
      if(Nw + Nr > N && Nr <= N) {
        done = true;
      }
    }
  }

  //If this server is not the coordinator, it needs to make contact with the true coordinator
  if(coordinator == 1){
    printf("I am the coordinator!\n");
  }

  //Make server's file subdirectory
  myPORT = argv[1];
  std::string dir_path = "ServerFiles" + myPORT;
  mkdir(dir_path.c_str(), 0777);

  if(!coordinator){
    std::string joinReturnString;
    shared_ptr<TTransport> socket(new TSocket(CoordIP, CoordPort));
    shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    ServerClient client(protocol);
    try {
      transport->open();
      printf("Joining: %s\t\t%s\n\r", IP, std::to_string(CoordPort).c_str());

      client.AnnounceLife(IP,std::to_string(port));
      printf("Coordinator Recieved Ping: %s\n", client.ping() ? "true" : "false");
      transport->close();
    }
    catch (TException& e) {
      printf("Error: %s\n", e.what());
    }

    //Start Own server
    shared_ptr<ServerHandler> handler(new ServerHandler());
    shared_ptr<TProcessor> processor(new ServerProcessor(handler));
    shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
    shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

    shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(160);
    shared_ptr<PosixThreadFactory> threadFactory = shared_ptr<PosixThreadFactory>(new PosixThreadFactory());
    threadManager->threadFactory(threadFactory);
    threadManager->start();

    TThreadPoolServer server(processor, serverTransport, transportFactory, protocolFactory, threadManager);
    printf("Listening on: %d\n", port);
    server.serve();
  }  // end non coordinator, non coordinator now listens
  else{
    //Start server right away

    //Store this server in the vector
    server_info temp;
    temp.PORT = "me";
    temp.IP = "me";
    servers.push_back(temp);

    shared_ptr<ServerHandler> handler(new ServerHandler());
    shared_ptr<TProcessor> processor(new ServerProcessor(handler));
    shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
    shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

    shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(160);
    shared_ptr<PosixThreadFactory> threadFactory = shared_ptr<PosixThreadFactory>(new PosixThreadFactory());
    threadManager->threadFactory(threadFactory);
    threadManager->start();

    TThreadPoolServer server(processor, serverTransport, transportFactory, protocolFactory, threadManager);
    printf("Listening on: %d\n", port);
    server.serve();
  }  // end coordinator, coordinator now listens


  return 0;
}  // end main()


// get random Nx servers
void GetServersQuorum(std::vector<server_info>& serversNx, const std::vector<server_info> servers, int Nx) {
  std::random_device rd;
  std::mt19937 gen(rd());
  int random = 0;
  int servers_added = 0;
  servers_added +=10; //To get rid of compiling warning for unused
  bool added = true;
  while (serversNx.size() < (unsigned) Nx) {
    std::uniform_int_distribution<> dis(0, N-1);
    random = (int) dis(gen);
    if (serversNx.empty())
      serversNx.push_back(servers.at(random));
    else {
      for(int i = 0;(unsigned) i < serversNx.size(); i++) {
        if( (servers.at(random).PORT) == (serversNx.at(i).PORT) ) {
          added = false;
          break;
        }
      }
      if(added) {
        serversNx.push_back(servers.at(random));
      }
      added = true;
    }
  }
  /*Debugging Print statement
  printf("Added %d servers to quorum request size: %d\n", (int) serversNx.size(), Nx);
  for(int i = 0; (unsigned) i < serversNx.size(); i++){
    printf("IP: %s\t\tPort: %s\n", serversNx.at(i).IP.c_str(), serversNx.at(i).PORT.c_str());
  }*/
}

//Gets non quorum servers for updating them in the background after a write request
void NonQuorumServers(std::vector<server_info>& non_quorum, const std::vector<server_info> serversNw) {
  int i = 0;
  for(i = 0; (unsigned) i < serversNw.size(); i++){
    if(servers.at(i).PORT != serversNw.at(i).PORT) {
      non_quorum.push_back(servers.at(i));
    }
  }
  while((unsigned) i < servers.size()){
    for(int j = 0; (unsigned) j < non_quorum.size(); j++) {
      if(servers.at(i).PORT != non_quorum.at(j).PORT) {
        non_quorum.push_back(servers.at(i));
      }
    }
    i++;
  }
}


// get random Nr and Nw values within constraints of user inputted N value
void SetValuesN(int N, int& Nr, int& Nw) {
  if(N == 1) {
    Nr = 1;
    Nw = 1;
    return;
  }
  int tempR;
  int tempW;
  std::random_device rd;
  std::mt19937 gen(rd());
  while (true) {
    std::uniform_int_distribution<> dis(1, N);
    tempR = (int) dis(gen);
    //std::uniform_int_distribution<> dis(1, N);might need to uncomment and fix w.e this is supposed to do TODO
    tempW = (int) dis(gen);
    if(((tempW + tempR) > N) && (tempW > (N/2))) {
      Nr = tempR;
      Nw = tempW;
      return;
    }
  }
  return;
}

//This function is used to find the index for a file in order to check the correct lock and queue
int FindMetaDataIndex(std::string filename){
  int metaIndex = -1;
  for(int i = 0; (unsigned) i < filesMetaData.size(); i++){
    //printf("comparing: %s\t%s\n",filesMetaData.at(i).filename.c_str(),  filename.c_str());
    if(!(strcmp(filesMetaData.at(i).filename.c_str(),filename.c_str()))){
      //update the version
      metaIndex = i;
      //printf("file %s pos in filesMetaData: %d\n",filesMetaData.at(i).filename.c_str(), i);
      return metaIndex;
    }
  }
  return metaIndex;
}

//Updates the metadata for a certain file. This can be used to correct a version number or add a new file to the metadata vector.
void UpdateMetaData(files updateInfo){
  //check if it exists
  int metaIndex = -1;
  for(int i = 0; (unsigned) i < filesMetaData.size(); i++){
    if(!(strcmp(filesMetaData.at(i).filename.c_str(),updateInfo.filename.c_str()))){
      //update the version
      metaIndex = updateInfo.version;
      filesMetaData.at(i).version = metaIndex;
      //printf("file %s pos in filesMetaData: %d\tver:%d\n",filesMetaData.at(i).filename.c_str(), i, metaIndex );
    }
  }
  //if not then push it onto vector
  if(metaIndex == -1){
    filesMetaData.push_back(updateInfo);
    //printf("after push updateInfo version = %d\n", updateInfo.version);
  }
}

//Almost identical to the thrift implementation, had to add so the coordinator wouldn't try to connect to itself.
int CoordWrite(std::string filename, std::string contents,  int version) {
  // Your implementation goes here
  //printf("CoordWrite file: %s version: %d\n", filename.c_str(), (int) version);
  writes++;
  //Update metadata
  files current;
  current.filename = filename;
  current.version = version;
  UpdateMetaData(current);

  //Write to the file
  //Each server has its own subdirectory
  std::string full_path = "ServerFiles" + myPORT + "/" + filename;
  std::ofstream file;
  file.open (full_path);
  std::string line = contents;// + std::to_string(version), pretty sure the contents already include the version
  file << line;
  file.close();
  return 1;
}

//Same as CoordWrite
//Almost identical to the thrift implementation, had to add so the coordinator wouldn't try to connect to itself.
std::string CoordRead(std::string filename) {
  //printf("CoordRead: %s\n", filename.c_str());
  std::string full_path = "ServerFiles" + myPORT + "/" + filename;
  std::string _return;
  for(int i = 0;(unsigned) i < filesMetaData.size(); i++){
    if(!strcmp(filesMetaData.at(i).filename.c_str(), filename.c_str())){
      //file = file + filesMetaData.at(i).filename;
      _return = "";
      char line[30];
      std::ifstream rfile;
      rfile.open(full_path);
      if (rfile.is_open()) {
        while (rfile.getline(line, 30)) {
          _return = _return + line;
        }
        rfile.close();
      }
      else {
        printf("Could not open file %s in Read function!", filename.c_str());
        _return = "DNE";
      }
      return _return;
    }
  }
  return "DNE";
}

// repeat process of writing to file in non quorum servers
// done in separate background thread, lock is released when
// all servers are done writing to file
void UpdateNonQuorum(const std::vector<server_info>& non_quorum, const std::string& filename, const std::string& contents, const int max, const int index) {
  std::thread::id this_id = std::this_thread::get_id();
  printf("Updating Non-Quorum Servers In The Background. File: %s\tContents:%s\n", filename.c_str(), contents.c_str());
  for(int i = 0; (unsigned) i < non_quorum.size(); i++) {

  //Current server is this one (happens when the coordinator is part of the Quorum).
    if(!strcmp(non_quorum.at(i).IP.c_str(), "me")){
      //write this server
      CoordWrite(filename, contents, max);
    }
    else{
      shared_ptr<TTransport> socket(new TSocket(non_quorum.at(i).IP, std::stoi(non_quorum.at(i).PORT)));
      shared_ptr<TTransport> transport(new TBufferedTransport(socket));
      shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
      ServerClient coordWriteBackground(protocol);
      //Check versions
      try {
        transport->open();
        coordWriteBackground.Write(filename, contents, max);//Or pass full_path
        transport->close();
      }
      catch (TException& e) {
        printf("Error: %s\n", e.what());
      }
    }
  }  // end for loop

  int ver = max;
  //Empty the queue while you have key
  while(!queue[index].empty()) {
    struct request next_request = queue[index].front();
    std::cout << "Queue: filename:\t" << next_request.filename << "\tcontents:\t" << next_request.contents << std::endl;
    queue[index].pop();
    // goes through queue FIFO and mimics sendrequest() write
    ver++;
    int success = ProcessRequest(next_request, ver, index);
    //debugging
    if(success == 1) {
      printf("Process Request successful\n");
    }
    else{
      printf("Process Request failure\n");
    }
    //if queue is finally empty, run background non quorum thread update
    if(queue[index].empty()) {
      std::thread{UpdateNonQuorum, non_quorum, next_request.filename, next_request.contents, ver, index}.detach();
    }
  }

  file_locks[index].unlock();
  std::cout << "Lock released in thread " << this_id << " with max version = " << (ver);
  std::cout << ", with filename = " << filename << " and contents = \"" << contents << "\"\n";
}

//This version of SendRequest() is used when ran in the background.
int ProcessRequest(struct request info, int max, int index) {
  //1 Grab random servers until # = Nw
  std::vector<server_info> serversNw;
  GetServersQuorum(serversNw, servers, Nw);

  //get servers not in quorum to update later
  std::vector<server_info> non_quorum;
  NonQuorumServers(non_quorum, serversNw);

  //write to Nw servers
  for(int i = 0; (unsigned) i < serversNw.size(); i++) {

    //Current server is this one (happens when the coordinator is part of the Quorum).
    if(!strcmp(serversNw.at(i).IP.c_str(), "me")){
      //write this server
      CoordWrite(info.filename, info.contents, max);
    }
    else{
      shared_ptr<TTransport> socket(new TSocket(serversNw.at(i).IP, std::stoi(serversNw.at(i).PORT)));
      shared_ptr<TTransport> transport(new TBufferedTransport(socket));
      shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
      ServerClient coordWrite2(protocol);
      try {
        transport->open();
        //printf("Server in Write Write Recieved Ping: %s\n", coordWrite2.ping() ? "true" : "false");

        coordWrite2.Write(info.filename, info.contents, max);//Or pass full_path
        transport->close();
      }
      catch (TException& e) {
        printf("Error: %s\n", e.what());
        return - 1;
      }
    }
  }
  return 1;
}
