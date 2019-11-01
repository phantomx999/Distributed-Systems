//COMPILE USING MAKEFILE: make node
//Or make all: make

//To start this: ./node <Port> <IP> <SuperNodeIP> <SuperNodePort>
//You can start these in any order ****AFTER THE SERVER IS STARTED***!!!!!!!!!!
//You can also have any number of nodes but it has to match the SuperNode's commandline input.
//It is 5 by default
//./node <ThisMachines Port> <This Machines IP> <SuperNode IP> <SuperNode Port>
//ex:
/*
./node 9001 localhost localhost 9090
./node 9002 localhost localhost 9090
./node 9003 localhost localhost 9090
./node 9004 localhost localhost 9090
./node 9005 localhost localhost 9090
*/

#include "gen-cpp/NodeService.h"
#include "gen-cpp/NodeService.cpp"
#include "gen-cpp/SuperNodeService.h"
#include "gen-cpp/SuperNodeService.cpp"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/TSocket.h>

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

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;

using boost::shared_ptr;

using namespace  ::PA2;

/******* Global Variables **********/
const int HASH_SIZE = 64; //The hash function returns size_t which on these machines is 64 bits
unsigned long long int finger[HASH_SIZE];
//n => this node, p => predecesssor, s => immediate successor
NodeStruct n,p,s,contact;

std::vector <std::string> PORTs;
std::vector <std::string> IPs;
std::vector <int> IDs;
bool oneNodeInDHT;
bool isDoneGettingContact;


/****** End Global Variables *******/

/****** Structs ********************/
struct books {
  std::vector <std::string> bookTitles;
  std::vector <std::string> genres;
}thisNodesBooks;

struct contact_info {
  std::vector <std::string> PORTs;
  std::vector <std::string> IPs;
  std::vector <int> IDs;
}nodes;
/****** End Structs ****************/

/******* Function Prototypes *******/
void DeterminePred(void);
void buildFingerTable(void);
void addIDToVect(int ID, std::string IP, std::string Port);
void SendUpdatesToPredSucc(void);
void UpdateDHT(void);
void UpdateFingers(void);
void ContactDHT(void);
void ParseReturnString(std::string str);
void ProcessContactString(std::string str);
void ProcessUpdatDHTString(std::string str);

//White space trimmers
static inline std::string &ltrim(std::string &s);
static inline std::string &rtrim(std::string &s);
static inline std::string &trim(std::string &s);

/***** End Function Prototypes *****/

class NodeServiceHandler : virtual public NodeServiceIf {
 public:
  NodeServiceHandler() {
    // Your initialization goes here
  }

  bool ping() {
    // Your implementation goes here
    //printf("ping\n");
    return true;
  }

