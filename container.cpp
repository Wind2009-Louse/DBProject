#pragma once
#include "container.h"

// 打印容器内容进行测试
template <typename value_t> void Container<value_t>::Print_Container() {
	Container* head_ctr = (Container<value_t>*)this->cptrs.head_ptr;
	Container* ctr_ptr = head_ctr;
	while (ctr_ptr) {
		if (this == ctr_ptr) {
			cout << "> ";
		}
		cout << ctr_ptr->size << ctr_ptr->nodes[0].c << "\t";
		for (int i = 0; i < JUMPPOINT_MAXHEIGHT; ++i) {
			if (ctr_ptr->cptrs.ptrs[i] == NULL) {
				cout << "NULL\t";
			}
			else {
				Container<value_t>* _ctr = (Container<value_t>*)ctr_ptr->cptrs.ptrs[i];
				cout << _ctr->size << _ctr->nodes[0].c << '\t';
			}
		}
		cout << endl;
		ctr_ptr = (Container<value_t>*)ctr_ptr->cptrs.ptrs[0];
	}
}

// 在container中查找在lower_str和upper_str范围的字符串
template<typename value_t> vector<pair<string, value_t*> > Container<value_t>::Rangesearch_in_container(
	const char* lower_str, const char* upper_str,
	bool is_lower_equal, bool is_upper_equal, bool is_no_upper
) {
	// 结果向量
	vector<pair<string, value_t*> > results;

	// 寻找范围查询起点
	Container<value_t>* ctr_ptr;
	if (strlen(lower_str) == 0) {
		ctr_ptr = (Container<value_t>*)this->cptrs.head_ptr;
	}
	else {
		ctr_ptr = this->Find_Container_with_sortkey(lower_str[0]);
	}

	// 开始查找
	while (ctr_ptr) {
		// T-search
		Node<value_t>* t_node = &ctr_ptr->nodes[0];
		while (t_node && t_node->type()) {
			// 判断是否超过上界，若超过则不继续向下搜索
			if (!is_no_upper && t_node->c > upper_str[0]) {
				break;
			}
			// 判断是否在下界内
			if (lower_str[0] <= t_node->c)
			{
				// 叶子判断
				if (t_node->is_leaf() && (lower_str[0] < t_node->c || (strlen(lower_str) < 2 && is_lower_equal))) {
					results.push_back(pair<string, value_t*>(string(1, t_node->c), t_node->value_ptr));
				}
				// 是否超出上界，若超过则不继续向下搜索
				if (is_no_upper || strlen(upper_str) > 1 || t_node->c < upper_str[0]) {
					// S-search
					Node<value_t>* s_node = t_node + 1;
					bool no_upper = is_no_upper || t_node->c < upper_str[0];
					char low_char = (strlen(lower_str) < 2 || lower_str[0] < t_node->c) ? 0 : lower_str[1];
					while (s_node && !s_node->type() && s_node->c != 0) {
						// 判断是否超出上界，若超过则不继续向下搜索
						if (!no_upper && s_node->c > upper_str[1]) {
							break;
						}
						// 判断是否超过下界
						if (low_char <= s_node->c) {
							string prefix = { t_node->c, s_node->c };
							// 叶子判断
							if (s_node->is_leaf() &&
								(strcmp(lower_str, prefix.c_str()) < 0 || (is_lower_equal && strlen(lower_str) < 3)) &&
								(no_upper || strcmp(upper_str, prefix.c_str()) > 0 || (is_upper_equal && strlen(upper_str) < 3))
								) {
								results.push_back(pair<string, value_t*>(prefix, s_node->value_ptr));
							}
							bool sub_no_upper = no_upper || s_node->c < upper_str[1];
							// 有子容器，往下搜索
							if (s_node->ptr) {
								const char* sub_lower = (strlen(lower_str) < 2 || strcmp(lower_str, prefix.c_str()) < 0) ? "" : &lower_str[2];
								vector<pair<string, value_t*>> subset = ((Container<value_t>*)s_node->ptr)->Rangesearch_in_container(sub_lower, &upper_str[2], is_lower_equal, is_upper_equal, sub_no_upper);
								// 添加前缀
								for (int i = 0; i < subset.size(); ++i) {
									subset[i].first = prefix + subset[i].first;
								}
								results.insert(results.end(), subset.begin(), subset.end());
							}
						}
						s_node++;
					}
				}
			}
			t_node = (Node<value_t>*)t_node->ptr;
		}
		ctr_ptr = (Container<value_t>*)ctr_ptr->cptrs.ptrs[0];
	}

	// TODO
	return results;
}

// 根据T-Node查找容器或者最接近的容器
template <typename value_t> Container<typename value_t>* Container<value_t>::Find_Container_with_sortkey(char sorykey) {
	Container<typename value_t>* head_ctr = (Container<typename value_t>*)this->cptrs.head_ptr;
	int level = JUMPPOINT_MAXHEIGHT - 1;
	Container<typename value_t>* current_ctr = head_ctr;
	while (level >= 0) {
		Container<value_t>* next_ctr = (Container<value_t>*)current_ctr->cptrs.ptrs[level];
		if (next_ctr == NULL || next_ctr->nodes[0].c > sorykey) {
			level--;
			continue;
		}
		current_ctr = next_ctr;
	}
	return current_ctr;
}

