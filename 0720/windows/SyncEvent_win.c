#include <stdio.h>
#include <windows.h>
#include <process.h>
#define STR_LEN 100

unsigned WINAPI NumberOfA(void* arg);
unsigned WINAPI NumberOfOthers(void* arg);

static char str[STR_LEN];
static HANDLE hEvent;

int main(int argc, char* argv[]) {
	HANDLE hThread1, hThread2;

	// event Ŀ�� ������Ʈ ����
	// �ι�° ���� TRUE�� manual-reset ���
	// ����° ���� FALSE�� non-signaled ����
	hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	hThread1 = (HANDLE)_beginthreadex(NULL, 0, NumberOfA, NULL, 0, NULL);
	hThread2 = (HANDLE)_beginthreadex(NULL, 0, NumberOfOthers, NULL, 0, NULL);

	fputs("input string: ", stdout);
	fgets(str, STR_LEN, stdin);

	// event Ŀ�� ������Ʈ�� signaled ���·� �����Ѵ�.
	// ���� ������̴� �ΰ��� ������ ��� �����¿� �������� ������ �̾��.
	SetEvent(hEvent);

	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);

	// �� ���������� ���� �ʿ����� ������ event Ŀ�� ������Ʈ�� non-signaled ���·� �����Ѵ�(�������� ������ ����ؼ� signaled ���¿� �ְ� �ȴ�).
	ResetEvent(hEvent);

	// event Ŀ�� ������Ʈ�� �Ҹ�
	CloseHandle(hEvent);

	return 0;
}

unsigned WINAPI NumberOfA(void* arg) {
	int i, cnt = 0;

	// event Ŀ�� ������Ʈ�� signaled ���°� �ɶ����� ���
	WaitForSingleObject(hEvent, INFINITE);
	for (i = 0; str[i] != 0; i++) {
		if (str[i] == 'A')
			cnt++;
	}

	printf("Num of A: %d\n", cnt);
	return 0;
}

unsigned WINAPI NumberOfOthers(void* arg) {
	int i, cnt = 0;

	WaitForSingleObject(hEvent, INFINITE);
	for (i = 0; str[i] != 0; i++) {
		if (str[i] != 'A')
			cnt++;
	}

	printf("Num of others: %d\n", cnt-1); // fgets �Լ��� ���๮�ڱ��� �ޱ� ������ 1�� ���ش�.
	return 0;
}