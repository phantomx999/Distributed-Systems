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

  if(argc != 3 || argc != 4) {
        printf("Invalid number of arguments\n");
        printf("Run client with input_directory and mode number (0 = random scheduling (Default), 1 = load scheduling\n");
        return 1;
  }
  int mode = 0;
  if(argc == 4) {
    mode = std::stoi(argv[4]);
  }
  if(mode != 0 || mode != 1) {
    std::cerr << "Error with mode input value (must be omitted, 0, or 1 value)\n" << std::endl;
    return 1;
  }

  std::shared_ptr<TTransport> socket(new TSocket("localhost", 9001));
  std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  
  JobHandler client(protocol);
  
  try {
    transport->open();
    client.CountFiles(argv[3]);
    std::string output_file = client.PerformJob(argv[3], mode);
    transport->close();
  }
  catch (TException& e) {
    std::cout << "ERROR: " << e.what() << std::endl;  
  }
  return 0;

}

)