// 对Container进行分割
template <typename value_t> void Container<value_t>::Container_Split() {
	// 找中间的T-Node
	Node<value_t>* last_ptr = &this->nodes[0];
	Node<value_t>* fast_ptr = &this->nodes[0];
	Node<value_t>* slow_ptr = &this->nodes[0];
	while (fast_ptr) {
		slow_ptr = (Node<value_t>*)slow_ptr->ptr;
		fast_ptr = (Node<value_t>*)fast_ptr->ptr;
		if (fast_ptr) fast_ptr = (Node<value_t>*)fast_ptr->ptr;
		if (fast_ptr) last_ptr = slow_ptr;
	}

	// 创建新容器
	char sortkey = slow_ptr->c;
	Container<value_t>* new_ctr = this->Create_Container(sortkey);

	// 搬运
	last_ptr->ptr = NULL;
	int count = 0;
	int copy_length = (this->size - (slow_ptr - &this->nodes[0])) * sizeof(Node<value_t>);
	memcpy_s(&new_ctr->nodes[0], copy_length, slow_ptr, copy_length);
	fast_ptr = slow_ptr;
	while (slow_ptr->c != 0) {
		if (new_ctr->nodes[count].ptr) {
			// T-Node需要更新指针坐标
			if (new_ctr->nodes[count].type()) {
				new_ctr->nodes[count].ptr = &new_ctr->nodes[count] + ((Node<value_t>*)slow_ptr->ptr - (Node<value_t>*)slow_ptr);
			}
			// S-Node需要让子容器指向新容器
			else {
				Container<value_t>* child_ctr = (Container<value_t>*)new_ctr->nodes[count].ptr;
				child_ctr->cptrs.parent_ptr = new_ctr;
			}
		}
		count++;
		slow_ptr++;
	}
	// 清空
	memset(fast_ptr, 0, copy_length);
	new_ctr->size = count;
	this->size -= count;
}

// 根据sortkey，在Container列表中创建一个新的Container
template <typename value_t> Container<value_t>* Container<value_t>::Create_Container(char sortkey) {
	Container<value_t>* head_ctr = (Container<value_t>*)this->cptrs.head_ptr;
	// 新Container的最高指针等级
	int level = 0;
	for (int i = 0; i < JUMPPOINT_MAXHEIGHT - 1 && randnum(2)>0; ++i) {
		level++;
	}

	// 最靠近目标的Container
	vector<Container<value_t>* > nearliest_ctrs(JUMPPOINT_MAXHEIGHT, head_ctr);
	int current_level = level;
	while (current_level >= 0) {
		Container<value_t>* next_ptr = (Container<value_t>*)nearliest_ctrs[current_level]->cptrs.ptrs[current_level];
		while (next_ptr && next_ptr->nodes[0].c <= sortkey) {
			nearliest_ctrs[current_level] = (Container<value_t>*)nearliest_ctrs[current_level]->cptrs.ptrs[current_level];
			next_ptr = (Container<value_t>*)nearliest_ctrs[current_level]->cptrs.ptrs[current_level];
		}
		current_level--;
	}

	// 创建
	Container<value_t>* new_ctr = new Container<value_t>(head_ctr);
	for (int i = 0; i <= level; ++i) {
		new_ctr->cptrs.ptrs[i] = nearliest_ctrs[i]->cptrs.ptrs[i];
		nearliest_ctrs[i]->cptrs.ptrs[i] = new_ctr;
	}
	return new_ctr;
}

