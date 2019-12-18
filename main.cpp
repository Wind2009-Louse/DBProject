#include "hyperion_db.cpp"
#include <map>

vector<string> string_split(string str, const string gap) {
	vector<string> results;
	int ptr = str.find_first_not_of(gap, 0);
	int next_ptr = str.find(gap, ptr);
	while (next_ptr != -1 || ptr != -1) {
		results.push_back(str.substr(ptr, next_ptr - ptr));
		ptr = str.find_first_not_of(gap, next_ptr);
		next_ptr = str.find(gap, ptr);
	}
	return results;
}

#define DB_INSERT 1
#define DB_SHOWALL 2
#define DB_DELETE 3
#define DB_DELETELAST 4
#define DB_SEARCH 5
#define DB_HELP 6
#define DB_EXIT 7

int main() {
	// 初始化
	string input_line;
	Hyperion_DB<string> db;

	// 查询
	vector<string> rangesearch_opes = {"<", ">","<=",">=", "!="};
	vector<pair<string, string*> > results;

	// 命令类型
	map<string, int> cmd_types;
	cmd_types["insert"] = DB_INSERT;
	cmd_types["showall"] = DB_SHOWALL;
	cmd_types["delete"] = DB_DELETE;
	cmd_types["deletelast"] = DB_DELETELAST;
	cmd_types["search"] = DB_SEARCH;
	cmd_types["help"] = DB_HELP;
	cmd_types["exit"] = DB_EXIT;

	// 输入
	while (true) {
		cout << "> ";
		getline(cin, input_line);
		vector<string> input_datas = string_split(input_line, " ");
		if (input_datas.size() == 0 || input_datas[0] == "exit") {
			break;
		}

		switch (cmd_types[input_datas[0]]){
			// 插入
			case DB_INSERT:{
				results.clear();
				if (input_datas.size() < 3) {
					cout << "Not enough arguments!" << endl;
					break;
				}
				db.Insert_into_db(input_datas[1].c_str(), new string(input_datas[2]));
				cout << "Input successfully!" << endl;
				break;
			}
			// 显示所有内容
			case DB_SHOWALL:{
				results.clear();
				results = db.Rangesearch_in_db("", "", true, true, true);
				cout << "Key\tValue\t\r\n";
				for (int i = 0; i < results.size(); ++i) {
					cout << results[i].first << "\t" << *results[i].second << endl;
				}
				cout << results.size() << " result(s)." << endl;
				break;
			}
			// 删除
			case DB_DELETE:{
				results.clear();
				try {
					if (db.Delete_in_db(input_datas[1].c_str())) {
						cout << "Delete successfully!" << endl;
					}
					else {
						cout << "Can't find such key!" << endl;
					}
				}
				catch (const char* error_msg) {
					cout << "Error occur: " << error_msg << endl;
				}
				break;
			}
			// 删除查询结果
			case DB_DELETELAST: {
				int remove_count = 0;
				for (int i = 0; i < results.size(); ++i) {
					try {
						if (db.Delete_in_db(results[i].first.c_str())) {
							remove_count++;
						}
					}
					catch (const char* error_msg) {
						cout << "Error occur: " << error_msg << endl;
					}
				}
				cout << "Remove " << remove_count << " data(s)!" << endl;
				break;
			}
			// 查询
			case DB_SEARCH: {
				if (input_datas.size() < 3 || input_datas.size() % 2 == 0) {
					cout << "Not enough arguments!" << endl;
					break;
				}

				// 点查询初始化
				if (input_datas[1] == "==") {
					if (input_datas.size() != 3) {
						cout << "Wrong arguments!" << endl;
						break;
					}
					results.clear();
					auto ps_result = db.Pointsearch_in_db(input_datas[2].c_str());
					if (ps_result.finded()) {
						results.push_back(pair<string, string*>(input_datas[2], ps_result.value_ptr));
					}
				}
				// 范围查询初始化
				else {
					string lower_str = "", upper_str = "";
					bool lower_equal, upper_equal, no_upper;
					lower_equal = upper_equal = no_upper = true;
					vector<string> except_list;
					int process_ptr = 1;

					while (input_datas.size() - process_ptr >= 2) {
						string ope = input_datas[process_ptr];
						// 出错处理
						if (find(rangesearch_opes.begin(), rangesearch_opes.end(), ope) == rangesearch_opes.end() ||
							(ope[0] == '>' && lower_str.size() > 0) ||
							(ope[0] == '<' && upper_str.size() > 0)) {
							cout << "Wrong arguments!" << endl;
							break;
						}
						if (ope[0] == '>') {
							lower_str = input_datas[process_ptr + 1];
							lower_equal = ope[1] == '=';
						}
						if (ope[0] == '<') {
							upper_str = input_datas[process_ptr + 1];
							upper_equal = ope[1] == '=';
							no_upper = false;
						}
						if (ope == "!=") {
							except_list.push_back(input_datas[process_ptr + 1]);
						}
						process_ptr += 2;
					}
					results.clear();
					results = db.Rangesearch_in_db(lower_str.c_str(), upper_str.c_str(), lower_equal, upper_equal, no_upper, except_list);
				}

				cout << "Key\tValue\t\r\n";
				for (int i = 0; i < results.size(); ++i) {
					cout << results[i].first << "\t" << *results[i].second << endl;
				}
				cout << results.size() << " result(s)." << endl;
				break;
			}
			// 帮助
			case DB_HELP: {
				cout << "Usage:\r\n\
* insert <key> <value>: Insert datas into db. \r\n\
* search [<operator> <key>]: Search by operator\r\n\
\tAvaliable operators: <, <=, >, >=, ==, !=\r\n\
* showall: Show all key-value in database.\r\n\
* delete <key>: Delete keys from db.\r\n\
* deletelast: Delete last search result.\r\n\
* help: Show help info.\r\n\
* exit: Shutdown." << endl;
				break;
			}
			// 退出
			case DB_EXIT: {
				db.erase();
				cout << "Exit." << endl;
				return 0;
				break;
			}
			// 缺省返回
			default:
				cout << "Invalid input!" << endl;
		}
	}
/*
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
	*/
}