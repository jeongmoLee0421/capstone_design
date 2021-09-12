#pragma comment(lib, "pdh.lib") // ���̺귯�� ��ũ��Ű�� ���� ���

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
		PdhOpenQuery(NULL, NULL, &cpuQuery); // ���� �������� ������ �����ϴµ� ���Ǵ� ���� ����
		PdhAddCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuCounter); // ���� �ڵ鿡 ī���� �ڵ��� ���δ�(ī���͸� �ĺ��ϱ� ���� ī������ ��θ� ���ڷ� �ѱ�).
		PdhCollectQueryData(cpuQuery); // ������ ���� ���� ��� ī���Ϳ��� ���� ������ ���� �����ϰ� �� ī������ ���� �ڵ带 ������Ʈ �Ѵ�.

	}

	double GetCPUUsage() {
		PDH_FMT_COUNTERVALUE counterVal; // ī������ ���� ���� ���¸� ������ ����ü

		PdhCollectQueryData(cpuQuery); // ������ ����
		PdhGetFormattedCounterValue(cpuCounter, PDH_FMT_DOUBLE, NULL, &counterVal); // ������ ī���Ϳ� ���� ǥ�ð����� ���� ����ؼ� counterVal ����ü�� ����
		return counterVal.doubleValue; // counterVal ����ü���� double������ ���� ī���� ���� ����
	}
};

int main(void) {
	CPUData cpuData;

	while (true) {
		cout << "CPU Usage: " << cpuData.GetCPUUsage() << endl;
		Sleep(1000);
	}

	return 0;
}