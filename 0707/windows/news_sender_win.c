#pragma warning (disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <WS2tcpip.h> // IP_MULTICAST_TTL �ɼ��� �����ϱ� ���� �������

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

	hSendSock = socket(PF_INET, SOCK_DGRAM, 0); // ��Ƽĳ��Ʈ�� udp������� �����Ѵ�.
	if (hSendSock == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&mulAddr, 0, sizeof(mulAddr));
	mulAddr.sin_family = AF_INET;
	mulAddr.sin_addr.s_addr = inet_addr(argv[1]); // sender�� ��Ƽĳ��Ʈ �ּҷ� �����⸸ �ϸ� �ȴ�
	mulAddr.sin_port = htons(atoi(argv[2]));

	// IP_MULTICAST_TTL �ɼ� ����
	setsockopt(hSendSock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&timeLive, sizeof(timeLive));

	/*errno_t errno;
	if ((errno = fopen_s(&fp, "news.txt", "r")) != 0)
		ErrorHandling("fopen_s() error");*/

	// ������ �ִµ� �ڲ� ������ ���淡 ã�ƺ� ���
	// ���� �̸��� news.txt�� �����ߴµ� �̰� �����δ� news.txt.txt����. ����������������������������
	// news.txt.txt�� �ϴϱ� ������.
	if ((fp = fopen("news.txt.txt", "r")) == NULL)
		ErrorHandling("fopen() error");

	//fopen_s(&fp, "news.txt", "r");
	//fp = fopen("news.txt", "r");

	// feof()�� ������ ���� �����ϸ� 0�� �ƴ� ���� ��ȯ�Ѵ�.
	// C�� feof�� '������' �Լ��� �����ϴ� ��쿡 true�� �����Ѵ�.
	// ���� �����Ͱ� ���� �����ѵ� �Լ��� �� �� �� ������ �ǰ�, �ű⼭ ������ ���;� true�� �Ǿ while�� ���� ���´ٴ� ��
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