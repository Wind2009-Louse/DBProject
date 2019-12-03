#include "container.h"

// 打印容器内容进行测试
void Container::Print_Container() {
	Container* head_ctr = (Container*)this->cptrs.head_ptr;
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
				Container* _ctr = (Container*)ctr_ptr->cptrs.ptrs[i];
				cout << _ctr->size << _ctr->nodes[0].c << '\t';
			}
		}
		cout << endl;
		ctr_ptr = (Container*)ctr_ptr->cptrs.ptrs[0];
	}
}

// 在container中查找在lower_str和upper_str范围的字符串
vector<string> Container::Rangesearch_in_container(
	const char* lower_str, const char* upper_str,
	bool is_lower_equal, bool is_upper_equal, bool is_no_upper
) {
	// 结果向量
	vector<string> results;

	// 寻找范围查询起点
	Container* ctr_ptr;
	if (strlen(lower_str) == 0) {
		ctr_ptr = (Container*)this->cptrs.head_ptr;
	}
	else {
		ctr_ptr = this->Find_Container_with_sortkey(lower_str[0]);
	}

	// 开始查找
	while (ctr_ptr) {
		// T-search
		Node* t_node = &ctr_ptr->nodes[0];
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
					results.push_back(string(1, t_node->c));
				}
				// 是否超出上界，若超过则不继续向下搜索
				if (is_no_upper || strlen(upper_str) > 1 || t_node->c < upper_str[0]) {
					// S-search
					Node* s_node = t_node + 1;
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
								results.push_back(prefix);
							}
							bool sub_no_upper = no_upper || s_node->c < upper_str[1];
							// 有子容器，往下搜索
							if (s_node->ptr) {
								const char* sub_lower = (strlen(lower_str) < 2 || strcmp(lower_str, prefix.c_str()) < 0) ? "" : &lower_str[2];
								vector<string> subset = ((Container*)s_node->ptr)->Rangesearch_in_container(sub_lower, &upper_str[2], is_lower_equal, is_upper_equal, sub_no_upper);
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
		ctr_ptr = (Container*)ctr_ptr->cptrs.ptrs[0];
	}

	// TODO
	return results;
}

// 根据T-Node查找容器或者最接近的容器
Container* Container::Find_Container_with_sortkey(char sorykey) {
	Container* head_ctr = (Container*)this->cptrs.head_ptr;
	int level = JUMPPOINT_MAXHEIGHT - 1;
	Container* current_ctr = head_ctr;
	while (level >= 0) {
		Container* next_ctr = (Container*)current_ctr->cptrs.ptrs[level];
		if (next_ctr == NULL || next_ctr->nodes[0].c > sorykey) {
			level--;
			continue;
		}
		current_ctr = next_ctr;
	}
	return current_ctr;
}

// 对Container进行分割
void Container::Container_Split() {
	// 找中间的T-Node
	Node* last_ptr = &this->nodes[0];
	Node* fast_ptr = &this->nodes[0];
	Node* slow_ptr = &this->nodes[0];
	while (fast_ptr) {
		slow_ptr = (Node*)slow_ptr->ptr;
		fast_ptr = (Node*)fast_ptr->ptr;
		if (fast_ptr) fast_ptr = (Node*)fast_ptr->ptr;
		if (fast_ptr) last_ptr = slow_ptr;
	}

	// 创建新容器
	char sortkey = slow_ptr->c;
	Container* new_ctr = this->Create_Container(sortkey);

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
				child_ctr->cptrs.parent_ptr = new_ctr;
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
	this->size -= count;
}

// 根据sortkey，在Container列表中创建一个新的Container
Container* Container::Create_Container(char sortkey) {
	Container* head_ctr = (Container*)this->cptrs.head_ptr;
	// 新Container的最高指针等级
	int level = 0;
	for (int i = 0; i < JUMPPOINT_MAXHEIGHT - 1 && randnum(2)>0; ++i) {
		level++;
	}

	// 最靠近目标的Container
	vector<Container*> nearliest_ctrs(JUMPPOINT_MAXHEIGHT, head_ctr);
	int current_level = level;
	while (current_level >= 0) {
		Container* next_ptr = (Container*)nearliest_ctrs[current_level]->cptrs.ptrs[current_level];
		while (next_ptr && next_ptr->nodes[0].c <= sortkey) {
			nearliest_ctrs[current_level] = (Container*)nearliest_ctrs[current_level]->cptrs.ptrs[current_level];
			next_ptr = (Container*)nearliest_ctrs[current_level]->cptrs.ptrs[current_level];
		}
		current_level--;
	}

	// 创建
	Container* new_ctr = new Container(head_ctr);
	for (int i = 0; i <= level; ++i) {
		new_ctr->cptrs.ptrs[i] = nearliest_ctrs[i]->cptrs.ptrs[i];
		nearliest_ctrs[i]->cptrs.ptrs[i] = new_ctr;
	}
	return new_ctr;
}

