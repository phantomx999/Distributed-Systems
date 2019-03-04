namespace cpp project1

struct Job_struct {
  1: i64 var1,
  2: list<string> var2,
  3: i64 var3,
  4: list<Node_struct> var4
}

struct Node_struct {
  1: string var1,
  2: string var2,
  3: list<Task_struct> var3
//6: map<std::string* s, float val> var5;
}

struct Task_struct {
  1: i64 var1,
  2: i64 var2,
  3: string val3,
  4: double var4,
  5: string var5,
  6: string var6
}

service Job {
  void CountFiles(1:string var1),
  string PerformJob(1:string input)
}

service Node {
  void StorePositiveWords(),
  void StoreNegativeWords()
  //map<std::string* s, float val> CreateIntermediateFile()
}

service Task {
  void CreateFileName(),
  double CalculateSentiment(),
  void Map(1:string filename, 2:string positives, 3:string negatives),
  string Reduce(1:string filenames),
  string MakeIntermediateFile(1:string input_file_name),
  string MakeOutputResult (1:string lists)
}
