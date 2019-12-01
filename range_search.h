#pragma once
#include "header.h"

// 在container中查找在lower_str和upper_str范围的字符串
vector<string> Rangesearch_in_container(
	Container* ctr, const char* lower_str, const char* upper_str,
	bool is_lower_equal = false, bool is_upper_equal = false, bool is_no_upper = false
) {
	// 结果向量
	vector<string> results;

	// 寻找范围查询起点
	Container* ctr_ptr;
	if (strlen(lower_str) == 0) {
		ctr_ptr = (Container*)ctr->cptrs->head_ptr;
	}
	else {
		ctr_ptr = Find_Container_with_sortkey(ctr, lower_str[0]);
	}

	// 开始查找
	while (ctr_ptr) {
		// T-search
		Node* t_node = &ctr_ptr->nodes[0];
		while (t_node && t_node->type()) {
			// 判断是否超过上界，若超过则不继续向下搜索
			if (!is_no_upper && (t_node->c > upper_str[0] || (!is_upper_equal && t_node->c == upper_str[0]))) {
				break;
			}
			// 判断是否在下界内
			if ((lower_str[0] < t_node->c || (is_lower_equal && lower_str[0] == t_node->c)))
			{
				// 叶子判断
				if (t_node->is_leaf() && (lower_str[0] < t_node->c || strlen(lower_str) < 2)) {
					results.push_back(string(1,t_node->c));
				}
				// 是否超出上界，若超过则不继续向下搜索
				if (is_no_upper || strlen(upper_str) > 1 || t_node->c < upper_str[0] ) {
					// S-search
					Node* s_node = t_node + 1;
					bool no_upper = is_no_upper || t_node->c < upper_str[0];
					while (s_node && !s_node->type() && s_node->c != 0) {
						// 判断是否超出上界，若超过则不继续向下搜索
						if (!no_upper && (s_node->c > upper_str[1] || (!is_upper_equal && s_node->c == upper_str[1]))) {
							break;
						}
						// 判断是否超过下界
						char low_char = strlen(lower_str) < 2 ? 0 : lower_str[1];
						if (low_char < s_node->c || (is_lower_equal && low_char == s_node->c)) {
							string prefix = { t_node->c, s_node->c };
							// 叶子判断
							if (s_node->is_leaf()) {
								results.push_back(prefix);
							}
							bool sub_no_upper = no_upper || s_node->c < upper_str[1];
							// 有子容器，往下搜索
							if (s_node->ptr) {
								const char* sub_lower = (strlen(lower_str) < 2) ? "" : &lower_str[2];
								vector<string> subset = Rangesearch_in_container((Container*)s_node->ptr, sub_lower, &upper_str[2], is_lower_equal, is_upper_equal, sub_no_upper);
								// 添加前缀
								for (int i = 0; i < subset.size(); ++i) {
									subset[i] = prefix + subset[i];
								}
								results.insert(results.end(), subset.begin(), subset.end());
							}
						}
						s_node++;
					}
				}
			}
			t_node = (Node*)t_node->ptr;
		}
		ctr_ptr = (Container*)ctr_ptr->cptrs->ptrs[0];
	}

	// TODO
	return results;
}