  //This function handles all of the inter-Node communication via different request codes
  //Request = 0 => a node is requesting successor info
  //Request = 1 => This node is being contacted by it's Successor requesting this node to update
  //Request = 2 => This node is being contacted by it's Predecessor requesting this node to update
  void Contact(std::string& _return, const std::string& IP, const std::string& Port, const int64_t ID, const int64_t request) {
    // Your implementation goes here
    //printf("Contact\n");
    if(request == 0){//return contact info of s
      //return ID,IP,PORT
      _return = std::to_string(s.ID) + ',' + s.ip + ',' + s.port;
    }
    else if(request == 1){//Update predecessor with given info
      //set s and finger
      std::hash<std::string> h;
      std::string str;

      s.ip = IP;
      s.port = Port;
      s.ID = ID;

      str = s.ip + ':' + s.port;
      const size_t HashedLowerBound = h(str);
      if((uint64_t ) HashedLowerBound < (uint64_t ) n.topRangeKeys){
        //This number is close to 2^64
        n.botRangeKeys = 18066194203217662120;
      }

      str = IP + ':' + Port;
      const size_t hashedID = h(str);
      addIDToVect(ID, IP, Port);
      finger[ID] = hashedID;
      //printf("My range of keys: %lu - %lu\n", n.botRangeKeys, n.topRangeKeys);
      //printf("Updated Successor to node[%d]\t Pred: node[%d]\n", ID, p.ID);
    }
    else if(request == 2){//update successor with given info
      std::hash<std::string> h;
      std::string str;
      //set p and finger
      p.ip = IP;
      p.port = Port;
      p.ID = ID;

      //Change n's lower bound on botRangeKeys
      str = p.ip + ':' + p.port;
      const size_t HashedLowerBound = h(str);
      if((uint64_t ) HashedLowerBound > (uint64_t ) n.topRangeKeys){
        n.botRangeKeys = 0;
      }
      else{
        n.botRangeKeys = HashedLowerBound;
      }

      if(n.ID == s.ID && n.ID == 0){
        s.ip = IP;
        s.port = Port;
        s.ID = ID;
        std::hash<std::string> h;
        std::string str;

        str = s.ip + ':' + s.port;
        const size_t HashedLowerBound = h(str);
        if((uint64_t ) HashedLowerBound < (uint64_t ) n.topRangeKeys){
          //This number is close to 2^64
          n.botRangeKeys = 18066194203217662120;
        }
        str = IP + ':' + Port;
        const size_t hashedID = h(str);
        addIDToVect(ID, IP, Port);
        finger[ID] = hashedID;
        //printf("Updated Successor to node[%d]\n", ID);
      }

      str = IP + ':' + Port;
      const size_t hashedID = h(str);
      addIDToVect(ID, IP, Port);
      finger[ID] = hashedID;
      //printf("My range of keys: %lu - %lu\n", n.botRangeKeys, n.topRangeKeys);
      //printf("Updated Predecessor to node[%d]\t Suc: node[%d]\n",ID, s.ID);
    }
  }

  //These functions are NOT used and can probably be safely removed
  void ContactPred(std::string& _return, const std::string& IP, const std::string& Port, const int64_t ID, const int64_t request) {
  }

  void ContactSucc(std::string& _return, const std::string& IP, const std::string& Port, const int64_t ID, const int64_t request) {
  }

