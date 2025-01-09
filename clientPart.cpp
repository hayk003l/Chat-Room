#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#pragma comment(lib, "ws2_32.lib")

#include <winsock2.h>
#include <string>
#include <vector>
#include <iostream>

#define maxMsgSize 256
#define NULL 0
#define isDebug true

SOCKET sock = INVALID_SOCKET;

std::vector<std::string> split(const std::string& strToSplit, const std::string& splitterStr) {
    std::string splittedStr = "";
    std::vector<std::string> returnStr;

    int i = 0;

    while (i < strToSplit.size()) {

        if (strToSplit[i] == splitterStr[0]) {
            bool isSplitted = true;

            for (int j = 1; j < splitterStr.size(); ++j) {
                if (strToSplit[i + j] == strToSplit[j]) {
                    isSplitted = false;
                    break;
                }
            }

            if (isSplitted) {
                returnStr.push_back(splittedStr);
                splittedStr = "";
                i += splitterStr.size();
            }
        }

        splittedStr += strToSplit[i];
        ++i;
    }
    
    returnStr.push_back(splittedStr);
    return returnStr;
}

void clientHandler() {
    int connectionStatus;
    char msg[256];

    while (true) {
        connectionStatus = recv(sock, msg, maxMsgSize, NULL);

        if (connectionStatus <= 0) {
            std::cout << "You are dissconected from server." << std::endl;
            closesocket(sock);
            sock = INVALID_SOCKET;
            break;
        }

        std::cout << msg << std::endl;
    }
    
}


bool connectToServer(std::string ipAddr, int port) {
    WSAData wsaData;

    if (WSAStartup(MAKEWORD(2, 1), &wsaData) != 0) {
		std::cout << "Winsock init fail!" << std::endl;
		return false;
	}

    SOCKADDR_IN addr;
    addr.sin_addr.s_addr = inet_addr(ipAddr.c_str());
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    sock = socket(AF_INET, SOCK_STREAM, NULL);

    if (connect(sock, (SOCKADDR*)&addr, sizeof(addr)) != 0) {
        std::cout << "Connection failed." << std::endl;
        return false;
    }

    std::cout << "Connected to server." << std::endl;

    CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)clientHandler, NULL, NULL, NULL);
    return true;
}


int main() {

    std::string message;

    std::cout << "Use command help to get information." << std::endl;

    if (isDebug) {
        connectToServer("192.168.1.5", 1111);
    }

    while (true) {
        getline(std::cin, message);
        
        if (message.size() > maxMsgSize) {
            std::cout << "Too long message." << std::endl;
            continue;
        }

        std::vector<std::string> msgVec = split(message, " ");

        if (msgVec[0] == "help") {
            std::cout << "Use command connect to connect server" << std::endl;
			std::cout << "Use command disconnect to disconnect server" << std::endl;
			std::cout << "Use command create to create room" << std::endl;
			std::cout << "Use command remove to remove room" << std::endl;
			std::cout << "Use command open to open room" << std::endl;
			std::cout << "Use command !exit to exit room" << std::endl;
			std::cout << "Use command ls to show all rooms" << std::endl;
			continue;
        }

        if (msgVec[0] == "connect" && msgVec.size() >= 3) {
            
            if (sock == INVALID_SOCKET) {
                connectToServer(msgVec[1], atoi(msgVec[2].c_str()));
                continue;
            }
            
            else {
                std::cout << "You already connected to server." << std::endl;
            }
            continue;
        }
        if (msgVec[0] == "connect" && msgVec.size() < 3) {
            std::cout << "Wrong command. You have to specify server ip address and port" << std::endl;
			std::cout << "Command usage: connect 127.0.0.1 1111" << std::endl;
			continue;
        }

        if (msgVec[0] == "disconnect") {
            if (sock != INVALID_SOCKET) {
                closesocket(sock);
                sock = INVALID_SOCKET;
                std::cout << "Disconnected from server." << std::endl;
            }
            else {
                std::cout << "You are not connected to any server." << std::endl;
            }
            continue;
        }

        send(sock, message.c_str(), maxMsgSize, NULL);
        Sleep(10);
    }
}