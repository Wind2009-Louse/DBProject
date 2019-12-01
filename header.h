#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
using namespace std;

// 容器可以容纳的节点数量
#define CONTAINER_SIZE 255
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
	Head_pointer* parent_ptr;
	Head_pointer* head_ptr;
	Head_pointer* ptrs[JUMPPOINT_MAXHEIGHT];
	Container_pointers(Head_pointer* hptr=NULL):head_ptr(hptr), parent_ptr(NULL){
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

// 根据T-Node查找容器或者最接近的容器
Container* Find_Container_with_sortkey(Container* ctr, char sorykey) {
	Container* head_ctr = (Container*)ctr->cptrs->head_ptr;
	int level = JUMPPOINT_MAXHEIGHT - 1;
	Container* current_ctr = head_ctr;
	while (level >= 0) {
		Container* next_ctr = (Container*)current_ctr->cptrs->ptrs[level];
		if (next_ctr == NULL || next_ctr->nodes[0].c > sorykey) {
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