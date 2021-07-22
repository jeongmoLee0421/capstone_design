#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define BUF_SIZE 1024
void ErrorHandling(char* msg);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hLisnSock, hRecvSock;
	SOCKADDR_IN lisnAddr, recvAddr;
	int recvAddrSize;

	// overlapped IO에 사용되는 버퍼, event 오브젝트, overlapped 구조체
	WSABUF dataBuf;
	WSAEVENT evObj;
	WSAOVERLAPPED overlapped;

	char buf[BUF_SIZE];
	int recvBytes = 0, flags = 0;

	if (argc != 2) {
		printf("usage: %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	// overlapped IO를 진행하기 위한 소켓 생성
	hLisnSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hLisnSock == INVALID_SOCKET)
		ErrorHandling("WSASocket() error");

	memset(&lisnAddr, 0, sizeof(lisnAddr));
	lisnAddr.sin_family = AF_INET;
	lisnAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	lisnAddr.sin_port = htons(atoi(argv[1]));

	if (bind(hLisnSock, (SOCKADDR*)&lisnAddr, sizeof(lisnAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	if (listen(hLisnSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	recvAddrSize = sizeof(recvAddr);
	hRecvSock = accept(hLisnSock, (SOCKADDR*)&recvAddr, &recvAddrSize);
	if (hRecvSock == INVALID_SOCKET)
		ErrorHandling("accept() error");

	evObj = WSACreateEvent();
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = evObj;
	dataBuf.len = BUF_SIZE;
	dataBuf.buf = buf;

	if (WSARecv(hRecvSock, &dataBuf, 1, &recvBytes, &flags, &overlapped, NULL) == SOCKET_ERROR) {
		if (WSAGetLastError() == WSA_IO_PENDING) { // 아직 데이터 수신이 진행중
			puts("background data receive");
			WSAWaitForMultipleEvents(1, &evObj, TRUE, WSA_INFINITE, FALSE); // evObj가 signaled 상태(데이터 수신이 완료되면)가 될때까지 대기
			WSAGetOverlappedResult(hRecvSock, &overlapped, &recvBytes, FALSE, NULL); // overlapped IO 결과 확인
		}
		else {
			ErrorHandling("WSARecv() error");
		}
	}

	printf("received message: %s\n", buf);
	WSACloseEvent(evObj);
	closesocket(hRecvSock);
	closesocket(hLisnSock);
	WSACleanup();

	return 0;
}

void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}