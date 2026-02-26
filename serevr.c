#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#include<winsock2.h>
#include<WS2tcpip.h>

#define BUFFER_SIZE 1024

#pragma comment(lib,"ws2_32.lib")

typedef struct {
	struct sockaddr_in addr;
	char nickname[30];
}Client;

Client* clients = NULL;
int clients_count;

void delClient(struct sockaddr_in delClient);
void addClient(struct sockaddr_in newClient,const char* str);
int isClient(struct sockaddr_in newClient);
void sendMsg(const char* str,SOCKET sock);
int getNick(const char* source,char* res);
const char* getCommand(const char* buf);
void getNick_fromUser(struct sockaddr_in curclient,char* res);


int main() {
	char buffer[BUFFER_SIZE];
	char buffer_sent[BUFFER_SIZE];
	WSADATA wsDATA;
	int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsDATA);
	if (wsaResult != 0) {
		printf("WSAStartup failed\n");
		return 1;
	}
	const char* server_ip = "127.0.0.1";
	struct sockaddr_in senderAddr;
	struct sockaddr_in myaddr = { .sin_family = AF_INET,
		.sin_port = htons(8080),
		.sin_addr.s_addr = inet_addr(server_ip)
	};
	SOCKET recvSock;
	recvSock=socket(AF_INET, SOCK_DGRAM,0);
	if (recvSock == INVALID_SOCKET) {
		printf("Socket creating failed%d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	int res=bind(recvSock, (struct sockaddr*)&myaddr, sizeof(myaddr));
	if (res == SOCKET_ERROR) {
		printf("Bind failed: %d\n", WSAGetLastError());
		closesocket(recvSock);
		WSACleanup();
		return 1;
	}
	socklen_t len = sizeof(senderAddr);
	while (1) {
		char* command;
		int recived = recvfrom(recvSock, buffer, sizeof(buffer)-1, 0, (struct sockaddr*)&senderAddr, &len);
		if (recived == SOCKET_ERROR) {
			continue;
		}
		if (recived > 0) {
			buffer[recived] = '\0';
		}
		command = getCommand(buffer);
		if (strncmp(command, "NICK", 4) == 0) {
			char nickName[30];
			if (getNick(buffer, nickName) == 1) {
				if (isClient(senderAddr) == 0) {
					addClient(senderAddr, nickName);
					char welcomeMsg[] = "Welcome to the chat! Start typing:\n";
					sendto(recvSock, welcomeMsg, strlen(welcomeMsg) + 1, 0, (struct sockaddr*)&senderAddr, sizeof(senderAddr));
				}
			}
		}
		else if(strncmp(command, "MSG", 3) == 0) {
			if (isClient(senderAddr) == 1) {
				char tempNick[30] = { 0 };
				getNick_fromUser(senderAddr, tempNick);
				char msg[BUFFER_SIZE];
				sprintf(msg, "[%s]:%s", tempNick,buffer+5);
				sendMsg(msg, recvSock);
			}
			else {
				char warnning[] = "You need to enter /nick<nikename> first\n";
				sendto(recvSock, warnning, strlen(warnning), 0, (struct sockaddr*)&senderAddr, sizeof(senderAddr));
			}
		}
		else if (strncmp(command, "QUIT", 4) == 0) {
			if (isClient(senderAddr) == 1) {
				char quit[] = "QUIT";
				//char tempNick[30] = { 0 };
				//getNick_fromUser(senderAddr, tempNick);
				//char msg[BUFFER_SIZE];
				//sprintf(msg, "[%s]: left the chat\n", tempNick);
				delClient(senderAddr);
				//sendMsg(msg, recvSock);
				sendto(recvSock, quit, strlen(quit), 0, (struct sockaddr*)&senderAddr, sizeof(senderAddr));
			}
		}
		else if (strncmp(command, "USERS", 5)==0) {
			if (isClient(senderAddr) == 1) {
				char userList[BUFFER_SIZE] = "--- Users online ---\n";
				for (int i = 0; i < clients_count; i++) {
					strcat(userList, "- ");
					strcat(userList, clients[i].nickname);
					strcat(userList, "\n");
				}
				sendto(recvSock, userList, strlen(userList) + 1, 0, (struct sockaddr*)&senderAddr, sizeof(senderAddr));
			}
			else {
				char warnning[] = "You need to enter /nick <nickname> first\n";
				sendto(recvSock, warnning, strlen(warnning) + 1, 0, (struct sockaddr*)&senderAddr, sizeof(senderAddr));
			}

		}
		else {
			char warnning[] = "Unknown command. Use /nick or /msg\n";
			sendto(recvSock, warnning, strlen(warnning) + 1, 0, (struct sockaddr*)&senderAddr, sizeof(senderAddr));
		}
	}
	closesocket(recvSock);
	WSACleanup();
	free(clients);
	system("pause");
	return 0;
}

void addClient(struct sockaddr_in newClient,const char* str) {
	Client* temp = (Client*)realloc(clients, (clients_count + 1) * (sizeof(Client)));
	if (temp == NULL) {
		printf("Memory allocation filed\n");
		return;
	}
	clients = temp;
	clients[clients_count].addr = newClient;
	strcpy(clients[clients_count].nickname, str);
	printf("New client connected\n");
	clients_count++;
	
}

void delClient(struct sockaddr_in delClient) {
	int indexDel=-1;
	for (int i = 0; i < clients_count; i++) {
		if (clients[i].addr.sin_addr.s_addr == delClient.sin_addr.s_addr && clients[i].addr.sin_port == delClient.sin_port) {
			indexDel = i;
		}
	}
	if (indexDel != -1) {
		for (int j = indexDel; j < clients_count - 1; j++) {
			clients[j] = clients[j + 1];
		}
		clients_count--;
		printf("User deleted\n");
	}
}

int isClient(struct sockaddr_in newClient) {
	for (int i = 0; i < clients_count; i++) {
		if (clients[i].addr.sin_addr.s_addr == newClient.sin_addr.s_addr && clients[i].addr.sin_port == newClient.sin_port) {
			return 1;
		}
	}
	return 0;
}

void sendMsg(const char* str,SOCKET sock) {
	for (int i = 0; i < clients_count; i++) {
		int sent=sendto(sock, str, strlen(str) + 1, 0, (struct sockaddr*)&clients[i].addr, sizeof(clients[i].addr));
	}
}

int getNick(const char* source, char* res) {
	while (*source != '\0' && !isspace(*source)) {
		source++;
	}
	while (isspace(*source)) {
		source++;
	}
	if (*source == '\0') {
		return -1;
	}
	while (isalpha(*source) || isdigit(*source)) { 
		*res++ = *source++;
	}
	*res = '\0';
	return 1;
}

const char* getCommand(const char* buf) {
	char temp[20] = "";
	char* tempP = temp;
	while (!isspace(*buf)) {
		*tempP++ = *buf++;
	}
	*tempP = '\0';
	if (strncmp(temp, "/nick", 5) == 0) {
		return "NICK";
	}
	if (strncmp(temp, "/msg", 4) == 0) {
		return "MSG";
	}
	if (strncmp(temp, "/quit", 5) == 0) {
		return "QUIT";
	}
	if (strncmp(temp, "/users", 6) == 0) {
		return "USERS";
	}
	else {
		return "UNKOWN";
	}
}

void getNick_fromUser(struct sockaddr_in curClient,char* res) {
	for (int i = 0; i < clients_count; i++) {
		if (clients[i].addr.sin_addr.s_addr == curClient.sin_addr.s_addr && clients[i].addr.sin_port == curClient.sin_port) {
			strcpy(res, clients[i].nickname);
			return;
		}
	}
}