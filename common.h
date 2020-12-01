#define once

#include <string>
#include <vector>

const int SERVER_A_PORT = 30949;
const int SERVER_B_PORT = 31949;
const int MAIN_SERVER_UDP_PORT = 32949;
const int MAIN_SERVER_TCP_PORT = 33949;

const int MAX_USER_ID_LENGTH = 10;
const int MAX_COUNTRY_NAME_LENGTH = 20;

const int USER_ALL_CONNECTED = -1;
const int USER_NOT_FOUND = -2;
const int COUNTRY_NOT_FOUND = -3;

std::vector<std::string> splitStrings(const std::string& line);
std::vector<int> splitInts(const std::string& line);

int bindUDP(int port);
void udpSend(int fd, int port, const void* data, size_t size);
std::string readToString(int fd);
int readToInt(int fd);
void wrappedWrite(int fd, const void* data, size_t size);

void serverAB(const std::string& filename, const std::string& serverName, int port);

class PipeBrokenException {
};
