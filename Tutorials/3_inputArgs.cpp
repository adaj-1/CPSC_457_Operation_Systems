#include <iostream>

// How do we ensure exactly k inputs?
using namespace std;
int main(int argc, char* argv[])
{
    cout << "Number of arguments: " << endl;
    cout << "argv contains: " << endl;

    for (int i = 0; i < argc; i++)
    {
        cout << "argv[" << i << "] = " << 0 << endl;
    }

    return 0;
}