// 将字符串插入到Container中
// 返回下一个需要查找的Container和str，通过迭代避免栈溢出
pair<Container*, const char*> Container::Insert_into_Container(const char* str) {
	// 结果初始化
	Container* result_ctr = NULL;
	const char* result_str = str;

	// 不插入空字符串
	if (strlen(str) == 0) {
		return pair<Container*, const char*>(result_ctr, result_str);
	}

	// 查找是否在自身Node中可以插入

	// 节点指针
	Node* node_ptr = &this->nodes[0];

	// 检查下一个容器
	bool check_next_container = false;
	Node* last_tnode = NULL;

	// 插入过内容，需要检查是否需要分裂
	bool inserted = false;

	// 在当前容器中查找是否可以插入
	while (node_ptr) {
		// T-search
		// 找不到匹配的T-Node
		if (this->size == 0 || node_ptr->c != str[0]) {
			// 找到当前容器的最后一个T-Node仍未符合，则直接在当前容器内插入
			if (this->size == 0 || node_ptr->c > str[0]) {
				int move_offset = strlen(str) > 1 ? 2 : 1;
				int point_index = node_ptr - &this->nodes[0];
				// 节点向前移动
				for (int idx = this->size - 1; idx >= point_index; --idx) {
					this->nodes[idx + move_offset] = this->nodes[idx];
					this->nodes[idx + move_offset].ptr += (this->nodes[idx].type() && (this->nodes[idx].ptr != NULL) ? move_offset : 0) * sizeof(Node);
				}

				// 插入T-Node
				this->nodes[point_index].header = T_NODE;
				if (strlen(str) == 1) {
					this->nodes[point_index].beleaf();
				}
				// 判断是在列表中插入新T-Node还是容器为空时新增T-Node
				this->nodes[point_index].ptr = this->size == 0 ? NULL : &this->nodes[point_index + move_offset];
				this->nodes[point_index].c = str[0];

				// 插入S-Node
				if (strlen(str) > 1) {
					this->nodes[point_index + 1].header = S_NODE;
					if (strlen(str) == 2) {
						this->nodes[point_index + 1].beleaf();
					}
					this->nodes[point_index + 1].c = str[1];
					if (strlen(str) > 2) {
						Container* new_ctr = Container::Create_Empty_Container(this);
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
					int point_index = node_ptr - &this->nodes[0];
					// 节点向前移动
					for (int idx = this->size - 1; idx >= point_index; --idx) {
						this->nodes[idx + 1] = this->nodes[idx];
						this->nodes[idx + 1].ptr += (this->nodes[idx].type() && this->nodes[idx].ptr != NULL ? 1 : 0) * sizeof(Node);
					}
					t_node_ptr->ptr += sizeof(Node) * (t_node_ptr->ptr == NULL ? 0 : 1);
					// 插入S-Node
					this->nodes[point_index].header = S_NODE;
					if (strlen(str) == 2) {
						this->nodes[point_index].beleaf();
					}
					this->nodes[point_index].c = str[1];
					// 指针指向
					if (strlen(str) > 2) {
						Container* new_ctr = Container::Create_Empty_Container(this);
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
					if (strlen(str) < 3) {
						node_ptr->beleaf();
					}
					if (strlen(str) > 2) {
						// 如果没有子容器，则创建新容器在其中搜索
						if (node_ptr->ptr == NULL) {
							Container* new_ctr = Container::Create_Empty_Container(this);
							node_ptr->ptr = new_ctr;
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
		Container* next_ctr = this->Find_Container_with_sortkey(str[0]);
		// 当前为最后一个适合的容器，则直接插入在当前容器中
		if (next_ctr == this) {
			// 插入T-Node
			this->nodes[this->size].header = T_NODE;
			if (strlen(str) == 1) {
				this->nodes[this->size].beleaf();
			}
			this->nodes[this->size].c = str[0];
			this->nodes[this->size].ptr = NULL;
			last_tnode->ptr = &this->nodes[this->size];
			// 插入S-Node
			if (strlen(str) > 1) {
				this->nodes[this->size + 1].header = S_NODE;
				if (strlen(str) == 2) {
					this->nodes[this->size + 1].beleaf();
				}
				this->nodes[this->size + 1].c = str[1];
				// 字符有剩余部分，在S-Node指向的新容器中更新
				if (strlen(str) > 2) {
					Container* new_ctr = Container::Create_Empty_Container(this);
					this->nodes[this->size + 1].ptr = new_ctr;
					// 在新容器中更新
					result_ctr = new_ctr;
					result_str = &str[2];
				}
				else {
					this->nodes[this->size + 1].ptr = NULL;
				}
			}

			inserted = true;
			this->size += strlen(str) > 1 ? 2 : 1;
		}
		// 查找下一个容器
		else {
			result_ctr = next_ctr;
			result_str = str;
		}
	}

	// 容器不足时分裂
	if (inserted && this->size * 4 / 3 > CONTAINER_SIZE) {
		this->Container_Split();
	}
	return pair<Container*, const char*>(result_ctr, result_str);
}