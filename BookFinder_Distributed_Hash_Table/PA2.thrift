//In command line: thrift -r --gen cpp PA2.thrift
namespace cpp PA2

struct NodeStruct{
  1:i64 ID,
  2:i64 topRangeKeys,
  3:i64 botRangeKeys,
  4:i64 predecessorID,
  5:i64 successorID,
  6:i64 numberOfBooks,
  7:list<string> books,
  8:list<i64> fingerTable, //This is definitely wrong but it is a place holder
  9:string ip,
  10:string port
}

service SuperNodeService {
  bool ping(),
  string GetNode(),
  string Join(1:string IP, 2:string Port),
  oneway void PostJoin(1:string IP, 2:string Port)
}

service NodeService {
  bool ping(),
  string Contact(1:string IP, 2:string Port, 3:i64 ID, 4:i64 request),
  string ContactPred(1:string IP, 2:string Port, 3:i64 ID, 4:i64 request),
  string ContactSucc(1:string IP, 2:string Port, 3:i64 ID, 4:i64 request),
  string Set(1:string titles, 2:string genres),
  string Get(1:string titles)
}
