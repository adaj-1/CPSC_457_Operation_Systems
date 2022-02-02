#include<iostream>
using namespace std;

class BigObj{
	public:
		int bigArr[100000];
		string name;
};

bool check_arr(BigObj big, int index);
bool check_arr_ref(BigObj& big, int index);
bool check_arr_point(BigObj* big, int index);

int main(){
	// 1- common strength
	// compare the run time of the two functions
	BigObj big;
	for(int i =0; i < 1000000 ; i ++)
		check_arr_ref(big, 10);
	//TODO To understand better, compare the size of big with &big
	// 2- dynamic memory allocation: What if we want to set the size of bigArr at runtime?
	
}

bool check_arr(BigObj big, int index){
	// check if array sub index is equal to 0 and return result
}

bool check_arr_ref(BigObj &big,int index){
	// check if array sub index is equal to 0 and return result
}

bool check_arr_pointer(BigObj &big, int index){
	// Exercise: implement the same functionality using pointers and compare to references
	return false;
}
