#pragma warning (disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
void ErrorHandling(char* msg);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hSocket;
	SOCKADDR_IN sendAddr;

	WSABUF dataBuf; // WSASend 함수를 사용할 때 두 번째 인자로 넘기는, 전송할 데이터 정보를 지니는 구조체
	char msg[] = "overlapped hEvent member!";
	int sendBytes = 0; // 전송된 바이트 수가 저장될 변수

	WSAEVENT evObj; // overlapped 구조체에 hEvent 멤버에 사용
	WSAOVERLAPPED overlapped; // overlapped IO를 진행하기 위해 사용되는 overlapped 구조체

	if (argc != 3) {
		printf("usage: %s <ip> <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	hSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED); // overlapped IO가 가능한 non-blocking 모드의 소켓 생성
	if (hSocket == INVALID_SOCKET)
		ErrorHandling("WSASocket() error");

	memset(&sendAddr, 0, sizeof(sendAddr));
	sendAddr.sin_family = AF_INET;
	sendAddr.sin_addr.s_addr = inet_addr(argv[1]);
	sendAddr.sin_port = htons(atoi(argv[2]));

	if (connect(hSocket, (SOCKADDR*)&sendAddr, sizeof(sendAddr)) == SOCKET_ERROR)
		ErrorHandling("connect() error");

	// event 오브젝트 생성
	evObj = WSACreateEvent();

	// overlapped 구조체 초기화 및 hEvent멤버에 event 오브젝트 저장
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = evObj;

	// 전송할 데이터 정보 설정
	dataBuf.len = strlen(msg) + 1;
	dataBuf.buf = msg;

	// dataBuf 변수의 내용을 전송, 전송된 바이트 수는 sendBytes에 저장(출력버퍼가 비어있고 데이터 크기가 작다면 함수 호출과 동시에 데이터의 전송이 완료될 수도 있다)
	if (WSASend(hSocket, &dataBuf, 1, &sendBytes, 0, &overlapped, NULL) == SOCKET_ERROR) {
		if (WSAGetLastError() == WSA_IO_PENDING) { // 데이터 전송이 완료되지 않았고 아직 진행중
			puts("Background data send");
			WSAWaitForMultipleEvents(1, &evObj, TRUE, WSA_INFINITE, FALSE); // 데이터 전송이 완료되면 evObj가 signaled 상태로 변경되고, 그 때가 되어야 함수가 반환
			WSAGetOverlappedResult(hSocket, &overlapped, &sendBytes, FALSE, NULL); // 데이터 전송결과 확인(sendBytes에 전송된 바이트 수 저장)
		}
		else {
			// printf("%d\n", WSAGetLastError()); // 에러 6(WSA_INVALID_HANDLE, 지정한 이벤트 개체 핸들이 잘못 되었습니다.)
			// WSACreateEvent 함수를 호출할 때 ()를 써주지 않아서 생긴 에러이다.
			// 컴파일러가 괄호를 왜 잡아주지 않았을까?
			// 함수의 주소를 evObj에 대입한다는 의미로 해석한 것 같다.
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