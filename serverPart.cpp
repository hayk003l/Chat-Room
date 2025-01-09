#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#pragma comment(lib, "ws2_32.lib")

#include <winsock2.h>
#include <string>
#include <vector>
#include <iostream>

#define maxMsgSize 256
#define NULL 0

struct Room {
    Room(const std::string& name, const std::string& password) : m_name(name), m_password(password){

    }
    std::string m_name;
    std::string m_password;
    std::vector<int> users;
};

std::vector<SOCKET> connections;

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


template <typename T>
void removeElementFromVecByName(std::vector<T>& vec, T name) {
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        if ((*it) == name) {
            vec.erase(it);
            break;
        }
    }
}


std::vector<Room> rooms;

void messageHandler(int userId, char msg[maxMsgSize], int* roomId) {
    
    std::vector<std::string> messageVec = split(std::string(msg), " ");
    std::string message;

    if (messageVec[0] == "exit") {
        removeElementFromVecByName(rooms[*roomId].users, userId);
        message = "You exited from room - " + rooms[*roomId].m_name;
        *roomId = -1;
        send(connections[userId], message.c_str(), maxMsgSize, NULL);
    }

    if (*roomId != -1) {
        for (auto elem : rooms[*roomId].users) {
            if (elem != userId) {
                send(connections[elem], msg, maxMsgSize, NULL);
            }
        }
        return;
    }


    if (messageVec[0] == "ls") {
        for (int i = 0; i < rooms.size(); ++i) {
            message = rooms[i].m_name;
            send(connections[userId], message.c_str(), maxMsgSize, NULL);
        }
        return;
    }

    if (messageVec[0] == "create" && messageVec.size() >= 3) {

        message = "You are created room" + messageVec[1];
        bool isTakenName = false;

        for (auto elem : rooms) {
            if (elem.m_name == messageVec[1]) {
                message = "This room name already taken";
                isTakenName = true;
                break;
            }
        }

        if (!isTakenName) {
            Room r(messageVec[1], messageVec[2]);
            rooms.push_back(r);
        }

        send(connections[userId], message.c_str(), maxMsgSize, NULL);
        return;
    }

    if (messageVec[0] == "create" && messageVec.size() < 3) {
        message = "Wrong command. You have to specify room name and password\nCommand usage: create room_name room_password";
		send(connections[userId], message.c_str(), maxMsgSize, NULL);
		return;
    }

    if (messageVec[0] == "remove" && messageVec.size() >= 3) {
        
        message = "You are removed room - " + messageVec[1];

        bool wasTrueName = false;

        for (auto it = rooms.begin(); it != rooms.end(); ++it) {
            if ((*it).m_name == messageVec[1]) {
                wasTrueName = true;
                if ((*it).m_password == messageVec[2]) {
                    rooms.erase(it);
                    break;
                }
                else {
                    message = "Incorrect password.";
                }
            }
        }

        if (!wasTrueName) {
            message = "Wrong name.";
        }

        send(connections[userId], message.c_str(), maxMsgSize, NULL);
        return;
    }

    if (messageVec[0] == "remove" && messageVec.size() < 3) {
        message = "Wrong command. You have to specify room name and password\nCommand usage: remove room_name room_password";
        send(connections[userId], message.c_str(), maxMsgSize, NULL);
        return;
    }

    if (messageVec[0] == "open" && messageVec.size() >= 3) {
        bool isValidData = false;
        message = "Opened room = " + messageVec[1];

        for (int i = 0; i < rooms.size(); ++i) {
            if (messageVec[1] == rooms[i].m_name && messageVec[2] == rooms[i].m_password) {
                rooms[i].users.push_back(userId);
                isValidData = true;
                *roomId = i;
                break;
            }
        }

        if (!isValidData) {
            message = "Incorrect room name or password.";
        }
        send(connections[userId], message.c_str(), maxMsgSize, NULL);
        return;
    }

    if (messageVec[0] == "open" && messageVec.size() < 3) {
        message = "Wrong command. You have to specify room name and password\nCommand usage: open room_name room_password";
        send(connections[userId], message.c_str(), maxMsgSize, NULL);
        return;
    }

}

void clientHandler(int i) {
    std::cout << "Client connected, id : " << i << std::endl;
    char msg[maxMsgSize] = "Welcome. You are connected to server.";
    send(connections[i], msg, maxMsgSize, NULL);

    int connectionStatus;
    int roomId = -1;

    while (true) {
        connectionStatus = recv(connections[i], msg, sizeof(msg), NULL);

        if (connectionStatus <= 0) {
            std::cout << "Client dissconected, id : " << i << std::endl;
            if (roomId != -1) {
                removeElementFromVecByName(rooms[roomId].users, i);
            }
            break;
        }

        messageHandler(i, msg, &roomId);
    }

    closesocket(i);

	connections[i] = INVALID_SOCKET;
	
    if (i == connections.size() - 1) {
        connections.pop_back();
    }
		
	std::cout << connections.size() << std::endl;

	return;
}


int main() {
    WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 1), &wsaData) != 0)
	{
		std::cout << "Error: Library initialization failure." << std::endl;
		exit(1);
	}

	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("192.168.1.5");
	addr.sin_port = htons(1111);
	addr.sin_family = AF_INET;

    SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
	listen(sListen, SOMAXCONN);

	SOCKET newConnection;

    while (true) {
        newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);

        if (newConnection == INVALID_SOCKET) {
            std::cout << "Error: Client connection failure." << std::endl;
        }
        else {
            bool wasReusedSocket = false;

            for (int i = 0; i < connections.size(); ++i) {
                if (connections[i] == INVALID_SOCKET) {
                    CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)clientHandler, (LPVOID)(i), NULL, NULL);
					connections[i] = newConnection;
					wasReusedSocket = true;
					std::cout << "Reused socket" << std::endl;
					break;
                }
            }

            if (wasReusedSocket == false) {
                CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)clientHandler, (LPVOID)(connections.size()), NULL, NULL);
				connections.push_back(newConnection);
            }
        }
    }
}