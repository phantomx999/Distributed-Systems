/*
COMPILE USING MAKEFILE: make super
COMPILE USING MAKEFILE ALL: make

To Run: ./super <Port> <Number Of Nodes>
Or
        ./super <Port>      => Uses default Number Of Nodes = 5
        ./super             => Uses default NUmber of Nodes = 5 and Port = 9090

App Notes: Ideally start this before ./client or ./node
*/

#include "gen-cpp/SuperNodeService.cpp"
#include "gen-cpp/SuperNodeService.h"
#include "gen-cpp/NodeService.h"
#include "gen-cpp/NodeService.cpp"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <thread>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;

using boost::shared_ptr;

using namespace  ::PA2;

/******* Global Variables **********/
const int MAX_NODES = 64;
bool empty = true;
bool busy = false;
bool isDHTBuilt = false;
int node_id = 0;
int current_node = -1;
int numberOfNodes;
NodeStruct nodes[MAX_NODES];
/****** End Global Variables *******/

/******* Function Prototypes *******/
void addNodeToList(int id, std::string ip, std::string port);
void PrintAllNodes();
void SendUpdatePing();
/***** End Function Prototypes *****/

class SuperNodeServiceHandler : virtual public SuperNodeServiceIf {
 public:
  SuperNodeServiceHandler() {
    // Your initialization goes here
  }

  bool ping() {
    // Your implementation goes here

    return isDHTBuilt;
  }

  // get node id and port
  void GetNode(std::string& _return) {
    // Your implementation goes here
    if(empty) {
     std::cerr << "Error: Empty DHT" << std::endl;
     return;
    }
    printf("GetNode\n");
    int random_index = 0;
    //NEED TO MAKE SURE DHT IS BUILT
    //fancy way to find random number......
    // stack overflow: https://stackoverflow.com/questions/7560114/random-number-c-in-some-range
    std::random_device hardware_num; // obtain a random number from hardware
    std::mt19937 eng(hardware_num()); // seed the generator
    std::uniform_int_distribution<> distr(0, numberOfNodes-1); // define the range
    random_index = (int) distr(eng);
    std::string ip_ = nodes[random_index].ip;
    std::string port_ = nodes[random_index].port;
    _return = ip_ + " " + port_;//How does client handle this?

  }

  void Join(std::string& _return, const std::string& IP, const std::string& Port) {
    // Your implementation goes here
    printf("Join:Recieved %s\t\t%s\n", IP.c_str(), Port.c_str());
    if(busy) {
      _return = "NACK";
    }
    else {
      busy = true;
      if(empty) {
        std::string temp = std::to_string(node_id);
        _return = temp;
        empty = false;
        busy = false;
      }
      else {
        std::string prev_node_ip = nodes[current_node].ip;
        std::string prev_node_port = nodes[current_node].port;
        std::string prev_node_id = std::to_string(nodes[current_node].ID);
        std::string temp = std::to_string(node_id) + "," + prev_node_ip + "," + prev_node_port + "," + prev_node_id;
        _return = temp;
        busy = false;
      }
    }
  }

  void PostJoin(const std::string& IP, const std::string& Port) {
    // Your implementation goes here
    printf("PostJoin: %s\t\t%s\n", IP.c_str(), Port.c_str());
    addNodeToList(node_id, IP, Port);
    node_id++;
    if (empty) empty = false;
    busy = false;
    if(node_id == numberOfNodes){
      isDHTBuilt = true;
      //SendUpdatePing();
    }
  }

  //private:
};

int main(int argc, char **argv) {
  int port;
  if(argc == 3) {
    port = std::stoi(argv[1]);
    numberOfNodes = std::stoi(argv[2]);
  }
  else if(argc == 2){
    port = std::stoi(argv[1]);
    printf("Defaulting to Number of Nodes = 5\n");
    numberOfNodes = 5;
  }
  else if(argc == 1){
    printf("Defaulting to Port 9090 and Number of Nodes = 5\n");
    port = 9090;
    numberOfNodes = 5;
  }
  else{
    printf("Invalid number of arguments\n");
    printf("Run ./super <Port> <Number Of Nodes> \nOr\n./super <Port>\nOr\n./super\n");
    return 1;
  }

  shared_ptr<SuperNodeServiceHandler> handler(new SuperNodeServiceHandler());
  shared_ptr<TProcessor> processor(new SuperNodeServiceProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(16);
  shared_ptr<PosixThreadFactory> threadFactory = shared_ptr<PosixThreadFactory>(new PosixThreadFactory());
  threadManager->threadFactory(threadFactory);
  threadManager->start();

  TThreadPoolServer server(processor, serverTransport, transportFactory, protocolFactory, threadManager);
  printf("Listening on: %d\n", port);
  server.serve();

  return 0;
}

void addNodeToList(int id, std::string ip, std::string port) {
  current_node++;
  struct NodeStruct temp;
  // FindNode(temp, id);
  // nodes[current_node] = temp;
  nodes[current_node].ID = id;
  nodes[current_node].ip = ip;
  nodes[current_node].port = port;
  return;
}

void PrintAllNodes(){
    if(empty) {
      std::cerr << "Error, Empty DHT" << std::endl;
      return;
    }
    for(int i = 0; i <= current_node; i++){
      std::cout << "ID:  " << nodes[i].ID;
      std::cout << "\tip:  " << nodes[i].ip;
      std::cout << "\tport:  " << nodes[i].port << std::endl;
    }
}

void SendUpdatePing(){
  //Delay for a second so the nodes get a chance to settle
  std::this_thread::sleep_for(std::chrono::milliseconds(4000));
  if(empty) {
    std::cerr << "Error, Empty DHT" << std::endl;
    return;
  }
  for(int i = 0; i <= current_node; i++){
    //ping them
    shared_ptr<TTransport> socket(new TSocket(nodes[i].ip, std::stoi(nodes[i].port)));
    shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    NodeServiceClient client(protocol);
    try {
      transport->open();
      if(!client.ping()){
        printf("SendUpdatePing: Failed\n");
      }
      else{
        printf("SendUpdatePing: Passed\n");
      }
      transport->close();
    }
    catch (TException& e) {
      printf("Error: %s\n", e.what());
    }
  }
}
