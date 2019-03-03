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



using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

//static int intermediate_fle_count = 0;

namespace project1 (

Class JobHandler {
  public:
    JobHandler() : number_tasks(0) {}
    virtual ~JobHandler() {}
    
    void CountFiles(std::string input) {
      unsigned long count = 0;
      struct dirent *direct;
      DIR *pdir = opendir(intput);
      if(!pdir)
      {
        printf("Failed to open direcoty in CountFiles()\n");
        return 1;
      }
      while(direct = readdir(pdir)) {
          ++count;
      }
      closedir(pdir);
      /*
      std::string line;
      ifstream count_file (input);
      if(!count_file) {
        std::cout<<"Error opening input file to COUNT number of files"<< std::endl;
        system("pause");
        return;
      }
      while (getline(count_file, line)) {
         count++;
      }
      count_file.close();
      */
      number_tasks = (int) count;
      }

  private: 
    int number_tasks;
};


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