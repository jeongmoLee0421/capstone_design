#pragma warning (disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <winsock2.h>
#include <windows.h>
#include <string.h>

#define BUF_SIZE 100
#define READ 3
#define WRITE 5
#define MAX_CLNT 200

typedef struct {
	SOCKET hClntSock;
	SOCKADDR_IN clntAddr;
}PER_HANDLE_DATA, * LPPER_HANDLE_DATA;

typedef struct {
	OVERLAPPED overlapped;
	WSABUF wsaBuf;
	char buffer[BUF_SIZE];
	int rwMode;
}PER_IO_DATA, * LPPER_IO_DATA;

DWORD WINAPI EchoThreadMain(LPVOID CompletionPortIO);
void ErrorHandling(char* msg);

int numOfClnt;
SOCKET clntArr[MAX_CLNT];

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

	hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	GetSystemInfo(&sysInfo);

	for (i = 0; i < sysInfo.dwNumberOfProcessors; i++)
		_beginthreadex(NULL, 0, EchoThreadMain, (LPVOID)hComPort, 0, NULL);

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

		// Ŭ���̾�Ʈ ���� �߰�
		clntArr[numOfClnt] = hClntSock;
		numOfClnt++;

		handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		handleInfo->hClntSock = hClntSock;
		memcpy(&(handleInfo->clntAddr), &clntAddr, clntAddrSize);


		CreateIoCompletionPort((HANDLE)hClntSock, hComPort, (DWORD)handleInfo, 0);

		ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		ioInfo->wsaBuf.len = BUF_SIZE;
		ioInfo->wsaBuf.buf = ioInfo->buffer;
		ioInfo->rwMode = READ;

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
	int i, j;
	char temp_buf[BUF_SIZE];

	while (1) {
		GetQueuedCompletionStatus(hComPort, &bytesTrans, (LPDWORD)&handleInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);

		sock = handleInfo->hClntSock;

		if (ioInfo->rwMode == READ) {
			puts("message received");
			if (bytesTrans == 0) {
				closesocket(sock);
				free(handleInfo);
				free(ioInfo);

				// Ŭ���̾�Ʈ ���� ����
				for (i = 0; i < numOfClnt; i++) {
					if (clntArr[i] == sock) {
						for (j = i; j < numOfClnt - 1; j++) {
							clntArr[j] = clntArr[j + 1];
						}
						break;
					}
				}
				numOfClnt--;
				continue;
			}

			// ������ ������ ���� �� �޸� ����
			//memcpy(temp_buf, ioInfo->buffer, sizeof(BUF_SIZE));
			strcpy(temp_buf, ioInfo->buffer);
			free(ioInfo);

			// ������ ������ ��� Ŭ���̾�Ʈ���� ä�� ����
			// �� ��¸��� ���ο� OVERLAPPED ����ü ���� �Ҵ�
			for (i = 0; i < numOfClnt; i++) {
				ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
				memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
				ioInfo->wsaBuf.len = bytesTrans;
				ioInfo->wsaBuf.buf = temp_buf;
				ioInfo->rwMode = WRITE;
				WSASend(clntArr[i], &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
			}

			// �Է¿� ���ؼ��� ���ο� OVERLAPPED ����ü ���� �Ҵ�
			ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = BUF_SIZE;
			ioInfo->wsaBuf.buf = ioInfo->buffer;
			ioInfo->rwMode = READ;
			WSARecv(sock, &(ioInfo->wsaBuf), 1, NULL, &flags, &(ioInfo->overlapped), NULL);
		}
		else {
			// ����� ������ ������ �޸� ����
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

/*
IOCP ����
������� �Ϸ�Ǿ��� ��, �� ������� ������ �����尡 �ƴ� �ٸ� �����尡 �ش� ����� �Ϸ� ������ ó���� �� �ִ�(CPU�� �ִ������� ��밡���ϰ� ��).
non-blocking���� �����ϱ� ������ IO �۾����� �ð� ������ ����.


������ ��
�񵿱� IO ��û�� ������ �� ���Ǵ� ������ ���ۿ� OVERLAPPED ����ü�� IO ��û�� �Ϸ�� ������ �Ű����ų� �������� �ʾƾ� �Ѵ�.
����� ���� �׸��� ������� ���� �Ŀ� �ü���� ���ۿ� OVERLAPPED ����ü�� �����Ͽ� �����͸� ���� �����̴�.
�̷� ���� �� ����¸��� ���ο� OVERLAPPED ����ü�� ���� �Ҵ��Ѵ�(����� �ϷḦ ó�������� �޸𸮸� �����ϴ� �͵� ��������).
*/