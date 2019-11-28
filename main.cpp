#include "header.h"

int main() {
	char c = 'c';
	int i = 1;
	long long int lli = 1;
	double d = 1.0;
	Container ct;

	char* c_ptr = &c;
	int* i_ptr = &i;
	long long int* lli_ptr = &lli;
	double* d_ptr = &d;
	Container* ct_ptr = &ct;

	cout << sizeof(Node) << endl;

	cout << sizeof(c_ptr) << "," << (int)c_ptr << endl;
	cout << sizeof(i_ptr) << endl;
	cout << sizeof(lli_ptr) << endl;
	cout << sizeof(d_ptr) << endl;
	cout << sizeof(ct_ptr) << endl;
	system("pause");
}