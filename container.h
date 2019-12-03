#pragma once
#include "header.h"

// Container定义
template <typename value_t> struct Container : Head_pointer {
	// 当前容纳的节点数量
	int size;
	// 跳表指针
	Container_pointers cptrs;
	// 容器中的节点
	Node<value_t> nodes[CONTAINER_SIZE];
	// 生成
	Container(Container* head_ptr = NULL) :size(0) {
		cptrs = Container_pointers(head_ptr);
	}

	// 创建一个空的Container，其头指针指向自己（当前列的首个容器）
	static Container* Create_Empty_Container(Container* parent_ctr = NULL) {
		Container* ctr = new Container();
		ctr->cptrs.head_ptr = ctr;
		ctr->cptrs.parent_ptr = parent_ctr;
		return ctr;
	};

	// 打印容器内容进行测试
	void Print_Container();

	// 根据T-Node查找容器或者最接近的容器
	Container<value_t>* Find_Container_with_sortkey(char sorykey);

	// 在container中查找在lower_str和upper_str范围的字符串
	vector<pair<string, value_t*> > Rangesearch_in_container(
		const char* lower_str, const char* upper_str,
		bool is_lower_equal = false, bool is_upper_equal = false, bool is_no_upper = false
	);

	// 对Container进行分割
	void Container_Split();

	// 根据sortkey，在Container列表中创建一个新的Container
	Container<value_t>* Create_Container(char sortkey);

	// 将字符串插入到Container中
	// 返回下一个需要查找的Container和str，通过迭代避免栈溢出
	pair<Container<value_t>*, const char*> Insert_into_Container(const char* str, value_t* value_p);
};