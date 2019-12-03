#pragma once
#include "header.h"
#include "pointer_search.h"

// 根据sortkey，在Container列表中创建一个新的Container
Container* Create_Container(Container* ctr, char sortkey) {
	Container* head_ctr = (Container*)ctr->cptrs->head_ptr;
	// 新Container的最高指针等级
	int level = 0;
	for (int i = 0; i < JUMPPOINT_MAXHEIGHT - 1 && randnum(2)>0; ++i) {
		level++;
	}

	// 最靠近目标的Container
	vector<Container*> nearliest_ctrs(JUMPPOINT_MAXHEIGHT, head_ctr);
	int current_level = level;
	while (current_level >= 0) {
		Container* next_ptr = (Container*)nearliest_ctrs[current_level]->cptrs->ptrs[current_level];
		while (next_ptr && next_ptr->nodes[0].c <= sortkey) {
			nearliest_ctrs[current_level] = (Container*)nearliest_ctrs[current_level]->cptrs->ptrs[current_level];
			next_ptr = (Container*)nearliest_ctrs[current_level]->cptrs->ptrs[current_level];
		}
		current_level--;
	}

	// 创建
	Container* new_ctr = new Container(head_ctr);
	for (int i = 0; i <= level; ++i) {
		new_ctr->cptrs->ptrs[i] = nearliest_ctrs[i]->cptrs->ptrs[i];
		nearliest_ctrs[i]->cptrs->ptrs[i] = new_ctr;
	}
	return new_ctr;
}

// 对Container进行分割
void Container_Split(Container* ctr) {
	// 找中间的T-Node
	Node* last_ptr = &ctr->nodes[0];
	Node* fast_ptr = &ctr->nodes[0];
	Node* slow_ptr = &ctr->nodes[0];
	while (fast_ptr) {
		slow_ptr = (Node*)slow_ptr->ptr;
		fast_ptr = (Node*)fast_ptr->ptr;
		if (fast_ptr) fast_ptr = (Node*)fast_ptr->ptr;
		if (fast_ptr) last_ptr = slow_ptr;
	}

	// 创建新容器
	char sortkey = slow_ptr->c;
	Container* new_ctr = Create_Container(ctr, sortkey);

	// 搬运
	last_ptr->ptr = NULL;
	int count = 0;
	while (slow_ptr->c != 0) {
		// 复制数据
		new_ctr->nodes[count].header = slow_ptr->header;
		new_ctr->nodes[count].c = slow_ptr->c;
		new_ctr->nodes[count].ptr = slow_ptr->ptr;
		if (slow_ptr->ptr) {
			// T-Node需要更新指针坐标
			if (new_ctr->nodes[count].type()) {
				new_ctr->nodes[count].ptr = &new_ctr->nodes[count] + ((Node*)slow_ptr->ptr - (Node*)slow_ptr);
			}
			// S-Node需要让子容器指向新容器
			else {
				Container* child_ctr = (Container*)new_ctr->nodes[count].ptr;
				child_ctr->cptrs->parent_ptr = new_ctr;
			}
		}
		count++;
		fast_ptr = slow_ptr;
		slow_ptr++;
		// 清空
		fast_ptr->header = 0;
		fast_ptr->c = 0;
		fast_ptr->ptr = NULL;
	}
	new_ctr->size = count;
	ctr->size -= count;
}

