/*
COMPILE USING MAKEFILE: make client
To Run: ./client <SuperNodeIP> <SuperNodePort> 
Or
To Run With Default (SuperNode) Values: ./client

********** START THE CLIENT LAST TO ENSURE PROPER DHT SETUP ****************
*/

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <vector>
#include <dirent.h>
#include <thread>
#include <inttypes.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <bits/stdc++.h>

#include "gen-cpp/SuperNodeService.h"
#include "gen-cpp/SuperNodeService.cpp"
#include "gen-cpp/NodeService.h"
#include "gen-cpp/NodeService.cpp"

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

using namespace ::PA2;

/******* Global Variables **********/
//Have to keep these 2 tightly coupled, a struct would be better
std::vector <std::string> bookTitles;
std::vector <std::string> genre;
/****** End Global Variables *******/

/******* Function Prototypes *******/
void PrepareSetRequests(std::string filename);
bool CheckTextFile(std::string filename);
std::string ParseGenre(std::string str);
bool OnlyWhiteSpaces(std::string& line);
void TrimWord(std::string& word);
//void PrintNode(const NodeStruct& current);

//White space trimmers
static inline std::string &ltrim(std::string &s);
static inline std::string &rtrim(std::string &s);
static inline std::string &trim(std::string &s);

/***** End Function Prototypes *****/

