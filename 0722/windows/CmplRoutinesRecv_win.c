#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define BUF_SIZE 1024
void ErrorHandling(char* msg);
void CALLBACK CompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD); // IO가 완료되면 이 함수를 호출해달라고 운영체제에게 요청

WSABUF dataBuf;
char buf[BUF_SIZE];
int recvBytes;

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hLisnSock, hRecvSock;
	SOCKADDR_IN lisnAddr, recvAddr;
	int recvAddrSize, idx, flags = 0;

	WSAEVENT evObj;
	WSAOVERLAPPED overlapped;

	if (argc != 2) {
		printf("usage: %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

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

	memset(&overlapped, 0, sizeof(overlapped));
	dataBuf.len = BUF_SIZE;
	dataBuf.buf = buf;
	evObj = WSACreateEvent(); // dummy event object(completion routine을 사용하기 때문에 event 오브젝트는 특별한 역할을 가지지 않는다. 여기서는 WSAWaitForMultipleEvents 함수를 호출하기 위해 사용)

	// 데이터 수신이 완료되면 운영체제가 CompRoutine을 호출한다.
	// 다만 IO를 요청한 스레드가 alertable wait 상태에 놓여있을 때만 completion routine을 호출하기로 되어있기 때문에(중요한 작업중에 completion routine이 호출되면 프로그램을 흐름을 망칠 수 있기 때문)
	// WaitForSingleObjectEx, WaitForMultipleObjectEx, WSAWaitForMultipleEvents, SleepEx 함수를 호출하여 alertable wait 상태가 되어야한다.
	if (WSARecv(hRecvSock, &dataBuf, 1, &recvBytes, &flags, &overlapped, CompRoutine) == SOCKET_ERROR) {
		if (WSAGetLastError() == WSA_IO_PENDING) {
			puts("background data receive");
		}
	}

	idx = WSAWaitForMultipleEvents(1, &evObj, FALSE, WSA_INFINITE, TRUE); // 다섯 번째 인자(fAlertable)로 TRUE를 전달하면 해당 스레드는 alertable wait 상태가 된다.
	if (idx == WAIT_IO_COMPLETION) // completion routine이 실행되면
		puts("overlapped IO completed");
	else
		ErrorHandling("WSARecv() error");

	WSACloseEvent(evObj);
	closesocket(hRecvSock);
	closesocket(hLisnSock);
	WSACleanup();

	return 0;
}

// completion routine에는 항상 IO 에러코드, 송수신된 바이트 수, OVERLAPPED 구조체의 주소, 입출력 함수호출 시 전달된 특성정보가 매개변수로 전달된다.
void CALLBACK CompRoutine(DWORD dwError, DWORD sizeRecvBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags) {
	if (dwError != 0) {
		ErrorHandling("CompRoutine error");
	}
	else {
		recvBytes = sizeRecvBytes;
		printf("received message: %s\n", buf);
	}
}

void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}