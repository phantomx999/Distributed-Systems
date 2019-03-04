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

static int intermediate_fle_count = 0;

using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

namespace project1 {

Class NodeHandler {
  public:
   explicit NodeHandler() {
     //task = new Task();
     positive_words = "";
     negative_words = "";
     //intermediate_filename = "intermediate";
   }
   
   virtual ~NodeHandler() {};
   
   void StorePositiveWords() override {
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
     
     ifstream pos_file ("positive.txt");
     if(!pos_file) {
       std::cout<<"Error opening positive.txt file to STORE words"<< std::endl;
       system("pause");
       return;
     }
     positive_words = new std::string[count_words];
     int index = 0; 
     while (!pos_file.eof()) {
       getline (pos_file, positive_words[index]);
       index++;     
     }
     pos_file.close();
     return;
   }
   
   void StoreNegativeWords() override {
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
     
     ifstream neg_file ("negative.txt");
     if(!neg_file) {
       std::cout<<"Error opening positive.txt file to STORE words"<< std::endl;
       system("pause");
       return;
     }
     negative_words = new std::string[count_words];
     int index = 0; 
     while (!neg_file.eof()) {
       getline (neg_file, negative_words[index]);
       index++;     
     }
     neg_file.close();
     return;
   }
   
  // map<std::string* s, float val> CreateIntermediateFile() override {
  //   intermediate_file.insert(pair<std::string*, float>(intermediate_filename, sentiment_value);
  // }
   
  private:
    std::string* positive_words;
    std::string* negative_words;
    std::vector<TaskHandler> tasks;
  //  std::map<std::string* s, float val> intermediate_file;
};

Class TaskHandler {
  public:
    explicit TaskHandler() : positive_count(0), negative_count(0), sentiment_value(0) {
      intermediate_filename = "intermediate";
    }
    virtual ~TaskHandler() {}
    
    float CalculateSentiment() override {
     return ((positive_count-negative_count)/(positive_count+negative_count));  
   }
   
   void CreateFileName() override {
     char str[intermediate_fle_count.length()];
     std::string str_file_num = sprintf(str, "%d", intermediate_fle_count);
     intermediate_filename += std::string str_file_num;
     intermediate_filename += ".txt"; 
     intermediate_fle_count++;
   }
    
    void Map(std::string* filename, std::string* positives, std::string* negatives) override {
      ifstream file;
      file.open (filename);
      if (!file.is_open()) return;
      std::string word;
      int pos_length = sizeof(positives)/sizeof(int);
      int neg_length = sizeof(negatives)/sizeof(int);

      while (file >> word) {
        for(int i = 0; i<pos_length; i++){
          if(word == positives[i]) {
            positive_count++;
          }           
        }
        for(int j = 0; j<pos_length; j++){
          if(word == negatives[j]) {
            negative_count++;
          }           
        }
      }
      file.close();
      return;  
    }
    
    string* Reduce(std::string* filenames) override {
      std::string temp[filenames.length()][2];
      ifstream file;
      for(int i = 0; i < filenames.length(); i++){
        file.open (filenames[i]);
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
      file_list = new std::string[filenames.length()][2];
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
      file_list = temp;
      return file_list;
    }
    
    std::string MakeIntermediateFile(std::string input_file_name) override {
     this.CreateFileName();
     //this.StorePositiveWords();
     //this.StoreNegativeWords();
     this.Map(&input_file_name, &positive_words, &negative_words);
     float sent_val = this.CalculateSentiment();
     ofstream inter (intermediate_filename);
     if (inter.is_open()) {
       inter << input_file_name;
       inter << " ";
       inter << std::toString(sent_val);
       inter.close();
     }
     else std::cout << "Unable to open file";
     return intermediate_filename;
   }
   
   std::string MakeOutputResult(std::string* lists) {
     output_result = new std::string(lists.length());
     output_result = this.Reduce(lists);
     ofstream out ("output.txt");
     while (out.is_open()) {
       for(int i = 0; i < lists.length(); i++) {
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
    std::string* file_list;
    float sentiment_value;
    std::string intermediate_filename;
    std::string* output_result;
};

int main() {
  TThreadedServer server(
   // std::make_shared<CalculatorProcessorFactory>(std::make_shared<CalculatorCloneFactory>()),
  // shared_ptr<HelloSvcHandler> handler(new HelloSvcHandler());
    std::make_shared<TServerSocket>(9002), //port
    std::make_shared<TBufferedTransportFactory>(),
    std::make_shared<TBinaryProtocolFactory>());
    
    server.serve();

}
