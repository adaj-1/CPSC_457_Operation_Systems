#include<iostream>

using namespace std;

int main(){
	int a;
	a = 12;
	int *pa;
	pa = &a;
	cout << "pa refers to: " << pa << '\n';
	int **ppa = &pa;
	cout << "ppa refers to: " << ppa << '\n';
	/*
	 * dereferencing
	cout << "value pointed to by pa or *pa: " << * pa << '\n';
	cout << "value pointed to by ppa or *ppa: " << * ppa << '\n';
	*/
	return 0;
}
	
	
