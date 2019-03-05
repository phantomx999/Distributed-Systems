/*
COMPILE USING MAKEFILE: make server
To Run: ./server
*/

#include <thrift/concurrency/ThreadManager.h>
//#include <thrift/concurrency/ThreadFactory.h>
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
#include <string>
#include <vector>
#include <thread> 
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

#include "gen-cpp/Job.h"
#include "gen-cpp/Job.cpp"
#include "gen-cpp/Node.h"
#include "gen-cpp/Node.cpp"
#include "gen-cpp/Task.h"
#include "gen-cpp/Task.cpp"


using namespace apache::thrift;
//using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace  ::project1;
using boost::shared_ptr;
using boost::make_shared;

Class JobHandler :  virtual public JobIf {
  public:
    JobHandler() : number_map_tasks(0), reduce_task(0), load_balancing_scheduler(0) {
      final_file = "";
      NodeHandler n1(1, 0.1, "localhost", 9002);
      NodeHandler n2(2, 0.5, "localhost", 9003);
      NodeHandler n3(3, 0.2, "localhost", 9004);
      NodeHandler n4(4, 0.9, "localhost", 9005);
    }
    virtual ~JobHandler() {}
    
    void CountFiles(std::string input) {
      unsigned long count = 0;
      struct dirent *direct;
      DIR *pdir = opendir(input);
      if(!pdir) {
        printf("Failed to open direcoty in CountFiles()\n");
        return -1;
      }
      while(direct = readdir(pdir)) {
          ++count;
      }
      closedir(pdir);
      number_map_tasks = (int) count;
      reduce_tasks = 1;
    }
    
    std::string PerformJob(std::string input, int mode) {
      // if 0: random task scheduler. 1: load-balancing scheduler
      load_balancing_scheduler = mode;
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
        for(int i = 0; i < (number_map_tasks+reduce_tasks); i++) { 
          NodeHandler node_chosen(0, 0, "", 0);
          while(node_chosen.node_num < 1 || node_chosen.node_num > 4) {
            srand (time(NULL));
            node_chosen.node_num = (rand() % 10) + 1;
          }
          switch(node_chosen.node_num) {
            case 1:
              node_chosen.node_num = n1.node_num;
              node_chosen.node_load = n1.node_load;
              node_chosen.node_host = n1.node_host;
              node_chosen.node_port = n1.node_port;
              break;
            case 2:
              node_chosen.node_num = n2.node_num;
              node_chosen.node_load = n2.node_load;
              node_chosen.node_host = n2.node_host;
              node_chosen.node_port = n2.node_port;
              break;
            case 3:
              node_chosen.node_num = n3.node_num;
              node_chosen.node_load = n3.node_load;
              node_chosen.node_host = n3.node_host;
              node_chosen.node_port = n3.node_port;
              break;
            case 4:
              node_chosen.node_num = n4.node_num;
              node_chosen.node_load = n4.node_load;
              node_chosen.node_host = n4.node_host;
              node_chosen.node_port = n4.node_port;
              break;
            default:
              std::cerr << "Error in server with choosing node for task\n";
              return -1;
          }
          std::shared_ptr<TTransport> socket(new TSocket(node_chosen.node_host, node_chosen.node_port));
          std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
          std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
          NodeHandler client(protocol);
          try {
            transport->open();
            if(i < number_map_tasks) {
              std::string inter_file = client.RandomMapTaskNode(data_task_files.at(i), node_chosen);
              intermediate_files.push_back(inter_file);
            }
            else{     //all map tasks completed, now need to execute reduce task
              final_file = client.RandomReduceTaskNode(intermediate_files, node_chosen);
            }
            transport->close();
          }
          catch (TException& e) {
            std::cout << "ERROR on server connection to node: " << e.what() << std::endl;  
          }
        }
        return final_file;
      }
    //  else {
    //    
    //  }
      //////////////////////////////      
      return "";
    }

  private: 
    int number_map_tasks;
    vector<string> data_task_files;
    int reduce_task;
    vector<string> intermediate_files;  
    std::string final_file;
    int load_balancing_scheduler;
    NodeHandler n1;  //computers to run as nodes
    NodeHandler n2;
    NodeHandler n3;
    NodeHandler n4;
    //vector<NodeHandler> clients;
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


int main() {
/*
  TThreadedServer server(
    std::make_shared<JobProcessorFactory>(shared_ptr<JobCloneFactory>()),
    std::make_shared<JobHandler> handler(new JobHandler());
    std::make_shared<TServerSocket>(9001), //port
    std::make_shared<TBufferedTransportFactory>(),
    std::make_shared<TBinaryProtocolFactory>());
    
  server.serve();
*/

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

  //This is a simple server for the Client connection

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


