#pragma once
#include "hyperion_db.h"

// 在数据库中查找指定Key是否存在
template <typename value_t> Pointsearch_result<value_t> Hyperion_DB<value_t>::Pointsearch_in_db(const char* str) {
	Pointsearch_result<value_t> result = Pointsearch_result<value_t>(this->ctr, str, PS_SEARCHING);
	// 直到有结果为止继续查找
	while (result.ctr && strlen(result.str)>0 && result.result == PS_SEARCHING) {
		result.ctr = result.ctr->Find_Container_with_sortkey(result.str[0]);
		result = result.Pointsearch_in_container();
	}
	return result;
}

// 释放内存
template <typename value_t> void Hyperion_DB<value_t>::erase() {
	if (this->ctr) {
		this->ctr->erase();
	}
}

// 在数据库查找区间内的Key对应的Value
template <typename value_t> vector<pair<string, value_t*> > Hyperion_DB<value_t>::Rangesearch_in_db(
	const char* lower_str, const char* upper_str,
	bool is_lower_equal, bool is_upper_equal, bool is_no_upper, vector<value_t> except_list
) {
	// 提前结束
	if (!this->ctr) {
		return vector<pair<string, value_t*> >();
	}
	vector<pair<string, value_t*> > sub_result = this->ctr->Rangesearch_in_container(
		lower_str, upper_str,
		is_lower_equal, is_upper_equal, is_no_upper);
	if (except_list.size() == 0) {
		return sub_result;
	}
	else {
		vector<pair<string, value_t*> > results;
		for (int i = 0; i < sub_result.size(); ++ i) {
			if (find(except_list.begin(), except_list.end(), *sub_result[i].second) == except_list.end()) {
				results.push_back(sub_result[i]);
			}
		}
		return results;
	}
}

// 将Key-value插入到数据库中
template <typename value_t> void Hyperion_DB<value_t>::Insert_into_db(const char* str, value_t* value_p) {
	// 提前结束
	if (!this->ctr) {
		return;
	}
	pair<Container<value_t>*, const char*> result = this->ctr->Insert_into_Container(str, value_p);
	while (result.first) {
		result = result.first->Insert_into_Container(result.second, value_p);
	}
}

