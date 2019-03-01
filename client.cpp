
#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include "utils.h"
using namespace std;
namespace po=boost::program_options;
int main(int argc, char** argv)
{
    po::options_description desc("Allowed options");
    string imageFile = "";
    string text = "";
    int channel_id = 0;
    desc.add_options()
            ("help", "produce help message")
            ("image", po::value< string >(&imageFile)->required(),"image file")
            ("text", po::value< string >(&text)->required(),"text for printing on image")
            ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    if(vm.count("help"))
    {
        std::cout << desc << "\n";
        return false;
    }

    if (!vm.count("image"))
    {
        std::cout << desc << "\n";
        return false;
    }
    if (!vm.count("text"))
    {
        std::cout << desc << "\n";
        return false;
    }

    ifstream ifs(imageFile, ios::binary );
// get length of file:
    ifs.seekg (0, ios::end);
    auto length = ifs.tellg();
    ifs.seekg (0, ios::beg);
// allocate memory:
    char *buffer = new char [length];
// read data as a block:
    ifs.read (buffer,length);
    ifs.close();
    invokeServer(buffer,length,text);

}