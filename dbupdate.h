#pragma once
#include "header.h"

// ����sortkey����Container�б��д���һ���µ�Container
Container* Create_Container(Container* ctr, char sortkey) {
	Container* head_ctr = (Container*)ctr->cptrs->head_ptr;
	// ��Container�����ָ��ȼ�
	int level = 0;
	for (int i = 0; i < JUMPPOINT_MAXHEIGHT - 1 && randnum(2)>0; ++i) {
		level++;
	}

	// ���Ŀ���Container
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

	// ����
	Container* new_ctr = new Container(head_ctr);
	for (int i = 0; i <= level; ++i) {
		new_ctr->cptrs->ptrs[i] = nearliest_ctrs[i]->cptrs->ptrs[i];
		nearliest_ctrs[i]->cptrs->ptrs[i] = new_ctr;
	}
	return new_ctr;
}

// ��Container���зָ�
void Container_Split(Container* ctr) {
	// ���м��T-Node
	Node* last_ptr = &ctr->nodes[0];
	Node* fast_ptr = &ctr->nodes[0];
	Node* slow_ptr = &ctr->nodes[0];
	while (fast_ptr) {
		slow_ptr = (Node*)slow_ptr->ptr;
		fast_ptr = (Node*)fast_ptr->ptr;
		if (fast_ptr) fast_ptr = (Node*)fast_ptr->ptr;
		if (fast_ptr) last_ptr = slow_ptr;
	}

	// ����������
	char sortkey = slow_ptr->c;
	Container* new_ctr = Create_Container(ctr, sortkey);

	// ����
	last_ptr->ptr = NULL;
	int count = 0;
	while (slow_ptr->c != 0) {
		// ��������
		new_ctr->nodes[count].header = slow_ptr->header;
		new_ctr->nodes[count].c = slow_ptr->c;
		new_ctr->nodes[count].ptr = slow_ptr->ptr;
		// T-Node��Ҫ����ָ��
		if (new_ctr->nodes[count].type() && slow_ptr->ptr) {
			new_ctr->nodes[count].ptr = &new_ctr->nodes[count] + ((Node*)slow_ptr->ptr - (Node*)slow_ptr);
		}
		count++;
		fast_ptr = slow_ptr;
		slow_ptr++;
		// ���
		fast_ptr->header = 0;
		fast_ptr->c = 0;
		fast_ptr->ptr = NULL;
	}
	new_ctr->size = count;
	ctr->size -= count;
}

