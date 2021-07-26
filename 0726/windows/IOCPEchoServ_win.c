#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <winsock2.h>
#include <windows.h>
#include <string.h>

#define BUF_SIZE 100
#define READ 3
#define WRITE 5

typedef struct { // 소켓 정보
	SOCKET hClntSock;
	SOCKADDR_IN clntAddr;
}PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

typedef struct { // 버퍼 정보
	OVERLAPPED overlapped; // 구조체 변수 주소 값은 구조체 첫 번째 멤버의 주소 값과 일치
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

	// cp 오브젝트 생성
	// 마지막 인자로 0을 전달했기 때문에, pc에 설치된 CPU의 개수를 동시에 수행 가능한 스레드의 최대 개수로 설정한다. 내 pc의 경우 6개
	hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	// 현재 실행중인 시스템 정보 얻기
	GetSystemInfo(&sysInfo);

	// cpu 수만큼 스레드를 생성하고, 스레드 생성시 cp 오브젝트의 핸들을 인자로 전달한다.
	for (i = 0; i < sysInfo.dwNumberOfProcessors; i++)
		_beginthreadex(NULL, 0, EchoThreadMain, (LPVOID)hComPort, 0, NULL);

	// OVERLAPPED IO가 가능한 소켓 생성
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

		// PER_HANDLE_DATA 구조체 변수를 동적 할당하고 클라이언트 소켓 정보를 담는다.
		handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		handleInfo->hClntSock = hClntSock;
		memcpy(&(handleInfo->clntAddr), &clntAddr, clntAddrSize);

		// 클라이언트 소켓(장치)과 cp 오브젝트를 연결한다.
		// 3번째 인자인 handleInfo는 CompletionKey로 GetQueuedCompletionStatus 함수가 반환하면서 얻을 수 있다.
		CreateIoCompletionPort((HANDLE)hClntSock, hComPort, (DWORD)handleInfo, 0);

		// IOCP는 입출력을 구분하지 않고 단지 입출력이 완료되었다는 통지만 해주기 때문에 read인지 write인지를 기록해둬야 한다.
		ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		ioInfo->wsaBuf.len = BUF_SIZE;
		ioInfo->wsaBuf.buf = ioInfo->buffer;
		ioInfo->rwMode = READ;

		// WSARecv 함수를 호출하면서 6번째 인자로 overlapped 구조체 변수의 주소 값을 전달했다.
		// 이 값은 GetQueuedCompletionStatus 함수가 반환하면서 얻을 수 있다.
		// 그런데 구조체 변수의 주소 값은 첫 번째 멤버의 주소 값과 동일하므로 PER_IO_DATA 구조체 변수의 주소 값을 전달한 것과 같다.
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
		// IO가 완료되어 IO Completion Queue에 항목이 추가되면 반환되는데
		// 이 때 송수신된 바이트 수, completion key, overlappd 구조체의 주소를 가져온다.
		// 여기서는 completion key에 handleInfo를 overlapped 구조체에 ioInfo를 가져왔다.
		GetQueuedCompletionStatus(hComPort, &bytesTrans, (LPDWORD)&handleInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);

		sock = handleInfo->hClntSock;

		if (ioInfo->rwMode == READ) {
			puts("message received");
			if (bytesTrans == 0) { // EOF라면
				closesocket(sock);
				free(handleInfo);
				free(ioInfo);
				continue;
			}

			// 데이터를 받았다면 echo 해준다.
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = bytesTrans;
			ioInfo->rwMode = WRITE;
			WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);

			// echo한 후에는 다음 데이터를 받기위해 WSARecv 함수를 호출한다.
			ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = BUF_SIZE;
			ioInfo->wsaBuf.buf = ioInfo->buffer;
			ioInfo->rwMode = READ;
			WSARecv(sock, &(ioInfo->wsaBuf), 1, NULL, &flags, &(ioInfo->overlapped), NULL);
		}
		else {
			// echo 후 해당 입출력에 사용된 ioInfo는 메모리 해제한다.
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