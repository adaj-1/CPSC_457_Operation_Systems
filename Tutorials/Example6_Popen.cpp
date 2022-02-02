#include <iostream>
#include <cstdio>

#define MAX_LEN 100

using namespace std;

int main()
{
    FILE *fp;
    char buffer[MAX_LEN];

    fp = popen("cat Example6_Popen.cpp | head -n 5", "r");

    while (fgets(buffer, MAX_LEN, fp))
    {
        cout << buffer << endl;
    }

    pclose(fp);
    return 0;
}