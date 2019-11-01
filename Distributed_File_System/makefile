make: server client

client: client.cpp
	g++ -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0 -Wall -I/usr/local/include/thrift client.cpp gen-cpp/PA3_constants.cpp gen-cpp/PA3_types.cpp -o client -lthrift -lpthread

server: server.cpp
	g++ -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0 -Wall -I/usr/local/include/thrift server.cpp gen-cpp/PA3_constants.cpp gen-cpp/PA3_types.cpp -o server -lthrift -lpthread

clean:
	rm client server
