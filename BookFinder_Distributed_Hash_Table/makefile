make: client super node

client: client.cpp
	g++ -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0 -Wall -I/usr/local/include/thrift client.cpp gen-cpp/PA2_constants.cpp gen-cpp/PA2_types.cpp -o client -lthrift

super: super.cpp
	g++ -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0 -Wall -I/usr/local/include/thrift super.cpp gen-cpp/PA2_constants.cpp gen-cpp/PA2_types.cpp -o super -lthrift -lpthread

node: node.cpp
	g++ -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0 -Wall -I/usr/local/include/thrift node.cpp gen-cpp/PA2_constants.cpp gen-cpp/PA2_types.cpp -o node -lthrift -lpthread

clean:
	rm client super node