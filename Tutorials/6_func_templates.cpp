#include<iostream>
using namespace std;

template<class T>
// T is called a template parameter
void generic_swap(T& t1, T& t2){
	// implement function body here
}

template<class T, class U>
T generic_sum(T& t1, U& t2){
	// implement function body here
}

int main(){
	int a ,b;
	cout << "enter integer value for a: ";
	cin >> a;
	cout << "enter integer value for b: ";
	cin >> b;
	generic_swap(a, b);
	cout << "new value for a is: " << a << "\nnew value for b is: " << b;
	
	// we can use the same function for different data types
	string str1, str2;
	cout << "\nenter string value for str1: ";
	cin >> str1;
	cout << "enter string value for str2: ";
	cin >> str2;
	generic_swap(str1, str2);
	cout << "new value for str1 is: " << str1 << "\nnew value for str2 is: " << str2;
	cout << endl;

	float c;
	int d;
	cout << "enter value for c: ";
	cin >> c;
	cout << "enter value for d: ";
	cin >> d;
	// In case of multiple template types you have to specify the types
	c = generic_sum<float, int>(c, d);
	cout << "the sum is: " << c << '\n';

	return 0;
}

