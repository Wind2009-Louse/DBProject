#pragma once
#include <iostream>
using namespace std;

// �����ָ࣬������
struct Head_pointer{
};

// Node����
struct Node: Head_pointer {
	char header;
	char c;
	Head_pointer* ptr;
};

// ����ָ��
struct Container_pointers {
	Container* ptr_1;
	Container* ptr_2;
	Container* ptr_4;
	Container* ptr_8;
};

// Container����
struct Container : Head_pointer {
	int size;
	Node nodes[255];
	Container_pointers *ptrs;
};
