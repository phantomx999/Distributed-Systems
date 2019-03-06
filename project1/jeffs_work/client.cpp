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
#include <inttypes.h>

#include "Job.h"
#include "Job.cpp"

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
  printf("%d\n",argc);
  if(argc != 3 && argc != 4) {
        printf("Invalid number of arguments\n");
        printf("Run ./client <serverIP> <inputDirectory> <mode number> where mode number is optional input\n");
        printf("mode number: 0 = random scheduling (Default if no mode number user inputted), 1 = load scheduling\n");
        return 1;
  }
  int64_t mode = 0;
  if(argc == 4) {
    mode = std::stoi(argv[3]);
  }
  if(mode != 0 && mode != 1) {
    std::cerr << "Error with mode input value (must be omitted, 0, or 1 value)\n" << std::endl;
    return 1;
  }

  char* serverIP = argv[1];
  char* inputDir = argv[2];
  shared_ptr<TTransport> socket(new TSocket(serverIP, 9001)); //MIGHT NOT WORK IF GIVEN AN IP INSTEAD OF LOCALHOST
  shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

  JobClient client(protocol);

  try {
    transport->open();
    client.ping();
    printf("Sending Input Directory. . . \n");
    //client.CountFiles(inputDir);
    std::string output_file;
    printf("Requesting Job. . .\n");
    //client.PerformJob(output_file, inputDir, mode);
    printf("Jobs Done!\n");
    while(1);
    transport->close();
  }
  catch (TException& e) {
    std::cout << "ERROR: " << e.what() << std::endl;
  }
  return 0;

}
