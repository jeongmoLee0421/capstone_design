#pragma warning (disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ws2tcpip.h> // ����ü ip_mreq
#include <winsock2.h>

#define BUF_SIZE 30
void ErrorHandling(char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hRecvSock;
	SOCKADDR_IN addr;
	struct ip_mreq joinAddr;
	char buf[BUF_SIZE];
	int strLen;

	if (argc != 3) {
		printf("Usage %s <group ip> <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	hRecvSock = socket(PF_INET, SOCK_DGRAM, 0); // ��Ƽĳ��Ʈ�� udp������� �����Ѵ�.
	if (hRecvSock == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY); // inet_addr�� ���ڿ��� �� �ּҰ� ���ڷ� �Ѿ�� �� ���
	addr.sin_port = htons(atoi(argv[2]));

	// �����͸� �������� �翬�� ��Ʈ�� �����ؼ� �ü������ �˷���� �Ѵ�.
	if (bind(hRecvSock, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	// ��Ƽĳ��Ʈ �ּ� ���
	joinAddr.imr_multiaddr.s_addr = inet_addr(argv[1]);
	joinAddr.imr_interface.s_addr = htonl(INADDR_ANY);
	
	// ��Ƽĳ��Ʈ �׷� ����
	if (setsockopt(hRecvSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&joinAddr, sizeof(joinAddr)) == SOCKET_ERROR)
		ErrorHandling("setsockopt() error");

	// ������ ��� ������� �ֱ� ������ sender�� �����͸� ���� ������ �� ���Ŀ� �ٸ� �����͵� ����ؼ� ������ �� �ִ�.
	while (1) {
		strLen = recvfrom(hRecvSock, buf, BUF_SIZE - 1, 0, NULL, 0);
		//printf("%d\n", strLen);
		if (strLen < 0) // ���� �߻�
			break;

		buf[strLen] = 0;
		fputs(buf, stdout);
	}

	closesocket(hRecvSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}