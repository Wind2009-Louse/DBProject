#include "dbupdate.h"
#include "pointer_search.h"
#include "range_search.h"

int main() {
	Container* ctr = Create_Empty_Container();
	
	// input
	while (true) {
		string ip;
		cin >> ip;
		if (ip == "0") {
			break;
		}
		Insert_into_db(ctr, ip.c_str());
	}

	// remove
	while (true) {
		string ip;
		cin >> ip;
		if (ip == "0") {
			break;
		}
		cout << (Delete_in_db(ctr, ip.c_str()) ? "True" : "False") << endl;
	}

	// point test
	while (true) {
		string ip;
		cin >> ip;
		if (ip == "0") {
			break;
		}
		bool result = Pointsearch_in_db(ctr, ip.c_str());
		cout << (result ? "True" : "False") << endl;
	}

	// range test
	string lower = "", upper = "";
	bool no_upper = true;
	while (true) {
		vector<string> results = Rangesearch_in_db(ctr, lower.c_str(), upper.c_str(), true, true, no_upper);
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

	Print_Container(ctr);
	//system("pause");
}