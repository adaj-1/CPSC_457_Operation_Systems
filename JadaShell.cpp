// parser for shell

#include <iostream> //defines cout, cin
#include <fstream>  // performs input/output operations on file
#include <string>
#include <vector>
#include <unistd.h> //defines read(), close()
#include <fcntl.h>  //defines open(), O_RDONLY
#include <numeric>
#include <algorithm>

#define MAX_WORD_SIZE 80
#define MAX_LETTER_SIZE 1000
#define BUFFER_SIZE 1024
#define MAX_CMDS 4
#define MAX_REDIRECTIONS 3

using namespace std;

// parser function
// Citation: Amir Week 3 Tutorial AssignExample1
int parser(vector<string> *parsed_args)
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
            parsed_args->push_back(command.substr(0, space_index));
            // erase removes command[0] to command[space_index] from command
            command.erase(0, space_index + 1);
        }
        // push_back next command
        parsed_args->push_back(command);

        if (parsed_args->front() == "q")
            return 0;
        else
            return 1;
    }
}

void check_commands(vector<string> *parsed_args, vector<int> *redirection_indices)
{

    for (auto it = parsed_args->begin(); it != parsed_args->end(); it++)
    {
        if (*it == "$" || *it == ">" || *it == "<" || *it == ">>" || *it == "<<")
        {
            redirection_indices->push_back(it - parsed_args->begin());
        }
    }

    /*

    if (*it == "$" || *it == ">" || *it == "<" || *it == ">>" || *it == "<<")
    {
        *io_redirection = "pipe";
        *redirection_index = it - parsed_args->begin();
        break;
    }
    if (*it == ">")
    {
        *io_redirection = "output overwrite";
        *redirection_index = it - parsed_args->begin();
        break;
    }
    if (*it == "<")
    {
        *io_redirection = "input overwrite";
        *redirection_index = it - parsed_args->begin();
        break;
    }
    if (*it == ">>")
    {
        *io_redirection = "append output";
        *redirection_index = it - parsed_args->begin();
        break;
    }
    if (*it == "<<")
    {
        *io_redirection = "append input";
        *redirection_index = it - parsed_args->begin();
        break;
    }

    *io_redirection = "";
    *redirection_index = 0;
    */
}

// https://stackoverflow.com/questions/52490877/execvp-using-vectorstring#:~:text=You%20can't%20use%20it,internal%20buffer%20into%20another%20vector%20.&text=You%20can%20call%20execvp%20something,char*%3E%20argv(arguments_.
void system_call(vector<string> *parsed_args)
{
    cout << "in system_call" << endl;
    vector<char *> argv;

    const char *cmd = parsed_args->front().c_str();

    for (int i = 0; i < parsed_args->size(); i++)
    {
        const char *args = parsed_args->at(i).c_str();
        argv.push_back((char *)args);
    }

    argv.push_back(nullptr);

    execvp(cmd, argv.data());
}

// Citation: Alex's Tutorial Example1_OpenClose.cpp
// use this for working with I/O Redirection
void open_close(vector<string> *parsed_args)
{
    cout << "in open_close" << endl;

    // char *pathname = "/home/ugd/jada.li/.vscode-server/bin/899d46d82c4c95423fb7e10e68eba52050e30ba3/bin:/home/ugd/jada.li/.vscode-server/bin/899d46d82c4c95423fb7e10e68eba52050e30ba3/bin:/usr/share/ucalgary/bin:/usr/lib64/ccache:/usr/local/bin:/usr/bin:/usr/local/sbin:/usr/sbin:/usr/local/cuda/bin/";

    // pathname += cmd[0];
    //  search directory
    //  Const keyword in C++:
    //  This means that the pointer is pointing to a const variable
    //  Convert a string to a char* - use method c_str() to get C string version

    /*
    // open an existing file
    int fd = open(pathname,O_RDONLY);

    char buffer[BUFFER_SIZE];
    if (read(fd,buffer,BUFFER_SIZE) > 0)
    {
        // read stuff, print it screen
        string s(buffer);
        cout << s << endl;
    }
    else
        cout << "Couldn't read anything" << endl;

    close(fd); // Always close the file!!
    */
}

void multi_cmd(vector<string> *parsed_args, vector<int> *redirection_indices)
{
    cout << "In multi_cmd" << endl;
    vector<vector<string>> cmds;
    vector<string> redirections;
    int j_start = 0;
    int redirection_index = 0;

    for (int i = 0; i < redirection_indices->size() + 1; i++)
    {
        vector<string> next_input;
        vector<string> next_parsed_args;

        // checks if added final redirection already
        if (j_start != redirection_indices->back() + 1)
        {
            redirection_index = redirection_indices->at(i);
            redirections.push_back(parsed_args->at(redirection_index));
        }
        // if already added final redirection, get final command
        else
        {
            redirection_index = parsed_args->size();
        }

        for (int j = j_start; j < redirection_index; j++)
        {
            next_input.push_back(parsed_args->at(j));
        }
        cmds.push_back(next_input);

        j_start = redirection_index + 1;
    }
}

// https://www.geeksforgeeks.org/making-linux-shell-c/
int main()
{
    // init_JadaShell();
    while (1)
    {

        int flag = 0;
        vector<string> parsed_args;
        vector<int> redirection_indices;
        int num_of_redirections;

        cout << "JadaShell> ";

        flag = parser(&parsed_args);
        // flag returns 0 if quit
        // flag returns 1 if command

        // Determine the command name, and construct the parameter list
        // argv[0] - command name
        // argv[1] - first parameter
        // argv[2] - second parameter
        // ...
        // argv[N] - Nth parameter
        // argv[N+1] - NULL to indicate end of parameter list
        // Find the full path name for the file
        // Launch the executable file with the specified parameters using
        // the execvp command and the argv array.
        if (flag == 1)
        {

            check_commands(&parsed_args, &redirection_indices);
            cout << "num of redirections: " << redirection_indices.size() << endl;

            if (redirection_indices.size() == 0) // no io_redirection called
            {
                system_call(&parsed_args);
            }
            else
            {
                multi_cmd(&parsed_args, &redirection_indices);
            }
        }
        else
        {
            cout << "Exiting JadaShell" << endl;
            break;
        }
    }
    return 0;
}