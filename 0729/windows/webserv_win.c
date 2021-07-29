#pragma warning (disable:4996)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>
#include <process.h>

#define BUF_SIZE 2048
#define BUF_SMALL 100

unsigned WINAPI RequestHandler(void* arg);
char* ContentType(char* file);
void SendData(SOCKET sock, char* ct, char* fileName);
void SendErrorMSG(SOCKET sock);
void ErrorHandling(char* msg);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAddr, clntAddr;

	HANDLE hThread;
	DWORD dwThreadID;
	int clntAddrSize;

	if (argc != 2) {
		printf("usage: %s <port>\n", argv[0]);
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

	while (1) {
		clntAddrSize = sizeof(clntAddr);
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &clntAddrSize);
		if (hClntSock == INVALID_SOCKET)
			ErrorHandling("accept() error");

		printf("connection requeset: %s:%d\n", inet_ntoa(clntAddr.sin_addr), ntohs(clntAddr.sin_port));

		// 요청에 응답하기 위한 스레드 생성
		hThread = (HANDLE)_beginthreadex(NULL, 0, RequestHandler, (void*)hClntSock, 0, (unsigned*)&dwThreadID);
	}

	closesocket(hServSock);
	WSACleanup();
	return 0;
}

unsigned WINAPI RequestHandler(void* arg) {
	SOCKET hClntSock = (SOCKET)arg;
	char buf[BUF_SIZE];
	char method[BUF_SMALL];
	char ct[BUF_SMALL]; // content type
	char fileName[BUF_SMALL];

	// 클라이언트 데이터 요청을 서버가 수신
	recv(hClntSock, buf, BUF_SIZE, 0);
	printf("%s\n", buf);

	// HTTP에 의한 요청인지 확인
	// 수신받은 buf에서 HTTP/ 문자열이 있는지 검색
	if (strstr(buf, "HTTP/") == NULL) {
		SendErrorMSG(hClntSock);
		closesocket(hClntSock);
		return 1;
	}

	// GET 방식 요청인지 확인
	// 수신받은 buf에서 " /"를 기준으로 구분해서 method로 복사(구분자 기준으로 앞쪽이 복사됨)
	strcpy(method, strtok(buf, " /"));
	if (strcmp(method, "GET"))
		SendErrorMSG(hClntSock);

	strcpy(fileName, strtok(NULL, " /")); // 요청 파일 이름 확인(strtok를 사용해 이전 문자열을 계속해서 검색하려면 첫 번째 인자를 NULL로 주면됨)
	strcpy(ct, ContentType(fileName)); // content-type 확인
	SendData(hClntSock, ct, fileName);

	return 0;
}

void SendData(SOCKET sock, char* ct, char* fileName) {
	char protocol[] = "HTTP/1.0 200 OK\r\n";
	char servName[] = "Server:simple web server\r\n";
	char cntLen[] = "Content-length: 2048\r\n";
	char cntType[BUF_SMALL];
	char buf[BUF_SIZE];
	FILE* sendFile;

	// printf 함수처럼 문자열을 구성한 뒤 buffer에 저장한다
	sprintf(cntType, "Content-type:%s\r\n\r\n", ct);
	if ((sendFile = fopen(fileName, "r")) == NULL) {
		SendErrorMSG(sock);
		return;
	}

	// 헤더 정보 전송
	send(sock, protocol, strlen(protocol), 0);
	send(sock, servName, strlen(servName), 0);
	send(sock, cntLen, strlen(cntLen), 0);
	send(sock, cntType, strlen(cntType), 0);

	// 요청 데이터 전송
	while (fgets(buf, BUF_SIZE, sendFile) != NULL)
		send(sock, buf, strlen(buf), 0);

	// http에 의해서 응답 후 종료
	closesocket(sock);
}

void SendErrorMSG(SOCKET sock) {
	char protocol[] = "HTTP/1.0 400 Bad Request\r\n";
	char servName[] = "Server:simple web server\r\n";
	char cntLen[] = "Content-length:2048\r\n";
	char cntType[] = "Content-type:text/html\r\n\r\n";
	char content[] = "<html><head><title>NETWORK</title></head>"
		"<body><font size=+5><br>오류 발생! 요청 파일명 및 요청 방식 확인"
		"</font></body></html>";

	send(sock, protocol, strlen(protocol), 0);
	send(sock, servName, strlen(servName), 0);
	send(sock, cntLen, strlen(cntLen), 0);
	send(sock, cntType, strlen(cntType), 0);
	send(sock, content, strlen(content), 0);
	closesocket(sock);
}

// content-type 구분
char* ContentType(char* file) {
	char extension[BUF_SMALL];
	char fileName[BUF_SMALL];
	strcpy(fileName, file);
	strtok(fileName, ".");
	strcpy(extension, strtok(NULL, "."));

	if (!strcmp(extension, "html") || !strcmp(extension, "htm"))
		return "text/html";
	else
		return "text/plain";
}

void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}