#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include <iostream>

#include "common.h"

#define MAX_MESSAGE_LENGTH 100

void runLoop(int clientID, int sockfd)
{
    while (true) {
        printf("Enter country name: ");
        std::string countryName;
        std::cin >> countryName;
        wrappedWrite(sockfd, countryName.data(), countryName.size());

        printf("Enter user id: ");

        std::string userId;
        std::cin >> userId;
        wrappedWrite(sockfd, userId.data(), userId.size());
        std::cout << "Client" << clientID << " has sent User" << userId << " and " << countryName << " to Main Server using TCP" << std::endl;
        
        int result = readToInt(sockfd);
        if (result == USER_ALL_CONNECTED) {
            std::cout << userId << " is already connected to all other users, no new recommendation" << std::endl;
        }
        else if (result == USER_NOT_FOUND) {
            std::cout << "User" << userId << " not found" << std::endl;
        }
        else if (result == COUNTRY_NOT_FOUND) {
            std::cout << countryName << " not found" << std::endl;
        }
        else {
            std::cout << "Client" << clientID << " has received results from Main Server:" << std::endl;
            std::cout << "User" << result << " is/are possible friend(s) of User" << userId << " in " << countryName << std::endl;
        }
    }
}

int main()
{
    // socket create
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }

    // assign IP, PORT
    sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(MAIN_SERVER_TCP_PORT);

    // connect the client socket to server socket
    if (connect(sockfd, (sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    
    int clientID = readToInt(sockfd);
    std::cout << "Client" << clientID << " is up and running" << std::endl;

    // function for chat
    runLoop(clientID, sockfd);

    // close the socket
    close(sockfd);
}

/*
<Country Name> not found
User<user ID> not found

Client1 has received results from Main Server:
User<user ID1>, User<user ID2> is/are possible friend(s)
of User<user ID> in <Country Name>
*/
