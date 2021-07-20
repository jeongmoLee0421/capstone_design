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

	// event 커널 오브젝트 생성
	// 두번째 인자 TRUE는 manual-reset 모드
	// 세번째 인자 FALSE는 non-signaled 상태
	hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	hThread1 = (HANDLE)_beginthreadex(NULL, 0, NumberOfA, NULL, 0, NULL);
	hThread2 = (HANDLE)_beginthreadex(NULL, 0, NumberOfOthers, NULL, 0, NULL);

	fputs("input string: ", stdout);
	fgets(str, STR_LEN, stdin);

	// event 커널 오브젝트를 signaled 상태로 변경한다.
	// 이후 대기중이던 두개의 스레드 모두 대기상태에 빠져나와 실행을 이어간다.
	SetEvent(hEvent);

	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);

	// 이 예제에서는 굳이 필요하지 않지만 event 커널 오브젝트를 non-signaled 상태로 변경한다(변경하지 않으면 계속해서 signaled 상태에 있게 된다).
	ResetEvent(hEvent);

	// event 커널 오브젝트를 소멸
	CloseHandle(hEvent);

	return 0;
}

unsigned WINAPI NumberOfA(void* arg) {
	int i, cnt = 0;

	// event 커널 오브젝트가 signaled 상태가 될때까지 대기
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

	printf("Num of others: %d\n", cnt-1); // fgets 함수는 개행문자까지 받기 때문에 1을 빼준다.
	return 0;
}