// 将字符串插入到Container中
// 返回下一个需要查找的Container和str，通过迭代避免栈溢出
pair<Container*, const char*> Insert_into_Container(Container* ctr, const char* str) {
	// 结果初始化
	Container* result_ctr = NULL;
	const char* result_str = str;

	// 不插入空字符串
	if (strlen(str) == 0) {
		return pair<Container*, const char*>(result_ctr, result_str);
	}

	// 查找是否在自身Node中可以插入

	// 节点指针
	Node* node_ptr = &ctr->nodes[0];

	// 检查下一个容器
	bool check_next_container = false;
	Node* last_tnode = NULL;

	// 插入过内容，需要检查是否需要分裂
	bool inserted = false;

	// 在当前容器中查找是否可以插入
	while (node_ptr) {
		// T-search
		// 找不到匹配的T-Node
		if (ctr->size == 0 || node_ptr->c != str[0]) {
			// 找到当前容器的最后一个T-Node仍未符合，则直接在当前容器内插入
			if (ctr->size == 0 || node_ptr->c > str[0]) {
				int move_offset = strlen(str) > 1 ? 2 : 1;
				int point_index = node_ptr - &ctr->nodes[0];
				// 节点向前移动
				for (int idx = ctr->size - 1; idx >= point_index; --idx) {
					ctr->nodes[idx + move_offset] = ctr->nodes[idx];
					ctr->nodes[idx + move_offset].ptr += (ctr->nodes[idx].type() && (ctr->nodes[idx].ptr != NULL) ? move_offset : 0) * sizeof(Node);
				}

				// 插入T-Node
				ctr->nodes[point_index].header = T_NODE;
				if (strlen(str) == 1) {
					ctr->nodes[point_index].beleaf();
				}
				// 判断是在列表中插入新T-Node还是容器为空时新增T-Node
				ctr->nodes[point_index].ptr = ctr->size == 0 ? NULL : &ctr->nodes[point_index + move_offset];
				ctr->nodes[point_index].c = str[0];

				// 插入S-Node
				if (strlen(str) > 1) {
					ctr->nodes[point_index + 1].header = S_NODE;
					if (strlen(str) == 2) {
						ctr->nodes[point_index + 1].beleaf();
					}
					ctr->nodes[point_index + 1].c = str[1];
					if (strlen(str) > 2) {
						Container* new_ctr = new Container();
						ctr->nodes[point_index + 1].ptr = new_ctr;
						new_ctr->cptrs->head_ptr = new_ctr;
						new_ctr->cptrs->parent_ptr = ctr;
						// 在新容器中更新
						result_ctr = new_ctr;
						result_str = &str[2];
					}
					else {
						ctr->nodes[point_index + 1].ptr = NULL;
					}
				}

				// 结束
				inserted = true;
				ctr->size += move_offset;
				break;
			}
			// 向前查下一个T-Node
			if (node_ptr->c < str[0]) {
				last_tnode = node_ptr;
				node_ptr = (Node*)node_ptr->ptr;
				// 查到当前Container尽头
				if (node_ptr == NULL) {
					check_next_container = true;
					break;
				}
			}
		}
		// S-search
		else {
			// 用LEAF_NODE标记奇数长度单词
			if (strlen(str) == 1) {
				node_ptr->beleaf();
				break;
			}
			Node* t_node_ptr = node_ptr;
			// 从下一个节点查起
			node_ptr++;

			while (node_ptr) {
				// 查到当前T节点尽头，或者查到比自己大的S-Node
				if (node_ptr->c == 0 || node_ptr->type() || node_ptr->c > str[1]) {
					int point_index = node_ptr - &ctr->nodes[0];
					// 节点向前移动
					for (int idx = ctr->size - 1; idx >= point_index; --idx) {
						ctr->nodes[idx + 1] = ctr->nodes[idx];
						ctr->nodes[idx + 1].ptr += (ctr->nodes[idx].type() && ctr->nodes[idx].ptr != NULL ? 1 : 0) * sizeof(Node);
					}
					t_node_ptr->ptr += sizeof(Node) * (t_node_ptr->ptr == NULL ? 0 : 1);
					// 插入S-Node
					ctr->nodes[point_index].header = S_NODE;
					if (strlen(str) == 2) {
						ctr->nodes[point_index].beleaf();
					}
					ctr->nodes[point_index].c = str[1];
					// 指针指向
					if (strlen(str) > 2) {
						Container* new_ctr = new Container();
						ctr->nodes[point_index].ptr = new_ctr;
						new_ctr->cptrs->head_ptr = new_ctr;
						new_ctr->cptrs->parent_ptr = ctr;
						// 在新容器中更新
						result_ctr = new_ctr;
						result_str = &str[2];
					}
					else {
						ctr->nodes[point_index].ptr = NULL;
					}

					// 结束
					inserted = true;
					ctr->size += 1;
					break;
				}
				// 找到相同前缀
				else if (node_ptr->c == str[1]) {
					if (strlen(str) < 3) {
						node_ptr->beleaf();
					}
					if (strlen(str) > 2) {
						// 如果没有子容器，则创建新容器在其中搜索
						if (node_ptr->ptr == NULL) {
							Container* new_ctr = new Container();
							node_ptr->ptr = new_ctr;
							new_ctr->cptrs->head_ptr = new_ctr;
							new_ctr->cptrs->parent_ptr = ctr;
							// 在新容器中更新
							result_ctr = new_ctr;
						}
						// 在旧容器中搜索
						else {
							result_ctr = (Container*)node_ptr->ptr;
						}
						result_str = &str[2];
					}
					break;
				}
				// 向前搜索
				else if (node_ptr->c < str[1]) {
					node_ptr++;
				}
			}
			break;
		}
	}

	if (check_next_container) {
		Container* next_ctr = Find_Container_with_sortkey(ctr, str[0]);
		// 当前为最后一个适合的容器，则直接插入在当前容器中
		if (next_ctr == ctr) {
			// 插入T-Node
			ctr->nodes[ctr->size].header = T_NODE;
			if (strlen(str) == 1) {
				ctr->nodes[ctr->size].beleaf();
			}
			ctr->nodes[ctr->size].c = str[0];
			ctr->nodes[ctr->size].ptr = NULL;
			last_tnode->ptr = &ctr->nodes[ctr->size];
			// 插入S-Node
			if (strlen(str) > 1) {
				ctr->nodes[ctr->size + 1].header = S_NODE;
				if (strlen(str) == 2) {
					ctr->nodes[ctr->size + 1].beleaf();
				}
				ctr->nodes[ctr->size + 1].c = str[1];
				if (strlen(str) > 2) {
					Container* new_ctr = new Container();
					ctr->nodes[ctr->size + 1].ptr = new_ctr;
					new_ctr->cptrs->head_ptr = new_ctr;
					new_ctr->cptrs->parent_ptr = ctr;
					// 在新容器中更新
					result_ctr = new_ctr;
					result_str = &str[2];
				}
				else {
					ctr->nodes[ctr->size + 1].ptr = NULL;
				}
			}

			inserted = true;
			ctr->size += strlen(str) > 1 ? 2 : 1;
		}
		// 查找下一个容器
		else {
			result_ctr = next_ctr;
			result_str = str;
		}
	}

	// 容器不足时分裂
	if (inserted && ctr->size * 4 / 3 > CONTAINER_SIZE) {
		Container_Split(ctr);
	}
	return pair<Container*, const char*>(result_ctr, result_str);
}

