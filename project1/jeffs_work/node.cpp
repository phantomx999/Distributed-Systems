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
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <dirent.h>
#include <stdio.h>
#include <pthread.h>

#include "gen-cpp/Job.h"
#include "gen-cpp/Job.cpp"

//static int intermediate_fle_count = 0;

using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace  ::project1;
using boost::shared_ptr;
using boost::make_shared;

Node_struct n;

/*
class NodeHandler : virtual public NodeIf {
  public:
   NodeHandler(int num, double load, std::string host, int port) {
     node_num = num;
     node_load = load;
     node_host = host;
     node_port = port;
     num_tasks = 0;
   }

   void StorePositiveWords() {

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

     std::string temp[1];
     while(!pos_file.eof()) {
       getline(pos_file, temp);
       positive_words.push_back(temp);
       temp = "";
     }
     pos_file.close();

     return;
   }

   void StoreNegativeWords() {

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

     std::string temp[1];
     while(!neg_file.eof()) {
       getline(neg_file, temp);
       negative_words.push_back(temp);
       temp = "";
     }
     neg_file.close();
     return;
   }

   void RandomMapTaskNode(std::string& _return, const std::string& begin_files, const Node_struct& n) {
     if(n.num_tasks == 0) {
       n.StorePositiveWords();
       n.StoreNegativeWords();
     }
     n.num_tasks++;
     TaskHandler ta = new TaskHandler();
     std::string intermediate = ta.MakeIntermediateFile(begin_files, n.positive_words, n.negative_words);
     _return = intermediate;
   }

   void RandomReduceTaskNode(std::string& _return, const std::vector<string> &int_files, const Node_struct &n) {
     if(n.num_tasks == 0) {
       n.StorePositiveWords();
       n.StoreNegativeWords();
     }
     n.num_tasks++;
     TaskHandler ta = new TaskHandler();
     std::string output = ta.MakeOutputResult(int_files);
     _return = output;
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
*/
/*
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
};*/
/************ THREADS ************/
#define MAX_THREADS 300
struct argsForThreads{
    //Put stuff here to pass to the threads
    std::string text_file;
    int index_in_vect;
};

//Function for threads
void* executeTask(void* args){
  struct argsForThreads* thread_input;
  thread_input = (struct argsForThreads *) args;
  printf("File: %s  and index: %d\n", thread_input->text_file.c_str(), thread_input->index_in_vect);

  std::vector <std::string> words;
  std::string line;
  float num_positive_words = 0;
  float num_negative_words = 0;
  float sentiment = 0;
  //Open folder and put words into a vector;
  std::ifstream file;
  file.open(thread_input->text_file.c_str());
  if(!file) {
    std::cout<<"Error opening file to STORE words"<< std::endl;
    system("pause");
    return;
  }

  while (!file.eof()) {
    getline (file, line);
    words.push_back(line);
  }
  printf("Positive words are loaded.\n");
  file.close();
  //Count positive and negative
  bool flag_to_reduce_time = false;
  for(int i = 0; i < words.size(); i++){
    for(int j = 0; j < positive_words.size(); j++){
      if(!positive_words.at(j).compare(words.at(i))){
        //Match
        num_positive_words++;
        flag_to_reduce_time = true;
        break;
      }
    }
    if(!flag_to_reduce_time){//if positive wasn't found, check negative
      for(int k = 0; k < negative_words.size(); k++){
        if(!negative_words.at(j).compare(words.at(i))){
          //Match
          num_negative_words++;
          break;
        }
      }
    }
    flag_to_reduce_time = false;
  }
  //CalculateSentiment
  sentiment = (num_positive_word - num_negative_words) / (num_positive_word + num_negative_words);
  //Store in node

  pthread_exit(NULL);
};
/********* END THREADS ***********/

std::vector <std::string> positive_words;
std::vector <std::string> negative_words;

