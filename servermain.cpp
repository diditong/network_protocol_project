#include <iostream>
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

#include <exception>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include "common.h"

int bindTCP() {
    // socket create and verification
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cout << "socket creation failed..." << std::endl;
        exit(0);
    }

    // assign IP, PORT
    sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(MAIN_SERVER_TCP_PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }

    return sockfd;
}

std::string intToServerString(int i) {
    if (i == 0) {
        return "A";
    } else {
        return "B";
    }
}

int getPortForServer(int i) {
    if (i == 0) {
        return SERVER_A_PORT;
    } else {
        return SERVER_B_PORT;
    }
}

void handleRequest(int tcpSocket, int udpSocket, int sendfd, const std::unordered_map<std::string, int>& country_backend_mapping, const std::string& countryName, const std::string& userId) {
    sockaddr_in tcpSockaddr{};
    socklen_t len = sizeof(tcpSockaddr);
    if (getsockname(tcpSocket, (sockaddr*)&tcpSockaddr, &len) == -1) {
        throw std::runtime_error("getsockname failed");
    }

    auto countryFound = country_backend_mapping.find(countryName);
    if (countryFound == country_backend_mapping.cend()) {
        std::cout << countryName << " does not show up in server A&B" << std::endl;
        wrappedWrite(tcpSocket, &COUNTRY_NOT_FOUND, sizeof(COUNTRY_NOT_FOUND));
        std::cout << "The Main Server has sent \"Country Name: Not found\" to client1/2 using TCP over port " << tcpSockaddr.sin_port << std::endl;
        return;
    }

    int serverId = countryFound->second;
    std::cout << countryName << " shows up in server " << intToServerString(serverId) << std::endl;
    std::string send = countryName + " " + userId;
    udpSend(sendfd, getPortForServer(serverId), send.c_str(), send.size());
    std::cout << "The Main Server has sent request from User " << userId << " to server " << intToServerString(serverId) << " using UDP over port " << getPortForServer(serverId) << std::endl;
    int result = readToInt(udpSocket);
    if (result == USER_NOT_FOUND) {
        std::cout << "The Main server has received \"User ID: Not found\" from server " << intToServerString(serverId) << std::endl;
    }
    else {
        std::cout << "The Main server has received searching result(s) of User " << userId << " from server " << intToServerString(serverId) << std::endl;
    }
    wrappedWrite(tcpSocket, &result, sizeof(result));
    if (result == USER_NOT_FOUND) {
        std::cout << "The Main Server has sent error to client using TCP over port " << tcpSockaddr.sin_port << std::endl;
    }
    else {
        std::cout << "The Main Server has sent searching result(s) to client using TCP over port " << tcpSockaddr.sin_port << std::endl;
    }
}

void handleConnection(unsigned int clientID, int tcpSocket, int udpSocket, int sendfd, const std::unordered_map<std::string, int>& country_backend_mapping) {
    // Function for chatting between client and server
    // infinite loop for chat
    try {
        wrappedWrite(tcpSocket, &clientID, sizeof(clientID));
    } catch (const PipeBrokenException& e) {
        // Client has finished, this is normal/ok
        close(tcpSocket);
        return;
    } catch (const std::exception& exception) {
        std::cout << "Error: " << exception.what() << std::endl;
        return;
    } catch (...) {
        std::cout << "Unknown error" << std::endl;
        return;
    }

    while (true) {
        try {
            std::string countryName = readToString(tcpSocket);
            std::string userId = readToString(tcpSocket);

            std::cout << "The Main server has received the request on User " << userId << " in " << countryName << " from client " << clientID << " using TCP over port 32949" << std::endl;

            handleRequest(tcpSocket, udpSocket, sendfd, country_backend_mapping, countryName, userId);
        } catch (const PipeBrokenException& e) {
            // Client has finished, this is normal/ok
            close(tcpSocket);
            return;
        } catch (const std::exception& exception) {
            std::cout << "Error: " << exception.what() << std::endl;
            return;
        } catch (...) {
            std::cout << "Unknown error" << std::endl;
            return;
        }
    }
}

