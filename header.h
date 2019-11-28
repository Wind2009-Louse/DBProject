#pragma once
#include <iostream>
#include <string>
#include <vector>
using namespace std;

// 公共类，指针重载
struct Head_pointer{
};

// Node定义
struct Node: Head_pointer {
	char header;
	char c;
	Head_pointer* ptr;
};

// 为叶子节点时返回true，否则返回false
inline bool node_isleaf(Node n) {
	return n.header & 0b10000000;
}

// 为T-Node时返回true，为S-Node时返回false
inline bool node_type(Node n) {
	return n.header & 0b01000000;
}

// 跳表指针
struct Container_pointers {
	Head_pointer* ptr_1;
	Head_pointer* ptr_2;
	Head_pointer* ptr_4;
	Head_pointer* ptr_8;
};

// Container定义
struct Container : Head_pointer {
	int size;
	Node nodes[255];
	Container_pointers *ptrs;
};