/************** MAIN ***************/
int main(int argc, char **argv) {
  int id = -1;				//node id
  int port;					//node port
  std::string ip;			//node ip
  int superNodePort;		//super node port
  char* serverIP;			//super node ip
  if(argc == 3) {
    printf("Using user input for SuperNode IP and Port\n");
    serverIP = argv[1];
    superNodePort = std::stoi(argv[2]);
  }
  else if(argc == 1){
    printf("Assuming SuperNodeIP = localhost and SuperNodePort = 9090\n");
    serverIP = "localhost";
    superNodePort = 9090;
  }
  else{
    printf("Invalid number of arguments\n");
    printf("Run ./client <SuperNodeIP> <SuperNodePort>\nOr\n./client\n");
    return 1;
  }
  std::string text_file = "";
  while(text_file == "") {
      std::cout << "Enter text file (with correct directory path!) to use for bookerfinder DHT:  ";
      std::cin >> text_file;
  }
  std::cout << "Now checking if " << text_file << " has correct <title>:<genre> format used for bookfinder DHT... " << std::endl;
  bool correct_input = CheckTextFile(text_file);
  if(!correct_input) {
    std::cerr << "Text file has incorrect input.  Must be <title>:<genre> format with no empty lines.  Preparing to exit program\n";
   // system("pause");
    exit(EXIT_FAILURE);
  }
  //Get requests setup, eventually offer manual or automatic (read from a file) mode
  PrepareSetRequests(text_file);
  for(int i = 0; i < bookTitles.size(); i++){
    printf("%s: %s\n", bookTitles.at(i).c_str(), genre.at(i).c_str());
  }
  shared_ptr<TTransport> socket(new TSocket(serverIP, superNodePort)); //MIGHT NOT WORK IF GIVEN AN IP INSTEAD OF LOCALHOST
  shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

  SuperNodeServiceClient client(protocol);

  try {
    transport->open();
    printf("SuperNode Ready: %s\n", client.ping() ? "true" : "false");

    std::string temp;
    std::string port_str;
    while(!client.ping());
    std::cout << "Client now getting node from SuperNode " << std::endl;
    client.GetNode(temp);
    // parse temp to get ip and port
    // geeks for geeks: https://www.geeksforgeeks.org/tokenizing-a-string-cpp/
    std::vector <std::string> tokens;
    // stringstream class check1
    std::stringstream check1(temp);
    std::string intermediate;
    // Tokenizing w.r.t. space ' '
    while(getline(check1, intermediate, ' '))
    {
        tokens.push_back(intermediate);
    }
    ip = tokens[0];	         //node ip
    port_str = tokens[1];
    port = std::stoi(port_str);  //node port
    std::cout << "Client got node at ip = " << ip << " and at port = " << port <<  std::endl;
    std::cout << "Client now attempting to connect with node " << std::endl;
    transport->close();
  }
  catch (TException& e) {
    std::cout << "ERROR: " << e.what() << std::endl;
  }

  shared_ptr<TTransport> socket1(new TSocket(ip, port)); //MIGHT NOT WORK IF GIVEN AN IP INSTEAD OF LOCALHOST
  shared_ptr<TTransport> transport1(new TBufferedTransport(socket1));
  shared_ptr<TProtocol> protocol1(new TBinaryProtocol(transport1));
  NodeServiceClient client1(protocol1);
  std::string return_string;

  try {
    transport1->open();
    printf("Node Recieved Ping: %s\n", client1.ping() ? "true" : "false");
    std::cout << "Client successfully connected to node!  " << std::endl;
    std::cout << "Client now setting all books with genres from text file " << text_file << " into the DHT " << std::endl;
    for(int i = 0; i < bookTitles.size(); i++){
      client1.Set(return_string, bookTitles.at(i), genre.at(i));
      printf("%d: This request went through with response: %s\n", i, return_string.c_str());
    }
    std::cout << "Client now using Get() to obtain every genre for each book title " << std::endl;
    std::string gen;
    for(int i = 0; i < bookTitles.size(); i++) {
      client1.Get(gen, bookTitles.at(i));
      std::string parse_gen = ParseGenre(gen);
      std::cout << "bookTitle:  "  << bookTitles.at(i) << " --->   Genre:  "  << parse_gen << std::endl;
    }    
    std::string choice = "";
    std::cout << "Client now all done with setting and getting for book finder with text file " << text_file << std::endl;
    while(choice != "3") {
       if(choice == "1") {
         std::string input_title = "";
         std::string input_genre = "";
         std::cin.ignore();  // clear cin buffer
         while(input_title == "") {
           std::cout << "Enter title to set into DHT:  ";
           std::getline(std::cin, input_title);
         }
         while(input_genre == "") {
           std::cout << "Enter genre that goes with title to set into DHT:  ";
           std::getline(std::cin, input_genre);
         }
         bookTitles.push_back(input_title);
         genre.push_back(input_genre);
         client1.Set(return_string, input_title, input_genre);
         std::cout << "Client set " << input_title << " with genre " << input_genre << " into DHT!\n";
       }
       else if(choice == "2") {
         std::string get_title = "";
         std::string get_genre = "";
         std::cin.ignore();  // clear cin buffer
         while(get_title == "") {
           std::cout << "Enter title to get genre from DHT:  ";
           std::getline(std::cin, get_title);
         }
         client1.Get(get_genre, get_title);
         std::string gen_parse = ParseGenre(get_genre);
         std::cout << "bookTitle:  "  << get_title << " --->   Genre:  "  << gen_parse << std::endl;
       }
       else if(choice != "") {
         std::cout << "Incorrect Input!" <<  std::endl;
       }
       std::cout << "User can continue to choose to: \n"
                 "(1)  Set an individual book title and genre into the DHT\n"
                 "(2)  Get a genre from a book title currently in the DHT.\n"
                 "(3)  Quit Program\n";
       std::cout << "Enter 1, 2, or 3 to choose option:  ";
       std::cin >> choice;
    }
    std::cout << "User chose to quit program... Client says Goodbye to User!\n\n";      
    transport1->close();
  }
  catch (TException& e) {
    std::cout << "ERROR: " << e.what() << std::endl;
  }
  return 0;
}

