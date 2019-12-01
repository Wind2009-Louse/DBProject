#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
using namespace std;

// 容器可以容纳的节点数量
#define CONTAINER_SIZE 127
// 得到一个比n小的随机整数
#define randnum(n) (rand()%n)

// 公共类，指针重载
struct Head_pointer{
};

#define LEAF_NODE 0b10000000
#define T_NODE 0b01000000
#define S_NODE 0b00000000

// Node定义
struct Node: Head_pointer {
	// 包含Node的标记信息
	char header;
	// Node存储的数据
	char c;
	// 指向的下一个目的地
	// 若为T-Node，则指向容器中的下一个T-Node；若为S-Node，则指向下一个容器。不存在时为NULL。
	Head_pointer* ptr;
	// 生成
	Node(bool isleaf = false, bool is_t = false) :c(0), ptr(NULL) {
		header = 0;
		if (isleaf) {
			header &= 0b10000000;
		}
		if (is_t) {
			header &= 0b01000000;
		}
	}
	// 为叶子节点时返回true，否则返回false
	inline bool is_leaf() {
		return header & 0b10000000;
	}
	// 为T-Node时返回true，为S-Node时返回false
	inline bool type() {
		return header & 0b01000000;
	}
};

// 跳表的大小
#define JUMPPOINT_MAXHEIGHT 5
// 跳表指针
struct Container_pointers {
	Head_pointer* head_ptr;
	Head_pointer* ptrs[JUMPPOINT_MAXHEIGHT];
	Container_pointers(Head_pointer* hptr=NULL):head_ptr(hptr){
		for (int i = 0; i < JUMPPOINT_MAXHEIGHT; ++i) {
			ptrs[i] = NULL;
		}
	}
};

// Container定义
struct Container : Head_pointer {
	// 当前容纳的节点数量
	int size;
	// 跳表指针
	Container_pointers* cptrs;
	// 容器中的节点
	Node nodes[CONTAINER_SIZE];
	// 生成
	Container(Container* head_ptr=NULL):size(0) {
		cptrs = new Container_pointers(head_ptr);
	}
	// 释放
	~Container() {
		delete cptrs;
	}
};

// 根据sortkey，在Container列表中创建一个新的Container
Container* Create_Container(Container* ctr, char sortkey) {
	Container* head_ctr = (Container*)ctr->cptrs->head_ptr;
	// 新Container的最高指针等级
	int level = 0;
	for (int i = 0; i < JUMPPOINT_MAXHEIGHT-1 && randnum(2)>0; ++i) {
		level++;
	}

	// 最靠近目标的Container
	vector<Container*> nearliest_ctrs(JUMPPOINT_MAXHEIGHT, head_ctr);
	int current_level = level;
	while (current_level>=0) {
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

// 根据T-Node查找容器或者最接近的容器
Container* Find_Container_with_sortkey(Container* ctr, char sorykey) {
	Container* head_ctr = (Container*)ctr->cptrs->head_ptr;
	int level = JUMPPOINT_MAXHEIGHT-1;
	Container* current_ctr = head_ctr;
	while (level>=0) {
		Container* next_ctr = (Container*)current_ctr->cptrs->ptrs[level];
		if (next_ctr==NULL || next_ctr->nodes[0].c > sorykey) {
			level--;
			continue;
		}
		current_ctr = next_ctr;
	}
	return current_ctr;
}

// 打印容器内容进行测试
void Print_Container(Container* ctr) {
	Container* head_ctr = (Container*)ctr->cptrs->head_ptr;
	Container* ctr_ptr = head_ctr;
	while (ctr_ptr) {
		if (ctr == ctr_ptr) {
			cout << "> ";
		}
		cout << ctr_ptr->size << ctr_ptr->nodes[0].c << "\t";
		for (int i = 0; i < JUMPPOINT_MAXHEIGHT; ++i) {
			if (ctr_ptr->cptrs->ptrs[i] == NULL) {
				cout << "NULL\t";
			}
			else {
				Container* _ctr = (Container*)ctr_ptr->cptrs->ptrs[i];
				cout << _ctr->size << _ctr->nodes[0].c << '\t';
			}
		}
		cout << endl;
		ctr_ptr = (Container*)ctr_ptr->cptrs->ptrs[0];
	}
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
		// T-Node需要更新指针
		if (new_ctr->nodes[count].type() && slow_ptr->ptr) {
			new_ctr->nodes[count].ptr = &new_ctr->nodes[count] + ((Node*)slow_ptr->ptr - (Node*)slow_ptr);
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
void Insert_into_Container(Container* ctr, const char* str) {
	// 不插入空字符串
	if (strlen(str) == 0) {
		return;
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
					ctr->nodes[point_index+1].header = S_NODE | (strlen(str) < 3 ? LEAF_NODE : 0);
					ctr->nodes[point_index+1].c = str[1];
					if (strlen(str) > 2) {
						Container* new_ctr = new Container();
						ctr->nodes[point_index + 1].ptr = new_ctr;
						// 在新容器中更新
						Insert_into_Container(new_ctr, &str[2]);
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
						// 在新容器中更新
						Insert_into_Container(new_ctr, &str[2]);
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
						Container* new_ctr = new Container();
						node_ptr->ptr = new_ctr;
						// 在新容器中更新
						Insert_into_Container(new_ctr, &str[2]);
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
			Insert_into_Container(next_ctr, str);
		}
	}

	// 容器不足时分裂
	if (inserted && ctr->size * 3 / 2 > CONTAINER_SIZE) {
		Container_Split(ctr);
	}
}