#include <iostream>
#include <process.h>
#include <windows.h>
#include <string>
#include <ctime>

using namespace std;

void help() {
	cout << "help" << endl;
}

/* 
 * ����������: ����� ������� ������ ���� �� ������: -d ��� -s
 * @param -f // ����� ������� � ������ �����
 * @param -d dd:mm:yyyy:hh:mm:ss // ����� ��������� - ���� � ����� - (����:�����:���:����:������:�������)
 * @param -s seconds // ����� ������� ��������� � ��������
 * @param --help // ������� �������
 */
int main(int argc, char *argv[]) {
	bool goHibernate = false; // ������� � ���
	long long sleepSeconds; // �� ������� ������ ������
	bool alsoHelpExist = false; // ��� ����������� �������
	bool findsKey = false; // ������ ���� ������ ���� -d ��� -s

	if ((argc > 5) || (argc == 1)) {
		help();
		alsoHelpExist = true;
		exit(1); // �������� ���������� ����������
	}

	for (int i = 0; i < argc; i++) {
		string tmp = argv[i];

		if (tmp == "-f") {
			//cout << "toHiber" << endl;
			goHibernate = true;
		}

		if (tmp == "--help") {
			alsoHelpExist = true;
			help();
		}

		if (tmp == "-d") {
			if (findsKey) {
				help();
				exit(4); // ��������� ������������ ���� �������
			}
			else
				findsKey = true;
			/// ������������� ��������� �������� � ������
			tm gtm; // ���������� �����

			sscanf_s(argv[i + 1], "%d:%d:%d:%d:%d:%d",
				&gtm.tm_mday,
				&gtm.tm_mon,
				&gtm.tm_year,
				&gtm.tm_hour,
				&gtm.tm_min,
				&gtm.tm_sec);

			gtm.tm_year = gtm.tm_year - 1900;
			gtm.tm_mon--;

			time_t t = mktime(&gtm);

			if (t == -1) {
				help();
				exit(5); // ����������� ������ ���� ��� �����
			}

			time_t s;

			time(&s);

			sleepSeconds = t - s;

			cout << "I wake up " << argv[i + 1] << endl;
		}

		if (tmp == "-s") {
			if (findsKey) {
				help();
				exit(4); // ��������� ������������ ���� �������
			}
			else
				findsKey = true;

			tmp = argv[i + 1];

			try {
				sleepSeconds = stoi(tmp);
			}
			catch (...) {
				if (!alsoHelpExist)
					help();

				exit(2); // � ��������� ��������� �����, � ������� ������
			}

			cout << "I awake through " << sleepSeconds << " seconds." << endl;
		}
	}

	if (!findsKey) {
		if (!alsoHelpExist)
			help();

		exit(3); // �� ������ ������������ ���� ������� (-s ��� -d)
	}

	if (goHibernate) { // ������� � ����� ���
		HANDLE hToken;

		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken)) {
			cerr << "OpenProcessToken failed" << endl;

			return 1;
		}

		LUID luid;

		if (!LookupPrivilegeValue(0, SE_SHUTDOWN_NAME, &luid)) {
			cerr << "LookupPrivilegeValue failed" << endl;

			return 1;
		}

		TOKEN_PRIVILEGES tp;

		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luid;
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), 0, 0) || GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
			cerr << "AdjustTokenPrivileges failed" << endl;

			return 1;
		}

		CloseHandle(hToken);
	}

	/// ������ ���������� �������
	LARGE_INTEGER duetime;

	duetime.QuadPart = -((long long)10000000 * (long long)sleepSeconds);

	HANDLE hTimer = CreateWaitableTimer(NULL, TRUE, NULL);

	if (!hTimer) {
		cerr << "CreateWaitableTimer failed" << endl;

		return 1;
	}

	SetWaitableTimer(hTimer, &duetime, 0, NULL, NULL, TRUE);

	SetSystemPowerState(TRUE, FALSE);

	int ret = WaitForSingleObject(hTimer, INFINITE);
	///
	cout << "ok!" << endl;

	return 0;
}
