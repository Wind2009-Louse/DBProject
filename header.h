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
template<typename value_t> struct Node: Head_pointer {
	// 指向数据
	value_t* value_ptr;
	// 指向的下一个目的地
	// 若为T-Node，则指向容器中的下一个T-Node；若为S-Node，则指向下一个容器。不存在时为NULL。
	Head_pointer* ptr;
	// 包含Node的标记信息
	char header;
	// Node存储的Key
	char c;

	// 生成
	Node<value_t>(bool isleaf = false, bool is_t = false) :c(0), ptr(NULL) {
		header = 0;
		value_ptr = NULL;
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

	// 去除叶子
	inline void deleaf() {
		header &= 0b01111111;
		if (this->value_ptr != NULL) {
			delete this->value_ptr;
			this->value_ptr = NULL;
		}
	}
	// 添加为叶子
	inline void beleaf() {
		header |= LEAF_NODE;
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