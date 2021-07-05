#pragma warning (disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 30
void ErrorHandling(char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	FILE* fp;

	char buf[BUF_SIZE];
	int readCnt;
	int clntAddrSize;
	SOCKADDR_IN servAddr, clntAddr;

	if (argc != 2) {
		printf("Usage %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	if ((fp = fopen("../server/file_server_win.c", "rb")) == NULL)
		ErrorHandling("fopen() error");

	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	if (hServSock == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));

	if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	if (listen(hServSock, 5) == -1)
		ErrorHandling("listen() error");

	clntAddrSize = sizeof(clntAddr);
	hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &clntAddrSize);
	if (hClntSock == INVALID_SOCKET)
		ErrorHandling("accept() error");

	while (1) {
		readCnt = fread((void*)buf, 1, BUF_SIZE, fp);
		if (readCnt < BUF_SIZE) {
			send(hClntSock, buf, readCnt, 0);
			break;
		}
		send(hClntSock, buf, BUF_SIZE, 0);
	}

	shutdown(hClntSock, SD_SEND); // 출력 스트림만 닫는다.
	recv(hClntSock, buf, BUF_SIZE, 0); // 입력 스트림은 열려 있기 때문에 클라이언트가 보내는 메시지를 수신할 수 있다.
	printf("message from client: %s\n", buf);

	fclose(fp);
	closesocket(hClntSock);
	closesocket(hServSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}