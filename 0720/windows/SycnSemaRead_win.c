#pragma warning (disable:4996)
#include <stdio.h>
#include <windows.h>
#include <process.h>

unsigned WINAPI Read(void* arg);
unsigned WINAPI Accu(void* arg);

static HANDLE semOne;
static HANDLE semTwo;
static int num;

int main(int argc, char* argv[]) {
	HANDLE hThread1, hThread2;

	// semaphore Ŀ�� ������Ʈ ����
	semOne = CreateSemaphore(NULL, 0, 1, NULL); // �ʱⰪ�� 0�̹Ƿ� non-signaled ����
	semTwo = CreateSemaphore(NULL, 1, 1, NULL); // �ʱⰪ�� 1�̹Ƿ� signaled ����

	hThread1 = (HANDLE)_beginthreadex(NULL, 0, Read, NULL, 0, NULL);
	hThread2 = (HANDLE)_beginthreadex(NULL, 0, Accu, NULL, 0, NULL);

	// �����尡 ����Ǿ� signaled ���°� �Ǹ� ��ȯ
	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);

	// semaphore Ŀ�� ������Ʈ �Ҹ�
	CloseHandle(semOne);
	CloseHandle(semTwo);

	return 0;
}

unsigned WINAPI Read(void* arg) {
	int i, temp;

	for (i = 0; i < 5; i++) {
		fputs("input num: ", stdout);
		scanf("%d", &temp); // ����ڷκ��� �Է¹޴� �Լ��� ���ð��� �����ɸ� �� �ֱ� ������ �Ӱ� ������ ���Խ�Ű�� �ʴ� ���� ����.

		WaitForSingleObject(semTwo, INFINITE);
		num = temp;
		ReleaseSemaphore(semOne, 1, NULL);
	}
	return 0;
}

unsigned WINAPI Accu(void* arg) {
	int sum = 0, i;

	for (i = 0; i < 5; i++) {
		WaitForSingleObject(semOne, INFINITE);
		sum += num;
		ReleaseSemaphore(semTwo, 1, NULL);
	}

	printf("result: %d\n", sum);
	return 0;
}