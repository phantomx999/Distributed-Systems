namespace cpp project1 {

struct Job_struct (
  1: i64 var1;
) 

struct Node_struct (
  1: std::string* var1;
  2: std::string* var2;
  3: vector<task> var3;  
//6: map<std::string* s, float val> var5;
)

typedef struct Task_struct (
  1: i64 var1;
  2: i64 var2;
  3: std::string* val3;
  4: float var4;
  5: std::string var5;
  6: std::string var6;
) task;

service Job {
  void CountFiles(1:std::string var1);
  std::string PerformJob(1:std::string input) {
}

service Node {
  void StorePositiveWords();
  void StoreNegativeWords();
  //map<std::string* s, float val> CreateIntermediateFile()  {
}

service Task {

  void CreateFileName(); 
  float CalculateSentiment();
  void Map(1:std::string filename, 2:std::string* positives, 3:std::string* negatives);
  std::string* Reduce(1:std::string* filenames);
  std::string MakeIntermediateFile(1:std::string input_file_name);
  std::string MakeOutputResult (1:std::string* lists);
}

}


