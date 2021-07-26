#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <winsock2.h>
#include <windows.h>
#include <string.h>

#define BUF_SIZE 100
#define READ 3
#define WRITE 5

typedef struct { // ���� ����
	SOCKET hClntSock;
	SOCKADDR_IN clntAddr;
}PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

typedef struct { // ���� ����
	OVERLAPPED overlapped; // ����ü ���� �ּ� ���� ����ü ù ��° ����� �ּ� ���� ��ġ
	WSABUF wsaBuf;
	char buffer[BUF_SIZE];
	int rwMode;
}PER_IO_DATA, *LPPER_IO_DATA;

DWORD WINAPI EchoThreadMain(LPVOID CompletionPortIO);
void ErrorHandling(char* msg);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	HANDLE hComPort;
	SYSTEM_INFO sysInfo;
	LPPER_IO_DATA ioInfo;
	LPPER_HANDLE_DATA handleInfo;

	SOCKET hServSock;
	SOCKADDR_IN servAddr;
	int recvBytes, i, flags = 0;

	if (argc != 2) {
		printf("usage %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	// cp ������Ʈ ����
	// ������ ���ڷ� 0�� �����߱� ������, pc�� ��ġ�� CPU�� ������ ���ÿ� ���� ������ �������� �ִ� ������ �����Ѵ�. �� pc�� ��� 6��
	hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	// ���� �������� �ý��� ���� ���
	GetSystemInfo(&sysInfo);

	// cpu ����ŭ �����带 �����ϰ�, ������ ������ cp ������Ʈ�� �ڵ��� ���ڷ� �����Ѵ�.
	for (i = 0; i < sysInfo.dwNumberOfProcessors; i++)
		_beginthreadex(NULL, 0, EchoThreadMain, (LPVOID)hComPort, 0, NULL);

	// OVERLAPPED IO�� ������ ���� ����
	hServSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hServSock == INVALID_SOCKET)
		ErrorHandling("WSASocket() error");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));

	if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	if (listen(hServSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	while (1) {
		SOCKET hClntSock;
		SOCKADDR_IN clntAddr;
		int clntAddrSize = sizeof(clntAddr);

		hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &clntAddrSize);
		if (hClntSock == INVALID_SOCKET)
			ErrorHandling("accept() error");

		// PER_HANDLE_DATA ����ü ������ ���� �Ҵ��ϰ� Ŭ���̾�Ʈ ���� ������ ��´�.
		handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		handleInfo->hClntSock = hClntSock;
		memcpy(&(handleInfo->clntAddr), &clntAddr, clntAddrSize);

		// Ŭ���̾�Ʈ ����(��ġ)�� cp ������Ʈ�� �����Ѵ�.
		// 3��° ������ handleInfo�� CompletionKey�� GetQueuedCompletionStatus �Լ��� ��ȯ�ϸ鼭 ���� �� �ִ�.
		CreateIoCompletionPort((HANDLE)hClntSock, hComPort, (DWORD)handleInfo, 0);

		// IOCP�� ������� �������� �ʰ� ���� ������� �Ϸ�Ǿ��ٴ� ������ ���ֱ� ������ read���� write������ ����ص־� �Ѵ�.
		ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		ioInfo->wsaBuf.len = BUF_SIZE;
		ioInfo->wsaBuf.buf = ioInfo->buffer;
		ioInfo->rwMode = READ;

		// WSARecv �Լ��� ȣ���ϸ鼭 6��° ���ڷ� overlapped ����ü ������ �ּ� ���� �����ߴ�.
		// �� ���� GetQueuedCompletionStatus �Լ��� ��ȯ�ϸ鼭 ���� �� �ִ�.
		// �׷��� ����ü ������ �ּ� ���� ù ��° ����� �ּ� ���� �����ϹǷ� PER_IO_DATA ����ü ������ �ּ� ���� ������ �Ͱ� ����.
		WSARecv(handleInfo->hClntSock, &(ioInfo->wsaBuf), 1, &recvBytes, &flags, &(ioInfo->overlapped), NULL);
	}
}

DWORD WINAPI EchoThreadMain(LPVOID pComPort) {
	HANDLE hComPort = (HANDLE)pComPort;
	SOCKET sock;
	DWORD bytesTrans;
	LPPER_HANDLE_DATA handleInfo;
	LPPER_IO_DATA ioInfo;
	DWORD flags = 0;

	while (1) {
		// IO�� �Ϸ�Ǿ� IO Completion Queue�� �׸��� �߰��Ǹ� ��ȯ�Ǵµ�
		// �� �� �ۼ��ŵ� ����Ʈ ��, completion key, overlappd ����ü�� �ּҸ� �����´�.
		// ���⼭�� completion key�� handleInfo�� overlapped ����ü�� ioInfo�� �����Դ�.
		GetQueuedCompletionStatus(hComPort, &bytesTrans, (LPDWORD)&handleInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);

		sock = handleInfo->hClntSock;

		if (ioInfo->rwMode == READ) {
			puts("message received");
			if (bytesTrans == 0) { // EOF���
				closesocket(sock);
				free(handleInfo);
				free(ioInfo);
				continue;
			}

			// �����͸� �޾Ҵٸ� echo ���ش�.
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = bytesTrans;
			ioInfo->rwMode = WRITE;
			WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);

			// echo�� �Ŀ��� ���� �����͸� �ޱ����� WSARecv �Լ��� ȣ���Ѵ�.
			ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = BUF_SIZE;
			ioInfo->wsaBuf.buf = ioInfo->buffer;
			ioInfo->rwMode = READ;
			WSARecv(sock, &(ioInfo->wsaBuf), 1, NULL, &flags, &(ioInfo->overlapped), NULL);
		}
		else {
			// echo �� �ش� ����¿� ���� ioInfo�� �޸� �����Ѵ�.
			puts("message sent");
			free(ioInfo);
		}
	}
	return 0;
}

void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}