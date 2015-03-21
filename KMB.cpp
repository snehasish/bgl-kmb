#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <string>
#include <fstream>

using namespace std;

typedef boost::adjacency_list <boost::listS, boost::vecS > BoostGraph;
string InputFileName, OutputFileName;
float multicastFraction = 0.0;

int main(int argc, char* argv[])
{
    if(argc != 4)
    {
        cerr << "Usage: " << argv[0] << " <Multicast Fraction> <Input.dot> <Output.dot>\n";
        return 1; 
    }

    try
    {
        multicastFraction = stof(string(argv[1]));
        if(multicastFraction < 0 || multicastFraction > 1)
            throw invalid_argument("Fraction should be > 0 and < 1.");
    }
    catch(exception &err)
    {
        cerr << err.what() << endl;
        cerr << "Invalid fraction argument\n";
        return 1;
    }

    InputFileName.assign(argv[2]);
    OutputFileName.assign(argv[3]);

    ifstream InputFile(InputFileName.c_str(), ios::in);
    ofstream OutputFile(OutputFileName.c_str(), ios::out);

    assert(OutputFile.is_open() && InputFile.is_open() && "Error opening files");

    BoostGraph Network(0);

    return 0;
}
