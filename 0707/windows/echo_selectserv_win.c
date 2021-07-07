#pragma warning (disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 1024
void ErrorHandling(char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAddr, clntAddr;
	TIMEVAL timeout;
	fd_set reads, cpyReads;

	int addrSize;
	int strLen, fdNum, i;
	char buf[BUF_SIZE];

	if (argc != 2) {
		printf("Usage %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

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

	FD_ZERO(&reads); // reads 배열 0으로 초기화
	FD_SET(hServSock, &reads); // reads 배열에 서버소켓의 file descriptor 정보 등록

	while (1) {
		cpyReads = reads; // select함수 호출 이후 reads 배열에 변화가 생기기 때문에 반복문에서 매번 복사해야 한다.
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;

		if ((fdNum = select(0, &cpyReads, 0, 0, &timeout)) == SOCKET_ERROR) // select error
			break;
		if (fdNum == 0) // timeout
			continue;

		for (i = 0; i < reads.fd_count; i++) {
			if (FD_ISSET(reads.fd_array[i], &cpyReads)) { // 세팅이 되어있는지를 물어보는 함수로 1이라면 세팅이 된 것이고 변화가 있다는 것을 의미
				if (reads.fd_array[i] == hServSock) { // 서버소켓의 fd에 변화가 있었다는 것은 접속 요청을 의미
					addrSize = sizeof(clntAddr);
					hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &addrSize);
					if (hClntSock == INVALID_SOCKET)
						ErrorHandling("accept() error");

					FD_SET(hClntSock, &reads); // 클라이언트와 연결된 소켓의 파일 디스크립터를 등록
					printf("connected client: %d\n", hClntSock);
				}
				else { // 서버소켓이 아닌 클라이언트 소켓의 fd에 변화가 있었다는 것으로 메시지를 echo해야 함
					strLen = recv(reads.fd_array[i], buf, BUF_SIZE - 1, 0);
					if (strLen == 0) { // close request (EOF)
						FD_CLR(reads.fd_array[i], &reads);
						closesocket(cpyReads.fd_array[i]);
						printf("closed client: %d\n", cpyReads.fd_array[i]);
					}
					else {
						send(reads.fd_array[i], buf, strLen, 0); // echo
					}
				}
			}
		}
	}

	closesocket(hServSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}