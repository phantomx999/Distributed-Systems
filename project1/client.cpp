#include <iostream>
#include <string>
#include <map> 
#include <fstream>
#include <vector>
#include <dirent.h>
#include <thread> 



//Boost libraries
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

//Thrift libraries
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/protocol/TBinaryProtocol.h>             
#include <thrift/transport/TBufferTransports.h> 
#include <thrift/transport/TSocket.h>                    

//Namespaces
using boost::make_shared;
using namespace apache::thrift;
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;

namespace project1 (

int main(int argc, char **argv) {

  if(argc != 3) {
        printf("Usage: ./count \"<path>\"\n");
        return 1;
  }

  std::shared_ptr<TTransport> socket(new TSocket("localhost", 9001);
  std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  
  JobHandler client(protocol);
  
  try {
    transport->open();
    client.CountFiles(&(argv[3]));
    std::string output_file = client.PerformJob(&(argv[3]));
    transport->close();
  }
  catch (TException& e) {
    std::cout << "ERROR: " << e.what() << std::endl;  
  }
  return 0;

}

)
