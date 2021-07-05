#pragma warning (disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 1024
void ErrorHandling(char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET sock;
	char message[BUF_SIZE];
	int strLen;
	SOCKADDR_IN servAddr;

	if (argc != 3) {
		printf("Usage %s <ip> <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(argv[1]);
	servAddr.sin_port = htons(atoi(argv[2]));

	// connected udp 소켓을 사용하면 sendto 함수를 사용해서 udp소켓에 목적지의 ip와 port번호 등록, 전송 후 upd소켓에 등록된 목적지 정보 삭제 동작을 하지 않아도 된다.
	// 결국 속도가 더 빨라진다.
	connect(sock, (SOCKADDR*)&servAddr, sizeof(servAddr));

	// 클라이언트의 경우 sendto를 먼저 호출한다.
	// sendto 호출 시 프로그램의 소켓에 ip와 port번호가 할당되어 있지 않다면 운영체제가 자동으로 ip와 port번호를 할당한다.
	while (1) {
		fputs("insert message(q to quit): ", stdout);
		fgets(message, sizeof(message), stdin);
		if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
			break;

		// connected udp 소켓을 사용하기 때문에 목적지를 알고 있다.
		// 그래서 send와 recv를 사용할 수 있다.
		send(sock, message, strlen(message), 0);
		strLen = recv(sock, message, sizeof(message) - 1, 0);
		message[strLen] = 0;
		printf("message from server: %s", message);
	}

	closesocket(sock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}