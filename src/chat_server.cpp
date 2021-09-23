#pragma warning(disable:4996)
#include <cstdio>
#include <WinSock2.h>
#include <thread>
#include <cstring>
#include <cstdlib>

#define BUF_SIZE 1024
#define READ 3
#define WRITE 5
#define MAX_CLNT 200
#define MAX_THREAD 18

typedef struct {
	SOCKET hClntSock;
	SOCKADDR_IN clntAddr;
}CLNT_INFO, * LP_CLNT_INFO;

typedef struct {
	OVERLAPPED overlapped;
	WSABUF wsaBuf;
	char buffer[BUF_SIZE];
	int rwMode;
}IO_INFO, * LP_IO_INFO;

void EchoThreadMain(HANDLE hComPort);
void ErrorHandling(const char* msg);

int numOfClnt;
SOCKET clntArr[MAX_CLNT];

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	HANDLE hComPort;
	SYSTEM_INFO sysInfo;
	LP_IO_INFO ioInfo;
	LP_CLNT_INFO clntInfo;

	SOCKET hServSock;
	SOCKADDR_IN servAddr;
	int i = 0;
	DWORD dwRecvBytes = 0, dwFlags = 0;
	std::thread* threadArr[MAX_THREAD];

	// ���ڰ� 2���� �ƴϸ� exit
	if (argc != 2) {
		printf("Usage %s <port>\n", argv[0]);
		exit(1);
	}

	// winsock �ʱ�ȭ
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	// completion port ����(���Ŀ� ������ �� ������Ʈ�� ����)
	// ������ ���� NumberOfConcurrentThreads = 0�� �ѱ�� �ý����� ���μ�����ŭ cp������Ʈ�� ������ ������ �� ���
	hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, MAX_THREAD);

	// �ý��� ������ �޾Ƽ� sysInfo ������ ����
	GetSystemInfo(&sysInfo);

	// ������ ����
	// c++���� ������ ������ �� join�� ����ؼ� �����尡 �����ϴ� ���� Ȯ���ϰڴٰ� ����ؾ���
	// ������� ������ abort() has been called ���� �߻�
	for (i = 0; i < sysInfo.dwNumberOfProcessors; i++)
		threadArr[i] = new std::thread(EchoThreadMain, hComPort);

	// overlapped�� ������ ���� ���� ����
	hServSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hServSock == INVALID_SOCKET)
		ErrorHandling("WSASocet() error");

	// �����ּ� ����ü�� ipv4, �ý��� ip�ּ�, ��Ʈ��ȣ �ʱ�ȭ
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));

	// �������Ͽ� ������ ip�ּҿ� ��Ʈ��ȣ�� ���ε�(�ü���� ����ؾ� Ŭ���̾�Ʈ�� ������ ��Ŷ�� ������ �� ����)
	if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	// ������ ���Ż��·� �д�(��� ť�� ũ��� 5)
	if (listen(hServSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	while (1) {
		SOCKET hClntSock;
		SOCKADDR_IN clntAddr;
		int clntAddrSize = sizeof(clntAddr);

		// Ŭ���̾�Ʈ�� �����û�� �����ϰ� ����ϱ� ���� ���� ���� ����
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &clntAddrSize);
		if (hClntSock == INVALID_SOCKET)
			ErrorHandling("accept() error");

		// Ŭ���̾�Ʈ ���� �߰�
		clntArr[numOfClnt] = hClntSock;
		numOfClnt++;

		// Ŭ���̾�Ʈ ���ϰ� �ּҸ� ������ clntInfo ����ü �ʱ�ȭ
		clntInfo = (LP_CLNT_INFO)malloc(sizeof(CLNT_INFO));
		clntInfo->hClntSock = hClntSock;
		memcpy(&(clntInfo->clntAddr), &clntAddr, clntAddrSize);

		// completion port Ŀ�� ������Ʈ�� Ŭ���̾�Ʈ ���� ����
		CreateIoCompletionPort((HANDLE)hClntSock, hComPort, (DWORD)clntInfo, 0);

		// ������ �ۼ��ſ� ���� ������ ������ ioInfo ����ü �ʱ�ȭ
		ioInfo = (LP_IO_INFO)malloc(sizeof(IO_INFO));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		ioInfo->wsaBuf.len = BUF_SIZE;
		ioInfo->wsaBuf.buf = ioInfo->buffer;
		ioInfo->rwMode = READ;

		// Ŭ���̾�Ʈ�κ��� ���� �޽����� ���� ����
		// 6��° ���ڷ� ioInfo ����ü�� overlapped �ּҸ� �����ϰ� �Ǹ� ioInfo ����ü�� �ٸ� �����鵵 ���� �����ϴ�.
		// �ֳ��ϸ� ioInfo ����ü�� �ּҰ� �� ����ü�� ù��° ������ overlapped �ּҿ� �����ϱ� ����
		WSARecv(clntInfo->hClntSock, &(ioInfo->wsaBuf), 1, &dwRecvBytes, &dwFlags, &(ioInfo->overlapped), NULL);
	}

	// ������ �����尡 ����� �� ���� ���� ������(main thread)�� ���ŷ��Ų��.
	// �����尡 ����Ǹ� join()�Լ��� ��ȯ�ǰ� ��� �����尡 ����Ǹ� ��� join()�Լ��� ��ȯ�Ǿ� main thread�� ��� ����
	for (i = 0; i < sysInfo.dwNumberOfProcessors; i++)
		threadArr[i]->join();
}

