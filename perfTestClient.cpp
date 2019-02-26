#include <string>
#include <iostream>
#include <thread>
#include <fstream>
#include "utils.h"
using namespace std;

int main() {
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

    thread t2(invokeServer,buffer,length,"hello server from thread");
    t2.join();
}