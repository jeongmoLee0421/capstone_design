#pragma warning (disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <WS2tcpip.h> // IP_MULTICAST_TTL 辛芝聖 竺舛馬奄 是廃 伯希督析

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

	hSendSock = socket(PF_INET, SOCK_DGRAM, 0); // 菰銅蝶什闘澗 udp奄鋼生稽 疑拙廃陥.
	if (hSendSock == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&mulAddr, 0, sizeof(mulAddr));
	mulAddr.sin_family = AF_INET;
	mulAddr.sin_addr.s_addr = inet_addr(argv[1]); // sender澗 菰銅蝶什闘 爽社稽 左鎧奄幻 馬檎 吉陥
	mulAddr.sin_port = htons(atoi(argv[2]));

	// IP_MULTICAST_TTL 辛芝 竺舛
	setsockopt(hSendSock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&timeLive, sizeof(timeLive));

	/*errno_t errno;
	if ((errno = fopen_s(&fp, "news.txt", "r")) != 0)
		ErrorHandling("fopen_s() error");*/

	// 督析戚 赤澗汽 切荷 神嫌亜 蟹掩掘 達焼沙 衣引
	// 督析 戚硯聖 news.txt稽 煽舌梅澗汽 戚惟 叔薦稽澗 news.txt.txt心陥. せせせせせせせせせせせせせせ
	// news.txt.txt稽 馬艦猿 伸鍵陥.
	if ((fp = fopen("news.txt.txt", "r")) == NULL)
		ErrorHandling("fopen() error");

	//fopen_s(&fp, "news.txt", "r");
	//fp = fopen("news.txt", "r");

	// feof()澗 督析税 魁拭 亀含馬檎 0戚 焼観 葵聖 鋼発廃陥.
	// C税 feof澗 '原走厳' 敗呪亜 叔鳶馬澗 井酔拭 true研 軒渡廃陥.
	// 督析 匂昔斗亜 魁聖 亜軒佃亀 敗呪澗 廃 腰 希 叔楳戚 鞠壱, 暗奄辞 拭君亜 蟹人醤 true亜 鞠嬢辞 while聖 匙閃 蟹紳陥澗 源
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