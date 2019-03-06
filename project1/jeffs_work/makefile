make: client server node

client: client.cpp
	g++ -std=c++11 -Wall -I/usr/local/include/thrift client.cpp gen-cpp/project1_constants.cpp gen-cpp/project1_types.cpp -o client -lthrift

server: server.cpp
	g++ -std=c++11 -Wall -I/usr/local/include/thrift server.cpp gen-cpp/project1_constants.cpp gen-cpp/project1_types.cpp -o server -lthrift

node: node.cpp
	g++ -std=c++11 -Wall -I/usr/local/include/thrift node.cpp gen-cpp/project1_constants.cpp gen-cpp/project1_types.cpp -o node -lthrift

clean:
	rm client server node