  //Used to Set a book title
  void Set(std::string& _return, const std::string& titles, const std::string& genres) {
    // Your implementation goes here
    //Return string goes: IP,Port,ID:IP,Port,ID....
    buildFingerTable();
    std::string builderString;
    std::hash<std::string> h;
    std::string str = s.ip + ':' + s.port;
    const size_t upperBoundCheck = h(str);

    //This is a check for the last node in the circle to completly include the full hash space
    if((uint64_t) upperBoundCheck < (uint64_t) n.topRangeKeys){
      n.topRangeKeys = 18436194203217662120;
    }
    const size_t hashedBook = h(titles);
    printf("Set: Recieved: %s , %s\nCorresponding Hash: %lu\n", titles.c_str(), genres.c_str(), (size_t) hashedBook);
    printf("Node[%d] has range of keys: %lu - %lu\n", n.ID, n.botRangeKeys, n.topRangeKeys);

    //check if book belongs here
    if((uint64_t) n.botRangeKeys < (uint64_t) hashedBook && (uint64_t) n.topRangeKeys > (uint64_t) hashedBook){//This is base case
      _return = n.ip + ',' + n.port + ',' + std::to_string(n.ID) + ':';

      //Check to make sure it isn't already set
      bool clear = true;
      for(int i = 0; i < thisNodesBooks.bookTitles.size(); i++){
        if(!strcmp(thisNodesBooks.bookTitles.at(i).c_str(), titles.c_str())){
          clear = false;
          thisNodesBooks.genres.at(i) = genres;
        }
      }
      if(clear){
        thisNodesBooks.bookTitles.push_back(titles);
        thisNodesBooks.genres.push_back(genres);
      }

      printf("BINGO!\nThis node now has %d book(s)\n", (int) thisNodesBooks.bookTitles.size());
      //print predecessor, successor, book titles, finger table
      printf("Node[%d] has Successor = Node[%d], Predecessor = Node[%d]\n", n.ID, s.ID, p.ID);

      //Print Books
      std::string builderString;
      for(int i = 0; i < thisNodesBooks.bookTitles.size(); i++){
        if(i != thisNodesBooks.bookTitles.size() - 2){
          builderString += thisNodesBooks.bookTitles.at(i) + ',';
        }
        else{
          builderString += thisNodesBooks.bookTitles.at(i);
        }
      }
      printf("This node has the books: %s\n", builderString.c_str());

      //Finger Table
      printf("Node[%d] Finger Table:\n", n.ID);
      for(int i = 0; i < IDs.size(); i++){
        printf("ID: %d\t Hash: %lu\n",IDs.at(i), finger[IDs.at(i)]);
      }
      printf("\n");
    }
    else{//Request needs to be routed elsewhere
      size_t largest = 18436194203217662120;
      size_t smallest = 18436194203217662120;
      int smallestIndex = 100;
      int largestIndex = 100;

      for(int i = 0; i < IDs.size(); i++){
        if((uint64_t) finger[IDs.at(i)] < (uint64_t) largest && (uint64_t) finger[IDs.at(i)] > (uint64_t) hashedBook){// && (uint64_t) finger[IDs.at(i)] > (uint64_t) n.topRangeKeys
          largest = finger[IDs.at(i)];
          largestIndex = i;
          //printf("found one at: %d\t\t%lu\n", IDs.at(largestIndex), largest);
        }
      }
      if(largestIndex == 100){
        //Finger table didn't find anything so forwarding this to Successor

        std::string tempReturn;
        printf("(small) Sending off to ID: %d\tIP:%s\tPort:%s\n", s.ID, s.ip.c_str(), s.port.c_str() );
        shared_ptr<TTransport> socket(new TSocket(s.ip, std::stoi(s.port)));
        shared_ptr<TTransport> transport(new TBufferedTransport(socket));
        shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
        NodeServiceClient client(protocol);
        try {
          transport->open();
          client.ping();
          client.Set(tempReturn, titles,genres);
          _return = n.ip + ',' + n.port + ',' + std::to_string(n.ID) + ':' + tempReturn;
          printf("Stringy: %s\n", _return.c_str());
	  printf("\n");
          transport->close();
        }
        catch (TException& e) {
          printf("Error: %s\n", e.what());
        }
      }
      else{
        //Routing this to the node found in the for loop above.
        std::string tempReturn;
        printf("Sending off to Node[%d]\tIP:%s\tPort:%s\n", IDs.at(largestIndex), IPs.at(largestIndex).c_str(), PORTs.at(largestIndex).c_str() );
        shared_ptr<TTransport> socket(new TSocket(IPs.at(largestIndex), std::stoi(PORTs.at(largestIndex))));
        shared_ptr<TTransport> transport(new TBufferedTransport(socket));
        shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
        NodeServiceClient client(protocol);
        try {
          transport->open();
          client.ping();
          client.Set(tempReturn, titles,genres);
          _return = n.ip + ',' + n.port + ',' + std::to_string(n.ID) + ':' + tempReturn;
          printf("Return String: %s\n", _return.c_str());
	  printf("\n");
          transport->close();
        }
        catch (TException& e) {
          printf("Error: %s\n", e.what());
        }
      }
    }

  }

