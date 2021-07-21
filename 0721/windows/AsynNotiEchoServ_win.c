#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 100

void CompressSockets(SOCKET hSockArr[], int idx, int total); // ���� �迭 ����
void CompressEvents(WSAEVENT hEventArr[], int idx, int total); // �̺�Ʈ �迭 ����
void ErrorHandling(char* msg);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAddr, clntAddr;

	// WSAWaitForMultipleEvents �Լ��� ���ÿ� ������ �� �ִ� �ִ� event ������Ʈ�� ���� ��ũ�� WSA_MAXIMUM_WAIT_EVENTS �� ���ǵǾ� �ִ�.
	SOCKET hSockArr[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT hEventArr[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT newEvent; // event ������Ʈ
	WSANETWORKEVENTS netEvents; // �߻��� �̺�Ʈ�� ���������� ���������� ä���� ����ü ����

	int numOfClntSock = 0;
	int strLen, i;
	int posInfo, startIdx; // ��ġ����, ���� �ε��� ����
	int clntAddrSize;
	char msg[BUF_SIZE];

	if (argc != 2) {
		printf("usage %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

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

	// manual-reset ���(WaitForSingleObject �Լ��� ��ȯ�� �� non-signaled ���·� �������� �ʴ� ���)�̸鼭 non-signaled ������ event ������Ʈ ����
	newEvent = WSACreateEvent();

	// hServSock�� FD_ACCEPT �̺�Ʈ�� �߻��ϸ� newEvent Ŀ�� ������Ʈ�� signaled ���·� �����ϵ��� ����
	if (WSAEventSelect(hServSock, newEvent, FD_ACCEPT) == SOCKET_ERROR)
		ErrorHandling("WSAEventSelect() error");

	// ���ϰ� event ������Ʈ�� ������ġ�� ���Ͻ�Ŵ���ν� �� ���� ���踦 ����
	hSockArr[numOfClntSock] = hServSock;
	hEventArr[numOfClntSock] = newEvent;
	numOfClntSock++;

	while (1) {
		posInfo = WSAWaitForMultipleEvents(numOfClntSock, hEventArr, FALSE, WSA_INFINITE, FALSE); // ����° ���ڰ� FALSE ��� ���� ���� event ������Ʈ �� �ϳ��� signaled ���°� �Ǿ ��ȯ
		startIdx = posInfo - WSA_WAIT_EVENT_0; // ��ȯ�� ������ posInfo���� WSA_WAIT_EVENT_0�� ����, �� ��° �Ű������� ���޵� �迭�� ��������, signaled ���°� �� event ������Ʈ�� �ڵ��� ����� �ε����� ���ȴ�.

		// �� �̻��� event ������Ʈ�� signaled ���·� ���� �Ǿ��ٸ�, ���� ���� �ε����� ���Ǳ� ������ �ݺ����� ���� signaled ���·� ���̵� event ������Ʈ�� ��� ã�� �� �ִ�.
		for (i = startIdx; i < numOfClntSock; i++) {
			int sigEventIdx = WSAWaitForMultipleEvents(1, &hEventArr[i], TRUE, 0, FALSE);

			// WSA_WAIT_TIMEOUT = �ð��ʰ� ������ ����߰� fWaitAll �Ű�����(����° �Ű������� TRUE��� ��� event ������Ʈ�� signaled ������ �� ��ȯ)�� ������ ������ �������� �ʾҴ�.
			// WSAWaitForMultipleEvents�� �׹�° ���ڰ� 0�̸� �ٷ� ��ȯ�ϴµ� �� �� �ش� event ������Ʈ�� non-signaled ���¶�� continue
			if ((sigEventIdx == WSA_WAIT_FAILED || sigEventIdx == WSA_WAIT_TIMEOUT))
				continue;
			else {
				sigEventIdx = i; // sigEventIdx�� signaled ������ event ������Ʈ�� �ε��� ����

				// ���ڷ� ���޵� ���ϰ� ����� event ������Ʈ�� � ������ signaled ���°� �Ǿ����� netEvents�� ����
				// ���� WSAEnumNetworkEvents �Լ��� manual-reset ����� event ������Ʈ�� non-signaled ���·� �ǵ����� ���� �ϱ� ������ ResetEvent �Լ��� ȣ���� �ʿ䵵 ����.
				WSAEnumNetworkEvents(hSockArr[sigEventIdx], hEventArr[sigEventIdx], &netEvents);

				if (netEvents.lNetworkEvents & FD_ACCEPT) { // �����û�̸�
					if (netEvents.iErrorCode[FD_ACCEPT_BIT] != 0) { // ���� �߻�
						puts("accept error");
						break;
					}

					clntAddrSize = sizeof(clntAddr);
					hClntSock = accept(hSockArr[sigEventIdx], (SOCKADDR*)&clntAddr, &clntAddrSize);
					if (hClntSock == INVALID_SOCKET)
						ErrorHandling("accpet() error");

					// ���� ����� Ŭ���̾�Ʈ ���ϰ� event ������Ʈ�� ����(������ ����, ���� ��û)
					newEvent = WSACreateEvent();
					WSAEventSelect(hClntSock, newEvent, FD_READ | FD_CLOSE);

					// Ŭ���̾�Ʈ ���ϰ� event ������Ʈ ���� ���
					hEventArr[numOfClntSock] = newEvent;
					hSockArr[numOfClntSock] = hClntSock;
					numOfClntSock++;

					puts("connected new client...");
				}

				if (netEvents.lNetworkEvents & FD_READ) { // ������ ����
					if (netEvents.iErrorCode[FD_READ_BIT] != 0) { // ���� �߻�
						puts("read error");
						break;
					}

					strLen = recv(hSockArr[sigEventIdx], msg, sizeof(msg), 0);
					send(hSockArr[sigEventIdx], msg, strLen, 0); // echo
				}

				if (netEvents.lNetworkEvents & FD_CLOSE) { // ���� ��û
					if (netEvents.iErrorCode[FD_CLOSE_BIT] != 0) { // ���� �߻�
						puts("close error");
						break;
					}

					// event ������Ʈ, ���� ����
					WSACloseEvent(hEventArr[sigEventIdx]);
					closesocket(hSockArr[sigEventIdx]);

					// ���� ��û�� �������� ����� ���� 1 ���ְ� ���� �迭�� event ������Ʈ �迭�� ����
					numOfClntSock--;
					CompressSockets(hSockArr, sigEventIdx, numOfClntSock);
					CompressEvents(hEventArr, sigEventIdx, numOfClntSock);
				}
			}
		}
	}

	WSACleanup();
	return 0;
}

void CompressSockets(SOCKET hSockArr[], int idx, int total) {
	int i;

	for (i = idx; i < total; i++)
		hSockArr[i] = hSockArr[i + 1];
}

void CompressEvents(WSAEVENT hEventArr[], int idx, int total) {
	int i;

	for (i = idx; i < total; i++)
		hEventArr[i] = hEventArr[i + 1];
}

void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}