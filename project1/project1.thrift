namespace cpp project1 {

struct Job_struct (
  1: i64 var1;
)

struct Node_struct (
  1: Task var0;
  2: float var1;
  3: std::string* var2;
  4: std::string* var3;
  5: std::string* var4;
  6: std::string var5;
  //6: map<std::string* s, float val> var5;
)

struct Task_struct (
  1: i64 var1;
  2: i64 var2;
  3: std::string* file_list;
)

service Job {
  void CountFiles(std::string* var1);
  std::string PerformJob(
}

service Node {
  void StorePositiveWords();
  void StoreNegativeWords();
  float CalculateSentiment(1:Task_struct);
  void CreateFileName(); 
  std::string* MakeIntermediateFile(1:std::string input_file_name);
  std::string* MakeOutputResult (std::string* lists);
  //map<std::string* s, float val> CreateIntermediateFile()  {
}

service Task {
  void Map(1:std::string filename, 2:std::string* positives, 3:std::string* negatives);
  std::string* Reduce(1:std::string* filenames);
}

}