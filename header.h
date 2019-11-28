#pragma once
#include <iostream>
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

// 跳表指针
struct Container_pointers {
	Container* ptr_1;
	Container* ptr_2;
	Container* ptr_4;
	Container* ptr_8;
};

// Container定义
struct Container : Head_pointer {
	int size;
	Node nodes[255];
	Container_pointers *ptrs;
};