// 将字符串插入到Container中
// 返回下一个需要查找的Container和str，通过迭代避免栈溢出
template <typename value_t> pair<Container<value_t>*, const char*> Container<value_t>::Insert_into_Container(const char* str, value_t* value_p) {
	// 结果初始化
	Container<value_t>* result_ctr = NULL;
	const char* result_str = str;

	// 不插入空字符串
	if (strlen(str) == 0) {
		return pair<Container*, const char*>(result_ctr, result_str);
	}

	// 查找是否在自身Node中可以插入

	// 节点指针
	Node<value_t>* node_ptr = &this->nodes[0];

	// 检查下一个容器
	bool check_next_container = false;
	Node<value_t>* last_tnode = NULL;

	// 插入过内容，需要检查是否需要分裂
	bool inserted = false;

	// 在当前容器中查找是否可以插入
	while (node_ptr) {
		// T-search
		// 找不到匹配的T-Node
		if (this->size == 0 || node_ptr->c != str[0]) {
			// 找到当前容器的最后一个T-Node仍未符合，则直接在当前容器内插入
			if (node_ptr->c == 0 || node_ptr->c > str[0]) {
				// 节点向前移动
				int move_offset = strlen(str) > 1 ? 2 : 1;
				int point_index = node_ptr - &this->nodes[0];
				int move_length = (this->size - point_index) * sizeof(Node<value_t>);
				memcpy_s(&this->nodes[point_index + move_offset], move_length, &this->nodes[point_index], move_length);
				for (int idx = this->size - 1 + move_offset; idx > point_index; --idx) {
					// 修改T-Node指针
					this->nodes[idx].ptr += (this->nodes[idx].type() && (this->nodes[idx].ptr != NULL) ? move_offset : 0) * sizeof(Node<value_t>);
				}

				// 插入T-Node
				this->nodes[point_index].header = T_NODE;
				if (strlen(str) == 1) {
					this->nodes[point_index].beleaf();
					this->nodes[point_index].value_ptr = value_p;
				}
				// 判断是在列表中插入新T-Node还是容器为空时新增T-Node
				this->nodes[point_index].ptr = this->size == 0 ? NULL : &this->nodes[point_index + move_offset];
				this->nodes[point_index].c = str[0];

				if (last_tnode) {
					last_tnode->ptr = &this->nodes[point_index];
				}

				// 插入S-Node
				if (strlen(str) > 1) {
					this->nodes[point_index + 1].header = S_NODE;
					if (strlen(str) == 2) {
						this->nodes[point_index + 1].beleaf();
						this->nodes[point_index + 1].value_ptr = value_p;
					}
					this->nodes[point_index + 1].c = str[1];
					if (strlen(str) > 2) {
						Container<value_t>* new_ctr = Container::Create_Empty_Container(this);
						this->nodes[point_index + 1].ptr = new_ctr;
						// 在新容器中更新
						result_ctr = new_ctr;
						result_str = &str[2];
					}
					else {
						this->nodes[point_index + 1].ptr = NULL;
					}
				}

				// 结束
				inserted = true;
				this->size += move_offset;
				break;
			}
			// 向前查下一个T-Node
			if (node_ptr->c < str[0]) {
				last_tnode = node_ptr;
				node_ptr = (Node<value_t>*)node_ptr->ptr;
				// 查到当前Container尽头
				if (node_ptr == NULL) {
					Container* next_ctr = this->Find_Container_with_sortkey(str[0]);
					if (next_ctr != this) {
						result_ctr = next_ctr;
						result_str = str;
						break;
					}
					node_ptr = &this->nodes[this->size];
				}
			}
		}
		// S-search
		else {
			// 用LEAF_NODE标记奇数长度单词
			if (strlen(str) == 1) {
				node_ptr->beleaf();
				node_ptr->value_ptr = value_p;
				break;
			}
			Node<value_t>* t_node_ptr = node_ptr;
			// 从下一个节点查起
			node_ptr++;

			while (node_ptr) {
				// 查到当前T节点尽头，或者查到比自己大的S-Node
				if (node_ptr->c == 0 || node_ptr->type() || node_ptr->c > str[1]) {
					// 节点向前移动
					int point_index = node_ptr - &this->nodes[0];
					int move_length = (this->size - point_index) * sizeof(Node<value_t>);
					memcpy_s(&this->nodes[point_index + 1], move_length, &this->nodes[point_index], move_length);
					for (int idx = this->size; idx > point_index; --idx) {
						this->nodes[idx].ptr += (this->nodes[idx].type() && this->nodes[idx].ptr != NULL ? 1 : 0) * sizeof(Node<value_t>);
					}
					t_node_ptr->ptr += sizeof(Node<value_t>) * (t_node_ptr->ptr == NULL ? 0 : 1);
					// 插入S-Node
					this->nodes[point_index].header = S_NODE;
					if (strlen(str) == 2) {
						this->nodes[point_index].beleaf();
						this->nodes[point_index].value_ptr = value_p;
					}
					this->nodes[point_index].c = str[1];
					// 指针指向
					if (strlen(str) > 2) {
						Container<value_t>* new_ctr = Container::Create_Empty_Container(this);
						this->nodes[point_index].ptr = new_ctr;
						// 在新容器中更新
						result_ctr = new_ctr;
						result_str = &str[2];
					}
					else {
						this->nodes[point_index].ptr = NULL;
					}

					// 结束
					inserted = true;
					this->size += 1;
					break;
				}
				// 找到相同前缀
				else if (node_ptr->c == str[1]) {
					if (strlen(str) == 2) {
						node_ptr->beleaf();
						node_ptr->value_ptr = value_p;
					}
					if (strlen(str) > 2) {
						// 如果没有子容器，则创建新容器在其中搜索
						if (node_ptr->ptr == NULL) {
							Container<value_t>* new_ctr = Container::Create_Empty_Container(this);
							node_ptr->ptr = new_ctr;
							// 在新容器中更新
							result_ctr = new_ctr;
						}
						// 在旧容器中搜索
						else {
							result_ctr = (Container<value_t>*)node_ptr->ptr;
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

	// 容器不足时分裂
	if (inserted && this->size * 8 / 7 > CONTAINER_SIZE) {
		this->Container_Split();
	}
	return pair<Container<value_t>*, const char*>(result_ctr, result_str);
}