  //Used to Set a book title
  void Get(std::string& _return, const std::string& titles) {
    // Your implementation goes here

    //Ensure that finger table is updated
    buildFingerTable();
    std::string builderString;
    std::hash<std::string> h;
    std::string str = s.ip + ':' + s.port;
    const size_t upperBoundCheck = h(str);

    //This is a check for the last node in the circle to completly include the full hash space
    if((uint64_t) upperBoundCheck < (uint64_t) n.topRangeKeys){
      n.topRangeKeys = 18436194203217662120;
    }
    const size_t hashedBook = h(titles);
    printf("Get: Recieved: %s\nCorresponding Hash: %lu\n", titles.c_str(), (size_t) hashedBook);
    printf("My range of keys: %lu - %lu\n", n.botRangeKeys, n.topRangeKeys);

    //check if book belongs here
    if((uint64_t) n.botRangeKeys < (uint64_t) hashedBook && (uint64_t) n.topRangeKeys > (uint64_t) hashedBook){//This is base case
      _return = n.ip + ',' + n.port + ',' + std::to_string(n.ID) + ',';

      //Check to see if we have it
      bool doesNotContain = true;
      for(int i = 0; i < thisNodesBooks.bookTitles.size(); i++){
        if(!strcmp(thisNodesBooks.bookTitles.at(i).c_str(), titles.c_str())){
          doesNotContain = false;
          //thisNodesBooks.genres.at(i) = genres;
          _return = _return + thisNodesBooks.genres.at(i);
          printf("Found requested book: %s", titles.c_str());
        }
      }
      if(doesNotContain){
        _return = _return + "DNE";
      }

      printf("Node[%d] has Successor = Node[%d], Predecessor = Node[%d]\n", n.ID, s.ID, p.ID);

      //Print Books
      std::string builderString;
      for(int i = 0; i < thisNodesBooks.bookTitles.size(); i++){
        if(i != thisNodesBooks.bookTitles.size() - 2){
          builderString += thisNodesBooks.bookTitles.at(i) + ',';
        }
        else{
          builderString += thisNodesBooks.bookTitles.at(i);
        }
      }
      printf("This node has the books: %s\n", builderString.c_str());

      //Finger Table
      printf("Node[%d] Finger Table:\n", n.ID);
      for(int i = 0; i < IDs.size(); i++){
        printf("ID: %d\t Hash: %lu\n",IDs.at(i), finger[IDs.at(i)]);
      }
      printf("\n");
    }
    else{//if not, route the request forward
      size_t largest = 18436194203217662120;
      size_t smallest = 18436194203217662120;
      int smallestIndex = 100;
      int largestIndex = 100;

      for(int i = 0; i < IDs.size(); i++){
        if((uint64_t) finger[IDs.at(i)] < (uint64_t) largest && (uint64_t) finger[IDs.at(i)] > (uint64_t) hashedBook){// && (uint64_t) finger[IDs.at(i)] > (uint64_t) n.topRangeKeys
          largest = finger[IDs.at(i)];
          largestIndex = i;
          //printf("found one at: %d\t\t%lu\n", IDs.at(largestIndex), largest);
        }
      }
      if(largestIndex == 100){
        //Forwards to Successor because an immediate solution wasn't found in the local table
        std::string tempReturn;
        printf("Routing request to Node[%d]\tIP:%s\tPort:%s\n", s.ID, s.ip.c_str(), s.port.c_str() );
        shared_ptr<TTransport> socket(new TSocket(s.ip, std::stoi(s.port)));
        shared_ptr<TTransport> transport(new TBufferedTransport(socket));
        shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
        NodeServiceClient client(protocol);
        try {
          transport->open();
          client.Get(tempReturn ,titles);
          _return = n.ip + ',' + n.port + ',' + std::to_string(n.ID) + ':' + tempReturn;
          printf("Get: Returning: %s\n", _return.c_str());
	  printf("\n");
          transport->close();
        }
        catch (TException& e) {
          printf("Error: %s\n", e.what());
        }
      }
      else{
        //Route to node found above

        std::string tempReturn;
        printf("Routing request to Node[%d]\tIP:%s\tPort:%s\n", IDs.at(largestIndex), IPs.at(largestIndex).c_str(), PORTs.at(largestIndex).c_str() );
        shared_ptr<TTransport> socket(new TSocket(IPs.at(largestIndex), std::stoi(PORTs.at(largestIndex))));
        shared_ptr<TTransport> transport(new TBufferedTransport(socket));
        shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
        NodeServiceClient client(protocol);
        try {
          transport->open();
          client.Get(tempReturn, titles);
          _return = n.ip + ',' + n.port + ',' + std::to_string(n.ID) + ':' + tempReturn;
          printf("Stringy: %s\n", _return.c_str());
	  printf("\n");
          transport->close();
        }
        catch (TException& e) {
          printf("Error: %s\n", e.what());
        }
      }
    }
    printf("\nThis node has %d book(s)\n", (int) thisNodesBooks.bookTitles.size());
  }

};

