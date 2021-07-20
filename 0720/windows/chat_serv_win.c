#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>

#define BUF_SIZE 100
#define MAX_CLNT 256

unsigned WINAPI HandleClnt(void* arg);
void SendMsg(char* msg, int len);
void ErrorHandling(char* msg);

int clntCnt = 0;
SOCKET clntSocks[MAX_CLNT];
HANDLE hMutex; // ����ȭ�ϱ� ���� mutex Ŀ�� ������Ʈ

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAddr, clntAddr;
	int clntAddrSize;
	HANDLE hThread;

	if (argc != 2) {
		printf("usage %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	hMutex = CreateMutex(NULL, FALSE, NULL); // �ι�° ���ڰ� FALSE �̱� ������ �����Ǵ� mutex Ŀ�� ������Ʈ�� �����ڰ� �������� ������, signaled ���·� ����

	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	if (hServSock == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));

	if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	if (listen(hServSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	while (1) {
		clntAddrSize = sizeof(clntAddr);

		hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &clntAddrSize);
		if (hClntSock == INVALID_SOCKET)
			ErrorHandling("accept() error");

		WaitForSingleObject(hMutex, INFINITE); // �Ӱ� ���� ������ ���� mutex Ŀ�� ������Ʈ ����
		clntSocks[clntCnt++] = hClntSock; // ���� ������ Ŭ���̾�Ʈ ���� ���
		ReleaseMutex(hMutex); // mutex Ŀ�� ������Ʈ �ݳ�

		hThread = (HANDLE)_beginthreadex(NULL, 0, HandleClnt, (void*)&hClntSock, 0, NULL);
		printf("connected client ip: %s\n", inet_ntoa(clntAddr.sin_addr));
	}

	closesocket(hServSock);
	WSACleanup();
	return 0;
}

unsigned WINAPI HandleClnt(void* arg) {
	SOCKET hClntSock = *((SOCKET*)arg);
	int strLen = 0, i;
	char msg[BUF_SIZE];

	while ((strLen = recv(hClntSock, msg, sizeof(msg), 0)) != 0)
		SendMsg(msg, strLen);

	// while���� Ż���ؼ� �Ѿ�Դٴ� ���� ���κ��� EOF�� �޾Ҵٴ� ���̹Ƿ� �ش� Ŭ���̾�Ʈ ������ �����ؾ� �Ѵ�.
	WaitForSingleObject(hMutex, INFINITE);
	for (i = 0; i < clntCnt; i++) {
		if (hClntSock == clntSocks[i]) {
			for (int j = i; j < clntCnt - 1; j++)
				clntSocks[j] = clntSocks[j + 1];

			break;
		}
	}

	clntCnt--;
	ReleaseMutex(hMutex);
	closesocket(hClntSock);
	return 0;
}

void SendMsg(char* msg, int len) {
	int i;

	WaitForSingleObject(hMutex, INFINITE);
	for (i = 0; i < clntCnt; i++)
		send(clntSocks[i], msg, len, 0);
	ReleaseMutex(hMutex);
}

void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}