// 将字符串插入到数据库中
void Insert_into_db(Container* ctr, const char* str) {
	pair<Container*, const char*> result = Insert_into_Container(ctr, str);
	while (result.first) {
		result = Insert_into_Container(result.first, result.second);
	}
}

// 在数据库中删除指定数据库，返回是否删除成功。
bool Delete_in_db(Container* input_ctr, const char* str) {
	Pointsearch_result result = Pointsearch_result(input_ctr, str, PS_SEARCHING);
	// 直到有结果为止继续查找
	while (result.ctr && strlen(result.str)>0 && result.result == PS_SEARCHING) {
		result.ctr = Find_Container_with_sortkey(result.ctr, result.str[0]);
		result = Pointsearch_in_container(result);
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
		Container* parent_ctr = (Container*)ctr->cptrs->parent_ptr;
		
		// 容器被清空，删除整个容器
		if (ctr->size == 0) {
			// 跳表中删除该容器
			Container* head_ctr = (Container*)ctr->cptrs->head_ptr;
			int level = JUMPPOINT_MAXHEIGHT - 1;
			while (level >= 0 && head_ctr) {
				Container* next_ctr = (Container*)head_ctr->cptrs->ptrs[level];
				if (next_ctr == NULL){
					level--;
				}
				else if (next_ctr != ctr) {
					head_ctr = next_ctr;
				}
				else {
					head_ctr->cptrs->ptrs[level] = ctr->cptrs->ptrs[level];
					level--;
				}
			}

			// 父容器中删除指向该容器的指针
			head_ctr = (Container*)ctr->cptrs->parent_ptr;
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