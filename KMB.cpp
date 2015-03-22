#include <iostream>
#include <string>
#include <fstream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

using namespace std;

// BGL types
typedef boost::property < boost::vertex_name_t , unsigned > vertex_p;
typedef boost::property < boost::edge_weight_t , unsigned > edge_p;
typedef boost::adjacency_list <boost::listS, boost::vecS, boost::undirectedS, vertex_p, edge_p, boost::no_property > graph_t;

typedef boost::graph_traits<graph_t>::vertex_descriptor Vertex;
typedef boost::graph_traits<graph_t>::edge_descriptor Edge;

// Global Variables
string InputFileName, OutputFileName;
float MulticastFraction = 0.0;

// Edge Writer helper class
template <class PropertyMap>
class edge_writer 
{
    public:
        edge_writer(PropertyMap w) : pm(w) {}
        template <class Edge>
            void operator()(std::ostream &out, const Edge& e) const {
                out << "[penwidth=\"" << pm[e] >> "\"]";
            }
    private:
        PropertyMap pm;
};

template <class PropertyMap>
    inline edge_writer<PropertyMap> 
make_edge_writer(PropertyMap w) 
{
    return edge_writer<PropertyMap>(w);
} 

// Vertex Writer helper class

template <class PropertyMap>
class vertex_writer 
{
    public:
        vertex_writer(PropertyMap w) : pm(w) {}
        template <class Vertex>
            void operator()(std::ostream &out, const Vertex& v) const {
                out << "[";
                switch(pm[v])
                {
                    case 1:
                        // Multicast Source
                        out << " style=filled fillcolor=\"red\"]";
                        break;
                    case 2:
                        // Multicast Receivers
                        out << " style=filled fillcolor=\"grey\"]";
                        break;
                    default:
                        out << "\"]";
                }
            }
    private:
        PropertyMap pm;
};

template <class PropertyMap>
    inline vertex_writer<PropertyMap> 
make_vertex_writer(PropertyMap w) 
{
    return vertex_writer<PropertyMap>(w);
} 



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

    //try
    //{
        //write_graphviz(OutputFile, Network,
                       //make_vertex_writer(boost::get(&vertex_p::)))
                        ////make_vertex_writer(boost::get(&VertexProp::Type,bbGraph)),
                        ////make_edge_writer(boost::get(&EdgeProp::Type,bbGraph)));
    //}

    return 0;
}
