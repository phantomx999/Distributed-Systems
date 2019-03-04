#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/ThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
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



using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

// if 0: random task scheduler. 1: load-balancing scheduler
static bool load_balancing_scheduler = 0;

namespace project1 (

Class JobHandler {
  public:
    JobHandler() : number_map_tasks(0), reduce_task(0) {}
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
    }
    
    std::string PerformJob(std::string input) {
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
      
    }

    static std::string SendTaskToNode(std::string filename) {
          
    }  

  private: 
    int number_map_tasks;
    int reduce_task;
    vector<string> data_task_files;
    vector<NodeClient> clients;
};

static std::string SendTaskToNode(std::string filename) {

}  



int main() {
  TThreadedServer server(
   // std::make_shared<CalculatorProcessorFactory>(std::make_shared<CalculatorCloneFactory>()),
  // shared_ptr<HelloSvcHandler> handler(new HelloSvcHandler());
    std::make_shared<TServerSocket>(9001), //port
    std::make_shared<TBufferedTransportFactory>(),
    std::make_shared<TBinaryProtocolFactory>());
    
  server.serve();
    
  return 0;
}

)