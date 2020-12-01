# network_protocol_project

Bootup
1. [Computation]: Backend server A and B read the files data1.txt and data2.txt
respectively, and construct a list of “graphs” (see “DETAILED EXPLANATION” for
description of suggested data structures for graphs).
○ Assume a “static” social network where contents in data1.txt and data2.txt do not
change throughout the entire process.
○ Backend servers only need to read the text files once. When Backend servers are
handling user queries, they will refer to the internally represented graphs, not the
text files.
○ For simplicity, there is no overlap of countries between data1.txt and data2.txt.
2. [Communication]: after step 1, Main server will ask each of Backend servers which
countries they are responsible for.
3. [Computation]: Main server will construct a data structure to book-keep such information
from step 2. When the client queries come, the main server can send a request to the
correct Backend server.
Query
1. [Communication]: Clients send their queries to the Main server.
○ A client can terminate itself only after it receives a reply from the server (in the
Reply phase).
○ Main server may be connected to both clients at the same time.
2. [Computation]: Once the Main server receives the queries, it decodes the queries and
decides which backend server(s) should handle the queries.
Recommendation
1. [Communication] Main server sends a message to the corresponding backend server so
that the Backend server can perform local computation to generate recommendations.
2. [Computation]: Backend server performs some operations on the graph for friend
recommendations, which is based on the number of common neighbors. You need to
implement the algorithm on Backend servers to count the number of common neighbors.
3. [Communication]: Backend servers, after generating the recommendations, will reply to
Main server.
Reply
1. [Computation]: Main server decodes the messages from Backend servers and then
decides which recommendations correspond to which client queries.
2. [Communication]: Main server prepares a reply message and sends it to the appropriate
Client.
3. [Communication]: Clients receive the recommendation from Main server and display it.
Clients should keep active for further inputted queries, until the program is manually
killed.
