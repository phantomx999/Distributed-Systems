make: client server node

client: client.cpp
	g++ -std=c++11 -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H -Wall -I/usr/local/include/thrift client.cpp -L/usr/local/lib/ -lthrift -o client

server: server.cpp
	g++ -std=c++11 -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H -Wall -I/usr/local/include/thrift server.cpp -L/usr/local/lib/ -lthrift -o server

node: node.cpp
	g++ -std=c++11 -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H -Wall -I/usr/local/include/thrift node.cpp -L/usr/local/lib/ -lthrift -o node

clean:
	rm client server node
