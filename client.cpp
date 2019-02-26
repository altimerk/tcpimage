
#include <iostream>
#include <fstream>
#include "utils.h"
using namespace std;

int main(int argc, char* argv[])
{
    ifstream ifs("1.jpg", ios::binary );
// get length of file:
    ifs.seekg (0, ios::end);
    auto length = ifs.tellg();
    ifs.seekg (0, ios::beg);
// allocate memory:
    char *buffer = new char [length];
// read data as a block:
    ifs.read (buffer,length);
    ifs.close();
    invokeServer(buffer,length,"hello, server!");

}