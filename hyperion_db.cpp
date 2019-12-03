#include "hyperion_db.h"

// 数据库初始化
void Hyperion_DB::Init() {
	this->ctr = Container::Create_Empty_Container();
}

// 在数据库中查找字符串
bool Hyperion_DB::Pointsearch_in_db(const char* str) {
	Pointsearch_result result = Pointsearch_result(this->ctr, str, PS_SEARCHING);
	// 直到有结果为止继续查找
	while (result.ctr && strlen(result.str)>0 && result.result == PS_SEARCHING) {
		result.ctr = result.ctr->Find_Container_with_sortkey(result.str[0]);
		result = result.Pointsearch_in_container();
	}
	return result.result == PS_SEARCHED;
}

// 在数据库查找区间内的字符串
vector<string> Hyperion_DB::Rangesearch_in_db(
	const char* str_1, const char* str_2,
	bool is_lower_equal, bool is_upper_equal, bool is_no_upper
) {
	// 判断字符串大小
	int cmp_result = strcmp(str_1, str_2);
	const char* lower_str = (cmp_result < 0) ? str_1 : str_2;
	const char* upper_str = (cmp_result > 0) ? str_1 : str_2;
	return this->ctr->Rangesearch_in_container(
		lower_str, upper_str,
		is_lower_equal, is_upper_equal, is_no_upper);
}

// 将字符串插入到数据库中
void Hyperion_DB::Insert_into_db(const char* str) {
	// 提前结束
	if (!ctr) {
		return;
	}
	pair<Container*, const char*> result = this->ctr->Insert_into_Container(str);
	while (result.first) {
		result = result.first->Insert_into_Container(result.second);
	}
}

// 在数据库中删除指定数据，返回是否删除成功。
bool Hyperion_DB::Delete_in_db(const char* str) {
	Pointsearch_result result = Pointsearch_result(this->ctr, str, PS_SEARCHING);
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
	Container* ctr = result.ctr;
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
		Node* t_node = &ctr->nodes[0];
		while (t_node->c < t_char) {
			t_node = (Node*)t_node->ptr;
		}
		// 找不到T-Node，出错返回
		if (t_node->c > t_char) {
			return false;
		}

		// S-Search
		if (s_char != 0) {
			Node* s_node = t_node + 1;
			while (s_node->c != 0 && !s_node->type() && s_node->c < s_char) {
				s_node++;
			}
			// 找不到S-Node，出错返回
			if (s_node->c == 0 || s_node->type()) {
				return false;
			}
			if (!deleted) {
				// 在没有删除过的情况下找到一个非叶子节点，出错返回
				if (!s_node->is_leaf()) {
					return false;
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
			Node* right_node = s_node;
			// 右侧节点内容左移
			while (right_node->c != 0) {
				Node* next_node = right_node + 1;
				right_node->header = next_node->header;
				right_node->c = next_node->c;
				right_node->ptr = next_node->ptr;
				right_node->ptr -= (right_node->type() && right_node->ptr != NULL ? 1 : 0) * sizeof(Node);
				right_node = next_node;
			}
			// 左侧T-Node指针修改
			Node* left_node = &ctr->nodes[0];
			while (left_node && left_node < s_node) {
				if (left_node->type() && left_node->ptr != NULL && left_node->ptr > s_node) {
					left_node->ptr -= sizeof(Node);
					break;
				}
				left_node = (Node*)left_node->ptr;
			}
			ctr->size--;
		}
		// 判断T-Node是否应该去叶
		else {
			if (!deleted) {
				// 在没有删除过的情况下找到一个非叶子节点，出错返回
				if (!t_node->is_leaf()) {
					return false;
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
			Node* next_node = t_node + 1;
			if (next_node->c != 0 && !next_node->type()) {
				return true;
			}
		}

		// 删除T-Node
		// 右侧节点内容左移
		Node* right_node = t_node;
		while (right_node->c != 0) {
			Node* next_node = right_node + 1;
			right_node->header = next_node->header;
			right_node->c = next_node->c;
			right_node->ptr = next_node->ptr;
			right_node->ptr -= (right_node->type() && right_node->ptr != NULL ? 1 : 0) * sizeof(Node);
			right_node = next_node;
		}
		ctr->size--;

		// 指向父节点
		Container* parent_ctr = (Container*)ctr->cptrs.parent_ptr;

		// 容器被清空，删除整个容器
		if (ctr->size == 0) {
			// 跳表中删除该容器
			Container* head_ctr = (Container*)ctr->cptrs.head_ptr;
			int level = JUMPPOINT_MAXHEIGHT - 1;
			while (level >= 0 && head_ctr) {
				Container* next_ctr = (Container*)head_ctr->cptrs.ptrs[level];
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
			head_ctr = (Container*)ctr->cptrs.parent_ptr;
			// 找不到父容器，出错返回
			if (head_ctr == NULL) {
				return false;
			}
			Node* node_ptr = &head_ctr->nodes[0];
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
				return false;
			}

			delete ctr;
		}
		ctr = parent_ctr;
	}
	return true;
}