#pragma warning (disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
void ErrorHandling(char* msg);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hSocket;
	SOCKADDR_IN sendAddr;

	WSABUF dataBuf; // WSASend �Լ��� ����� �� �� ��° ���ڷ� �ѱ��, ������ ������ ������ ���ϴ� ����ü
	char msg[] = "overlapped hEvent member!";
	int sendBytes = 0; // ���۵� ����Ʈ ���� ����� ����

	WSAEVENT evObj; // overlapped ����ü�� hEvent ����� ���
	WSAOVERLAPPED overlapped; // overlapped IO�� �����ϱ� ���� ���Ǵ� overlapped ����ü

	if (argc != 3) {
		printf("usage: %s <ip> <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	hSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED); // overlapped IO�� ������ non-blocking ����� ���� ����
	if (hSocket == INVALID_SOCKET)
		ErrorHandling("WSASocket() error");

	memset(&sendAddr, 0, sizeof(sendAddr));
	sendAddr.sin_family = AF_INET;
	sendAddr.sin_addr.s_addr = inet_addr(argv[1]);
	sendAddr.sin_port = htons(atoi(argv[2]));

	if (connect(hSocket, (SOCKADDR*)&sendAddr, sizeof(sendAddr)) == SOCKET_ERROR)
		ErrorHandling("connect() error");

	// event ������Ʈ ����
	evObj = WSACreateEvent();

	// overlapped ����ü �ʱ�ȭ �� hEvent����� event ������Ʈ ����
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = evObj;

	// ������ ������ ���� ����
	dataBuf.len = strlen(msg) + 1;
	dataBuf.buf = msg;

	// dataBuf ������ ������ ����, ���۵� ����Ʈ ���� sendBytes�� ����(��¹��۰� ����ְ� ������ ũ�Ⱑ �۴ٸ� �Լ� ȣ��� ���ÿ� �������� ������ �Ϸ�� ���� �ִ�)
	if (WSASend(hSocket, &dataBuf, 1, &sendBytes, 0, &overlapped, NULL) == SOCKET_ERROR) {
		if (WSAGetLastError() == WSA_IO_PENDING) { // ������ ������ �Ϸ���� �ʾҰ� ���� ������
			puts("Background data send");
			WSAWaitForMultipleEvents(1, &evObj, TRUE, WSA_INFINITE, FALSE); // ������ ������ �Ϸ�Ǹ� evObj�� signaled ���·� ����ǰ�, �� ���� �Ǿ�� �Լ��� ��ȯ
			WSAGetOverlappedResult(hSocket, &overlapped, &sendBytes, FALSE, NULL); // ������ ���۰�� Ȯ��(sendBytes�� ���۵� ����Ʈ �� ����)
		}
		else {
			// printf("%d\n", WSAGetLastError()); // ���� 6(WSA_INVALID_HANDLE, ������ �̺�Ʈ ��ü �ڵ��� �߸� �Ǿ����ϴ�.)
			// WSACreateEvent �Լ��� ȣ���� �� ()�� ������ �ʾƼ� ���� �����̴�.
			// �����Ϸ��� ��ȣ�� �� ������� �ʾ�����?
			// �Լ��� �ּҸ� evObj�� �����Ѵٴ� �ǹ̷� �ؼ��� �� ����.
			// https://stackoverflow.com/questions/11082329/why-doesnt-the-c-compiler-complain-when-i-use-functions-without-parentheses
			ErrorHandling("WSASend() error");
		}
	}

	printf("send data size: %d\n", sendBytes);
	WSACloseEvent(evObj);
	closesocket(hSocket);
	WSACleanup();

	return 0;
}

void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}