int main(int argc, char **argv) {
  int port;
  int superNodePort;
  std::string str;
  std::hash<std::string> h;
  //const size_t value = h("127.0.0.1:9001");
  char* serverIP;
  char* thisMachinesIP;
  if(argc == 5) {
    printf("Using user input for SuperNode IP and Port\n");
    thisMachinesIP = argv[2];
    serverIP = argv[3];
    superNodePort = std::stoi(argv[4]);
    port = std::stoi(argv[1]);
  }
  else if(argc == 2){
    printf("Assuming SuperNodeIP = localhost and SuperNodePort = 9090\n");
    port = std::stoi(argv[1]);
    serverIP = "localhost";
    thisMachinesIP = "localhost";
    superNodePort = 9090;
  }
  else{
    printf("Invalid number of arguments\n");
    printf("Run ./node <Port> <IP> <SuperNodeIP> <SuperNodePort>\nOr\n./node <Port> <IP>\nOr\n./node <Port>");
    return 1;
  }

  //Learn some info about this machine and store it
  std::string test = thisMachinesIP;
  str = test + ":" + std::to_string(port);
  n.ip = thisMachinesIP;
  n.port = std::to_string(port);
  const size_t thisMachinesID = h(str);
  printf("%lu\n", thisMachinesID);
  n.topRangeKeys = thisMachinesID;

  //Start Own Server
  std::string joinReturnString;
  shared_ptr<TTransport> socket(new TSocket(serverIP, superNodePort));
  shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  SuperNodeServiceClient client(protocol);
  try {
    transport->open();
    printf("Joining: %s\t\t%s\n\r", thisMachinesIP, std::to_string(port).c_str());

    client.Join(joinReturnString, thisMachinesIP, std::to_string(port));
    while(!strcmp(joinReturnString.c_str(), "NACK")){
      client.Join(joinReturnString, thisMachinesIP, std::to_string(port));
    }

    //Parse String and get Predecessor Node
    ParseReturnString(joinReturnString);
    if(!oneNodeInDHT){
      //Get contact info for all nodes
      ContactDHT();

      //Calculate the finger table values
      UpdateFingers();

      //Determine where you fit
      DeterminePred();

      //Updating connections in the DHT is done in SendUpdatesTOPredSucc()
      SendUpdatesToPredSucc();


      //TODO BuildFingerTable() more extensively, kind of like in ContactDHT where you follow the successors
      //around the ring to extensively establish the finger table
    }

    client.PostJoin(thisMachinesIP, std::to_string(port));
    //printf("SuperNode Recieved Ping: %s\n", client.ping() ? "true" : "false");
    transport->close();
  }
  catch (TException& e) {
    printf("Error: %s\n", e.what());
  }

  //This information is subject to change.
  printf("Node[%d] Finger Table:\n", n.ID);
  for(int i = 0; i < IDs.size(); i++){
    printf("ID: %d\t Hash: %lu\n",IDs.at(i), finger[IDs.at(i)]);
  }
  printf("Node[%d] has Successor = Node[%d], Predecessor = Node[%d]\n", n.ID, s.ID, p.ID);
  printf("\n");

  shared_ptr<NodeServiceHandler> handler(new NodeServiceHandler());
  shared_ptr<TProcessor> processor(new NodeServiceProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(16);
  shared_ptr<PosixThreadFactory> threadFactory = shared_ptr<PosixThreadFactory>(new PosixThreadFactory());
  threadManager->threadFactory(threadFactory);
  threadManager->start();

  TThreadPoolServer server(processor, serverTransport, transportFactory, protocolFactory, threadManager);
  printf("Listening on: %d\n", port);
  server.serve();

  return 0;
}

//This function notifies the predecessor and successor that I am here and to act accordingly
//Since when this function is called it has the best view of the DHT and gets to be bossy to it's neighbors
void SendUpdatesToPredSucc(){
  std::hash<std::string> h;
  std::string str;
  //const size_t hashedID = h(str);

  std::string returnString;
  if(p.ID != n.ID){
    str = p.ip + ':' + p.port;
    const size_t HashedLowerBound = h(str);
    if((uint64_t ) HashedLowerBound > (uint64_t ) n.topRangeKeys){
      n.botRangeKeys = 0;
    }
    else{
      n.botRangeKeys = HashedLowerBound;
    }
    //Send message to pred request == 1
    shared_ptr<TTransport> socket(new TSocket(p.ip, std::stoi(p.port)));
    shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    NodeServiceClient client(protocol);
    try {
      transport->open();
      client.Contact(returnString, n.ip, n.port, n.ID, 1);
      //printf("Update sent to Predecessor\n");
      transport->close();
    }
    catch (TException& e) {
      printf("Error: %s\n", e.what());
    }
  }

  if(s.ID != n.ID){
    //First update current
    str = s.ip + ':' + s.port;
    const size_t HashedLowerBound = h(str);
    if((uint64_t ) HashedLowerBound < (uint64_t ) n.topRangeKeys){
      //This number is close to 2^64
      n.botRangeKeys = 18066194203217662120;
    }
    //Send message to succ request == 2
    shared_ptr<TTransport> socket1(new TSocket(s.ip, std::stoi(s.port)));
    shared_ptr<TTransport> transport1(new TBufferedTransport(socket1));
    shared_ptr<TProtocol> protocol1(new TBinaryProtocol(transport1));
    NodeServiceClient client1(protocol1);
    try {
      transport1->open();
      client1.Contact(returnString, n.ip, n.port, n.ID, 2);
      //printf("Update sent to Successor\n");
      transport1->close();
    }
    catch (TException& e) {
      printf("Error: %s\n", e.what());
    }
  }
  //printf("My id: %d\t Predecessor node[%d]\t Suc: node[%d]\n",n.ID, p.ID, s.ID);
  //printf("My range of keys: %lu - %lu\n", n.botRangeKeys, n.topRangeKeys);
}

//This function establishes the successor and predecessor of this node.
//This code is janky and disgusting as there are a lot of edge cases in using this setup.
void DeterminePred(){
  //Suc has to be > n closest
  //Pred has to be < n closest
  int predIndex = 100;
  int succIndex = 100;
  size_t largest = 17366194203217662120;
  size_t smallest = 0;

  for(int i = 0; i < IDs.size(); i++){
    //printf("finger: %lu\tsmall: %lu\t%lu\n", finger[IDs.at(i)], smallest, largest );
    if(finger[IDs.at(i)] < largest && finger[IDs.at(i)] > (size_t) n.topRangeKeys){
      largest = finger[IDs.at(i)];
      succIndex = i;
    }
    else if(finger[IDs.at(i)] > smallest && finger[IDs.at(i)] < (size_t) n.topRangeKeys){
      smallest = finger[IDs.at(i)];
      predIndex = i;
    }
  }

  //Change ID,IP,Port of s
  //A predecessor wasn't found
  //printf("Index p/s: %d/%d\n",predIndex, succIndex);
  if(predIndex == 100){//May not work but in a 2 node setup this is necessary to maintain circularity
    p.ID = n.ID;
    p.ip = n.ip;
    p.port = n.port;
  }
  else{
    p.ID = IDs.at(predIndex);
    p.ip = IPs.at(predIndex);
    p.port = PORTs.at(predIndex);
  }
  if(succIndex == 100){//successor wasn't found
    s.ID = n.ID;
    s.ip = n.ip;
    s.port = n.port;
  }
  else{
    s.ID = IDs.at(succIndex);
    s.ip = IPs.at(succIndex);
    s.port = PORTs.at(succIndex);
  }

  //This is necessary to maintain circularity when only 2 nodes.
  if(n.ID == 1){
    p.ID = IDs.at(0);
    p.ip = IPs.at(0);
    p.port = PORTs.at(0);
    s.ID = IDs.at(0);
    s.ip = IPs.at(0);
    s.port = PORTs.at(0);
  }
  else if(s.ID == n.ID && n.ID != 1){
    //Set s to smallest in circle. This is closing the loop
    size_t compare = 17366194203217662120;
    int index = 100;
    for(int i = 0; i < IDs.size(); i++){
      if(finger[IDs.at(i)] < compare){
        compare = finger[IDs.at(i)];
        index = i;
      }
    }
    if(index != 100){
      s.ID = IDs.at(index);
      s.ip = IPs.at(index);
      s.port = PORTs.at(index);
    }
  }
}

//This function Traverses the DHT and pushes the info into the vectors IPs,PORTs,IDs
//TODO move those 3 vectors into a struct and fix that elsewhere
//This function uses the helper function ProcessContactString()
void ContactDHT(){
  //contact nodes and keep following the sucessor links until a value is returned that I already Have
  std::string returnString;

  isDoneGettingContact = false;
  //isDoneGettingContact is flipped in ProcessContactString()
  while(!isDoneGettingContact){
    IPs.push_back(contact.ip);
    PORTs.push_back(contact.port);
    IDs.push_back(contact.ID);

    shared_ptr<TTransport> socket(new TSocket(contact.ip, std::stoi(contact.port)));
    shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    NodeServiceClient client(protocol);
    try {
      transport->open();
      //printf("Contacting node[%d]\n", (int) contact.ID);
      client.Contact(returnString, contact.ip, contact.port, contact.ID, 0);
      ProcessContactString(returnString);
      //printf("ContactDHT: Received: %s\n", returnString.c_str());
      transport->close();
    }
    catch (TException& e) {
      printf("Error: %s\n", e.what());
    }
  }
}

//Parses and stores info from ContactDHT
//This function helps establish by parsing the response from other nodes and storing it.
void ProcessContactString(std::string str){
  printf("ProcessContactString: Now Processing: %s\n",str.c_str());
  int stringCounter = 0;
  std::stringstream ss(str);
  std::string token;
  while(std::getline(ss,token, ',')){
    switch(stringCounter){
      case 0: {//'S'
        contact.ID = std::stoi(token);
        stringCounter++;
        break;
      }
      case 1: {//s.ip
        contact.ip = token;
        stringCounter++;
        break;
      }
      case 2: {//s.port
        contact.port = token;
        stringCounter++;
        break;
      }
      default: {
        printf("ProcessContactString: Something went terribly wrong if I'm here!\n");
      }
    }
  }
  //Check if we already have this contact info
  for(int i = 0; i < IDs.size(); i++){
    if(contact.ID == IDs.at(i)){
      isDoneGettingContact = true;
      printf("Done making contact.\n");
    }
  }
}

void ProcessUpdatDHTString(std::string str){
  //printf("ProcessUpdateDHTString: Now Processing: %s\n",str.c_str());
  if(str.size() == 2){//Received OK
    //Do nothing
  }
  else{
    int stringCounter = 0;
    std::stringstream ss(str);
    std::string token;
    while(std::getline(ss,token, ',')){
      switch(stringCounter){
        case 0: {//'S'
          stringCounter++;
          break;
        }
        case 1: {//s.ip
          s.ip = token;
          stringCounter++;
          break;
        }
        case 2: {//s.port
          s.port = token;
          stringCounter++;
          break;
        }
        case 3: {//s.id
          s.ID = std::stoi(token);
          stringCounter++;
          break;
        }
        case 4: {//'P'
          stringCounter++;
          break;
        }
        case 5: {//'p.ip'
          p.ip = token;
          stringCounter++;
          break;
        }
        case 6: {//p.port
          p.port = token;
          stringCounter++;
          break;
        }
        case 7: {//p.id
          p.ID = std::stoi(token);
          stringCounter++;
          break;
        }
        default: {
          printf("ProcessUpdatDHTString: Something went terribly wrong if I'm here!\n");
        }
      }
    }
  }
}

//This function is called in main to parse the contact info for the first node in order to join the DHT
void ParseReturnString(std::string str){
  //printf("Processing Return String: %s\n", str.c_str());
  std::string token;
  int stringCounter = 0;
  //This is the first node and the string will only contain it's ID
  //THIS MIGHT BREAK WITH > 10 NODES
  if(str.size() == 1){
    //n = s = p for
    n.botRangeKeys = 0;
    n.ID = std::stoi(str);
    p.ID = n.ID;
    s.ID = n.ID;
    s.ip = n.ip;
    s.port = n.port;
    p.ip = n.ip;
    p.port = n.port;
    printf("ID: %d\n", (int) n.ID);
    oneNodeInDHT = true;
  }
  else{
    oneNodeInDHT = false;
    std::stringstream ss(str);
    while(std::getline(ss,token, ',')){
      switch(stringCounter){
        case 0: {
          n.ID = std::stoi(token);
          stringCounter++;
          break;
        }
        case 1: {
          //Other nodes IP
          contact.ip = token;
          stringCounter++;
          break;
        }
        case 2: {
          //Other nodes port
          contact.port = token;
          stringCounter++;
          break;
        }
        case 3: {
          //Prev nodes ID, can fill in finger table entry here
          contact.ID = std::stoi(token);
          stringCounter++;
          break;
        }
        default: {
          printf("ParseReturnString: Something went terribly wrong if I'm here!\n");
        }
      }
    }
    //Calc key range
    std::hash<std::string> h;
    std::string str = p.ip + ':' + p.port;
    const size_t thisMachinesLowerBound = h(str);
    n.botRangeKeys = thisMachinesLowerBound + 1;
    p.topRangeKeys = thisMachinesLowerBound;
  }
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

//Function updates the finger table
void UpdateFingers(){
  std::hash<std::string> h;
  std::string str;
  for(int i = 0; i < IDs.size(); i++){
    str = IPs.at(i) + ':' + PORTs.at(i);
    const size_t hashedID = h(str);
    finger[IDs.at(i)] = hashedID;
  }
}

//This function is used to help maintain a safe way to add nodes to the vectors.
void addIDToVect(int ID, std::string IP, std::string Port){
  //Check if it is in here
  bool toAdd = true;
  for(int i = 0; i < IDs.size(); i++){
    if(IDs.at(i) == ID){
      toAdd = false;
    }
  }
  if(toAdd){
    nodes.IDs.push_back(ID);
    nodes.IPs.push_back(IP);
    nodes.PORTs.push_back(Port);
    IDs.push_back(ID);
    IPs.push_back(IP);
    PORTs.push_back(Port);
  }
}

//This function uses the vectors to help build the Finger table
void buildFingerTable(void){
  //printf("Building Finger Table: Size %d\n", IDs.size());
  for(int i = 0; i < IDs.size(); i++){
    std::hash<std::string> h;
    std::string str = IPs.at(i) + ':' + PORTs.at(i);
    const size_t IDHash = h(str);
    finger[IDs.at(i)] = IDHash;
    //printf("Setting finger[%d] to %lu\tIP:%s\tPort:%s\n", IDs.at(i), finger[IDs.at(i)], IPs.at(i).c_str(), PORTs.at(i).c_str());
  }
}
