# EE450: Computer Networks - Network Protocol Project

## Description
In this project, you will implement a simple application to generate customized recommendations based on user queries. Specifically, consider that Facebook users in different countries want to follow new friends. They would send their queries to a Facebook server (i.e., main server) and receive the new friend suggestions as the reply from the same main server. Now since the Facebook social network is so large that it is impossible to store all the user information in a single machine. So we consider a distributed system design where the main server is further connected to many (in our case, two) backend servers. Each backend server
stores the Facebook social network for different countries. For example, backend server A may store the user data of Canada and the US, and backend server B may store the data of Europe. Therefore, once the main server receives a user query, it decodes the query and further sends a request to the corresponding backend server. The backend server will search through its local data, identify the new friend to be recommended and reply to the main server. Finally, the main server will reply to the user to conclude the process.




## Phases of Communication
### Phase 1: Bootup
1. [Computation]: Backend server A and B read the files data1.txt and data2.txt respectively, and construct a list of “graphs” (see “DETAILED EXPLANATION” for description of suggested data structures for graphs).
○ Assume a “static” social network where contents in data1.txt and data2.txt do not change throughout the entire process.
○ Backend servers only need to read the text files once. When Backend servers are handling user queries, they will refer to the internally represented graphs, not the text files.
○ For simplicity, there is no overlap of countries between data1.txt and data2.txt.
2. [Communication]: after step 1, Main server will ask each of Backend servers which countries they are responsible for.
3. [Computation]: Main server will construct a data structure to book-keep such information from step 2. When the client queries come, the main server can send a request to the correct Backend server.

### Phase 2: Query
1. [Communication]: Clients send their queries to the Main server.
○ A client can terminate itself only after it receives a reply from the server (in the Reply phase).
○ Main server may be connected to both clients at the same time.
2. [Computation]: Once the Main server receives the queries, it decodes the queries and decides which backend server(s) should handle the queries.

### Phase 3: Recommendation
1. [Communication] Main server sends a message to the corresponding backend server so that the Backend server can perform local computation to generate recommendations.
2. [Computation]: Backend server performs some operations on the graph for friend recommendations, which is based on the number of common neighbors. You need to implement the algorithm on Backend servers to count the number of common neighbors.
3. [Communication]: Backend servers, after generating the recommendations, will reply to Main server.

### Phase 4: Reply
1. [Computation]: Main server decodes the messages from Backend servers and then decides which recommendations correspond to which client queries.
2. [Communication]: Main server prepares a reply message and sends it to the appropriate Client.
3. [Communication]: Clients receive the recommendation from Main server and display it. Clients should keep active for further inputted queries, until the program is manually killed.

## Source Code Files
The implementation includes the source code files described below, for each component of the system.
1. servermain.cpp: the main server app
2. serverA.cpp: server A app
3. serverB.cpp: server B app
4. client.cpp: the client side app

## Notes for Users
1. Users have to start the processes in this order: Backend-server (A), Backend-server (B), Main-server, and Client 1, Client 2.
2. The data1.txt and data2.txt files are created before your program starts.
