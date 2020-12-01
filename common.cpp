#include <stdio.h>
#include <ctype.h>
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

#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "common.h"

std::vector<std::string> splitStrings(const std::string& line) {
    std::vector<std::string> result;
    std::string current;
    for (size_t n = 0; n < line.size(); ++n) {
        if (line[n] != ' ') {
            current.push_back(line[n]);
        } else {
            if (!current.empty()) {
                result.push_back(current);
                current.clear();
            }
        }
    }

    if (!current.empty()) {
        result.push_back(current);
        current.clear();
    }

    return result;
}

std::vector<int> splitInts(const std::string& line) {
    std::vector<std::string> strings = splitStrings(line);
    std::vector<int> result;
    for (const std::string& string : strings) {
        result.push_back(std::stoi(string));
    }
    return result;
}


std::unordered_map<std::string, std::unordered_map<int, std::unordered_set<int>>> getCountries(const std::string& filename) {
    std::ifstream inputFile(filename);
    std::string line;

    std::unordered_map<std::string, std::unordered_map<int, std::unordered_set<int>>> countryToConnections;
    std::unordered_map<int, std::unordered_set<int>> currentCountryConnections;
    std::string currentCountry;

    while (std::getline(inputFile, line)) {
        if (isalpha(line[0])) {
            if (!currentCountry.empty()) {
                countryToConnections[currentCountry] = currentCountryConnections;
                currentCountryConnections.clear();
            }
            currentCountry = line;
        } else if (isdigit(line[0])) {
            std::vector<int> values = splitInts(line);
            int userId = values.front();
            std::unordered_set<int> otherUserIds;
            otherUserIds.insert(values.cbegin() + 1, values.cend());
            currentCountryConnections[userId] = otherUserIds;
        }
    }

    countryToConnections[currentCountry] = currentCountryConnections;
    return countryToConnections;
}

std::unordered_set<int> unorderedSetIntersection(const std::unordered_set<int>& a, const std::unordered_set<int>& b) {
    std::unordered_set<int> result;
    for (int value : a) {
        if (b.count(value) > 0) {
            result.insert(value);
        }
    }
    return result;
}

int calculateFriend(const ::std::string& serverName, const std::unordered_map<std::string, std::unordered_map<int, std::unordered_set<int>>>& countries, const std::string& country, int userId) {
    const auto& currentCountry = countries.at(country);
    auto currentUserConnectionsIterator = currentCountry.find(userId);
    if (currentUserConnectionsIterator == currentCountry.cend()) {
        std::cout << "User" << userId << " does not show up in " << country << std::endl;
        return USER_NOT_FOUND;
    }
    const auto& currentUserConnections = currentCountry.at(userId);
    int bestUserId = -1;
    int bestUserCommonConnections = -1;
    for (const auto& otherUser : currentCountry) {
        if (otherUser.first == userId) { // Self
            continue;
        } else if (currentUserConnections.count(otherUser.first) != 0) { // Already connected
            continue;
        }

        std::unordered_set<int> intersection = unorderedSetIntersection(currentUserConnections, otherUser.second);
        if ((int)intersection.size() > bestUserCommonConnections) {
            bestUserId = otherUser.first;
            bestUserCommonConnections = intersection.size();
        } else if ((int)intersection.size() == bestUserCommonConnections) {
            if (otherUser.first < bestUserId) {
                bestUserId = otherUser.first;
                bestUserCommonConnections = intersection.size();
            }
        }
    }
    std::string bestUserIdString;
    if (bestUserId == -1) {
        bestUserIdString = "none";
    }
    else {
        bestUserIdString = std::to_string(bestUserId);
    }
    std::cout << "The server " << serverName << " is searching possible friends for User" << userId << std::endl << "Here are the results: User" << bestUserIdString << std::endl;
    return bestUserId;
}

int bindUDP(int port) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in main_serv_addr{};
    main_serv_addr.sin_family = AF_INET;
    main_serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    main_serv_addr.sin_port = htons(port);

    // bind server address to socket descriptor
    if (bind(sockfd, (sockaddr*)&main_serv_addr, sizeof(main_serv_addr)) != 0) {
        std::cout << "Failed to bind, errno " << errno << std::endl;
        exit(-1);
    }
    return sockfd;
}

