#pragma warning(disable:4996)
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <WinSock2.h>

#define BUF_SIZE 1024
#define NAME_SIZE 20

void SendMsg(SOCKET sock);
void RecvMsg(SOCKET sock);
void ErrorHandling(const char* msg);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET hSock;
	SOCKADDR_IN servAddr;
	std::thread SendThread, RecvThread;

	// ����μ� 4�� = ���α׷� �̸�, ip, port, name
	if (argc != 4) {
		printf("Usage %s <ip> <port> <name>\n", argv[0]);
		exit(1);
	}

	// winsock �ʱ�ȭ
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	// ������ ������ Ŭ���̾�Ʈ ���� ����
	hSock = socket(PF_INET, SOCK_STREAM, 0);
	if (hSock == INVALID_SOCKET)
		ErrorHandling("socket() error");

	// �Է� ���� ��� �μ� ip�ּҿ� port��ȣ�� ���� �ּ� ����ü �ʱ�ȭ
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(argv[1]);
	servAddr.sin_port = htons(atoi(argv[2]));

	// Ŭ���̾�Ʈ �������� ������ ���� �õ�
	if (connect(hSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("connect() error");

	// �ۼ����� ���� ��ũ���͸� ���ڷ� �ѱ�鼭 ������ ����
	SendThread = std::thread(SendMsg, hSock);
	RecvThread = std::thread(RecvMsg, hSock);

	// �ۼ��� �����尡 ������ ������ main ������� ���ŷ
	SendThread.join();
	RecvThread.join();
	
	return 0;
}

void SendMsg(SOCKET sock) {
	const char* dummy_str = "[32] Sed ut perspiciatis, unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, "
		"totam rem aperiam eaque ipsa, quae ab illo inventore veritatis et quasi architecto beatae vitae dicta sunt, explicabo. "
		"Nemo enim ipsam voluptatem, quia voluptas sit, aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos, "
		"qui ratione voluptatem sequi nesciunt, neque porro quisquam est, qui dolorem ipsum, quia dolor sit, amet, consectetur,"
		"adipisci velit, sed quia non numquam eius modi tempora incidunt, ut labore et dolore magnam aliquam quaerat voluptatem. "
		"Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi "
		"consequatur? Quis autem vel eum iure reprehenderit, qui in ea voluptate velit esse, quam nihil molestiae consequatur, "
		"vel illum, qui dolorem eum fugiat, quo voluptas nulla pariatur? [33] At vero eos et accusamus et iusto odio dignissimos ducimus, "
		"qui blanditiis praesentium voluptatum deleniti atque corrupti, quos dolores et quas molestias excepturi sint, obcaecati "
		"cupiditate non provident, similique sunt in culpa, qui officia deserunt mollitia animi, id est laborum et dolorum fuga. "
		"Et harum quidem rerum facilis est et expedita distinctio. Nam libero tempore, cum soluta nobis est eligendi optio, cumque "
		"nihil impedit, quo minus id, quod maxime placeat, facere possimus, omnis voluptas assumenda est, omnis dolor repellendus. "
		"Temporibus autem quibusdam et aut officiis debitis aut rerum necessitatibus saepe eveniet, ut et voluptates repudiandae sint "
		"et molestiae non recusandae. Itaque earum rerum hic tenetur a sapiente delectus, ut aut reiciendis voluptatibus maiores alias "
		"consequatur aut perferendis doloribus asperiores repellat.";

	int i = 0;
	int strLen = strlen(dummy_str);
	while (1) {
		send(sock, dummy_str, strLen, 0);
		Sleep(10);
		i++;

		// 100�� �����ϰ� ������ ����
		if (i == 100)
			return;
	}
}

void RecvMsg(SOCKET sock) {
	int strLen;
	char buf[BUF_SIZE];

	while (1) {
		strLen = recv(sock, buf, BUF_SIZE - 1, 0);
		if (strLen == -1)
			return;

		buf[strLen] = 0;
		fputs(buf, stdout);
	}
	return;
}

void ErrorHandling(const char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}