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

	// 인자가 2개가 아니면 exit
	if (argc != 2) {
		printf("Usage %s <port>\n", argv[0]);
		exit(1);
	}

	// winsock 초기화
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	// completion port 생성(이후에 소켓을 이 오브젝트와 연결)
	// 마지막 변수 NumberOfConcurrentThreads = 0을 넘기면 시스템의 프로세서만큼 cp오브젝트에 연결할 스레드 수 허용
	hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, MAX_THREAD);

	// 시스템 정보를 받아서 sysInfo 변수에 저장
	GetSystemInfo(&sysInfo);

	// 스레드 생성
	// c++에서 스레드 생성할 때 join을 사용해서 스레드가 종료하는 것을 확인하겠다고 명시해야함
	// 명시하지 않으면 abort() has been called 에러 발생
	for (i = 0; i < sysInfo.dwNumberOfProcessors; i++)
		threadArr[i] = new std::thread(EchoThreadMain, hComPort);

	// overlapped가 가능한 서버 소켓 생성
	hServSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hServSock == INVALID_SOCKET)
		ErrorHandling("WSASocet() error");

	// 서버주소 구조체의 ipv4, 시스템 ip주소, 포트번호 초기화
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));

	// 서버소켓에 지정한 ip주소와 포트번호를 바인딩(운영체제에 등록해야 클라이언트가 보내는 패킷을 수신할 수 있음)
	if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	// 소켓을 수신상태로 둔다(대기 큐의 크기는 5)
	if (listen(hServSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	while (1) {
		SOCKET hClntSock;
		SOCKADDR_IN clntAddr;
		int clntAddrSize = sizeof(clntAddr);

		// 클라이언트의 연결요청을 수락하고 통신하기 위한 전용 소켓 생성
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &clntAddrSize);
		if (hClntSock == INVALID_SOCKET)
			ErrorHandling("accept() error");

		// 클라이언트 정보 추가
		clntArr[numOfClnt] = hClntSock;
		numOfClnt++;

		// 클라이언트 소켓과 주소를 저장할 clntInfo 구조체 초기화
		clntInfo = (LP_CLNT_INFO)malloc(sizeof(CLNT_INFO));
		clntInfo->hClntSock = hClntSock;
		memcpy(&(clntInfo->clntAddr), &clntAddr, clntAddrSize);

		// completion port 커널 오브젝트에 클라이언트 소켓 연결
		CreateIoCompletionPort((HANDLE)hClntSock, hComPort, (DWORD)clntInfo, 0);

		// 데이터 송수신에 관한 정보를 저장할 ioInfo 구조체 초기화
		ioInfo = (LP_IO_INFO)malloc(sizeof(IO_INFO));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		ioInfo->wsaBuf.len = BUF_SIZE;
		ioInfo->wsaBuf.buf = ioInfo->buffer;
		ioInfo->rwMode = READ;

		// 클라이언트로부터 오는 메시지를 먼저 수신
		// 6번째 인자로 ioInfo 구조체의 overlapped 주소를 전달하게 되면 ioInfo 구조체의 다른 변수들도 접근 가능하다.
		// 왜냐하면 ioInfo 구조체의 주소가 이 구조체의 첫번째 변수인 overlapped 주소와 동일하기 때문
		WSARecv(clntInfo->hClntSock, &(ioInfo->wsaBuf), 1, &dwRecvBytes, &dwFlags, &(ioInfo->overlapped), NULL);
	}

	// 생성한 스레드가 종료될 때 까지 현재 스레드(main thread)를 블로킹시킨다.
	// 스레드가 종료되면 join()함수가 반환되고 모든 스레드가 종료되면 모든 join()함수가 반환되어 main thread가 계속 진행
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
		// IO Completion Queue에 완료통지 항목이 존재하면 GetQueuedCompletionStatus 함수가 반환한다.
		// 반환하면서 3번째 인자(clntInfo)에는 CreateIoCompletionPort함수의 3번째 인자(clntInfo)가 전달되고
		// 4번째인자(ioInfo)에는 WSARecv 또는 WSASend의 6번째 인자&(ioInfo->overlapped)가 전달된다.
		GetQueuedCompletionStatus(hComPort, &bytesTrans, (LPDWORD)&clntInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);

		// clntInfo 구조체에서 클라이언트 소켓 정보 가져옴
		sock = clntInfo->hClntSock;

		// 데이터의 수신이라면
		if (ioInfo->rwMode == READ) {
			puts("message received");

			// bytesTrans가 0이라는 것은 EOF가 전송된 것
			if (bytesTrans == 0) {
				
				// 소켓을 닫고 동적할당한 메모리(클라이언트 정보, IO 정보)를 해제
				closesocket(sock);
				free(clntInfo);
				free(ioInfo);

				// 소켓 연결을 종료한 클라이언트는 메시지를 보낼 필요가 없기 때문에 정보 제거
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

			// 수신한 데이터를 전송하기 위해 복사 후 메모리 해제
			strcpy(temp_buf, ioInfo->buffer);
			free(ioInfo);

			// 서버에 접속한 모든 클라이언트에게 채팅 전송
			// 매 출력마다 새로운 OVERLAPPED 구조체 동적 할당
			for (i = 0; i < numOfClnt; i++) {
				ioInfo = (LP_IO_INFO)malloc(sizeof(IO_INFO));
				memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
				ioInfo->wsaBuf.len = bytesTrans;
				ioInfo->wsaBuf.buf = temp_buf;
				ioInfo->rwMode = WRITE;
				WSASend(clntArr[i], &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
			}
			
			// 채팅을 전송했으면 또 다른 채팅을 받을 수 있어야함
			ioInfo = (LP_IO_INFO)malloc(sizeof(IO_INFO));
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = BUF_SIZE; // 받는 입장에서 버퍼 크기가 작으면 여러번 나눠 받아야함
			ioInfo->wsaBuf.buf = ioInfo->buffer;
			ioInfo->rwMode = READ;
			WSARecv(sock, &(ioInfo->wsaBuf), 1, NULL, &flags, &(ioInfo->overlapped), NULL);
		}
		else {
			// 출력이 완료되면 IO정보는 메모리 해제한다.
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