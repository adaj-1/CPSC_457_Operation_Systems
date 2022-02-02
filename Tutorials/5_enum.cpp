// enumeration 
#include <iostream>
// How is the behavior of empty cases?

using namespace std;

enum Color 
{
    red, 
    green = 10, 
    blue, 
    purple, 
    pink,
};

int main()
{
    Color r = red;

    switch (r)
    {
        case red  : 
            cout << "red\n";   
            break;
        case green: 
            cout << "green\n"; 
            break;
        case blue : 
            cout << "blue\n"; 
            break;
        case purple: 
            cout << "purple\n"; 
            break;
        case pink:
            cout << "pink\n"; 
            break;
    

    int a = red;
    int g = green;
    int b = blue;
    int p = purple;
    cout << a << " " << g << " " << b << " " << p << endl;
    
}