// Driver code
int main()
{
    int udpSocket = bindUDP(MAIN_SERVER_UDP_PORT);
    std::cout << "The main server is up and running using UDP on port 32949" << std::endl;

    int sendfd = socket(AF_INET, SOCK_DGRAM, 0);
    std::string startup("startup");
    udpSend(sendfd, SERVER_A_PORT, startup.c_str(), startup.size());
    udpSend(sendfd, SERVER_B_PORT, startup.c_str(), startup.size());

    std::vector<std::string> serverA;
    serverA.push_back("Server A");
    std::vector<std::string> serverB;
    serverB.push_back("Server B");
    std::unordered_map<std::string, int> country_backend_mapping;
    for (int n = 0; n < 2; ++n) {
        std::string buffer = readToString(udpSocket);
        std::vector<std::string> fromServer = splitStrings(buffer);
        const std::string& serverId = fromServer.front();
        std::cout << "The Main server has received the country list from server " << serverId << " using UDP over port 32949" << std::endl;

        int serverIdInt;
        if (serverId == "A") {
            serverIdInt = 0;
            std::copy(fromServer.cbegin(), fromServer.cend(), std::inserter(serverA, serverA.end()));
        } else {
            serverIdInt = 1;
            std::copy(fromServer.cbegin(), fromServer.cend(), std::inserter(serverB, serverB.end()));
        }

        std::unordered_set<std::string> countries;
        countries.insert(fromServer.cbegin() + 1, fromServer.cend());
        for (const std::string& country : countries) {
            country_backend_mapping[country] = serverIdInt;
        }
    }

    std::string filler(20, ' ');
    while (serverA.size() < serverB.size()) {
        serverA.push_back(filler);
    }
    while (serverA.size() > serverB.size()) {
        serverB.push_back(filler);
    }

    for (auto n = 0u; n < serverA.size(); ++n) {
        serverA[n].resize(20, ' ');
        serverB[n].resize(20, ' ');
        std::cout << serverA[n] << " | " << serverB[n] << std::endl;
    }

    int tcpSocket = bindTCP();

    // Now server is ready to listen and verification
    if ((listen(tcpSocket, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }

    // Don't cause the process to die due to SIGPIPE
    signal(SIGPIPE, SIG_IGN);
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    unsigned int clientID = 1;
    while (true) {
        sockaddr_in cli{};
        socklen_t len = sizeof(cli);
        // Accept the data packet from client and verification
        int connfd = accept(tcpSocket, (sockaddr*)&cli, &len);
        if (connfd < 0) {
            printf("server accept failed...\n");
            exit(0);
        }

        std::thread thread(handleConnection, clientID, connfd, udpSocket, sendfd, country_backend_mapping);
        thread.detach();
        ++clientID;
    }

    // After chatting close the socket
    close(tcpSocket);

    // close the descriptor
    close(udpSocket);
}

/*

The Main server is up and running.
The Main server has received the country list from server A using UDP over port <Main server UDP port number>
The Main server has received the country list from server B using UDP over port <Main server UDP port number>
Server A | Server B
<Country Name 1> | <Country Name 3>
<Country Name 2> |

The Main server has received the request on User <user ID> in <Country Name> from client<client ID> using TCP over port <Main server TCP port number>

<Country Name> does not show up in server A&B

The Main Server has sent “Country Name: Not found” to client1/2 using TCP over port <Main server TCP port

<Country Name> shows up in server A/B

The Main Server has sent request from User <user ID> to server A/B using UDP over port <Main server UDP port number>


The Main server has received searching result(s) of User <user ID> from server<A/B>

The Main Server has sent searching result(s) to client using TCP over port <Main Server TCP port number>

The Main server has received “User ID: Not found” from server <A/B>

The Main Server has sent error to client using TCP over <Main Server UDP port number>
*/