void StorePositiveWords(std::string pos_words_file) {

 int count_words = 0;

 std::string line;
 printf("Opening %s for positive words.\n", pos_words_file.c_str());
 std::cout <<"Opening "<< pos_words_file << " for pos words" << std::endl;
 std::ifstream count_file;
 count_file.open(pos_words_file);
 if(!count_file) {
   std::cout<<"Error opening file to COUNT words"<< std::endl;
   system("pause");
   return;
 }
 while (getline(count_file, line)) {
    count_words++;
 }

 count_file.close();
 printf("%s has %d words\n", pos_words_file.c_str(), count_words);

 std::ifstream pos_file;
 pos_file.open(pos_words_file);
 if(!pos_file) {
   std::cout<<"Error opening file to STORE words"<< std::endl;
   system("pause");
   return;
 }

 while (!pos_file.eof()) {
   getline (pos_file, line);
   positive_words.push_back(line);
 }
 printf("Positive words are loaded.\n");
 pos_file.close();

 return;
}

void StoreNegativeWords(std::string neg_words_file) {

  int count_words = 0;

  std::string line;
  printf("Opening %s for negative words.\n", neg_words_file.c_str());
  std::ifstream count_file;
  count_file.open(neg_words_file);
  if(!count_file) {
    std::cout<<"Error opening file to COUNT words"<< std::endl;
    system("pause");
    return;
  }
  while (getline(count_file, line)) {
     count_words++;
  }

  count_file.close();
  printf("%s has %d words\n", neg_words_file.c_str(), count_words);

  std::ifstream neg_file;
  neg_file.open(neg_words_file);
  if(!neg_file) {
    std::cout<<"Error opening file to STORE words"<< std::endl;
    system("pause");
    return;
  }

  while (!neg_file.eof()) {
    getline (neg_file, line);
    negative_words.push_back(line);
  }
  printf("Negative words are loaded.\n");
  neg_file.close();

  return;
}


int main(int argc, char **argv) {
  printf("%d\n",argc);
  if(argc != 2 && argc != 3) {
        printf("Invalid number of arguments\n");
        printf("Run ./node <serverIP> <port>\n");
        return 1;
  }
  //int64_t port = 0;
  if(argc == 3) {
    //port = std::stoi(argv[3]);
  }
  printf("Getting ready for connection\n");
  //char* serverIP = argv[1];
  shared_ptr<TTransport> socket(new TSocket("localhost", 9001));
  shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

  JobClient client(protocol);

  //Load Pos and Neg words into memory
  StorePositiveWords("data/positive.txt");
  StoreNegativeWords("data/negative.txt");
  /*
  for(int i = 0; i < positive_words.size() && i < negative_words.size(); i++){
    printf("Pos: %s\n",positive_words.at(i).c_str());
    printf("Neg: %s\n",negative_words.at(i).c_str());
    i+= 50;
  }*/
  pthread_t tid[MAX_THREADS];
  int tidIndex = 0;
  struct argsForThreads temp[MAX_THREADS];

  std::string _return;

  try {
    transport->open();
    client.ping();
    while(!client.GetStatus());
    if(client.GetStatus()){
      //Get Tasks
      client.GetTasks(n);

      //For each file create a thread
      for(int i = 0; i < n.numberOfFiles; i++){
        temp[i].text_file = n.fileNames.at(i);
        temp[i].index_in_vect = i;
        if(tidIndex == (MAX_THREADS - 1)){
          for(int i =0; i < MAX_THREADS; i++){
            pthread_join(tid[i],NULL);
          }
          tidIndex = 0;
          pthread_create(&tid[tidIndex], NULL, executeTask, (void*) &temp[i]);
          tidIndex++;
        }
        else{
          pthread_create(&tid[tidIndex], NULL, executeTask, (void*) &temp[i]);
          tidIndex++;
        }
      }
      for(int i = 0; i < MAX_THREADS; i++){
        pthread_join(tid[i], NULL);
      }
      //Spin a thread for each task, CalculateSentiment
    }
    printf("Received: Node[%d] It has %d files\n", (int) n.uniqueID, (int) n.numberOfFiles);
    printf("Making Contact. . . \n");
    printf("Requesting Job. . .\n");
    printf("Jobs Done!\n");
    transport->close();
  }
  catch (TException& e) {
    std::cout << "ERROR: " << e.what() << std::endl;
  }

  //Vector Cleanup
  positive_words.clear();
  negative_words.clear();
  return 0;


}
