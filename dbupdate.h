#pragma once
#include "header.h"

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
		if (ctr->size == 0 || node_ptr->c != str[0]) {
			// 向后，插入
			if (ctr->size == 0 || node_ptr->c > str[0]) {
				int move_offset = strlen(str) > 1 ? 2 : 1;
				int point_index = node_ptr - &ctr->nodes[0];
				// 节点向前移动
				for (int idx = ctr->size - 1; idx >= point_index; --idx) {
					ctr->nodes[idx + move_offset] = ctr->nodes[idx];
					ctr->nodes[idx + move_offset].ptr += (ctr->nodes[idx].type() && (ctr->nodes[idx].ptr != NULL) ? move_offset : 0) * sizeof(Node);
				}

				// 插入T-Node
				ctr->nodes[point_index].header = T_NODE | (strlen(str) < 2 ? LEAF_NODE : 0);
				// 判断是在列表中插入新T-Node还是容器为空时新增T-Node
				ctr->nodes[point_index].ptr = ctr->size == 0 ? NULL : &ctr->nodes[point_index + move_offset];
				ctr->nodes[point_index].c = str[0];

				// 插入S-Node
				if (strlen(str) > 1) {
					bool updated = ctr->nodes[point_index + 1].c == str[1];
					ctr->nodes[point_index + 1].header = S_NODE | (strlen(str) < 3 ? LEAF_NODE : 0);
					ctr->nodes[point_index + 1].c = str[1];
					if (strlen(str) > 2) {
						Container* new_ctr = new Container();
						ctr->nodes[point_index + 1].ptr = new_ctr;
						new_ctr->cptrs->head_ptr = new_ctr;
						new_ctr->cptrs->parent_ptr = ctr;
						// 在新容器中更新
						result_ctr = new_ctr;
						result_str = &str[2];
						//Insert_into_Container(new_ctr, &str[2]);
					}
					else {
						if (updated) {
							ctr->nodes[point_index + 1].ptr = NULL;
						}
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
				node_ptr->header |= LEAF_NODE;
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
					ctr->nodes[point_index].header = S_NODE | (strlen(str) < 3 ? LEAF_NODE : 0);
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
					node_ptr->header |= (strlen(str) < 3 ? LEAF_NODE : 0);
					if (strlen(str) > 2) {
						if (node_ptr->ptr == NULL) {
							Container* new_ctr = new Container();
							node_ptr->ptr = new_ctr;
							new_ctr->cptrs->head_ptr = new_ctr;
							new_ctr->cptrs->parent_ptr = ctr;
							result_ctr = new_ctr;
						}
						else {
							result_ctr = (Container*)node_ptr->ptr;
						}
						// 在新容器中更新
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
			ctr->nodes[ctr->size].header = T_NODE | (strlen(str) < 2 ? LEAF_NODE : 0);
			ctr->nodes[ctr->size].c = str[0];
			ctr->nodes[ctr->size].ptr = NULL;
			last_tnode->ptr = &ctr->nodes[ctr->size];
			// 插入S-Node
			if (strlen(str) > 1) {
				ctr->nodes[ctr->size + 1].header = S_NODE | (strlen(str) < 3 ? LEAF_NODE : 0);
				ctr->nodes[ctr->size + 1].c = str[1];
				if (strlen(str) > 2) {
					Container* new_ctr = new Container();
					ctr->nodes[ctr->size + 1].ptr = new_ctr;
					// 在新容器中更新
					Insert_into_Container(new_ctr, &str[2]);
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
			//Insert_into_Container(next_ctr, str);
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