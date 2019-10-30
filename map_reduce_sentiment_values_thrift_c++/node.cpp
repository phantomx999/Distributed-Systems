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
#include <algorithm>
#include "gen-cpp/Job.h"
#include "gen-cpp/Job.cpp"
#include <set>
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

std::vector <std::string> positive_words;
std::vector <std::string> negative_words;

std::set<std::string> positive_words_set;
std::set<std::string> negative_words_set;

// trim from start TAKEN FROM /*https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring*/
void WriteIntermediaryFile();
void SortFiles(std::string& directory);

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

/************ THREADS ************/
#define MAX_THREADS 300
struct argsForThreads{
    //Put stuff here to pass to the threads
    std::string text_file;
    int index_in_vect;
    int thread_id;
};

std::string StringPurify(std::string str_input){
  bool was_change_flag = false;
  std::string input = str_input;
  bool keep_running = true;
  while(keep_running){
    was_change_flag = false;
    std::transform(input.begin(), input.end(), input.begin(), ::tolower);
    if(input[input.size() - 1] == '.' || input[input.size() - 1] == ',' || input[input.size() - 1] == '!'
    || input[input.size() - 1] == ':' || input[input.size() - 1] == ';' || input[input.size() - 1] == ']'
    || input[input.size() - 1] == '}' || input[input.size() - 1] == '?' ){
      //printf("before 1: %s\n", space_token.c_str());
      input.pop_back();
      was_change_flag = true;
      //printf("after: %s\n", space_token.c_str());
    }
    if(input[0] == '.' || input[0] == ',' || input[0] == '!' || input[input.size() - 1] == '[' || input[input.size() - 1] == '{' ){
      //printf("before 2: %s\n", space_token.c_str());
      input.erase(input.begin());
      was_change_flag = true;
      //printf("after: %s\n", space_token.c_str());
    }
    for(int i = 0; i < input.size(); i++){
      if((int) input[i] == 39){
        //printf("before 3: %s\n", space_token.c_str());
        input.erase(i,1);
        was_change_flag = true;
        //printf("after: %s\n", space_token.c_str());
        //printf("Apost\n");
      }
    }
    keep_running = was_change_flag;
  }
  return input;
}

