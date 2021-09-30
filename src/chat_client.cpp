#pragma warning(disable:4996)
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <WinSock2.h>

#define BUF_SIZE 1024
#define NAME_SIZE 20

void SendMsg(SOCKET sock);
void RecvMsg(SOCKET sock);
void ErrorHandling(const char* msg);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hSock;
	SOCKADDR_IN servAddr;
	std::thread SendThread, RecvThread;

	// 명령인수 4개 = 프로그램 이름, ip, port, name
	if (argc != 4) {
		printf("Usage %s <ip> <port> <name>\n", argv[0]);
		exit(1);
	}

	// winsock 초기화
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	// 서버와 연결할 클라이언트 소켓 생성
	hSock = socket(PF_INET, SOCK_STREAM, 0);
	if (hSock == INVALID_SOCKET)
		ErrorHandling("socket() error");

	// 입력 받은 명령 인수 ip주소와 port번호로 서버 주소 구조체 초기화
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(argv[1]);
	servAddr.sin_port = htons(atoi(argv[2]));

	// 클라이언트 소켓으로 서버에 연결 시도
	if (connect(hSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("connect() error");

	// 송수신할 소켓 디스크립터를 인자로 넘기면서 스레드 생성
	SendThread = std::thread(SendMsg, hSock);
	RecvThread = std::thread(RecvMsg, hSock);

	// 송수신 스레드가 종료할 때까지 main 스레드는 블로킹
	SendThread.join();
	RecvThread.join();
	
	return 0;
}

void SendMsg(SOCKET sock) {
	const char* dummy_str = "dummy_str";

	int i = 0;
	int strLen = strlen(dummy_str);
	while (1) {
		send(sock, dummy_str, strLen, 0);
		Sleep(10);
		i++;

		// 1000번 전송하고 스레드 종료
		if (i == 1000)
			return;
	}
}

void RecvMsg(SOCKET sock) {
	int strLen;
	char buf[BUF_SIZE];

	while (1) {
		strLen = recv(sock, buf, BUF_SIZE - 1, 0);
		if (strLen == -1)
			return;

		buf[strLen] = 0;
		fputs(buf, stdout);
	}
	return;
}

void ErrorHandling(const char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}