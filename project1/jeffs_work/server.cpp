/*
COMPILE USING MAKEFILE: make server

To Run: ./server
*/

#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/ThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
//#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/TToString.h>

#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <map>
#include <fstream>
#include <iostream>
#include <string.h>
#include <vector>
#include <thread>
#include <dirent.h>
#include <stdio.h>

#include "Job.h"
#include "Job.cpp"

/*
#include <thrift/protocol/TBinaryProtocol.h>

#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>

#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
*/


/*
using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace project1;
*/
// if 0: random task scheduler. 1: load-balancing scheduler
static bool load_balancing_scheduler = 0;

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using namespace  ::project1;
using boost::shared_ptr;
using boost::make_shared;


class JobHandler : virtual public JobIf {
 public:
  JobHandler() : number_map_tasks(0), reduce_task(0) {}

  bool ping() {
    // Your implementation goes here
    printf("Recieved ping()\n");
    return true;
  }

  void CountFiles(const std::string& var1) {
    // Your implementation goes here
    printf("Counting files in: %s\n", var1.c_str());
    /*
    unsigned long count = 0;
    struct dirent *direct;
    DIR *pdir = opendir(input);
    if(!pdir) {
      printf("Failed to open direcoty in CountFiles()\n");
    }
    while(direct = readdir(pdir)) {
        ++count;
    }
    closedir(pdir);
    number_map_tasks = (int) count;*/
  }

  void PerformJob(std::string& _return, const std::string& input) {
    // Your implementation goes here

    struct dirent *direct;
    DIR *pdir = opendir(input);
    if(!pdir) {
      printf("Failed to open direcoty in CountFiles()\n");
      return -1;
    }
    while(direct = readdir(pdir)) {
      data_task_files.push_back(direct);
    }
    if(!load_balancing_scheduler) {
      int port_number = 9002;
      for(int i = 0; i < number_map_tasks; i++) {
        std::shared_ptr<TTransport> socket(new TSocket("localhost", port_number);
        std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
        std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
        clients.push_back(protocol);
        port_number++
        if(port_number > 9005){
          port_number = 9002;
        }
        transport->open();
      }

    }
    NodeClient client2(protocol);
    printf("PerformJob\n");
  }

  //static std::string SendTaskToNode(std::string filename) {

  //}

 private:
  int number_map_tasks;
  int reduce_task;
  //vector<string> data_task_files;
  //vector<Node_struct> clients;
};

class JobCloneFactory : virtual public JobIfFactory {
 public:
  ~JobCloneFactory() override = default;
  JobIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) override
  {
    std::shared_ptr<TSocket> sock = std::dynamic_pointer_cast<TSocket>(connInfo.transport);
    cout << "Incoming connection\n";
    cout << "\tSocketInfo: "  << sock->getSocketInfo() << "\n";
    cout << "\tPeerHost: "    << sock->getPeerHost() << "\n";
    cout << "\tPeerAddress: " << sock->getPeerAddress() << "\n";
    cout << "\tPeerPort: "    << sock->getPeerPort() << "\n";
    return new JobHandler;
  }
  void releaseHandler( ::shared::SharedServiceIf* handler) override {
    delete handler;
  }
};

/*
static std::string SendTaskToNode(std::string filename) {

}  */



int main() {

  const int workerCount = 4;
  shared_ptr<ThreadManager> threadManager =
    ThreadManager::newSimpleThreadManager(workerCount);
  threadManager->threadFactory(
    make_shared<ThreadFactory>());
  threadManager->start();
  // This server allows "workerCount" connection at a time, and reuses threads
  TThreadPoolServer server1(
    shared_ptr<JobProcessorFactory>(shared_ptr<JobCloneFactory>()),
    shared_ptr<TServerSocket>(9090),
    shared_ptr<TBufferedTransportFactory>(),
    shared_ptr<TBinaryProtocolFactory>(),
    threadManager);

  /*This is a simple server for the Client connection*/

  shared_ptr<JobHandler> handler(new JobHandler());
  shared_ptr<TProcessor> processor(new JobProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(9001));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  printf("We are up and running!\n");
  server.serve();

  return 0;
}
