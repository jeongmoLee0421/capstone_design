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

	// semaphore 커널 오브젝트 생성
	semOne = CreateSemaphore(NULL, 0, 1, NULL); // 초기값이 0이므로 non-signaled 상태
	semTwo = CreateSemaphore(NULL, 1, 1, NULL); // 초기값이 1이므로 signaled 상태

	hThread1 = (HANDLE)_beginthreadex(NULL, 0, Read, NULL, 0, NULL);
	hThread2 = (HANDLE)_beginthreadex(NULL, 0, Accu, NULL, 0, NULL);

	// 스레드가 종료되어 signaled 상태가 되면 반환
	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);

	// semaphore 커널 오브젝트 소멸
	CloseHandle(semOne);
	CloseHandle(semTwo);

	return 0;
}

unsigned WINAPI Read(void* arg) {
	int i, temp;

	for (i = 0; i < 5; i++) {
		fputs("input num: ", stdout);
		scanf("%d", &temp); // 사용자로부터 입력받는 함수는 대기시간이 오래걸릴 수 있기 때문에 임계 구역에 포함시키지 않는 것이 좋다.

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