// 在数据库中删除指定Key对应的数据，返回是否删除成功。
template <typename value_t> bool Hyperion_DB<value_t>::Delete_in_db(const char* str) {
	Pointsearch_result<value_t> result = Pointsearch_result<value_t>(this->ctr, str, PS_SEARCHING);
	// 直到有结果为止继续查找
	while (result.ctr && strlen(result.str)>0 && result.result == PS_SEARCHING) {
		result.ctr = result.ctr->Find_Container_with_sortkey(result.str[0]);
		result = result.Pointsearch_in_container();
	}
	// 若查不到结果，则返回false
	if (result.result == PS_FAILED) {
		return false;
	}

	string remain_str(str);
	Container<value_t>* ctr = result.ctr;
	bool deleted = false;
	while (remain_str.size() > 0 && ctr) {
		// 获取需要查找的字符
		char t_char, s_char;
		// 偶数长度
		if (remain_str.size() % 2 == 0) {
			t_char = remain_str[remain_str.size() - 2];
			s_char = remain_str[remain_str.size() - 1];
			remain_str.pop_back();
			remain_str.pop_back();
		}
		// 奇数长度
		else {
			t_char = remain_str[remain_str.size() - 1];
			s_char = 0;
			remain_str.pop_back();
		}

		// T-search
		Node<value_t>* t_node = &ctr->nodes[0];
		while (t_node->c < t_char) {
			t_node = (Node<value_t>*)t_node->ptr;
		}
		// 找不到T-Node，出错返回
		if (t_node->c > t_char) {
			throw "Unable to find T-Node!";
		}

		// S-Search
		if (s_char != 0) {
			Node<value_t>* s_node = t_node + 1;
			while (s_node->c != 0 && !s_node->type() && s_node->c < s_char) {
				s_node++;
			}
			// 找不到S-Node，出错返回
			if (s_node->c == 0 || s_node->type()) {
				throw "Unable to find S-Node!";
			}
			if (!deleted) {
				// 在没有删除过的情况下找到一个非叶子节点，出错返回
				if (!s_node->is_leaf()) {
					throw "Trying to delete non-leaf S-Node!";
				}
				deleted = true;
				s_node->deleaf();
			}
			// 在删除过的情况下找到一个叶子节点，或者该S-Node下仍有容器，说明非唯一S-Node
			// 提早结束
			if (s_node->is_leaf() || s_node->ptr != NULL) {
				return true;
			}
			// 删除S-Node
			Node<value_t>* right_node = s_node;
			int move_length = (ctr->size - (s_node - &ctr->nodes[0])) * sizeof(Node<value_t>);
			memcpy_s(s_node, move_length, s_node+1, move_length);
			// 右侧节点内容左移
			while (right_node->c != 0) {
				right_node->ptr -= (right_node->type() && right_node->ptr != NULL ? 1 : 0) * sizeof(Node<value_t>);
				right_node++;
			}
			// 左侧T-Node指针修改
			Node<value_t>* left_node = &ctr->nodes[0];
			while (left_node && left_node < s_node) {
				if (left_node->type() && left_node->ptr != NULL && left_node->ptr > s_node) {
					left_node->ptr -= sizeof(Node<value_t>);
					break;
				}
				left_node = (Node<value_t>*)left_node->ptr;
			}
			ctr->size--;
		}
		// 判断T-Node是否应该去叶
		else {
			if (!deleted) {
				// 在没有删除过的情况下找到一个非叶子节点，出错返回
				if (!t_node->is_leaf()) {
					throw "Trying to delete non-leaf T-Node!";
				}
				deleted = true;
				t_node->deleaf();
			}
		}

		// 在删除过的情况下找到一个叶子节点，或者该T-Node下仍有S-Node，说明非唯一T-Node
		// 提早结束
		if (t_node->is_leaf()) {
			return true;
		}
		else {
			Node<value_t>* next_node = t_node + 1;
			if (next_node->c != 0 && !next_node->type()) {
				return true;
			}
		}

		// 删除T-Node
		// 右侧节点内容左移
		Node<value_t>* right_node = t_node;
		int move_length = (ctr->size - (t_node - &ctr->nodes[0])) * sizeof(Node<value_t>);
		memcpy_s(t_node, move_length, t_node + 1, move_length);
		while (right_node->c != 0) {
			right_node->ptr -= (right_node->type() && right_node->ptr != NULL ? 1 : 0) * sizeof(Node<value_t>);
			right_node++;
		}
		ctr->size--;

		// 指向父节点
		Container<value_t>* parent_ctr = (Container<value_t>*)ctr->cptrs.parent_ptr;

		// 容器被清空，删除整个容器
		if (ctr->size == 0) {
			// 跳表中删除该容器
			Container<value_t>* head_ctr = (Container<value_t>*)ctr->cptrs.head_ptr;
			int level = JUMPPOINT_MAXHEIGHT - 1;
			while (level >= 0 && head_ctr) {
				Container<value_t>* next_ctr = (Container<value_t>*)head_ctr->cptrs.ptrs[level];
				if (next_ctr == NULL) {
					level--;
				}
				else if (next_ctr != ctr) {
					head_ctr = next_ctr;
				}
				else {
					head_ctr->cptrs.ptrs[level] = ctr->cptrs.ptrs[level];
					level--;
				}
			}

			// 父容器中删除指向该容器的指针
			head_ctr = (Container<value_t>*)ctr->cptrs.parent_ptr;
			// 找不到父容器，出错返回
			if (head_ctr == NULL) {
				throw "Trying to delete a container without parent!";
			}
			Node<value_t>* node_ptr = &head_ctr->nodes[0];
			bool removed_from_parent = false;
			while (node_ptr->c != 0) {
				if (node_ptr->ptr == ctr) {
					node_ptr->ptr = NULL;
					removed_from_parent = true;
					break;
				}
				node_ptr++;
			}

			// 没有在父容器中找到自己，出错返回
			if (!removed_from_parent) {
				throw "Trying to delete a container can't find from its parent!";
			}

			delete ctr;
		}
		ctr = parent_ctr;
	}
	return true;
}