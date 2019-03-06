make: client server node

client: client.cpp
	g++ -std=c++11 -Wall -I/usr/local/include/thrift client.cpp project1_constants.cpp project1_types.cpp -o client -lthrift

server: server.cpp
	g++ -std=c++11 -Wall -I/usr/local/include/thrift server.cpp project1_constants.cpp project1_types.cpp -o server -lthrift

node: node.cpp
	g++ -std=c++11 -Wall -I/usr/local/include/thrift node.cpp project1_constants.cpp project1_types.cpp -o node -lthrift

test: Job_server.skeleton.cpp
	g++ -std=c++11 -Wall -I/usr/local/include/thrift Job_server.skeleton.cpp project1_constants.cpp project1_types.cpp -o server_test -lthrift
clean:
	rm client server node server_test
