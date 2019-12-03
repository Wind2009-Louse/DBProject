#pragma once
#include "pointsearch_result.h"

template<typename value_t>
Pointsearch_result<value_t> Pointsearch_result<value_t>::Pointsearch_in_container() {
	// 初始化结果
	Container<value_t>* return_ctr = this->ctr;
	const char* return_str = this->str;
	short return_result = PS_FAILED;
	value_t* return_value_ptr = NULL;

	// T-Search
	Node<value_t>* node_ptr = &this->ctr->nodes[0];
	while (node_ptr && node_ptr->c < this->str[0]) {
		node_ptr = (Node<value_t>*)node_ptr->ptr;
	}
	// 查找失败
	if (!node_ptr || node_ptr->c > this->str[0]) {
		return_result = PS_FAILED;
	}
	// 奇数长度判断
	else if (strlen(this->str) == 1 && node_ptr->is_leaf()) {
		return_result = PS_SEARCHED;
		return_value_ptr = node_ptr->value_ptr;
	}
	// S-Search
	else {
		node_ptr++;
		while (node_ptr->c != 0 && !node_ptr->type() && node_ptr->c != this->str[1]) {
			node_ptr++;
		}
		// 查找失败
		if (node_ptr->c == 0 || node_ptr->type()) {
			return_result = PS_FAILED;
		}
		// 字符串末尾
		else if (strlen(this->str) == 2) {
			return_result = node_ptr->is_leaf() ? PS_SEARCHED : PS_FAILED;
			return_value_ptr = node_ptr->is_leaf() ? node_ptr->value_ptr : NULL;
		}
		// 到下个容器查询
		else {
			return_ctr = (Container<value_t>*)node_ptr->ptr;
			return_str = &this->str[2];
			return_result = PS_SEARCHING;
		}
	}

	// 返回结果
	return Pointsearch_result(return_ctr, return_str, return_result, return_value_ptr);
}