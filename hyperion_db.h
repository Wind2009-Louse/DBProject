#pragma once
#include "container.cpp"
#include "pointsearch_result.cpp"

template <typename value_t> class Hyperion_DB {
	Container<value_t>* ctr;
public:
	// 数据库初始化
	Hyperion_DB() {
		ctr = Container<value_t>::Create_Empty_Container();
	}

	// 在数据库中查找指定Key是否存在
	Pointsearch_result<value_t> Pointsearch_in_db(const char* str);

	// 在数据库查找区间内的Key对应的Value
	vector<pair<string, value_t*> > Rangesearch_in_db(
		const char* str_1, const char* str_2,
		bool is_lower_equal = false, bool is_upper_equal = false, bool is_no_upper = false, vector<value_t> except_list = vector<value_t>()
	);

	// 将Key-value插入到数据库中
	void Insert_into_db(const char* str, value_t* value_p);

	// 在数据库中删除指定Key对应的数据，返回是否删除成功。
	bool Delete_in_db(const char* str);
};