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

	sema = CreateSemaphore(NULL, 2, 2, NULL); // 초기값 2, 최대값 2인 semaphore 커널 오브젝트 생성 

	// 문자열을 먼저 입력받지 않으면 쓰레기값이 있기 때문에 잘못된 값을 출력한다.
	fputs("input string: ", stdout);
	fgets(str, STR_LEN, stdin);

	hThread1 = (HANDLE)_beginthreadex(NULL, 0, NumberOfA, NULL, 0, NULL);
	hThread2 = (HANDLE)_beginthreadex(NULL, 0, NumberOfOthers, NULL, 0, NULL);

	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);

	CloseHandle(sema); // semaphore 오브젝트 소멸

	return 0;
}

unsigned WINAPI NumberOfA(void* arg) {
	int i, cnt = 0;

	// semaphore 커널 오브젝트가 signaled 상태가 될때까지 대기
	WaitForSingleObject(sema, INFINITE);
	for (i = 0; str[i] != 0; i++) {
		if (str[i] == 'A')
			cnt++;
	}
	printf("Num of A: %d\n", cnt);
	ReleaseSemaphore(sema, 1, NULL); // semaphore 커널 오브젝트 반납
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