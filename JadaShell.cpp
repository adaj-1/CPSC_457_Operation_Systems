// parser for shell

#include <iostream> //defines cout, cin
#include <fstream>  // performs input/output operations on file
#include <string>
#include <vector>
#include <unistd.h> //defines read(), close()
#include <fcntl.h>  //defines open(), O_RDONLY

#define MAX_WORD_SIZE 80
#define MAX_LETTER_SIZE 1000
#define BUFFER_SIZE 1024

using namespace std;

void init_JadaShell()
{
    cout << "JadaShell> ";
}

/*  OPENING AND CLOSING FILE CODE
        i = 0;
        while(words[i] != NULL)
        {
            //cout << words[i] << endl;
            const char *pathname = words[i];
            int fd = open(pathname, O_RDONLY);

            char buffer[BUFFER_SIZE];
            if (read(fd,buffer,BUFFER_SIZE) > 0)
            {
                string s(buffer);
                cout << s << endl;
            }
            else
            cout << "Couldn't read anything." << endl;

            close(fd);

            i++;
        }
        */

// parser function
// Citation: Amir Week 3 Tutorial AssignExample1
int parser(vector<string> *parsedArgs)
{
    string command;

    while (true)
    {
        getline(cin, command);

        int space_index;
        // command.find(' ') returns an iterator to the first element in the range that is a ' '
        // npos is a constant value w greatest possible value for an element of type size_t
        // while loop searches for ' ' until the last element is reached
        while ((space_index = command.find(' ')) != string::npos)
        {
            // push_back adds new elem at end of vector after the current last elem
            // substr takes string values between command[0] and command[space_index]
            parsedArgs->push_back(command.substr(0, space_index));
            // erase removes command[0] to command[space_index] from command
            command.erase(0, space_index + 1);
        }
        // push_back next command
        parsedArgs->push_back(command);
        
        if (parsedArgs->front() == "q")
            return 0;
        else
            return 1;
    }
}

void execute(char **parsed_input)
{
    cout << "in execute" << endl;
}

// https://www.geeksforgeeks.org/making-linux-shell-c/
int main()
{
    string argc;
    
    int flag = 0;
    init_JadaShell();

    while (1)
    {
        vector<string> parsedArgs;
        flag = parser(&parsedArgs);
        // flag returns 0 if quit
        // flag returns 1 if command
        

        for (auto it = parsedArgs.begin(); it != parsedArgs.end(); it++)
            cout << *it << " ";

        cout << "flag returns: " << flag << endl;
    }
    return 0;
}