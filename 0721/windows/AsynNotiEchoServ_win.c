#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 100

void CompressSockets(SOCKET hSockArr[], int idx, int total); // 소켓 배열 갱신
void CompressEvents(WSAEVENT hEventArr[], int idx, int total); // 이벤트 배열 갱신
void ErrorHandling(char* msg);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAddr, clntAddr;

	// WSAWaitForMultipleEvents 함수가 동시에 관찰할 수 있는 최대 event 오브젝트의 수는 매크로 WSA_MAXIMUM_WAIT_EVENTS 로 정의되어 있다.
	SOCKET hSockArr[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT hEventArr[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT newEvent; // event 오브젝트
	WSANETWORKEVENTS netEvents; // 발생한 이벤트의 유형정보와 오류정보로 채워질 구조체 변수

	int numOfClntSock = 0;
	int strLen, i;
	int posInfo, startIdx; // 위치정보, 시작 인덱스 변수
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

	// manual-reset 모드(WaitForSingleObject 함수가 반환될 때 non-signaled 상태로 변경하지 않는 모드)이면서 non-signaled 상태인 event 오브젝트 생성
	newEvent = WSACreateEvent();

	// hServSock에 FD_ACCEPT 이벤트가 발생하면 newEvent 커널 오브젝트를 signaled 상태로 변경하도록 연결
	if (WSAEventSelect(hServSock, newEvent, FD_ACCEPT) == SOCKET_ERROR)
		ErrorHandling("WSAEventSelect() error");

	// 소켓과 event 오브젝트의 저장위치를 통일시킴으로써 이 둘의 관계를 유지
	hSockArr[numOfClntSock] = hServSock;
	hEventArr[numOfClntSock] = newEvent;
	numOfClntSock++;

	while (1) {
		posInfo = WSAWaitForMultipleEvents(numOfClntSock, hEventArr, FALSE, WSA_INFINITE, FALSE); // 세번째 인자가 FALSE 라는 것은 여러 event 오브젝트 중 하나만 signaled 상태가 되어도 반환
		startIdx = posInfo - WSA_WAIT_EVENT_0; // 반환된 정수값 posInfo에서 WSA_WAIT_EVENT_0를 빼면, 두 번째 매개변수로 전달된 배열을 기준으로, signaled 상태가 된 event 오브젝트의 핸들이 저장된 인덱스가 계산된다.

		// 둘 이상의 event 오브젝트가 signaled 상태로 전이 되었다면, 가장 작은 인덱스가 계산되기 때문에 반복문을 통해 signaled 상태로 전이된 event 오브젝트를 모두 찾을 수 있다.
		for (i = startIdx; i < numOfClntSock; i++) {
			int sigEventIdx = WSAWaitForMultipleEvents(1, &hEventArr[i], TRUE, 0, FALSE);

			// WSA_WAIT_TIMEOUT = 시간초과 간격이 경과했고 fWaitAll 매개변수(세번째 매개변수로 TRUE라면 모든 event 오브젝트가 signaled 상태일 때 반환)에 지정된 조건이 충족되지 않았다.
			// WSAWaitForMultipleEvents의 네번째 인자가 0이면 바로 반환하는데 이 때 해당 event 오브젝트가 non-signaled 상태라면 continue
			if ((sigEventIdx == WSA_WAIT_FAILED || sigEventIdx == WSA_WAIT_TIMEOUT))
				continue;
			else {
				sigEventIdx = i; // sigEventIdx에 signaled 상태인 event 오브젝트의 인덱스 저장

				// 인자로 전달된 소켓과 연결된 event 오브젝트가 어떤 이유로 signaled 상태가 되었는지 netEvents에 저장
				// 또한 WSAEnumNetworkEvents 함수는 manual-reset 모드의 event 오브젝트를 non-signaled 상태로 되돌리기 까지 하기 때문에 ResetEvent 함수를 호출할 필요도 없다.
				WSAEnumNetworkEvents(hSockArr[sigEventIdx], hEventArr[sigEventIdx], &netEvents);

				if (netEvents.lNetworkEvents & FD_ACCEPT) { // 연결요청이며
					if (netEvents.iErrorCode[FD_ACCEPT_BIT] != 0) { // 오류 발생
						puts("accept error");
						break;
					}

					clntAddrSize = sizeof(clntAddr);
					hClntSock = accept(hSockArr[sigEventIdx], (SOCKADDR*)&clntAddr, &clntAddrSize);
					if (hClntSock == INVALID_SOCKET)
						ErrorHandling("accpet() error");

					// 새로 연결된 클라이언트 소켓과 event 오브젝트를 연결(데이터 수신, 종료 요청)
					newEvent = WSACreateEvent();
					WSAEventSelect(hClntSock, newEvent, FD_READ | FD_CLOSE);

					// 클라이언트 소켓과 event 오브젝트 정보 등록
					hEventArr[numOfClntSock] = newEvent;
					hSockArr[numOfClntSock] = hClntSock;
					numOfClntSock++;

					puts("connected new client...");
				}

				if (netEvents.lNetworkEvents & FD_READ) { // 데이터 수신
					if (netEvents.iErrorCode[FD_READ_BIT] != 0) { // 오류 발생
						puts("read error");
						break;
					}

					strLen = recv(hSockArr[sigEventIdx], msg, sizeof(msg), 0);
					send(hSockArr[sigEventIdx], msg, strLen, 0); // echo
				}

				if (netEvents.lNetworkEvents & FD_CLOSE) { // 종료 요청
					if (netEvents.iErrorCode[FD_CLOSE_BIT] != 0) { // 오류 발생
						puts("close error");
						break;
					}

					// event 오브젝트, 소켓 종료
					WSACloseEvent(hEventArr[sigEventIdx]);
					closesocket(hSockArr[sigEventIdx]);

					// 종료 요청이 들어왔으니 사용자 수를 1 빼주고 소켓 배열과 event 오브젝트 배열을 갱신
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