#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define BUF_SIZE 1024
void ErrorHandling(char* msg);
void CALLBACK CompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD); // IO�� �Ϸ�Ǹ� �� �Լ��� ȣ���ش޶�� �ü������ ��û

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
	evObj = WSACreateEvent(); // dummy event object(completion routine�� ����ϱ� ������ event ������Ʈ�� Ư���� ������ ������ �ʴ´�. ���⼭�� WSAWaitForMultipleEvents �Լ��� ȣ���ϱ� ���� ���)

	// ������ ������ �Ϸ�Ǹ� �ü���� CompRoutine�� ȣ���Ѵ�.
	// �ٸ� IO�� ��û�� �����尡 alertable wait ���¿� �������� ���� completion routine�� ȣ���ϱ�� �Ǿ��ֱ� ������(�߿��� �۾��߿� completion routine�� ȣ��Ǹ� ���α׷��� �帧�� ��ĥ �� �ֱ� ����)
	// WaitForSingleObjectEx, WaitForMultipleObjectEx, WSAWaitForMultipleEvents, SleepEx �Լ��� ȣ���Ͽ� alertable wait ���°� �Ǿ���Ѵ�.
	if (WSARecv(hRecvSock, &dataBuf, 1, &recvBytes, &flags, &overlapped, CompRoutine) == SOCKET_ERROR) {
		if (WSAGetLastError() == WSA_IO_PENDING) {
			puts("background data receive");
		}
	}

	idx = WSAWaitForMultipleEvents(1, &evObj, FALSE, WSA_INFINITE, TRUE); // �ټ� ��° ����(fAlertable)�� TRUE�� �����ϸ� �ش� ������� alertable wait ���°� �ȴ�.
	if (idx == WAIT_IO_COMPLETION) // completion routine�� ����Ǹ�
		puts("overlapped IO completed");
	else
		ErrorHandling("WSARecv() error");

	WSACloseEvent(evObj);
	closesocket(hRecvSock);
	closesocket(hLisnSock);
	WSACleanup();

	return 0;
}

// completion routine���� �׻� IO �����ڵ�, �ۼ��ŵ� ����Ʈ ��, OVERLAPPED ����ü�� �ּ�, ����� �Լ�ȣ�� �� ���޵� Ư�������� �Ű������� ���޵ȴ�.
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