// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "Job.h"
#include "Job.cpp"
#include "Node.cpp"
#include "Node.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/transport/TBufferTransports.h>

#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PosixThreadFactory.h>

#include <iostream>
#include <sys/types.h>
#include <stdexcept>
#include <sstream>
#include <map>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;

using boost::shared_ptr;

using namespace  ::project1;

bool isReady = false;
bool isDone = false;

const int NUMBER_OF_NODES = 4;
Node_struct node[NUMBER_OF_NODES];


class NodeHandler : virtual public NodeIf {
 public:
  NodeHandler() {
    // Your initialization goes here
  }

  void StorePositiveWords() {
    // Your implementation goes here
    printf("StorePositiveWords\n");
  }

  void StoreNegativeWords() {
    // Your implementation goes here
    printf("StoreNegativeWords\n");
  }

  void RandomMapTaskNode(std::string& _return, const std::string& begin_files, const Node_struct& n) {
    // Your implementation goes here
    printf("RandomMapTaskNode\n");
  }

  void RandomReduceTaskNode(std::string& _return, const std::vector<std::string> & int_files, const Node_struct& n) {
    // Your implementation goes here
    printf("RandomReduceTaskNode\n");
  }

};


class JobHandler : virtual public JobIf {
 public:
  JobHandler() : number_map_tasks(0), load_balancing_scheduler(0), reduce_tasks(0){
    // Your initialization goes here
  }

  bool ping(){
    printf("Received Ping\n");
    return true;
  }

  void CountFiles(const std::string& var1) {
    // Your implementation goes here
    unsigned long count = 0;
    printf("Opening: %s\n", var1.c_str());
    struct dirent* direct;
    DIR *pdir = opendir(var1.c_str());
    if(!pdir) {
      printf("Failed to open directory in CountFiles()\n");
      exit(-1);
    }
    while((direct = readdir(pdir)) != NULL) {
        ++count;
    }
    closedir(pdir);
    number_map_tasks = (int) count;
  }

  bool GetStatus() {
    // The nodes poll this function to tell when the server is ready to send them tasks
    if(isReady){
      return true;
    }
    printf("getStatus\n");
  }

  void GetTasks(std::string& _return, const Node_struct& n) {
    // Your implementation goes here
    printf("GetTasks\n");
  }

  void PerformJob(std::string& _return, const std::string& input, const int64_t mode) {
    // Your implementation goes here

    printf("PerformJob\n");
    std::vector <std::string> data_task_files;
    std::string temp_for_pushing = "";
    load_balancing_scheduler = mode;
    int random_num;
    struct dirent *direct;
    DIR *pdir = opendir(input.c_str());
    if(!pdir) {
      printf("Failed to open directory in CountFiles()\n");
    }
    while((direct = readdir(pdir)) != NULL) {
      data_task_files.push_back(direct->d_name);
    }
    if(!load_balancing_scheduler) {
      //Assign tasks randomly
      for(int i = 0; i < (number_map_tasks+reduce_tasks); i++) {
        srand (time(NULL));
        random_num = (rand() % 4);
        temp_for_pushing = data_task_files[i];
        node[random_num].fileNames.push_back(temp_for_pushing);
      }

    }
    else {

    }
    //////////////////////////////
    _return = "";

  }
private:
  int number_map_tasks = 0;
  int reduce_tasks = 0;
  //vector<string> intermediate_files;
  //std::string final_file;
  int load_balancing_scheduler = 0;
};

int main(int argc, char **argv) {

  //initializating the node_structs
  for(int i = 0; i < NUMBER_OF_NODES; i++){
    node[i].uniqueID = i;
  }

  int port = 9001; //GET FROM CMD LINE????
  shared_ptr<JobHandler> handler(new JobHandler());
  shared_ptr<TProcessor> processor(new JobProcessor(handler));
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
