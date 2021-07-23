#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define BUF_SIZE 1024
void CALLBACK ReadCompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK WriteCompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void ErrorHandling(char* msg);

// ������ �ڵ�, ���۰��� ������ �ϳ��� ����ü�� ����. ���Ŀ� OVERLAPPED ����ü�� ��� hEvent�� ��
typedef struct {
	SOCKET hClntSock;
	char buf[BUF_SIZE];
	WSABUF wsaBuf;
}PER_IO_DATA, * LPPER_IO_DATA;

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hLisnSock, hRecvSock;
	SOCKADDR_IN lisnAddr, recvAddr;
	LPWSAOVERLAPPED lpOvLp;
	DWORD recvBytes;
	LPPER_IO_DATA hbInfo;
	int mode = 1, recvAddrSize, flagInfo = 0;

	if (argc != 2) {
		printf("usage: %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	// OVERLAPPED IO�� ������ ���� ����
	hLisnSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hLisnSock == INVALID_SOCKET)
		ErrorHandling("WSASocket() error");

	// non-blocking �������� ����(mode�� 0�̸� blocking, 0�� �ƴϸ� non-blocking) 
	ioctlsocket(hLisnSock, FIONBIO, &mode);

	memset(&lisnAddr, 0, sizeof(lisnAddr));
	lisnAddr.sin_family = AF_INET;
	lisnAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	lisnAddr.sin_port = htons(atoi(argv[1]));

	if (bind(hLisnSock, (SOCKADDR*)&lisnAddr, sizeof(lisnAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	if (listen(hLisnSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	recvAddrSize = sizeof(recvAddr);

	while (1) {
		// �ü���� completion routine�� ȣ���Ϸ��� �ش� �����尡 ���ͷ�Ʈ�� ������ alertable wait ���¿� �־�� �ϱ� ������ SleepEx �Լ� ȣ���Ͽ� alertable wait ���¿� �����Ѵ�.
		SleepEx(100, TRUE);

		// ������ �Ӽ��� non-blocking ���� ����Ǹ� ���� Ư¡�� ���ϰ� ��
		// 1. Ŭ���̾�Ʈ�� �����û�� �������� �ʴ� ���¿��� accpet �Լ��� ȣ��Ǹ� INVALID_SOCKET�� ��ٷ� ��ȯ�ǰ� �̾ WSAGetLastError �Լ��� ȣ���ϸ� WSAEWOULDBLOCK�� ��ȯ�ȴ�.
		// 2. accept �Լ�ȣ���� ���ؼ� ���� �����Ǵ� ���� ���� non-blocking �Ӽ��� ���Ѵ�.
		hRecvSock = accept(hLisnSock, (SOCKADDR*)&recvAddr, &recvAddrSize);
		if (hRecvSock == INVALID_SOCKET) {
			if (WSAGetLastError() == WSAEWOULDBLOCK) // ���� ��û�� �������� �ʴ� ������� continue
				continue;
			else
				ErrorHandling("accept() error");
		}

		puts("client connected...");

		// �ü���� ����� �������� OVERLAPPED ����ü�� ���� �����ϱ� ������ �� ����¸��� OVERLAPPED ����ü�� ���� �����ؾ� ��.
		lpOvLp = (LPWSAOVERLAPPED)malloc(sizeof(WSAOVERLAPPED));

		// ������ ����� �������� ���� ������ memset �Լ� ȣ�� ���������� ����
		// lpOvLp ���� ��ü�� ���������̱� ������ memset�� ù��° ���ڷ� �ּ� �����ڸ� ����� �ʿ䰡 ����.
		//memset(&lpOvLp, 0, sizeof(WSAOVERLAPPED));
		memset(lpOvLp, 0, sizeof(WSAOVERLAPPED));

		// hbInfo�� ���� �Ҵ��Ͽ� ���� �ڵ�� ���� ������ ����
		hbInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		//hbInfo->hClntSock = (DWORD)hRecvSock;
		hbInfo->hClntSock = hRecvSock;
		(hbInfo->wsaBuf).buf = hbInfo->buf;
		(hbInfo->wsaBuf).len = BUF_SIZE;

		// IO �Ϸ� ���� ���� ������� �̺�Ʈ ������Ʈ �ñ׳θ��� ������� �ʴ´ٸ� OVERLAPPED ����ü�� ��� hEvent�� ū �ǹ̸� ���� ���Ѵ�.
		// �׷��� ����� ���Ƿ� ����� �� �ִ�. ���⼭�� ���� �ڵ�� ���� ������ ��Ҵ�
		lpOvLp->hEvent = (HANDLE)hbInfo;

		// hRecvSock�� ���� �Է��� �Ϸ�Ǹ� RecvCompRoutine�� ȣ��ȴ�.
		// ���⼭ ���� ��° ���ڷ� ������ WSAOVERLAPPED ����ü ������ �ּ� ���� completion routine�� �� ��° �Ű������� ���޵ȴ�.
		// �ᱹ completion routine �Լ� ������ ������� �Ϸ�� ������ �ڵ�� ���ۿ� ������ �� �ִ�.
		WSARecv(hRecvSock, &(hbInfo->wsaBuf), 1, &recvBytes, &flagInfo, lpOvLp, ReadCompRoutine);
	}

	closesocket(hRecvSock);
	closesocket(hLisnSock);
	WSACleanup();
	return 0;
}

void CALLBACK ReadCompRoutine(DWORD dwError, DWORD szRecvBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags) {
	// completion routine ȣ��� OVERLAPPED ����ü �ּҰ� ���ڷ� ���޵Ǿ���, �� ����ü�� ��� hEvent�� ������ ������ �ڵ�� ���������� ���� �� �ִ�.
	LPPER_IO_DATA hbInfo = (LPPER_IO_DATA)(lpOverlapped->hEvent);
	SOCKET hSock = hbInfo->hClntSock;
	LPWSABUF bufInfo = &(hbInfo->wsaBuf);
	DWORD sentBytes;

	if (szRecvBytes == 0) { // EOF�� �����ߴٸ�
		closesocket(hSock);
		free(lpOverlapped->hEvent); // ������ �ڵ�� ���������� hbInfo�� ���� hEvent�� ���� �޸� �����Ѵ�.
		free(lpOverlapped); // OVERLAPPED ����ü�� �޸� �����Ѵ�.
		puts("client disconnected...");
	}
	else { // echo
		bufInfo->len = szRecvBytes; // ������ ����Ʈ�� �����ϰ� �׸�ŭ�� echo

		// WSASend �Լ��� echo�� �ϰ� ����� �Ϸ�Ǹ� WriteCompRoutine�� ȣ��ȴ�.
		// �̴� �۽� �Ŀ� ���� �޽����� �ޱ� ���� ��ƾ�̴�.
		WSASend(hSock, bufInfo, 1, &sentBytes, 0, lpOverlapped, WriteCompRoutine);
	}
}

void CALLBACK WriteCompRoutine(DWORD dwError, DWORD szRecvBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags) {
	LPPER_IO_DATA hbInfo = (LPPER_IO_DATA)(lpOverlapped->hEvent);
	SOCKET hSock = hbInfo->hClntSock;
	LPWSABUF bufInfo = &(hbInfo->wsaBuf);
	DWORD recvBytes;
	int flagInfo = 0;

	// �����͸� echo �ߴٸ� ���� �޽����� �ޱ� ���� �ٽ� WSARecv �Լ��� ȣ���Ѵ�.
	// �Է��� �Ϸ�Ǹ� ReadCompRoutine�� ȣ��Ǽ� echo�� �����Ѵ�.
	WSARecv(hSock, bufInfo, 1, &recvBytes, &flagInfo, lpOverlapped, ReadCompRoutine);
}

void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

/*
�� overlapeed IO ������ ����
non-blocking ����� accept �Լ��� alertable wait ���·��� ������ ���� SleepEx �Լ��� ������ ���� �ݺ� ȣ��Ǵ� ���� ���ɿ� ������ ��ĥ �� �ִ�.
������ �ϳ��� �ݺ��ؼ� non-blocking ���� accpet �Լ��� ȣ���ϰ� �ִµ�,
�� ���߿� �ü���� �޽����� �ޱ� ���� ���ͷ�Ʈ ������ alertable wait ���·� �����ϱ� ���� SleepEx �Լ��� �ݺ� ȣ���ϱ� ����

���� �ذ� ���
accpet �Լ��� ȣ���� main �����尡(main �Լ� ������) ó���ϵ��� �ϰ�, ������ �����带 �߰��� �ϳ� �����ؼ� Ŭ���̾�Ʈ���� ������� ����ϰ� �Ѵ�.
�̰��� �ٷ� IOCP���� �����ϴ� ������ ���� �𵨷�, IOCP������ IO�� �����ϴ� �����带 ������ �����Ѵ�.
*/