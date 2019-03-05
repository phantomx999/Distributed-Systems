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

#include "gen-cpp/Node.h"
#include "gen-cpp/Node.cpp"
#include "gen-cpp/Task.h"
#include "gen-cpp/Task.cpp"

static int intermediate_fle_count = 0;

using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace  ::project1;
using boost::shared_ptr;
using boost::make_shared;

Class NodeHandler : virtual public NodeIf {
  public:
   explicit NodeHandler(int num, double load, std::string host, int port) {
     node_num = num;
     node_load = load;
     node_host = host;
     node_port = port;
     num_tasks = 0;
   }
   
   virtual ~NodeHandler() {};
   
   void StorePositiveWords() override {
     /*
     int count_words = 0;
     std::string line;
     ifstream count_file ("positive.txt");
     if(!count_file) {
       std::cout<<"Error opening positive.txt file to COUNT words"<< std::endl;
       system("pause");
       return;
     }
     while (getline(count_file, line)) {
        count_words++;
     }
     count_file.close();
     */
     ifstream pos_file ("positive.txt");
     if(!pos_file) {
       std::cout<<"Error opening positive.txt file to STORE words"<< std::endl;
       system("pause");
       return;
     }
     /*
     positive_words = new std::string[count_words];
     int index = 0; 
     while (!pos_file.eof()) {
       getline (pos_file, positive_words[index]);
       index++;     
     }
     */
     std::string temp[1];
     while(!pos_file.eof()) {
       getline(pos_file, temp);
       positive_words.push_back(temp);
       temp = "";
     }
     pos_file.close();

     return;
   }
   
   void StoreNegativeWords() override {
     /*
     int count_words = 0;
     std::string line;
     
     ifstream count_file ("negative.txt");
     if(!count_file) {
       std::cout<<"Error opening negative.txt file to COUNT words"<< std::endl;
       system("pause");
       return;
     }
     
     while (getline(count_file, line)) {
        count_words++;
     }
     count_file.close();
     */
     ifstream neg_file ("negative.txt");
     if(!neg_file) {
       std::cout<<"Error opening positive.txt file to STORE words"<< std::endl;
       system("pause");
       return;
     }
     /*
     negative_words = new std::string[count_words];
     int index = 0; 
     while (!neg_file.eof()) {
       getline (neg_file, negative_words[index]);
       index++;     
     }
     */
     std::string temp[1];
     while(!neg_file.eof()) {
       getline(neg_file, temp);
       negative_words.push_back(temp);
       temp = "";
     }
     neg_file.close();
     return;
   }
   
   std::string RandomMapTaskNode(std::string begin_files, Node &n) {
     if(n.num_tasks == 0) {
       n.StorePositiveWords();
       n.StoreNegativeWords();
     }
     n.num_tasks++;
     TaskHandler ta = new TaskHandler();
     std::string intermediate = ta.MakeIntermediateFile(begin_files, n.positive_words, n.negative_words);
     return intermediate;
   }
   
   std::string RandomReduceTaskNode(std::vector<string> &int_files, Node &n) {
     if(n.num_tasks == 0) {
       n.StorePositiveWords();
       n.StoreNegativeWords();
     }
     n.num_tasks++;
     TaskHandler ta = new TaskHandler();
     std::string output = ta.MakeOutputResult(int_files);
     return output;
   }
   
  private:
    std::vector<std::string> positive_words;
    std::vector<std::string> negative_words;
    std::vector<TaskHandler> tasks;
    int node_num;
    double node_load;
    std::string node_host;
    int node_port;
    int num_tasks;
};

