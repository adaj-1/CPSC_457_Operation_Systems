#include <iostream>
// The typing system
//#include <climits>
//data types with exact width
//int can be 2 types long
//can you have int12_t?
//usr/include/bits/types.h
using namespace std;

int main()
{
    
    cout << "Minimum short = " << SHRT_MIN << endl;
    cout << "Maximum short = "<< SHRT_MAX << endl;
    cout << "Minimum int = " << SHRT_MIN;
    cout << "Maximum int = "<< SHRT_MAX << endl;
    cout << "Minimum long = " << SHRT_MIN << endl;
    cout << "Maximum long = " << SHRT_MAX << endl;
    
	
    cout << "Size of int = " << sizeof(int) << endl;
	// int arr[4];
    cout << "Size of int64_t = " << sizeof(int64_t) << endl;
    cout << "Size of long = " << sizeof(long) << endl;
    return 0;
}
