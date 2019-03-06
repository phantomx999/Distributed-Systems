namespace cpp project1 

struct Job_struct {
  1: i64 var1,
  2: list<string> var2,
  3: i64 var3,
  4: list<string> var4,
  5: string var5,
  6: i64 var6
  //7: Node_struct var7,
  //8: Node_struct var8,
  //9: Node_struct var9,
  //10: Node_struct var10
}

struct Node_struct {
  1: list<string> fileNames,
  2: list<double> sentiment,
  3: i64 uniqueID,  
  4: i64 numberOfFiles,
  5: double var5,
  6: string intermediary,
  7: i64 var7,
  8: bool isTaken,
  9: bool isLastTask
}

struct Task_struct {
  1: i64 var1,
  2: i64 var2,
  3: list<string> var3,
  4: double var4,
  5: string var5,
  6: list<string> var6
} 

service Job {
  bool ping();
  bool GetStatus(),
  string GetTasks(1:Node_struct n),
  void CountFiles(1:string var1),
  string PerformJob(1:string input, 2:i64 mode)
}

service Node {
  void StorePositiveWords(),
  void StoreNegativeWords(),
  string RandomMapTaskNode(1:string begin_files, 2:Node_struct n),
  string RandomReduceTaskNode(1:list<string> int_files, 2:Node_struct n)
}


service Task {
  double CalculateSentiment(),
  void CreateFileName(), 
  void Map(1:string filename, 2:list<string> positives, 3:list<string> negatives),
  list<string> Reduce(1:list<string> filenames),
  string MakeIntermediateFile(1:string input_file_name, 2:list<string> positive, 3:list<string> negative),
  string MakeOutputResult (1:list<string> lists)
}