void udpSend(int fd, int port, const void* data, size_t size) {
    sockaddr_in cliaddr{};
    cliaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    cliaddr.sin_port = htons(port);
    cliaddr.sin_family = AF_INET;
    if (sendto(fd, data, size, 0, (sockaddr*)&cliaddr, sizeof(cliaddr)) == -1) {
        if (errno == EPIPE) {
            throw PipeBrokenException();
        }
        else {
            std::cout << "Send error" << std::endl;
            exit(-1);
        }
    }
}

std::string readToString(int fd) {
    std::vector<char> buffer;
    buffer.resize(100);
    int readReturn = read(fd, buffer.data(), buffer.size());
    if (readReturn == 0) {
        throw PipeBrokenException();
    }
    else if (readReturn == -1) {
        if (errno == EPIPE) {
            throw PipeBrokenException();
        }
        else {
            std::cout << "Read error" << std::endl;
            exit(-1);
        }
    }
    std::string s(buffer.data());
    return s;
}

int readToInt(int fd) {
    int result;
    int readReturn = read(fd, &result, sizeof(result));
    if (readReturn == 0) {
        throw PipeBrokenException();
    }
    else if (readReturn == -1) {
        if (errno == EPIPE) {
            throw PipeBrokenException();
        }
        else {
            std::cout << "Read error" << std::endl;
            exit(-1);
        }
    }
    return result;
}

void wrappedWrite(int fd, const void* data, size_t size) {
    if (write(fd, data, size) == -1) {
        if (errno == EPIPE) {
            throw PipeBrokenException();
        }
        else {
            std::cout << "Write error" << std::endl;
            exit(-1);
        }
    }
}

void serverAB(const std::string& filename, const std::string& serverName, int port) {
    // Create a UDP Socket
    int sendfd = socket(AF_INET, SOCK_DGRAM, 0);

    int udpSocket = bindUDP(port);
    std::cout << "The server " << serverName << " is up and running using UDP on port " << port << std::endl;

    // Wait for main server to startup before sending country list
    std::string buffer = readToString(udpSocket);

    /** BUILDING THE NEEDED MATRICES, ARRAYS */
    auto countries = getCountries(filename);

    std::string countriesSingleString;
    for (const auto& country : countries) {
        countriesSingleString += country.first;
        countriesSingleString += " ";
    }

    // send a country list to main server
    std::string sendBuffer(serverName + " " + countriesSingleString);
    udpSend(sendfd, MAIN_SERVER_UDP_PORT, sendBuffer.c_str(), sendBuffer.size());
    std::cout << "The server " << serverName << " has sent a country list to Main Server" << std::endl;

    while (true) {
        // receive country and userid from the main server
        buffer = readToString(udpSocket);
        std::vector<std::string> requestSplit = splitStrings(buffer);
        
        if (requestSplit.size() != 2) {
            std::cout << "format error from main server" << std::endl;
            exit(-1);
        }

        /** GENERATING RECOMMENDATION BASED THE COUNTRY NAME AND USER ID */
        std::string country = requestSplit[0];
        int userId = std::stoi(requestSplit[1]);

        std::cout << "The server " << serverName << " has received request for finding possible friends of User" << userId << " in " << country << std::endl;

        int result = calculateFriend(serverName, countries, country, userId);
        udpSend(sendfd, MAIN_SERVER_UDP_PORT, &result, sizeof(result));
        if (result == USER_NOT_FOUND) {
            std::cout << "The server " << serverName << " has sent \"User" << userId << " not found\" to Main Server" << std::endl;
        }
        else {
            std::cout << "The server " << serverName << " has sent the result(s) to Main Server" << std::endl;
        }
    }
}

/*
The server A is up and running using UDP on port <server A port number>
The server A has sent a country list to Main Server
The server A has received request for finding possible friends of User<user ID> in <Country Name>
User<user ID> does not show up in <Country Name>
The server A has sent “User<user ID> not found” to Main Server
The server A is searching possible friends for User<user ID> … Here are the results: User<user ID1>, User<user ID2>...
The server A has sent the result(s) to Main Server
*/
