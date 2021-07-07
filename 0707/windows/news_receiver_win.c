#pragma warning (disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ws2tcpip.h> // 구조체 ip_mreq
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

	hRecvSock = socket(PF_INET, SOCK_DGRAM, 0); // 멀티캐스트는 udp기반으로 동작한다.
	if (hRecvSock == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY); // inet_addr는 문자열로 된 주소가 인자로 넘어올 때 사용
	addr.sin_port = htons(atoi(argv[2]));

	// 데이터를 받으려면 당연히 포트를 지정해서 운영체제에게 알려줘야 한다.
	if (bind(hRecvSock, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	// 멀티캐스트 주소 등록
	joinAddr.imr_multiaddr.s_addr = inet_addr(argv[1]);
	joinAddr.imr_interface.s_addr = htonl(INADDR_ANY);
	
	// 멀티캐스트 그룹 가입
	if (setsockopt(hRecvSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&joinAddr, sizeof(joinAddr)) == SOCKET_ERROR)
		ErrorHandling("setsockopt() error");

	// 소켓을 계속 열어놓고 있기 때문에 sender가 데이터를 전부 보내고 난 이후에 다른 데이터도 계속해서 수신할 수 있다.
	while (1) {
		strLen = recvfrom(hRecvSock, buf, BUF_SIZE - 1, 0, NULL, 0);
		//printf("%d\n", strLen);
		if (strLen < 0) // 오류 발생
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