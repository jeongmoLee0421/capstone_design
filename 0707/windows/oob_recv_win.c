#pragma warning (disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define BUF_SIZE 30
void ErrorHandling(char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hAcptSock, hRecvSock;

	SOCKADDR_IN recvAddr, sendAddr;
	int sendAddrSize, strLen;
	char buf[BUF_SIZE];
	int result;

	fd_set read, except, readCopy, exceptCopy;
	struct timeval timeout;

	if (argc != 2) {
		printf("Usage %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	hAcptSock = socket(PF_INET, SOCK_STREAM, 0);
	if (hAcptSock == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&recvAddr, 0, sizeof(recvAddr));
	recvAddr.sin_family = AF_INET;
	recvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	recvAddr.sin_port = htons(atoi(argv[1]));

	if (bind(hAcptSock, (SOCKADDR*)&recvAddr, sizeof(recvAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	if (listen(hAcptSock, 5) == -1)
		ErrorHandling("listen() error");

	sendAddrSize = sizeof(sendAddr);
	hRecvSock = accept(hAcptSock, (SOCKADDR*)&sendAddr, &sendAddrSize);
	if (hRecvSock == INVALID_SOCKET)
		ErrorHandling("hRecvSock");

	FD_ZERO(&read);
	FD_ZERO(&except);
	FD_SET(hRecvSock, &read);
	FD_SET(hRecvSock, &except);

	while (1) {
		readCopy = read;
		exceptCopy = except;
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		result = select(0, &readCopy, 0, &exceptCopy, &timeout);

		if (result > 0) { // ��ȭ�� �ִ� ������ �ִ� ���, ���� ����ŭ select �Լ��� ��ȯ
			if (FD_ISSET(hRecvSock, &exceptCopy)) { // out-of-band �����ʹ� ���ܻ�Ȳ�� �ش�ȴ�.
				strLen = recv(hRecvSock, buf, BUF_SIZE - 1, MSG_OOB);
				buf[strLen] = 0;
				printf("urgent message: %s\n", buf);
			}

			if (FD_ISSET(hRecvSock, &readCopy)) {
				strLen = recv(hRecvSock, buf, BUF_SIZE - 1, 0);
				if (strLen == 0) { // EOF
					break;
					closesocket(hRecvSock);
				}
				else {
					buf[strLen] = 0;
					puts(buf);
				}
			}
		}
	}

	closesocket(hAcptSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}