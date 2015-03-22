#include <iostream>
#include <string>
#include <fstream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

using namespace std;

enum vertex_kmb_color_t { vertex_kmb_color };
enum edge_penwidth_t { edge_penwidth = 1 };

namespace boost
{
    BOOST_INSTALL_PROPERTY(vertex, kmb_color);
    BOOST_INSTALL_PROPERTY(edge, penwidth);
}

// BGL types
typedef boost::property < boost::vertex_name_t , unsigned,
        boost::property < boost::vertex_color_t, float ,
        boost::property < vertex_kmb_color_t, string > > > vertex_p;
typedef boost::property < boost::edge_weight_t , unsigned,
        boost::property < edge_penwidth_t , unsigned > > edge_p;
typedef boost::adjacency_list <boost::listS, boost::vecS, boost::undirectedS, vertex_p, edge_p, boost::no_property > graph_t;

typedef boost::graph_traits<graph_t>::vertex_descriptor Vertex;
typedef boost::graph_traits<graph_t>::edge_descriptor Edge;

// Global Variables
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
    boost::property_map<graph_t, vertex_kmb_color_t>::type color = boost::get(vertex_kmb_color, Network);
    dp.property("fillcolor", color);
    boost::property_map<graph_t, boost::edge_weight_t>::type weight = boost::get(boost::edge_weight, Network);
    dp.property("weight", weight); 
    boost::property_map<graph_t, edge_penwidth_t>::type penwidth = boost::get(edge_penwidth, Network);
    dp.property("penwidth", penwidth); 

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

    try
    {
        write_graphviz_dp(OutputFile, Network, dp);
    }
    catch(exception &err)
    {
        cerr << err.what() << endl;
        cerr << "write_graphviz failed for " << OutputFileName << "\n";
        return 1;
    }

    return 0;
}
