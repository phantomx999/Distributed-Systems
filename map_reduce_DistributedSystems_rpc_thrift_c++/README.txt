Map Reduce Project written by Andrew Steinbrueck and Jeffrey Dickinson
(to run project, please refer to user_document.txt file)


C++ Map Reduce Project using rpc via thrift to communicate between distributed systems


The purpose of this project is to have a client computer send a job to a server computer
to calculate the "sentiment" value for an inputted directory that contains text files.  

What is the sentiment value?
The inputted directory from the client computer has a list of text
files which have words that are compared with positive words 
(words from a text file called "positive.txt") and negative words 
(words from a text filed called "negative.txt").  If a word from a text file matches
positive words or negative words, a positive/negative word counter is incremented.
From these positive/negative word counters, a sentiment value is calculated using:

sentiment = (#positive - #negative)/(#positive + #negative)

In order to accomplish this, the server that receives the job from the client
sends out 1 task to different nodes (other computers) 
to calculate a "map" task where each task in a node calculates the sentiment value for 
each file.  Afterwards, an intermediate file that contains the original file's name as 
well as a sentiment value is sent back to the server.  After the server receives all 
intermediate files from all tasks, the server sends a final task to a node to compute a 
"reduce" task, which sorts all intermediate files based on sentiment values in ascending 
order.  The node writes all of this sorted data in a text file, 
which is then returned back to the server.  The server then returns this output text
file back to the original client.

Lastly, the user can choose to run random or load probability scheduling for the job, 
where random scheduling entails the server randomly sending out tasks to the nodes, while
load probability scheduling entails nodes accepting or rejecting tasks from server 
based on their load probability value. 




Our Design:
In order to accomplish this project, 3 files were created (client.cpp, server.cpp, node.cpp)
The server.cpp file is run first to set up a server, and the client.cpp and node.cpp files
are then run to set up client(s) communication with the server.

The user then sends out an input directory as a job, as well as a mode value (scheduling
mode) from the client to the server.  But first, the nodes poll
the server and see if the server is ready so that the nodes can get tasks from the
server.  The server responds to the nodes by indicating it is not ready until it has received a job 
from the client and allocated all map tasks to the nodes.  

After user input, the client then sends out an input directory as a job, as well as a mode value (scheduling
mode) from the client to the server.  The server traverses the input directory and counts
the number of text files in order to get a number of tasks to send to nodes (for 500 text
files, then the number of tasks would be 501 because it would be 500 map tasks and 1 reduce
tasks).  Each task has a value and the name of one of the text files being analyzed.  The server then stores these
"map" tasks one at a time to a list of tasks for each node (1 list per node).  

Based on user input, the server sends out the tasks...
1) randomly, where each node is assigned a list of tasks at random.
2) using load probability, where each computer node has a load probability value n
   and the node accepts or rejects server sending out list of tasks to it based on this probability n value.

For random:
Each node has a value, and the server randomly chooses one of these node values at a time and sends
a task out to this particular node.  The node then adds this task to the node's list to compute.

For load prob:
The node randomly rejects the task by factoring the load probabiliy.

Each node before executing counts the number of positive and negative words in the positive.txt and
negative.txt text files.  

After the server sends out all map tasks (equal to number of files in input directory), the node
then spawn threads to execute each of these tasks in the node's specific list of tasks.       

These map tasks read the specific files assigned to each task and compares the words in the file with
the stored positive and negative words stored in the node.  A positive and negative count are stored and
then a sentiment value is calculated.  Each task then writes the corresponding filename, as well as the sentiment
value in an intermediate file.  After all tasks are finished in a node, this intermediate file will have
all of task's in the lists results in...

<filename1> <sentiment value1>
<filename2> <sentiment value2>
.....

format.  Once all tasks are run in node list, node returns the intermediate file back to the server.  Once the server receives
all intermediate files (1 per node), it then sends out a reduce task to node 0, which then runs the reduce sort function to sort
intermediate files by ascending order all wrote out to 1 final output final.  This output file is returned to the server, which then 
returns this outputted file to the client.  

Design Layout:
JobHandler class is the only class that uses thrift, and the only class we use across client/server/node.


 
