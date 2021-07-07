#pragma warning (disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <WS2tcpip.h> // IP_MULTICAST_TTL 옵션을 설정하기 위한 헤더파일

#define TTL 64
#define BUF_SIZE 30
void ErrorHandling(char* message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hSendSock;

	SOCKADDR_IN mulAddr;
	int timeLive = TTL;
	FILE* fp;
	char buf[BUF_SIZE];

	if (argc != 3) {
		printf("usage %s <group ip> <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	hSendSock = socket(PF_INET, SOCK_DGRAM, 0); // 멀티캐스트는 udp기반으로 동작한다.
	if (hSendSock == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&mulAddr, 0, sizeof(mulAddr));
	mulAddr.sin_family = AF_INET;
	mulAddr.sin_addr.s_addr = inet_addr(argv[1]); // sender는 멀티캐스트 주소로 보내기만 하면 된다
	mulAddr.sin_port = htons(atoi(argv[2]));

	// IP_MULTICAST_TTL 옵션 설정
	setsockopt(hSendSock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&timeLive, sizeof(timeLive));

	/*errno_t errno;
	if ((errno = fopen_s(&fp, "news.txt", "r")) != 0)
		ErrorHandling("fopen_s() error");*/

	// 파일이 있는데 자꾸 오류가 나길래 찾아본 결과
	// 파일 이름을 news.txt로 저장했는데 이게 실제로는 news.txt.txt였다. ㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋㅋ
	// news.txt.txt로 하니까 열린다.
	if ((fp = fopen("news.txt.txt", "r")) == NULL)
		ErrorHandling("fopen() error");

	//fopen_s(&fp, "news.txt", "r");
	//fp = fopen("news.txt", "r");

	// feof()는 파일의 끝에 도달하면 0이 아닌 값을 반환한다.
	// C의 feof는 '마지막' 함수가 실패하는 경우에 true를 리턴한다.
	// 파일 포인터가 끝을 가리켜도 함수는 한 번 더 실행이 되고, 거기서 에러가 나와야 true가 되어서 while을 빠져 나온다는 말
	while (!feof(fp)) {
		fgets(buf, BUF_SIZE, fp);
		//printf("%s\n", buf);
		sendto(hSendSock, buf, strlen(buf), 0, (SOCKADDR*)&mulAddr, sizeof(mulAddr));
		Sleep(1000);
	}

	fclose(fp);
	closesocket(hSendSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}