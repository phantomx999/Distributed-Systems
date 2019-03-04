namespace cpp project1 {

struct Job_struct (
  1: i64 var1;
  2: vector<string> var2;
  3: i64: var3;
  4: list<string> var4;
  5: string var5;
  6: i64 var6;
  7: Node var7;
  8: Node var8;
  9: Node var9;
  10: Node var9;
) 

typedef struct Node_struct (
  1: list<string> var1;
  2: list<string> var2;
  3: list<task> var3;  
  4 i64 var4;
  5: double var5;
//6: map<std::string* s, float val> var5;
) Node

typedef struct Task_struct (
  1: i64 var1;
  2: i64 var2;
  3: std::string* val3;
  4: double var4;
  5: string var5;
  6: std::string var6;
) Task;

service Job {
  void CountFiles(1:std::string var1);
  std::string PerformJob(1:std::string input, 2:i64 mode) {
}

service Node {
  void StorePositiveWords();
  void StoreNegativeWords();
  string RandomMapTaskNode(1:string begin_files, 2: Node& file) {
  string RandomReduceTaskNode(1:list<string> int_files, 2: Node last) {
  //map<std::string* s, float val> CreateIntermediateFile()  {
}

service Task {
  void CreateFileName(); 
  double CalculateSentiment();
  void Map(1:std::string filename, 2:std::string* positives, 3:std::string* negatives);
  std::string* Reduce(1:std::string* filenames);
  std::string MakeIntermediateFile(1:std::string input_file_name, 2:std::string* positive, 3:std::string* negative);
  std::string MakeOutputResult (1:std::string* lists);
}

}


