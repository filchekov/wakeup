#include <iostream>
#include <process.h>
#include <windows.h>
#include <string>
#include <ctime>

using namespace std;

void help() {
	cout << "COMMAND SYNTAX:" << endl;
	cout << "       wakeup [-f] [--help] {<-s secons count> | <-d datetime format>}" << endl;
	cout << endl;
	cout << "OPTIONS:" << endl;
	cout << "-f     sleep immediately after launch" << endl;
	cout << "--help this help" << endl;
	cout << "-s     wake up after a specified number of seconds" << endl;
	cout << "-d     wake up at a specified time and date in the format dd:mm:yyyy:hh:mm:ss" << endl;
	cout << "           where:" << endl;
	cout << "               dd - day" << endl;
	cout << "               mm - month" << endl;
	cout << "               yyyy - year" << endl;
	cout << "               hh - hour" << endl;
	cout << "               mm - minute" << endl;
	cout << "               ss - second" << endl;
	cout << "EXAMPLE:" << endl;
	cout << "        wakeup -f -s 60" << endl;
	cout << "            sleep and then wake up in a minute." << endl;
	cout << endl;
	cout << "        wakeup -f -d 16:10:2016:15:08:02" << endl;
	cout << "            sleep and then wake up to the 10/16/2016 15/08/02." << endl;
}

/* 
 * примечание: можно выбрать только один из ключей: -d или -s
 * @param -f // сразу перейти в спящий режим
 * @param -d dd:mm:yyyy:hh:mm:ss // когда разбудить - дата и время - (день:месяц:год:часы:минуты:секунды)
 * @param -s seconds // через сколько разбудить в секундах
 * @param --help // вывести справку
 */
int main(int argc, char *argv[]) {
	bool goHibernate = false; // перейти в сон
	long long sleepSeconds; // на сколько секунд уснуть
	bool alsoHelpExist = false; // для однократной справки
	bool findsKey = false; // истина если указан ключ -d или -s

	if ((argc > 5) || (argc == 1)) {
		help();
		alsoHelpExist = true;
		exit(1); // неверное количество параметров
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
				exit(4); // повторный указательный ключ времени
			}
			else
				findsKey = true;
			/// преоброзовать следующий параметр в секуды
			tm gtm; // полученное время

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
				exit(5); // неправильно задано дата или время
			}

			time_t s;

			time(&s);

			sleepSeconds = t - s;

			cout << "I wake up " << argv[i + 1] << endl;
		}

		if (tmp == "-s") {
			if (findsKey) {
				help();
				exit(4); // повторный указательный ключ времени
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

				exit(2); // в параметре ожидалось число, а указана строка
			}

			cout << "I awake through " << sleepSeconds << " seconds." << endl;
		}
	}

	if (!findsKey) {
		if (!alsoHelpExist)
			help();

		exit(3); // не найден указательный ключ времени (-s или -d)
	}

	if (goHibernate) { // перейти в режим сна
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

	/// запуск ожидающего таймера
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
