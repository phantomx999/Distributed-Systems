/*
COMPILE USING MAKEFILE: make client
To Run: ./client <Server_IP> <Server_Port>
Or
To Run With Default (Server) Values: ./client

NOTE: Start after the server setup to ensure correct functionallity

Example 3 clients:

Connect to the default coordinator
./client localhost 9001
./client localhost 9004
./client localhost 9003
*/


#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <vector>
#include <dirent.h>
#include <thread>
#include <mutex>          // std::mutex
#include <inttypes.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <bits/stdc++.h>
#include <time.h>
#include <stdio.h>

//Boost libraries
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

//Thrift libraries
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TSocket.h>

#include "gen-cpp/Server.cpp"
#include "gen-cpp/Server.h"

//Namespaces
using boost::make_shared;
using boost::shared_ptr;
using namespace apache::thrift;
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;

using namespace ::PA3;

//IMPORTANT: RAISING THIS MAY CAUSE THE PROGRAM TO BECOME UNSTABLE
const int NUMOFTHREADS = 5;

/******* Global Variables **********/
std::mutex mut;
bool write_heavy = false;
/****** End Global Variables *******/

/******* Function Prototypes *******/
int AnalyzeOperationUI(std::string& operation);
bool OnlyWhiteSpaces(std::string& line);
void MakeRequest(int i, std::string& _return, std::string& filename, std::string& contents, ServerClient& client);

//White space trimmers
void TrimWord(std::string& word);
static inline std::string &ltrim(std::string &s);
static inline std::string &rtrim(std::string &s);
static inline std::string &trim(std::string &s);

/***** End Function Prototypes *****/

/************** MAIN ***************/
int main(int argc, char **argv) {
  char* serverIP;				 // server ip
  int serverPort;				 // server port
  int OPCODE = -1;				 // read or write operation
  std::string operation = "";	 // operation string UI input
  std::string input_file = "";   // file to send to server
  std::string contents = "";	 // contents to write to file
  std::string return_value = ""; // return value from server
  clock_t start_t, end_t, total_t;
  if(argc == 3) {
    printf("Using user input for Server IP and Server Port\n");
    serverIP = argv[1];
    serverPort = std::stoi(argv[2]);
  }
  else if(argc == 1){
    printf("Assuming Server IP = \"localhost\" and Server Port = 9090\n");
    serverIP = (char *) "localhost"; //server ip, typecast get rid of compiler warning
    serverPort = 9090;
  }
  else{
    printf("Invalid number of arguments\n");
    printf("Run ./client <Server IP> <Server Port>\nOr\n./client\n");
    return 1;
  }

  shared_ptr<TTransport> socket(new TSocket(serverIP, serverPort));
  shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

  ServerClient client(protocol);

  //The following secion uses OPCODES to simplify the server side of things.
  /*
  The codes are as following:
  Write => 0      Write Heavy Operation => 3
  Read => 1       Read Heavy Operation => 4
  Print => 2      STOP => -2
  */

  try {
    transport->open();

    //Run until the stop command is given
    while(OPCODE != -2) {

      //Read a command
      while(OPCODE != 0 && OPCODE != 1 && OPCODE != 2 && OPCODE != 3 && OPCODE != 4 && OPCODE != -2){
        std::cout << "Enter \"r\" to read file, \"w\" to write to file,  \"s\" to stop program, ";
        std::cout << "\"p\" to print SDFS files/versions, \"x\" to read-heavy, \"z\" to write-heavy:  ";
        std::cin >> operation;
        OPCODE = AnalyzeOperationUI(operation);
      }

      //Print Request => Prints in the coordinator, does not return anything.
      if (OPCODE == 2) {
        client.SendRequest(return_value, input_file, contents, OPCODE);
        printf("Received: %s\n", return_value.c_str());
        OPCODE = 200;
      }
      else if(OPCODE != -2){
        //Get filename that the user wants to work with
        while(input_file == ""){
          std::cout << "Enter filename you wish to read or write to:  ";
          std::cin >> input_file;
        }
        trim(input_file);
        //Write or heavy write command, need to get the contents
        if(OPCODE == 0 || OPCODE == 3){
          std::cout << "Enter contents you wish to write to file:  ";
          char holder[100];
          std::cin.ignore();
          std::cin.getline(holder, 100);
          contents = std::string(holder);
          std::cout << "contents = " << contents << '\n';
        }
        else if(OPCODE == 1 || OPCODE == 4) {  // read and read-heavy
          contents = "";
        }

        // Multithreaded read-heavy and write-heavy
        if(OPCODE == 3 || OPCODE == 4) {
          std::string cont[NUMOFTHREADS];

          //Write Heavy
          if(OPCODE == 3) {
            write_heavy = true;
            for(int j = 0; j < NUMOFTHREADS; j++) {
              std::string new_contents = contents;
              new_contents = new_contents + " thread_" + std::to_string(j) + " ";
              cont[j] = new_contents;
            }
          }
          else {
            //Fill the content array with junk if a ready-heavy order is called
            for(int j = 0; j < NUMOFTHREADS; j++) {
              cont[j] = "";
            }
          }

          std::thread t[NUMOFTHREADS + 2];
          printf("Launching %d threads\n", NUMOFTHREADS);

          //start time
          start_t = clock();
          for(int j = 0; j < NUMOFTHREADS; j++) { //NOTE FOR DATA RECORDING WE USED NUMOFTHREADS*NUMOFTHREADS TO OBTAIN BETTER DATA BUT IT MADE THE PROGRAM UNSTABLE
            for(int i = 0; i < NUMOFTHREADS; ++i) {
              t[i] = std::thread(MakeRequest, i, std::ref(return_value), std::ref(input_file), std::ref(cont[i]), std::ref(client));
            }
            for(int i = 0; i < NUMOFTHREADS; ++i) {
              t[i].join();
            }
          }

          //end time
          end_t = clock();
          double total = (double)(end_t - start_t) / CLOCKS_PER_SEC;
          total = total * 1000;
          printf("Total time taken by CPU in milliseconds: %f\n", total);

          double time_all_threads =  total/(NUMOFTHREADS*NUMOFTHREADS);
          printf("Total elapsed time (milliseconds/all_total_threads) from spawn to join: %f\n", time_all_threads);
        }
        else if (OPCODE == 0 || OPCODE == 1) { //Single request from the user, either write or read
          start_t = clock();
          client.SendRequest(return_value, input_file, contents, OPCODE);
          end_t = clock();

          double total = (double)(end_t - start_t) / CLOCKS_PER_SEC;
          total = total * 1000;
          printf("Total time taken by CPU in milliseconds: %f\n", total);
        }
        write_heavy = false;
        printf("Received: %s\n\n", return_value.c_str());
        OPCODE = 200;
        input_file = "";
      }
    }
    std::cout << "Goodbye!!!!!"  << std::endl;
    transport->close();
  }
  catch (TException& e) {
    std::cout << "ERROR: " << e.what() << std::endl;
  }
  return 0;
}

