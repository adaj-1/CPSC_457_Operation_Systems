#include<iostream>
using namespace std;

int main(){
	int a = 9;
	// Note how you do not need to use & to assign to references
	int & ra = a;
	int & rra = ra;
	// Now whenever I change the value of a, the value of ra will change and vice versa
	a = 10;
	cout << ra<< '\n';
	ra = 11;
	cout << a << '\n';
	// Also dereferencing is done automatically 
	/*
	cout << ra << '\n';
	cout << rra << '\n';
	*/
	return 0;
}
