#include "hyperion_db.cpp"

int main() {
	Hyperion_DB<int> db;

	// input
	while (true) {
		string ip;
		cin >> ip;
		if (ip == "0") {
			break;
		}
		int* value_ptr = new int(ip.size());
		db.Insert_into_db(ip.c_str(), value_ptr);
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
		Pointsearch_result<int> result = db.Pointsearch_in_db(ip.c_str());
		if (!result.finded()) {
			cout << "False";
		}
		else {
			cout << ip << "\t" << *result.value_ptr;
		}
		cout << endl;
	}

	// range test
	string lower = "", upper = "";
	bool no_upper = true;
	while (true) {
		vector<pair<string, int*> > results = db.Rangesearch_in_db(lower.c_str(), upper.c_str(), true, true, no_upper);
		//cout << "Results:" << endl;
		for (int i = 0; i < results.size(); ++i) {
			//cout << '\t' << results[i] << endl;
			cout << results[i].first << "\t" << *results[i].second << endl;
		}

		no_upper = false;
		cin >> lower >> upper;
		if (lower == "0" && upper == "0") {
			break;
		}
	}
	//system("pause");
}