//Thread function that sends the request to the server
void MakeRequest(int i, std::string& _return, std::string& filename, std::string& contents, ServerClient& client) {
    if(write_heavy) {
      client.SendRequest(_return, filename, contents, 0);
    }
    else {
      client.SendRequest(_return, filename, contents, 1);
    }
}

/*
This function returns an OPCODE appropriate for the desired command
Write => 0      Write Heavy Operation => 3
Read => 1       Read Heavy Operation => 4
Print => 2      STOP => -2
*/
int AnalyzeOperationUI(std::string& operation) {
  if(!(strcmp("w", operation.c_str())) || !(strcmp("W", operation.c_str())) || !(strcmp("write", operation.c_str())) || !(strcmp("Write", operation.c_str())) || !(strcmp("WRITE", operation.c_str()))){
    return 0;
  }
  else if(!(strcmp("r", operation.c_str())) || !(strcmp("R", operation.c_str())) || !(strcmp("read", operation.c_str())) || !(strcmp("Read", operation.c_str())) || !(strcmp("READ", operation.c_str()))){
    return 1;
  }
  else if(!(strcmp("p", operation.c_str())) || !(strcmp("P", operation.c_str())) || !(strcmp("print", operation.c_str())) || !(strcmp("Print", operation.c_str())) || !(strcmp("PRINT", operation.c_str()))){
    return 2;
  }
  else if(!(strcmp("z", operation.c_str())) || !(strcmp("Z",  operation.c_str()))) {
    return 3;
  }
  else if(!(strcmp("x", operation.c_str())) || !(strcmp("X",  operation.c_str()))) {
    return 4;
  }
  else if(!(strcmp("s", operation.c_str())) || !(strcmp("S", operation.c_str())) || !(strcmp("stop", operation.c_str())) || !(strcmp("Stop", operation.c_str())) || !(strcmp("STOP", operation.c_str()))){
    return -2;
  }
  else{
    return -1;
  }
}

//The next few functions are all for trimming white space off inputs
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}

bool OnlyWhiteSpaces(std::string& line) {
  bool no_characters = std::all_of(line.begin(),line.end(),isspace);
  if(no_characters) {
    return true;
  }
  return false;
}

//  trims
void TrimWord(std::string& word)
{
    if (word.empty()) return;

    // Trim spaces from left side
    while (word.find(" ") == 0)
    {
        word.erase(0, 1);
    }

    // Trim spaces from right side
    size_t len = word.size();
    if(len == 0) {return;}
    while (word.rfind(" ") == --len)
    {
        word.erase(len, len + 1);
    }
}
