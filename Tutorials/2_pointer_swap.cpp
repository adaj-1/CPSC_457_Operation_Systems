#include<iostream>
using namespace std;

void swap_value(int a, int b);
void swap_pointer(int * a, int * b);

int main(){
	int a, b;
	cout << "enter value of a: ";
	cin >> a;
	cout << "enter value of b: ";
	cin >> b;

	swap_value(a, b);
	cout << "value of a is: " << a << "\nvalue of b is: " << b << '\n';
	return 0;
}

void swap_value(int a, int b){
	// has access only to copies of a, b
	// implement function body here
}

void swap_pointer(int *a, int *b){
	// has access to a, b addresses
	// implement function body here
}
