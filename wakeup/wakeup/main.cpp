#include <iostream>
#include <process.h>
#include <windows.h>
#include <string>
#include <ctime>

using namespace std;

void help() {
	cout << "help" << endl;
}

// @ -f
// @ -d dd:mm:yyyy:hh:mm:ss || -s 666
// @ --help
int main(int argc, char *argv[]) {
	bool goHibernate = false; // перейти в сон
	long unsigned int sleepSeconds; // на сколько секунд уснуть
	bool alsoHelpExist = false; // для однократной справки
	bool findsKey = false; //

	if (argc > 5) {
		help();
		exit(1); // много параметров
	}

	for (int i = 0; i < argc; i++) {
		string tmp = argv[i];

		if (tmp == "-f") {
			cout << "toHiber" << endl;
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

			sscanf(argv[i + 1], "%d:%d:%d:%d:%d:%d",
				&gtm.tm_mday,
				&gtm.tm_mon,
				&gtm.tm_year,
				&gtm.tm_hour,
				&gtm.tm_min,
				&gtm.tm_sec);

			gtm.tm_year = gtm.tm_year - 1900;
			time_t t = mktime(&gtm);

			if (t == -1) {
				help();
				exit(5); // неправильно задано дата или время
			}

			//cout << t << endl;
			sleepSeconds = t;
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
		}
	}

	if (!findsKey) {
		help();
		exit(3); // не найден указательный ключ времени
	}

	cout << "Hybernate" << endl;

	cout << "ok!" << endl;

	return 0;
}
