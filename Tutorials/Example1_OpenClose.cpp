#include <iostream> //defines cout, cin
#include <unistd.h> //defines read(), close()
#include <fcntl.h>  //defines open(), O_RONDLY

using namespace std;

int main()
{
    string input;
    cin >> input;

    const char *pathname = input.c_str();

    int fd = open(pathname, O_RDONLY); //fd= file descriptor, O_RDONLY = open read only

    char buffer[BUFFER_SIZE];
    if (read(fd, buffer, BUFFER_SIZE > 0))
    {
        string s(buffer);
        cout << s << endl;
    }
    else
    {
        cout << "Couldn't read anything" << endl;
    }
    close(fd); //Always close this

    return 0;
}

// g++ -o out Example1_OpenClose.cpp            out is the executible
// ./out
// ^C
// rm file.txt
// cat > file.txt
// Hello how are you!?
// I am good.
//^C
// cat file.txt
// Hello how are you!?
// I am good.
