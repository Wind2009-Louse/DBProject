#pragma once
#include <iostream>
#include <string>
#include <vector>
using namespace std;

// 公共类，指针重载
struct Head_pointer{
};

#define LEAF_NODE 0b10000000
#define T_NODE 0b01000000
#define S_NODE 0b00000000

// Node定义
struct Node: Head_pointer {
	char header;
	char c;
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

// 跳表指针
struct Container_pointers {
	Head_pointer* ptr_1;
	Head_pointer* ptr_2;
	Head_pointer* ptr_4;
	Head_pointer* ptr_8;
	Head_pointer* ptr_m1;
	Head_pointer* ptr_m2;
	Head_pointer* ptr_m4;
	Head_pointer* ptr_m8;
	Container_pointers():
		ptr_1(NULL), ptr_2(NULL), ptr_4(NULL), ptr_8(NULL),
		ptr_m1(NULL), ptr_m2(NULL), ptr_m4(NULL), ptr_m8(NULL) {}
};

// Container定义
struct Container : Head_pointer {
	int size;
	Node nodes[255];
	Container_pointers *ptrs;
	// 生成
	Container():size(0) {
		ptrs = new Container_pointers();
	}
	// 释放
	~Container() {
		delete ptrs;
	}
};

// 对Container进行分割
void Container_Split(Container ctr) {
	// TODO
}

// 将字符串插入到Container中
void Insert_into_Container(Container* ctr, char* str) {
	// TODO

	// 不插入空字符串
	if (strlen(str) == 0) {
		return;
	}

	// 查找是否在自身Node中可以插入

	// 节点指针
	Node* node_ptr = &ctr->nodes[0];

	// 检查下一个容器
	bool check_next_container = false;

	// 插入过内容，需要检查是否需要分裂
	bool inserted = false;

	while (node_ptr) {
		// T-search
		if (ctr->size == 0 || node_ptr->c != str[0]) {
			// 向后，插入
			if (ctr->size == 0 || node_ptr->c > str[0]) {
				Node* next_ptr = (Node*)node_ptr->ptr;
				int move_offset = strlen(str) > 1 ? 2 : 1;
				int point_index = node_ptr - &ctr->nodes[0];
				// 节点向前移动
				for (int idx = ctr->size - 1; idx >= point_index; --idx) {
					ctr->nodes[idx + move_offset] = ctr->nodes[idx];
				}
				// 插入T-Node
				ctr->nodes[point_index].header = T_NODE | (strlen(str) < 2 ? LEAF_NODE : 0);
				ctr->nodes[point_index].c = str[0];
				ctr->nodes[point_index].ptr = next_ptr;
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
				node_ptr = (Node*)node_ptr->ptr;
				// 查到当前Container尽头
				if (node_ptr == NULL) {
					check_next_container = true;
					break;
				}
			}
		}
		else {
			// 用LEAF_NODE标记奇数长度单词
			if (strlen(str) == 1) {
				node_ptr->header |= LEAF_NODE;
				break;
			}
			// S-search
			// 从下一个节点查起
			node_ptr += 1;

			while (node_ptr) {
				// 查到当前T节点尽头，或者查到比自己大的S-Node
				if (node_ptr->c == 0 || node_ptr->type() || node_ptr->c > str[1]) {
					int move_offset = 1;
					int point_index = node_ptr - &ctr->nodes[0];
					// 节点向前移动
					for (int idx = ctr->size - 1; idx >= point_index; --idx) {
						ctr->nodes[idx + move_offset] = ctr->nodes[idx];
					}
					// 插入S-Node
					ctr->nodes[point_index].header = S_NODE | (strlen(str) < 3 ? LEAF_NODE : 0);
					ctr->nodes[point_index].c = str[1];
					if (strlen(str) > 2) {
						Container* new_ctr = new Container();
						ctr->nodes[point_index].ptr = new_ctr;
						// 在新容器中更新
						Insert_into_Container(new_ctr, &str[2]);
					}
					else {
						ctr->nodes[point_index].ptr = NULL;
					}

					// 结束
					inserted = true;
					ctr->size += move_offset;
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
					node_ptr += 1;
				}
			}
			break;
		}
	}
}