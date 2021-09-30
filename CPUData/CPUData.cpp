#pragma comment(lib, "pdh.lib") // ���̺귯�� ��ũ��Ű�� ���� ���
#pragma warning(disable:4996) // 4996���� �����

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
	double CPUUsage = 0.0;
	double sum = 0.0; // cpu���� ����
	double avg = 0.0; // cpu���� ���
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