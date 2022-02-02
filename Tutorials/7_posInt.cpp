#include <iostream>

using namespace std;

int main(int argc, char argv[])
{
	// stoi works as well
    int c = atoi(argv[1]);  // convert string to integer
    int i = 0;

	// We can put expressions anywhere
    while (i < c) 
    {
        cout << i++ << endl;
    }

    return 0;
}
