# Distributed-Systems in C++

# 3 projects:
1.   Map/Reduce with rpc distributed systems via different computers (cluster).  Client computer sends job (directory) to server computer.
     Server computer allocates tasks to other computers to use map function to count number of specific words in text files that within 
     the job directory that client sent to server..  
     Server receives word count results from these task computers and allocates a reduce task to one computer to add up individual
     text file results and calculate the final sentiment value (see README in map_reduce... project for more details)
     
2.   Distributed Hash Table to store and query a database of books (title and genre) using distributed system cluster via thrift RPC.

3.   Distributed File System that handles replicas and synchronization of files in cluster network via RPC thrift.

All projects used thrift and C++