Class TaskHandler : virtual public TaskIf {
  public:
    explicit TaskHandler() : positive_count(0), negative_count(0), sentiment_value(0.0) {
      intermediate_filename = "intermediate";
    }
    
    virtual ~TaskHandler() {}
    
   double CalculateSentiment() override {
     return ((this.positive_count-this.negative_count)/(this.positive_count+this.negative_count));  
   }
   
   void CreateFileName() override {
     std::string str_file_num = std::to_string(intermediate_fle_count);
     intermediate_filename += std::string str_file_num;
     intermediate_filename += ".txt"; 
     intermediate_fle_count++;
   }
    
    void Map(std::string filename, std::vector<string> &positives, std::vector<string> &negatives) override {
      ifstream file;
      file.open (filename);
      if (!file.is_open()) {
       std::cerr << "Error with opening file in map function task\n" << std::endl;
       return;
      }
      std::string word = "";
      while (file >> word) {
        for(std::vector<string>::iterator i = positives.begin() ; i != positives.end(); ++i){
          if(word == positives.at(i)) {
            positive_count++;
          }           
        }
        for(std::vector<string>::iterator j = negatives.begin() ; j != negatives.end(); ++j){
          if(word == negatives.at(j)) {
            negative_count++;
          }           
        }
      }
      file.close();
      return;  
    }
    
    std::vector<string> Reduce(std::vector<string> &filenames) override {
      std::string temp[filenames.size()][2];
      ifstream file;
      for(int i = 0; i < filenames.size(); i++){
        file.open (filenames.at(i));
        if (!file.is_open()) {
          std::cerr << "Error opening a file in Reduce task" << std::endl;
          return;
        }
        int j = 0;
        while (file >> word) {
          word >> temp[i][j];
          j++;     
        }
        file.close;
      }
      //file_list = new std::string[filenames.size()][2];
      std::string temp2[2];
      for(int i = 0; i < temp.length()-1; i++) {  
        for(int j = 0; j < temp.length()-i-1; j++) {
          if(std::stof(temp[j][1]) < std::stof(temp[j+1][1])) {
           temp2[0] = temp[j][0];
           temp[j][0] = temp[j+1][0];
           temp[j+1][0] = temp2[0];
           temp2[1] = temp[j][1];
           temp[j][1] = temp[j+1][1];
           temp[j+1][1] = temp2[1];
          } 
        }
      }
      for(int i = 0; i < temp.length(); i++) {
        file_list.push_back(temp[i]);
      }
      return file_list;
    }
    
    std::string MakeIntermediateFile(std::string input_file_name, std::vector<string> &positive, 
                                     std::vector<string> &negative) override {
      this.CreateFileName();
      this.Map(input_file_name, positive, negative);
      double sent_val = this.CalculateSentiment();
      ofstream inter (intermediate_filename);
      if (inter.is_open()) {
        inter << input_file_name;
        inter << " ";
        inter << std::toString(sent_val);
        inter.close();
      }
      else {
        std::cout << "Unable to open intermediate file\n";
        return;
      }
      return intermediate_filename;
    }
   
   std::string MakeOutputResult(std::vector<string> &lists) {
     //output_result = new std::string(lists.size());
     output_result = this.Reduce(lists);
     ofstream out ("output.txt");
     while (out.is_open()) {
       for(int i = 0; i < lists.size(); i++) {
         out << output_result[i][0];
         out << " ";
         out << output_result[i][1] << std::endl;
     }
     out.close();
     return "output.txt";
   }
    
  private:
    int positive_count;
    int negative_count;
    std::vector<std::string> file_list;
    double sentiment_value;
    std::string intermediate_filename;
    std::vector<std::string> output_result;
};

int main() {
  ////////////////////////////
  int port_num = 9002;
  for(int i = 0; i < 4; i++) {
    TThreadedServer server(
   // std::make_shared<CalculatorProcessorFactory>(std::make_shared<CalculatorCloneFactory>()),
  // shared_ptr<HelloSvcHandler> handler(new HelloSvcHandler());
      std::make_shared<TServerSocket>(port_num), //port
      std::make_shared<TBufferedTransportFactory>(),
      std::make_shared<TBinaryProtocolFactory>());
      port_num++;
    server.serve();
  }
  return 0;
  /////////////////////////////-

}
