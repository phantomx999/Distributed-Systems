//In command line: thrift -r --gen cpp PA3.thrift
namespace cpp PA3

service Server {
  bool ping(),
  string SendRequest(1:string filename, 2:string contents, 3:i64 OPCODE),
  i64 CheckVersion(1:string filename),
  i64 Write(1:string filename, 2:string contents, 3:i64 version),
  string Read(1:string filename),
  void AnnounceLife(1:string IP, 2:string Port),
}
