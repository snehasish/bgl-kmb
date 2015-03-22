#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

typedef boost::property < boost::vertex_name_t , unsigned > vertex_p;
typedef boost::property < boost::edge_weight_t , unsigned > edge_p;
typedef boost::adjacency_list <boost::listS, boost::vecS, boost::undirectedS, vertex_p, edge_p, boost::no_property > graph_t;

string InputFileName, OutputFileName;
float MulticastFraction = 0.0;

int main(int argc, char* argv[])
{
    if(argc != 4)
    {
        cerr << "Usage: " << argv[0] << " <Multicast Fraction> <Input.dot> <Output.dot>\n";
        return 1; 
    }

    try
    {
        MulticastFraction = stof(string(argv[1]));
        if(MulticastFraction < 0 || MulticastFraction > 1)
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

    graph_t Network(0);
    boost::dynamic_properties dp;
    boost::property_map<graph_t, boost::vertex_name_t>::type name = boost::get(boost::vertex_name, Network);
    dp.property("node_id", name);
    boost::property_map<graph_t, boost::edge_weight_t>::type weight = boost::get(boost::edge_weight, Network);
    dp.property("weight", weight); 

    try
    {
        read_graphviz(InputFile, Network, dp, "node_id");
    }
    catch(exception &err)
    {
        cerr << err.what() << endl;
        cerr << "read_graphviz failed for " << InputFileName << "\n";
        return 1;
    }

    return 0;
}
