#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
    const string codeword = "cpsc457";
    
    // Mini exercise: add error handling to ensure exactly 1 input. 

    string input = argv[1];
    bool result;

    if(input == codeword)
    {
        result = true;
    }
    else 
    {
        result = false;
    }

    cout << result << endl;

    return 0;
}
