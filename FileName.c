#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#include<winsock2.h>
#include<WS2tcpip.h>

#define BUFFER_SIZE 1024

#pragma comment(lib,"ws2_32.lib")

int main() {
	char client_msg[BUFFER_SIZE];
	char recvData[BUFFER_SIZE];
	WSADATA wsDATA;
	int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsDATA);
	if (wsaResult != 0) {
		printf("WSAStartup failed\n");
		return 1;
	}
	SOCKET udpSOCKET = socket(AF_INET, SOCK_DGRAM, 0);
	if (udpSOCKET == INVALID_SOCKET) {
		printf("Socket creating failed%d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	struct sockaddr_in recvUnit;
	struct sockaddr_in newServer;
	newServer.sin_family = AF_INET;
	newServer.sin_port = htons(8080);
	newServer.sin_addr.s_addr = inet_addr("127.0.0.1");
	while (1) {
		printf("Enter your msg to server\n");
		fgets(client_msg, BUFFER_SIZE, stdin);
		int sent = sendto(udpSOCKET, client_msg, strlen(client_msg) + 1, 0, (struct sockaddr*)&newServer, sizeof(newServer));
		if (sent == SOCKET_ERROR) {
			printf("Sent was failed%d\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}
		socklen_t len = sizeof(recvUnit);
		int recv = recvfrom(udpSOCKET, recvData, sizeof(recvData) - 1, 0, (struct sockaddr*)&recvUnit, &len);
		if (recv == SOCKET_ERROR) {
			printf("Reciving failed:%d\n", WSAGetLastError());
			closesocket(udpSOCKET);
			WSACleanup();
			return 1;
		}
		if (recv > 0) {
			recvData[recv] = '\0';
		}
		if (strncmp(recvData, "QUIT", 4) == 0) {
			break;
		}
		else {
			printf("Recived:%s\n", recvData);
		}
	}
	closesocket(udpSOCKET);
	WSACleanup();
	return 0;
}
