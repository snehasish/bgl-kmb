#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <string>
#include <fstream>

using namespace std;

typedef boost::adjacency_list <boost::listS, boost::vecS > BoostGraph;

int main(int argc, char* argv[])
{
    if(argc != 4)
    {
        cout << "Usage: " << argv[0] << " <multicast_set_string> <input.dot> <output.dot>\n";
        cout << "multicast_set_string = 1,2,3\n";
        return 1; 
    }
    string InputFileName, OutputFileName;
    InputFileName.assign(argv[1]);
    OutputFileName.assign(argv[2]);

    ifstream InputFile(InputFileName.c_str(), ios::in);
    ofstream OutputFile(OutputFileName.c_str(), ios::out);

    assert(OutputFile.is_open() && InputFile.is_open() && "Error opening files");

    return 0;
}
