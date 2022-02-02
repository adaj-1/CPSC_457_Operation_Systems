#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main(int argc, char* argv[])
{
	string command;
	do
	{
		cin >> command;
		system(command.c_str());
	} while (command != "q")
	return 0;
}