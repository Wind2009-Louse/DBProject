#pragma once
#include "container.h"

#define PS_SEARCHING 0
#define PS_SEARCHED 1
#define PS_FAILED 2

// 点查询返回数据格式
struct Pointsearch_result {
	// 下一个需要查找的容器
	Container* ctr;
	// 需要查找的字符串
	const char* str;
	// 查询结果
	short result;
	// 初始化
	Pointsearch_result(Container* c, const char* s, short r = PS_SEARCHING) :ctr(c), str(s), result(r) {
	}

	// 在container中查找字符串
	Pointsearch_result Pointsearch_in_container();
};