//Function for threads
void* executeTask(void* args){
  struct argsForThreads* thread_input;
  thread_input = (struct argsForThreads *) args;
  //printf("File: %s  and index: %d\n", thread_input->text_file.c_str(), thread_input->index_in_vect);
  srand (time(NULL));
  float random_load = (float) ((float) rand() / (RAND_MAX));
  //printf("random_load: %f\n", random_load);
  if(n.load >= random_load) {
    printf("Injected Delay...\n");
    sleep(3);
  }

  std::string cwd = n.cwd + thread_input->text_file;
  std::string cwd_test = "data/example/test.txt";
  std::vector <std::string> words;
  std::set<std::string> words_set;
  std::string line;
  float num_positive_words = 0;
  float num_negative_words = 0;
  float sentiment = 0;
  //Open folder and put words into a vector;
  std::ifstream file;
  file.open(cwd);//thread_input->text_file.c_str()
  if(!file) {
    std::cout<<"Error opening file to STORE words"<< std::endl;
    system("pause");
    pthread_exit(NULL);
  }


  std::string space_token;

  while (!file.eof()) {
    getline (file, line);

    std::stringstream ss(line);
    std::string word1_of_split;
    std::string word2_of_split;
    //printf("line: %s\n", line.c_str());
    bool splitflag = false;
    //clean the input
    while(std::getline(ss, space_token, ' ')){
      //scrub periods, commas, exclamation points...
      //if(space_token.size() < 2) continue;
      bool was_change_flag = false;
      bool keep_running = true;
      while(keep_running){
        was_change_flag = false;
        std::transform(space_token.begin(), space_token.end(), space_token.begin(), ::tolower);
        if(space_token[space_token.size() - 1] == '.' || space_token[space_token.size() - 1] == ',' || space_token[space_token.size() - 1] == '!'
        || space_token[space_token.size() - 1] == ':' || space_token[space_token.size() - 1] == ';' || space_token[space_token.size() - 1] == ']'
        || space_token[space_token.size() - 1] == '}' || space_token[space_token.size() - 1] == '?' ){
          //printf("before 1: %s\n", space_token.c_str());
          space_token.pop_back();
          was_change_flag = true;
          //printf("after: %s\n", space_token.c_str());
        }
        if(space_token[0] == '.' || space_token[0] == ',' || space_token[0] == '!' || space_token[space_token.size() - 1] == '[' || space_token[space_token.size() - 1] == '{' ){
          //printf("before 2: %s\n", space_token.c_str());
          space_token.erase(space_token.begin());
          was_change_flag = true;
          //printf("after: %s\n", space_token.c_str());
        }
        for(int i = 0; i < space_token.size(); i++){
          if((int) space_token[i] == 39){
            //printf("before 3: %s\n", space_token.c_str());
            space_token.erase(i,1);
            was_change_flag = true;
            //printf("after: %s\n", space_token.c_str());
            //printf("Apost\n");
          }
        }
        keep_running = was_change_flag;
      }
        //printf("what is going on: %s\n", space_token.c_str());
        //words.push_back(space_token.c_str());
      splitflag = false;
      for(int i = 0; i < space_token.size(); i++){
        if(i+1 < space_token.size()){
          if(space_token[i]=='-' && space_token[i+1] == '-'){
            //printf("Have to split: %s\n", space_token.c_str());
            space_token.erase(i,2);
            word1_of_split = space_token.substr(0,i);
            word2_of_split = space_token.substr(i, space_token.size());
            word1_of_split = StringPurify(word1_of_split);
            word2_of_split = StringPurify(word2_of_split);
            //printf("1: %s \t\t 2: %s\n", word1_of_split.c_str(), word2_of_split.c_str());
          }
        }
      }
      bool non_space_flag = false;
      std::string string_for_push;
      string_for_push = trim(space_token);
      //printf("trimmed: %s\n", string_for_push.c_str());
      for(int i = 0; i < string_for_push.size(); i++){
        if(string_for_push[i] != ' '){
          non_space_flag = true;
        }
      }
      if(non_space_flag && !splitflag){
        words.push_back(string_for_push);
      }
      non_space_flag = false;
      if(splitflag){
        words.push_back(word1_of_split);
        for(int i = 0; i < string_for_push.size(); i++){
          if(string_for_push[i] != ' '){
            non_space_flag = true;
          }
        }
        if(non_space_flag){
          words.push_back(word2_of_split);
        }
      }
      splitflag = false;
    }
  }
  file.close();

  //Count positive and negative
  //using sets reduced this to logarithmic time instead of linear
  for(int i = 0; i < words.size(); i++){
    if(positive_words_set.find(words.at(i)) != positive_words_set.end()){
      //positive word match
      //printf("pos match: %s \t and \t \n", words.at(i).c_str());
      num_positive_words++;
    }
    else if(negative_words_set.find(words.at(i)) != negative_words_set.end()){
      //positive word match
      num_negative_words++;
      //printf("neg match: %s \t and \t \n", words.at(i).c_str());
    }
  }
  //printf("Seg after here? 2 thread: %d\n", thread_input->thread_id);
  //CalculateSentiment
  sentiment = (num_positive_words - num_negative_words) / (num_positive_words + num_negative_words);
  //Store in node
  n.neg_words[thread_input->index_in_vect] = num_negative_words;
  n.pos_words[thread_input->index_in_vect] = num_positive_words;
  n.sentiment[thread_input->index_in_vect] = sentiment;

  words.clear();
  words_set.clear();
  //printf("Thread %d is done.\n", thread_input->thread_id);
  pthread_exit(NULL);
};
/********* END THREADS ***********/

void StorePositiveWords(std::string pos_words_file) {

 int count_words = 0;

 std::string line;
 printf("Opening %s for positive words. . .\n", pos_words_file.c_str());
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
   line = trim(line);
   positive_words.push_back(line);
   positive_words_set.insert(line);
 }
 printf("Positive words are loaded.\n\n");
 pos_file.close();

 return;
}

void StoreNegativeWords(std::string neg_words_file) {

  int count_words = 0;

  std::string line;
  printf("Opening %s for negative words. . .\n", neg_words_file.c_str());
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
    line = trim(line);
    negative_words.push_back(line);
    negative_words_set.insert(line);
  }
  printf("Negative words are loaded.\n\n");
  neg_file.close();

  return;
}


