#include "pointer_search.h"
#include "range_search.h"

int main() {
	Container* ctr = new Container();
	Insert_into_Container(ctr, "aa");
	Insert_into_Container(ctr, "a");
	Insert_into_Container(ctr, "ac");
	Insert_into_Container(ctr, "ab");
	Insert_into_Container(ctr, "ada");
	Insert_into_Container(ctr, "aaa");
	Insert_into_Container(ctr, "aaaaa");

	cout << sizeof(Container) << endl;
	cout << sizeof(Node) << endl;

	system("pause");
}