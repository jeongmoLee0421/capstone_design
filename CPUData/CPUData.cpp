#pragma comment(lib, "pdh.lib") // 라이브러리 링크시키기 위해 명시
#pragma warning(disable:4996) // 4996에러 숨기기

#include <iostream>
#include <Pdh.h>
#include <windows.h> // Sleep()

using std::cout;
using std::endl;

static PDH_HQUERY cpuQuery;
static PDH_HCOUNTER cpuCounter;

class CPUData {
private:

public:
	CPUData() {
		PdhOpenQuery(NULL, NULL, &cpuQuery); // 성능 데이터의 수집을 관리하는데 사용되는 쿼리 생성
		PdhAddCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuCounter); // 쿼리 핸들에 카운터 핸들을 붙인다(카운터를 식별하기 위해 카운터의 경로를 인자로 넘김).
		PdhCollectQueryData(cpuQuery); // 지정된 쿼리 내의 모든 카운터에서 현재 데이터 값을 수집하고 각 카운터의 상태 코드를 업데이트 한다.

	}

	double GetCPUUsage() {
		PDH_FMT_COUNTERVALUE counterVal; // 카운터의 계산된 값과 상태를 저장할 구조체

		PdhCollectQueryData(cpuQuery); // 데이터 수집
		PdhGetFormattedCounterValue(cpuCounter, PDH_FMT_DOUBLE, NULL, &counterVal); // 지정한 카운터에 대해 표시가능한 값을 계산해서 counterVal 구조체에 대입
		return counterVal.doubleValue; // counterVal 구조체에서 double형으로 계산된 카운터 값을 리턴
	}
};

int main(void) {
	CPUData cpuData;
	double CPUUsage = 0.0;
	double sum = 0.0; // cpu사용률 총합
	double avg = 0.0; // cpu사용률 평균
	int cnt = 0;

	while (true) {
		CPUUsage = cpuData.GetCPUUsage();
		sum += CPUUsage;
		cnt++;
		cout << "CPU Usage: " << CPUUsage << endl;
		Sleep(10);
		if (cnt == 1000)break;
	}
	avg = sum / cnt;
	cout << "CPU Usage average: " << avg << endl;
	return 0;
}