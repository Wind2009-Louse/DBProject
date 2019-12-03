#include "hyperion_db.h"
#include "container.h"

int main() {
	Hyperion_DB db;
	db.Init();

	// input
	while (true) {
		string ip;
		cin >> ip;
		if (ip == "0") {
			break;
		}
		db.Insert_into_db(ip.c_str());
	}

	// remove
	while (true) {
		string ip;
		cin >> ip;
		if (ip == "0") {
			break;
		}
		cout << (db.Delete_in_db(ip.c_str()) ? "True" : "False") << endl;
	}

	// point test
	while (true) {
		string ip;
		cin >> ip;
		if (ip == "0") {
			break;
		}
		bool result = db.Pointsearch_in_db(ip.c_str());
		cout << (result ? "True" : "False") << endl;
	}

	// range test
	string lower = "", upper = "";
	bool no_upper = true;
	while (true) {
		vector<string> results = db.Rangesearch_in_db(lower.c_str(), upper.c_str(), true, true, no_upper);
		//cout << "Results:" << endl;
		for (int i = 0; i < results.size(); ++i) {
			//cout << '\t' << results[i] << endl;
			cout << results[i] << endl;
		}

		no_upper = false;
		cin >> lower >> upper;
		if (lower == "0" && upper == "0") {
			break;
		}
	}
	//system("pause");
}