int main(int argc, char **argv) {
  if(argc != 3) {
        printf("Invalid number of arguments\n");
        printf("Run ./node <serverIP> <port>\n");
        return 1;
  }
  char* serverIP = argv[1];
  int port = std::stoi(argv[2]);
  //float load_prob = std::stof(argv[3]);
  //printf("Load prob: %f\n", load_prob);
  //port = std::stoi(argv[3]);
  printf("Getting ready for connection\n");
  //char* serverIP = argv[1];
  shared_ptr<TTransport> socket(new TSocket(serverIP, port));
  shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

  JobClient client(protocol);

  //Load Pos and Neg words into memory
  StorePositiveWords("data/positive.txt");
  StoreNegativeWords("data/negative.txt");

  pthread_t tid[MAX_THREADS];
  int tidIndex = 0;
  struct argsForThreads temp[MAX_THREADS];

  std::string _return;
  //n.load = load_prob;
  try {
    transport->open();
    client.ping();
    //client.SendLoad(n, load_prob);
    while(!client.GetStatus());
    if(client.GetStatus()){
      //Get Tasks
      printf("Requesting Job. . .\n");
      client.GetTasks(n);
      printf("Received: Node[%d] It has %d task(s)\n", (int) n.uniqueID, (int) n.numberOfFiles);
      //For each file create a thread
      for(int i = 0; i < n.numberOfFiles; i++){ //n.numberOfFiles
        temp[i].text_file = n.fileNames.at(i);
        printf("Task: %s \n", temp[i].text_file.c_str());
        temp[i].index_in_vect = i;
        temp[i].thread_id = tidIndex;
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
      for(int i = 0; i < n.numberOfFiles; i++){
        pthread_join(tid[i], NULL);
      }

    }


    if(n.numberOfFiles > 0){
      for(int i = 0; i < n.numberOfFiles; i++){
          printf("Report: name: %s \t\tNegWords: %f \t\tPosWords: %f\t\tSentinment: %f\n", n.fileNames[i].c_str(), (double) n.neg_words[i], (double) n.pos_words[i],  n.sentiment[i]);
      }
    }
    WriteIntermediaryFile();
    client.StatusUpdate(n);
    if(n.uniqueID == 0){
      while(!client.ReadyForSort());
      std::string directory = "inter/";
      SortFiles(directory);
    }
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

void WriteIntermediaryFile() {
  std::ofstream intermediaryFile;
  std::string interFileName = "inter/intermediaryFile" + std::to_string(n.uniqueID) + ".txt";
  intermediaryFile.open(interFileName);
  for(int i = 0; i < n.numberOfFiles; i++) {
    intermediaryFile << n.fileNames[i];
    intermediaryFile << " ";
    intermediaryFile << n.sentiment[i];
    intermediaryFile << "\n";
  }
  intermediaryFile.close();
  return;
}

static bool sort_greater_than(double u, double v)
{
   return u > v;
}

void SortFiles(std::string& directory) {
  int total_number_lines = 0;
  std::vector<std::string> unsorted_files;
  std::vector<std::string> all_lines;
  std::vector<std::string> sorted;
  struct dirent* direct;
  DIR *pdir = opendir(directory.c_str());
  if(!pdir) {
    printf("Failed to open intermediate files directory ()\n");
    exit(-1);
  }
  while((direct = readdir(pdir)) != NULL) {
    if (!strcmp(direct->d_name, ".") || !strcmp(direct->d_name, "..")) continue;
    unsorted_files.push_back(direct->d_name);
  }
  closedir(pdir);
  //std::vector<std::string> files_content;
  for(int i = 0; i < unsorted_files.size(); i++) {
    std::ifstream current_file;
    current_file.open(unsorted_files.at(i));
    if(!current_file) {
        std::cout<<"Error opening intermediate file for sort task"<< std::endl;
        system("pause");
        return;
    }
    std::string line;
    while (!current_file.eof()) {
      getline(current_file, line);
      all_lines.push_back(line);
      total_number_lines++;/*
      bool space_flag = true;
      for(int i = 0; i < line.size(); i++){
        if(line[i] != ' '){
          false;
        }
      }
      if(!space_flag){

      }*/
    }
    current_file.close();
  }
  for(int i = 0; i < all_lines.size(); i++){
    printf("%s \n", all_lines.at(i).c_str());
  }
  std::ofstream intermediaryFile;
  std::string interFileName = "final_output.txt";
  intermediaryFile.open(interFileName);
  for(int i = 0; i < all_lines.size(); i++) {
    intermediaryFile << all_lines.at(i);
    intermediaryFile << "\n";
  }
  intermediaryFile.close();
  /*
  printf("here\n");
  std::string numbers[total_number_lines];
  std::string all_words[total_number_lines][2];
  for(int k = 0; k < unsorted_files.size(); k++) {
    std::ifstream current_file2;
    current_file2.open(unsorted_files.at(k));
    if(!current_file2) {
        std::cout<<"Error opening intermediate file second time for sort task"<< std::endl;
        system("pause");
        return;
    }
    int i = 0;
    int flag = 0;
    std::string one_word;
    while(current_file2 >> one_word) {
      if(flag == 1) {
        numbers[i] = one_word;
        all_words[i][1] = one_word;
        i++;
        flag = 0;
      }
      else {
        all_words[i][0] = one_word;
        flag = 1;
      }
    }
    current_file2.close();
  }
  printf("there\n");
  double temp = 0.0;
  std::vector<double> sent_vals;
  for(int j = 0; j < total_number_lines; j++) {//numbers.lenght()
      temp = std::stof(numbers[j]);
      sent_vals.push_back(temp);
  }
  printf("hefdsre\n");
  sort(sent_vals.begin(), sent_vals.end(), sort_greater_than);
  printf("herefdsagagnd\n");
  std::string make_line;
  for(int i = 0; i < sent_vals.size(); i++) {
      for(int j = 0; j < total_number_lines; j++) {
        if(sent_vals[i] == std::stof(all_words[j][1])) {
          make_line = all_words[j][0];
          make_line = " ";
          make_line += all_words[j][1];
          sorted.push_back(make_line);
        }
      }
   }*/
}