void EchoThreadMain(HANDLE hComPort) {
	SOCKET sock;
	DWORD bytesTrans;
	LP_CLNT_INFO clntInfo;
	LP_IO_INFO ioInfo;
	DWORD flags = 0;
	int i, j;
	char temp_buf[BUF_SIZE];

	while (1) {
		// IO Completion Queue�� �Ϸ����� �׸��� �����ϸ� GetQueuedCompletionStatus �Լ��� ��ȯ�Ѵ�.
		// ��ȯ�ϸ鼭 3��° ����(clntInfo)���� CreateIoCompletionPort�Լ��� 3��° ����(clntInfo)�� ���޵ǰ�
		// 4��°����(ioInfo)���� WSARecv �Ǵ� WSASend�� 6��° ����&(ioInfo->overlapped)�� ���޵ȴ�.
		GetQueuedCompletionStatus(hComPort, &bytesTrans, (LPDWORD)&clntInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);

		// clntInfo ����ü���� Ŭ���̾�Ʈ ���� ���� ������
		sock = clntInfo->hClntSock;

		// �������� �����̶��
		if (ioInfo->rwMode == READ) {
			puts("message received");

			// bytesTrans�� 0�̶�� ���� EOF�� ���۵� ��
			if (bytesTrans == 0) {
				
				// ������ �ݰ� �����Ҵ��� �޸�(Ŭ���̾�Ʈ ����, IO ����)�� ����
				closesocket(sock);
				free(clntInfo);
				free(ioInfo);

				// ���� ������ ������ Ŭ���̾�Ʈ�� �޽����� ���� �ʿ䰡 ���� ������ ���� ����
				for (i = 0; i < numOfClnt; i++) {
					if (clntArr[i] == sock) {
						for (j = i; j < numOfClnt; j++) {
							clntArr[j] = clntArr[j + 1];
						}
						break;
					}
				}

				numOfClnt--;
				continue;
			}

			// ������ �����͸� �����ϱ� ���� ���� �� �޸� ����
			strcpy(temp_buf, ioInfo->buffer);
			free(ioInfo);

			// ������ ������ ��� Ŭ���̾�Ʈ���� ä�� ����
			// �� ��¸��� ���ο� OVERLAPPED ����ü ���� �Ҵ�
			for (i = 0; i < numOfClnt; i++) {
				ioInfo = (LP_IO_INFO)malloc(sizeof(IO_INFO));
				memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
				ioInfo->wsaBuf.len = bytesTrans;
				ioInfo->wsaBuf.buf = temp_buf;
				ioInfo->rwMode = WRITE;
				WSASend(clntArr[i], &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
			}
			
			// ä���� ���������� �� �ٸ� ä���� ���� �� �־����
			ioInfo = (LP_IO_INFO)malloc(sizeof(IO_INFO));
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = BUF_SIZE; // �޴� ���忡�� ���� ũ�Ⱑ ������ ������ ���� �޾ƾ���
			ioInfo->wsaBuf.buf = ioInfo->buffer;
			ioInfo->rwMode = READ;
			WSARecv(sock, &(ioInfo->wsaBuf), 1, NULL, &flags, &(ioInfo->overlapped), NULL);
		}
		else {
			// ����� �Ϸ�Ǹ� IO������ �޸� �����Ѵ�.
			puts("message sent");
			free(ioInfo);
		}
	}
	return;
}

void ErrorHandling(const char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}