#pragma once
#include "header.h"

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
};

// 在container中查找字符串
Pointsearch_result Pointsearch_in_container(Pointsearch_result datas) {
	// 初始化结果
	Container* return_ctr = NULL;
	const char* return_str = datas.str;
	short return_result = PS_FAILED;

	// 空字符串直接返回
	if (strlen(datas.str) == 0) {
		// 返回结果
		return Pointsearch_result(return_ctr, return_str, return_result);
	}

	// 开始查找
	Container* ctr = Find_Container_with_sortkey(datas.ctr, datas.str[0]);
	// 在当前容器中查找
	if (ctr == datas.ctr) {
		// T-Search
		Node* node_ptr = &datas.ctr->nodes[0];
		while (node_ptr && node_ptr->c < datas.str[0]) {
			node_ptr = (Node*)node_ptr->ptr;
		}
		// 查找失败
		if (!node_ptr || node_ptr->c > datas.str[0]) {
			return_result = PS_FAILED;
		}
		// 奇数长度判断
		else if (strlen(datas.str) == 1 && node_ptr->is_leaf()) {
			return_result = PS_SEARCHED;
		}
		// S-Search
		else {
			node_ptr++;
			while (node_ptr->c != 0 && !node_ptr->type() && node_ptr->c != datas.str[1]) {
				node_ptr++;
			}
			// 查找失败
			if (node_ptr->c == 0 || node_ptr->type()) {
				return_result = PS_FAILED;
			}
			// 字符串末尾
			else if (strlen(datas.str) == 2) {
				return_result = node_ptr->is_leaf() ? PS_SEARCHED : PS_FAILED;
			}
			// 到下个容器查询
			else {
				return_ctr = (Container*)node_ptr->ptr;
				return_str = &datas.str[2];
				return_result = PS_SEARCHING;
			}
		}
	}
	// 到下一个容器查找
	else {
		return_ctr = ctr;
		return_result = PS_SEARCHING;
	}
	
	// 返回结果
	return Pointsearch_result(return_ctr, return_str, return_result);
}

// 在数据库中查找字符串
bool Pointsearch_in_db(Container* ctr, const char* str) {
	Pointsearch_result result = Pointsearch_in_container(Pointsearch_result(ctr, str));
	while (result.ctr && (result.result == PS_SEARCHING)) {
		result = Pointsearch_in_container(result);
	}
	return result.result == PS_SEARCHED;
}
