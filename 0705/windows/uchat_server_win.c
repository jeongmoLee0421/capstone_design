#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 1024
void ErrorHandling(char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET servSock;
	char message[BUF_SIZE];
	int strLen;
	int clntAddrSize;

	SOCKADDR_IN servAddr, clntAddr;

	if (argc != 2) {
		printf("Usage %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	servSock = socket(PF_INET, SOCK_DGRAM, 0);
	if (servSock == INVALID_SOCKET)
		ErrorHandling("udp socket() error");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));

	if (bind(servSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	while (1) {
		clntAddrSize = sizeof(clntAddr);
		strLen = recvfrom(servSock, message, BUF_SIZE, 0, (SOCKADDR*)&clntAddr, &clntAddrSize);
		message[strLen] = 0;
		printf("message from clinet: %s", message);

		fputs("input message(q to quit): ", stdout);
		fgets(message, sizeof(message), stdin);
		if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
			break;

		sendto(servSock, message, strlen(message), 0, (SOCKADDR*)&clntAddr, clntAddrSize);
	}

	closesocket(servSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}