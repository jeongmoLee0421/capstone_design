#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define BUF_SIZE 1024
void CALLBACK ReadCompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK WriteCompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void ErrorHandling(char* msg);

// 소켓의 핸들, 버퍼관련 정보를 하나의 구조체로 묶음. 이후에 OVERLAPPED 구조체의 멤버 hEvent에 들어감
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

	// OVERLAPPED IO가 가능한 소켓 생성
	hLisnSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hLisnSock == INVALID_SOCKET)
		ErrorHandling("WSASocket() error");

	// non-blocking 소켓으로 변경(mode가 0이면 blocking, 0이 아니면 non-blocking) 
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
		// 운영체제가 completion routine을 호출하려면 해당 스레드가 인터럽트가 가능한 alertable wait 상태에 있어야 하기 때문에 SleepEx 함수 호출하여 alertable wait 상태에 진입한다.
		SleepEx(100, TRUE);

		// 소켓의 속성이 non-blocking 모드로 변경되면 다음 특징을 지니게 됨
		// 1. 클라이언트의 연결요청이 존재하지 않는 상태에서 accpet 함수가 호출되면 INVALID_SOCKET이 곧바로 반환되고 이어서 WSAGetLastError 함수를 호출하면 WSAEWOULDBLOCK이 반환된다.
		// 2. accept 함수호출을 통해서 새로 생성되는 소켓 역시 non-blocking 속성을 지닌다.
		hRecvSock = accept(hLisnSock, (SOCKADDR*)&recvAddr, &recvAddrSize);
		if (hRecvSock == INVALID_SOCKET) {
			if (WSAGetLastError() == WSAEWOULDBLOCK) // 연결 요청이 존재하지 않는 오류라면 continue
				continue;
			else
				ErrorHandling("accept() error");
		}

		puts("client connected...");

		// 운영체제가 입출력 과정에서 OVERLAPPED 구조체의 값을 변경하기 때문에 매 입출력마다 OVERLAPPED 구조체를 새로 생성해야 함.
		lpOvLp = (LPWSAOVERLAPPED)malloc(sizeof(WSAOVERLAPPED));

		// 서버가 제대로 동작하지 않은 이유는 memset 함수 호출 과정에서의 문제
		// lpOvLp 변수 자체가 포인터형이기 때문에 memset의 첫번째 인자로 주소 연산자를 사용할 필요가 없다.
		//memset(&lpOvLp, 0, sizeof(WSAOVERLAPPED));
		memset(lpOvLp, 0, sizeof(WSAOVERLAPPED));

		// hbInfo를 동적 할당하여 소켓 핸들과 버퍼 정보를 저장
		hbInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		//hbInfo->hClntSock = (DWORD)hRecvSock;
		hbInfo->hClntSock = hRecvSock;
		(hbInfo->wsaBuf).buf = hbInfo->buf;
		(hbInfo->wsaBuf).len = BUF_SIZE;

		// IO 완료 통지 수신 방법으로 이벤트 오브젝트 시그널링을 사용하지 않는다면 OVERLAPPED 구조체의 멤버 hEvent는 큰 의미를 갖지 못한다.
		// 그래서 사용자 임의로 사용할 수 있다. 여기서는 소켓 핸들과 버퍼 정보를 담았다
		lpOvLp->hEvent = (HANDLE)hbInfo;

		// hRecvSock을 통한 입력이 완료되면 RecvCompRoutine이 호출된다.
		// 여기서 여섯 번째 인자로 전달한 WSAOVERLAPPED 구조체 변수의 주소 값은 completion routine의 세 번째 매개변수에 전달된다.
		// 결국 completion routine 함수 내에서 입출력이 완료된 소켓의 핸들과 버퍼에 접근할 수 있다.
		WSARecv(hRecvSock, &(hbInfo->wsaBuf), 1, &recvBytes, &flagInfo, lpOvLp, ReadCompRoutine);
	}

	closesocket(hRecvSock);
	closesocket(hLisnSock);
	WSACleanup();
	return 0;
}

void CALLBACK ReadCompRoutine(DWORD dwError, DWORD szRecvBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags) {
	// completion routine 호출로 OVERLAPPED 구조체 주소가 인자로 전달되었고, 이 구조체의 멤버 hEvent를 참조해 소켓의 핸들과 버퍼정보를 얻을 수 있다.
	LPPER_IO_DATA hbInfo = (LPPER_IO_DATA)(lpOverlapped->hEvent);
	SOCKET hSock = hbInfo->hClntSock;
	LPWSABUF bufInfo = &(hbInfo->wsaBuf);
	DWORD sentBytes;

	if (szRecvBytes == 0) { // EOF을 수신했다면
		closesocket(hSock);
		free(lpOverlapped->hEvent); // 소켓의 핸들과 버퍼정보인 hbInfo를 담은 hEvent를 통해 메모리 해제한다.
		free(lpOverlapped); // OVERLAPPED 구조체도 메모리 해제한다.
		puts("client disconnected...");
	}
	else { // echo
		bufInfo->len = szRecvBytes; // 수신한 바이트를 저장하고 그만큼만 echo

		// WSASend 함수로 echo를 하고 출력이 완료되면 WriteCompRoutine이 호출된다.
		// 이는 송신 후에 다음 메시지를 받기 위한 루틴이다.
		WSASend(hSock, bufInfo, 1, &sentBytes, 0, lpOverlapped, WriteCompRoutine);
	}
}

void CALLBACK WriteCompRoutine(DWORD dwError, DWORD szRecvBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags) {
	LPPER_IO_DATA hbInfo = (LPPER_IO_DATA)(lpOverlapped->hEvent);
	SOCKET hSock = hbInfo->hClntSock;
	LPWSABUF bufInfo = &(hbInfo->wsaBuf);
	DWORD recvBytes;
	int flagInfo = 0;

	// 데이터를 echo 했다면 다음 메시지를 받기 위해 다시 WSARecv 함수를 호출한다.
	// 입력이 완료되면 ReadCompRoutine이 호출되서 echo를 진행한다.
	WSARecv(hSock, bufInfo, 1, &recvBytes, &flagInfo, lpOverlapped, ReadCompRoutine);
}

void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

/*
위 overlapeed IO 예제의 단점
non-blocking 모드의 accept 함수와 alertable wait 상태로의 진입을 위한 SleepEx 함수가 번갈아 가며 반복 호출되는 것은 성능에 영향을 미칠 수 있다.
스레드 하나가 반복해서 non-blocking 모드로 accpet 함수를 호출하고 있는데,
그 와중에 운영체제의 메시지를 받기 위해 인터럽트 가능한 alertable wait 상태로 진입하기 위한 SleepEx 함수도 반복 호출하기 때문

문제 해결 방안
accpet 함수의 호출은 main 스레드가(main 함수 내에서) 처리하도록 하고, 별도의 스레드를 추가로 하나 생성해서 클라이언트와의 입출력을 담당하게 한다.
이것이 바로 IOCP에서 제안하는 서버의 구현 모델로, IOCP에서는 IO를 전담하는 스레드를 별도로 생성한다.
*/