//this function checks a text file to make sure it follows title:genre book finder convention
//for DHT data
bool CheckTextFile(std::string filename) {
  printf("Opening %s for data in CheckTextFile. . .\n", filename.c_str());
  std::ifstream file;
  file.open(filename);
  if(!file){
    std::cout<<"Error opening file in Client > CheckTextFile"<< std::endl;
    //system("pause");
    //return;
    exit(EXIT_FAILURE);
  }
  if(file.peek() == std::ifstream::traits_type::eof()) {
     // Check for Empty File
     std::cout << "Error, Empty file, no content!  Exiting Program" << std::endl;
     //system("pause");
     return false;
  }
  //Close file
  file.close();

  //check line by line
  std::string line;
  std::ifstream input(filename);
  for( std::string line; getline( input, line ); )
  {
    // for line that only has newline, ignore
    if(line[0] == '\n') {
      continue;
    }
    //  for line that only has whitespaces
    bool white_space = OnlyWhiteSpaces(line);
    if(white_space) {
      input.close();
      std::cout << "Error, Only whitespaces in line" << std::endl;
     // system("pause");
      return false;
    }
    //check if line does not have ":" delimiter
    if(line.find(":") == std::string::npos) {
      std::cout << "Error, No \':\' delimiter in text file line" << std::endl;
      input.close();
    //  system("pause");
      return false;
    }
    //check if line does not have a title or a genre
    if((line[0] == ':') || line[line.size()-1] == ':') {
      std::cout << "Error, No title or genre in text file line" << std::endl;
      input.close();
    //  system("pause");
      return false;
    }
    //check if line has more than one ":"
    int times =count(line.begin(),line.end(),':');
    if(times > 1) {
      std::cout << "Error, Too many \':\' in text file line" << std::endl;
      input.close();
    //  system("pause");
      return false;
    }
  }
  input.close();
  std::cout << "Text file is in correct <title>:<genre> format...." << std::endl;
  return true;
}
//This function takes a file and converts the contents into a Set request friendly format
void PrepareSetRequests(std::string filename){
  //Open file
  printf("Opening %s for data in PrepareSetRequests. . .\n", filename.c_str());
  std::ifstream file;
  file.open(filename);
  if(!file){
    std::cout<<"Error opening file in Client > PrepareSetRequests"<< std::endl;
    system("pause");
    //return;
    exit(EXIT_FAILURE);
  }
  //Parse each line into Book Title and Genre and pushback into vectors
  std::string line;
  std::string token;
  int isBook = true;

  while (!file.eof()) {
    getline (file, line);
    std::stringstream ss(line);

    while(std::getline(ss, token, ':')){
      if(isBook){
        bool white_space = OnlyWhiteSpaces(token);
        if(white_space) {
          file.close();
          std::cout << "Error, Only whitespaces in a line for book title, exiting Client Program" << std::endl;
         // system("pause");
          exit(EXIT_FAILURE);
        }
        trim(token);
        bookTitles.push_back(token);
        isBook = false;
      }
      else{
        bool white_space = OnlyWhiteSpaces(token);
        if(white_space) {
          file.close();
          std::cout << "Error, Only whitespaces in a line for genre, exiting Client Program" << std::endl;
         // system("pause");
          exit(EXIT_FAILURE);
        }
        trim(token);
        genre.push_back(token);
        isBook = true;
      }
    }
  }
  //Close file
  printf("Data is loaded.\n\n");
  file.close();
}

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

//  This function parses return string from Get() request to node 
//  in order to find and return genre in this returned string
//  if no such book title exists in Get() request, "DNE" is returned 
std::string ParseGenre(std::string str) {
  for(int i = 0; i < genre.size(); i++) {
      if(str.find(genre.at(i)) != std::string::npos) {
        return (genre.at(i));
      }
  }
  return "DNE";  // DNE = "Does Not Exist" 
}

/*
void PrintNode(const NodeStruct& current) {
  std::cout << "Node ID = " << current.ID << std::endl;
  std::cout << "Node topRangeKeys = " << current.topRangeKeys << std::endl;
  std::cout << "Node botRangeKeys = " << current.botRangeKeys << std::endl;
  std::cout << "Node predecessor = " << current.predecessorID << std::endl;
  std::cout << "Node successor = " << current.successorID << std::endl;
  std::cout << "Node number of books = " << current.numberOfBooks << std::endl;
  std::cout << "Node ip = " << current.ip << std::endl;
  std::cout << "Node port = " << current.port << std::endl;
  std::cout << "*****************************************" << std::endl;
  std::cout << "Node books: " << std::endl;
  for(int i = 0; i < current.book.size(); i++) {
     std::cout << current.books.at(i) << std::endl;
  }
  std::cout << "*****************************************" << std::endl;
  std::cout << "Node fingerTable: " << std::endl;
  for(int i = 0; i < current.fingerTable.size(); i++) {
     std::cout << current.fingerTable.at(i) << std::endl;
  }
  std::cout << "*****************************************\n" << std::endl;
}
*/
