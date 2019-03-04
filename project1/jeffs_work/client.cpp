/*
COMPILE USING MAKEFILE: make client

To Run: ./client "<serverIP>" "<inputDirectory>"
*/

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <vector>
#include <dirent.h>
#include <thread>

#include "Job.cpp"
#include "Job.h"

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
using boost::shared_ptr;
using namespace apache::thrift;
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;

using namespace ::project1;

int main(int argc, char **argv) {

  if(argc != 3) {
        printf("Usage: ./count \"<path>\"\n");
        return 1;
  }
  char* serverIP = argv[1];
  char* inputDir = argv[2];
  shared_ptr<TTransport> socket(new TSocket(serverIP, 9001));
  shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

  JobClient client(protocol);


  try {
    transport->open();
    client.ping();
    printf("Pinging()..\n");
    client.CountFiles(inputDir);
    //std::string output_file = client.PerformJob(&(argv[3]));
    transport->close();
  }
  catch (TException& e) {
    std::cout << "ERROR: " << e.what() << std::endl;
  }
  return 0;

}