// ���ַ������뵽Container��
void Insert_into_Container(Container* ctr, const char* str) {
	// ��������ַ���
	if (strlen(str) == 0) {
		return;
	}

	// �����Ƿ�������Node�п��Բ���

	// �ڵ�ָ��
	Node* node_ptr = &ctr->nodes[0];

	// �����һ������
	bool check_next_container = false;
	Node* last_tnode = NULL;

	// ��������ݣ���Ҫ����Ƿ���Ҫ����
	bool inserted = false;

	// �ڵ�ǰ�����в����Ƿ���Բ���
	while (node_ptr) {
		// T-search
		if (ctr->size == 0 || node_ptr->c != str[0]) {
			// ��󣬲���
			if (ctr->size == 0 || node_ptr->c > str[0]) {
				int move_offset = strlen(str) > 1 ? 2 : 1;
				int point_index = node_ptr - &ctr->nodes[0];
				// �ڵ���ǰ�ƶ�
				for (int idx = ctr->size - 1; idx >= point_index; --idx) {
					ctr->nodes[idx + move_offset] = ctr->nodes[idx];
					ctr->nodes[idx + move_offset].ptr += (ctr->nodes[idx].type() && (ctr->nodes[idx].ptr != NULL) ? move_offset : 0) * sizeof(Node);
				}

				// ����T-Node
				ctr->nodes[point_index].header = T_NODE | (strlen(str) < 2 ? LEAF_NODE : 0);
				// �ж������б��в�����T-Node��������Ϊ��ʱ����T-Node
				ctr->nodes[point_index].ptr = ctr->size == 0 ? NULL : &ctr->nodes[point_index + move_offset];
				ctr->nodes[point_index].c = str[0];

				// ����S-Node
				if (strlen(str) > 1) {
					ctr->nodes[point_index + 1].header = S_NODE | (strlen(str) < 3 ? LEAF_NODE : 0);
					ctr->nodes[point_index + 1].c = str[1];
					if (strlen(str) > 2) {
						Container* new_ctr = new Container();
						ctr->nodes[point_index + 1].ptr = new_ctr;
						// ���������и���
						Insert_into_Container(new_ctr, &str[2]);
					}
					else {
						ctr->nodes[point_index + 1].ptr = NULL;
					}
				}

				// ����
				inserted = true;
				ctr->size += move_offset;
				break;
			}
			// ��ǰ����һ��T-Node
			if (node_ptr->c < str[0]) {
				last_tnode = node_ptr;
				node_ptr = (Node*)node_ptr->ptr;
				// �鵽��ǰContainer��ͷ
				if (node_ptr == NULL) {
					check_next_container = true;
					break;
				}
			}
		}
		// S-search
		else {
			// ��LEAF_NODE����������ȵ���
			if (strlen(str) == 1) {
				node_ptr->header |= LEAF_NODE;
				break;
			}
			Node* t_node_ptr = node_ptr;
			// ����һ���ڵ����
			node_ptr++;

			while (node_ptr) {
				// �鵽��ǰT�ڵ㾡ͷ�����߲鵽���Լ����S-Node
				if (node_ptr->c == 0 || node_ptr->type() || node_ptr->c > str[1]) {
					int point_index = node_ptr - &ctr->nodes[0];
					// �ڵ���ǰ�ƶ�
					for (int idx = ctr->size - 1; idx >= point_index; --idx) {
						ctr->nodes[idx + 1] = ctr->nodes[idx];
						ctr->nodes[idx + 1].ptr += (ctr->nodes[idx].type() && ctr->nodes[idx].ptr != NULL ? 1 : 0) * sizeof(Node);
					}
					t_node_ptr->ptr += sizeof(Node) * (t_node_ptr->ptr == NULL ? 0 : 1);
					// ����S-Node
					ctr->nodes[point_index].header = S_NODE | (strlen(str) < 3 ? LEAF_NODE : 0);
					ctr->nodes[point_index].c = str[1];
					// ָ��ָ��
					if (strlen(str) > 2) {
						Container* new_ctr = new Container();
						ctr->nodes[point_index].ptr = new_ctr;
						new_ctr->cptrs->head_ptr = new_ctr;
						// ���������и���
						Insert_into_Container(new_ctr, &str[2]);
					}
					else {
						ctr->nodes[point_index].ptr = NULL;
					}

					// ����
					inserted = true;
					ctr->size += 1;
					break;
				}
				// �ҵ���ͬǰ׺
				else if (node_ptr->c == str[1]) {
					node_ptr->header |= (strlen(str) < 3 ? LEAF_NODE : 0);
					if (strlen(str) > 2) {
						Container* new_ctr = new Container();
						node_ptr->ptr = new_ctr;
						// ���������и���
						Insert_into_Container(new_ctr, &str[2]);
					}
					break;
				}
				// ��ǰ����
				else if (node_ptr->c < str[1]) {
					node_ptr++;
				}
			}
			break;
		}
	}

	if (check_next_container) {
		Container* next_ctr = Find_Container_with_sortkey(ctr, str[0]);
		// ��ǰΪ���һ���ʺϵ���������ֱ�Ӳ����ڵ�ǰ������
		if (next_ctr == ctr) {
			// ����T-Node
			ctr->nodes[ctr->size].header = T_NODE | (strlen(str) < 2 ? LEAF_NODE : 0);
			ctr->nodes[ctr->size].c = str[0];
			ctr->nodes[ctr->size].ptr = NULL;
			last_tnode->ptr = &ctr->nodes[ctr->size];
			// ����S-Node
			if (strlen(str) > 1) {
				ctr->nodes[ctr->size + 1].header = S_NODE | (strlen(str) < 3 ? LEAF_NODE : 0);
				ctr->nodes[ctr->size + 1].c = str[1];
				if (strlen(str) > 2) {
					Container* new_ctr = new Container();
					ctr->nodes[ctr->size + 1].ptr = new_ctr;
					// ���������и���
					Insert_into_Container(new_ctr, &str[2]);
				}
				else {
					ctr->nodes[ctr->size + 1].ptr = NULL;
				}
			}

			inserted = true;
			ctr->size += strlen(str) > 1 ? 2 : 1;
		}
		// ������һ������
		else {
			Insert_into_Container(next_ctr, str);
		}
	}

	// ��������ʱ����
	if (inserted && ctr->size * 3 / 2 > CONTAINER_SIZE) {
		Container_Split(ctr);
	}
}