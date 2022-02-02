#include<iostream>
using namespace std;

void swap(int &a, int &b);

int main(){
	int a ,b;
	cout << "enter value for a: ";
	cin >> a;
	cout << "enter value for b: ";
	cin >> b;
	swap(...);
	cout << "value of a is: " << a << "\nvalue of b is: " << b << '\n';
	return 0;
}

void swap(int &a, int &b){
	// We have access to a and b aliases in this scope
	//implement function body here
}
