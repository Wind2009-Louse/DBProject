#pragma once
#include "container.cpp"

// 正在查询中
#define PS_SEARCHING 0
// 查询结束，找到结果
#define PS_SEARCHED 1
// 查询结束，找不到结果
#define PS_FAILED 2

// 点查询返回数据格式
template<typename value_t> struct Pointsearch_result {
	// 下一个需要查找的容器
	Container<value_t>* ctr;
	// 需要查找的字符串
	const char* str;
	// value指针
	value_t* value_ptr;
	// 查询结果
	short result;

	// 初始化
	Pointsearch_result(Container<value_t>* c, const char* s, short r = PS_SEARCHING, value_t* result_ptr = NULL) :ctr(c), str(s), result(r), value_ptr(result_ptr) {
	}

	inline bool finded() {
		return result == PS_SEARCHED;
	}

	// 在container中查找字符串
	Pointsearch_result<value_t> Pointsearch_in_container();
};
