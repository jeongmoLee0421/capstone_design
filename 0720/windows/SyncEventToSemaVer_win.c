#include <stdio.h>
#include <windows.h>
#include <process.h>
#define STR_LEN 100

unsigned WINAPI NumberOfA(void* arg);
unsigned WINAPI NumberOfOthers(void* arg);

static char str[STR_LEN];
static HANDLE sema;

int main(int argc, char* argv[]) {
	HANDLE hThread1, hThread2;

	sema = CreateSemaphore(NULL, 2, 2, NULL); // �ʱⰪ 2, �ִ밪 2�� semaphore Ŀ�� ������Ʈ ���� 

	// ���ڿ��� ���� �Է¹��� ������ �����Ⱚ�� �ֱ� ������ �߸��� ���� ����Ѵ�.
	fputs("input string: ", stdout);
	fgets(str, STR_LEN, stdin);

	hThread1 = (HANDLE)_beginthreadex(NULL, 0, NumberOfA, NULL, 0, NULL);
	hThread2 = (HANDLE)_beginthreadex(NULL, 0, NumberOfOthers, NULL, 0, NULL);

	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);

	CloseHandle(sema); // semaphore ������Ʈ �Ҹ�

	return 0;
}

unsigned WINAPI NumberOfA(void* arg) {
	int i, cnt = 0;

	// semaphore Ŀ�� ������Ʈ�� signaled ���°� �ɶ����� ���
	WaitForSingleObject(sema, INFINITE);
	for (i = 0; str[i] != 0; i++) {
		if (str[i] == 'A')
			cnt++;
	}
	printf("Num of A: %d\n", cnt);
	ReleaseSemaphore(sema, 1, NULL); // semaphore Ŀ�� ������Ʈ �ݳ�
	return 0;
}

unsigned WINAPI NumberOfOthers(void* arg) {
	int i, cnt = 0;

	WaitForSingleObject(sema, INFINITE);
	for (i = 0; str[i] != 0; i++) {
		if (str[i] != 'A')
			cnt++;
	}
	printf("Num of others: %d\n", cnt - 1);
	ReleaseSemaphore(sema, 1, NULL);
	return 0;
}