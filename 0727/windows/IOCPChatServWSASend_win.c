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

		// 클라이언트 정보 추가
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

				// 클라이언트 정보 제거
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

			// 수신한 데이터 복사 후 메모리 해제
			//memcpy(temp_buf, ioInfo->buffer, sizeof(BUF_SIZE));
			strcpy(temp_buf, ioInfo->buffer);
			free(ioInfo);

			// 서버에 접속한 모든 클라이언트에게 채팅 전송
			// 매 출력마다 새로운 OVERLAPPED 구조체 동적 할당
			for (i = 0; i < numOfClnt; i++) {
				ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
				memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
				ioInfo->wsaBuf.len = bytesTrans;
				ioInfo->wsaBuf.buf = temp_buf;
				ioInfo->rwMode = WRITE;
				WSASend(clntArr[i], &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
			}

			// 입력에 대해서도 새로운 OVERLAPPED 구조체 동적 할당
			ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = BUF_SIZE;
			ioInfo->wsaBuf.buf = ioInfo->buffer;
			ioInfo->rwMode = READ;
			WSARecv(sock, &(ioInfo->wsaBuf), 1, NULL, &flags, &(ioInfo->overlapped), NULL);
		}
		else {
			// 출력이 끝났기 때문에 메모리 해제
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
IOCP 장점
입출력이 완료되었을 때, 그 입출력을 실행한 스레드가 아닌 다른 스레드가 해당 입출력 완료 통지를 처리할 수 있다(CPU를 최대한으로 사용가능하게 함).
non-blocking으로 동작하기 때문에 IO 작업에서 시간 지연이 없다.


주의할 점
비동기 IO 요청을 수행할 때 사용되는 데이터 버퍼와 OVERLAPPED 구조체는 IO 요청이 완료될 때까지 옮겨지거나 삭제되지 않아야 한다.
입출력 과정 그리고 입출력이 끝난 후에 운영체제가 버퍼와 OVERLAPPED 구조체를 참조하여 데이터를 쓰기 때문이다.
이로 인해 매 입출력마다 새로운 OVERLAPPED 구조체를 동적 할당한다(입출력 완료를 처리했으면 메모리를 해제하는 것도 잊지말자).
*/