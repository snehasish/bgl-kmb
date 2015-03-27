#include <iostream>
#include <cassert>
#include <string>
#include <fstream>
#include <set>
#include <vector>
#include <map>
#include <random>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>

using namespace std;
using namespace boost;

// Typedefs for Input and output graphs
struct vertex_p
{
    int name;
    float color;
    string fillcolor;
};

struct edge_p
{
    double weight;
    unsigned penwidth;
};

typedef adjacency_list <listS, vecS, undirectedS, vertex_p, edge_p, no_property > graph_t;

typedef graph_traits<graph_t>::vertex_descriptor Vertex;
typedef graph_traits<graph_t>::edge_descriptor Edge;

// Typedefs for intermediate graphs

struct vertex_q
{
   unsigned name;
   unsigned label;
   float color;
};

struct edge_q
{
    double weight;
};

typedef adjacency_list <listS, vecS, undirectedS, vertex_q, edge_q, no_property > graph_q_t;
typedef graph_traits<graph_q_t>::vertex_descriptor VertexQ;
typedef graph_traits<graph_q_t>::edge_descriptor EdgeQ;

// Global Variables
string InputFileName, OutputFileName;
float MulticastFraction = 0.0;
unsigned NumVertices = 0;
set<unsigned> MulticastVertices;


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
    catch(std::exception &err)
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
    dynamic_properties dp;
    auto name = get(&vertex_p::name, Network);
    dp.property("node_id", name);
    auto fillcolor = get(&vertex_p::fillcolor, Network);
    dp.property("fillcolor", fillcolor);
    auto weight = get(&edge_p::weight, Network);
    dp.property("weight", weight); 
    auto penwidth = get(&edge_p::penwidth, Network);
    dp.property("penwidth", penwidth); 

    try
    {
        read_graphviz(InputFile, Network, dp, "node_id");
    }
    catch(std::exception &err)
    {
        cerr << err.what() << endl;
        cerr << "read_graphviz failed for " << InputFileName << "\n";
        return 1;
    }

    // Initialize all dynamic properties
    
    BGL_FORALL_VERTICES(v, Network, graph_t)
    {
        fillcolor[v] = "white";
    }

    BGL_FORALL_EDGES(e, Network, graph_t)
    {
        penwidth[e] = 1;
    }

    // TODO: Add timing code
    
    NumVertices = num_vertices(Network);
    unsigned NumMulticast = (unsigned)MulticastFraction * NumVertices;

    // Debug
    NumMulticast = 4;
    cout << "Num Vertex: " << NumVertices << endl;
    cout << "Num MCast: " << NumMulticast << endl;

    // Assume that vertices are numbered 0 -- N-1 (generated by BRITE)
    // Randomly select multicast nodes using uniform distribution
   
    random_device RandomDevice;
    mt19937 Generator(RandomDevice());
    uniform_int_distribution<> Dist(0, NumMulticast - 1);

    //do
    //{
        //// Insert Vertex Names ( not Vertex descriptors )
        //MulticastVertices.insert((unsigned)Dist(Generator));
    //} while( MulticastVertices.size() < NumMulticast);
   
    // debug for steiner.dot
    MulticastVertices.insert(0);
    MulticastVertices.insert(1);
    MulticastVertices.insert(2);
    MulticastVertices.insert(3);

    // TODO: Fix possible issue for Vertex descriptor mismatch
    // Note : Vertex Name is an unsigned int which is the same type as 
    // the Vertex descriptor, however we need to be careful when we are using 
    // one or the other and convert between them using the name_map

    // Debug
    ostream dbg(std::cout.rdbuf());
    write_graphviz_dp(dbg, Network, dp);

    graph_q_t G1(0);
    map<unsigned, VertexQ> RMap;

    for(auto &V : MulticastVertices)
    {
        VertexQ Vx = add_vertex(G1); 
        //cout << Vx << " " << V << "\n";
        G1[Vx].label = V;
        G1[Vx].name = Vx;
        RMap.insert(make_pair(V,Vx));
    }


    // Debug
    dynamic_properties dp_q;
    auto name_q = get(&vertex_q::name, G1);
    dp_q.property("node_id", name_q);
    auto label_q = get(&vertex_q::label, G1);
    dp_q.property("label", label_q);
    //write_graphviz_dp(dbg, G1, dp_q);
    
    map<Vertex, vector<Vertex> > AllPreds;
    map<Vertex, vector<unsigned> > AllDistances;
    // Step 1 -- Construct undirected distance graph G1, G and S.
    for(auto V = MulticastVertices.begin(), E = MulticastVertices.end(); V != E; V++)
    {
        Vertex Vx = *V;
        // Get the shortest path between Vx,All 
        vector<unsigned> D(NumVertices);

        AllPreds.insert(pair<Vertex,vector<Vertex> >(Vx, vector<Vertex>(NumVertices)));
        AllDistances.insert(pair<Vertex,vector<unsigned> >(Vx, vector<unsigned>(NumVertices)));
                                
        dijkstra_shortest_paths(Network, Vx,
                                weight_map(get(&edge_p::weight,Network))
                                .distance_map(make_iterator_property_map(AllDistances[Vx].begin(), get(vertex_index, Network)))
                                .predecessor_map(make_iterator_property_map(AllPreds[Vx].begin(), get(vertex_index, Network))));

        for(auto W = std::next(V); W != E; W++)
        {
            Vertex Wx = *W; 
            //// Debug
            //cout << "Adding Edge (Vx, Wx, D[Wx]): " << Vx << "," << Wx << "," << D[Wx] << endl;
            EdgeQ e; bool found;
            tie(e,found) = add_edge(RMap[*V], RMap[*W], G1);
            G1[e].weight = AllDistances[Vx][Wx];
        }
    }

    // Debug
    //write_graphviz_dp(dbg, G1, dp_q);

    // Step 2a -- Construct Minimum Spanning Tree from G1
    
    vector<EdgeQ> st;
    kruskal_minimum_spanning_tree(G1, back_inserter(st), 
                                  weight_map(get(&edge_q::weight, G1)));

    // Step 2b -- Trim G1 based on MST
    
    set<VertexQ> vKeep, vRemove;
    set<EdgeQ> eKeep, eRemove;

    for(auto &e : st)
    {
        cout << "From: " << source(e, G1) << " To: " << target(e, G1) << "\n";
        vKeep.insert(source(e, G1)); vKeep.insert(target(e, G1));
        eKeep.insert(e);
    }

    BGL_FORALL_VERTICES(v, G1, graph_q_t) 
    {
        if(vKeep.count(v) == 0) 
            vRemove.insert(v);
    }
    
    BGL_FORALL_EDGES(e, G1, graph_q_t)
    {
        if(eKeep.count(e) == 0)
            eRemove.insert(e);
    }

    for(auto Ex : eRemove) remove_edge(Ex, G1);
    for(auto Vx : vRemove) remove_vertex(Vx, G1);

    write_graphviz_dp(dbg, G1, dp_q);

    // Step 3 -- Add the shortest paths back to G1 (expand each edge)
   
    map<unsigned, VertexQ> labelVertexQmap;
    set<unsigned> addedLabels;
    for(auto Vx: vKeep) 
    {
        addedLabels.insert(G1[Vx].label);
        labelVertexQmap.insert(make_pair(G1[Vx].label, Vx));
    }

    for(auto &Ex : eKeep)
    {
        cout << "Source: " << G1[source(Ex,G1)].label << "\n";
        auto predecessor_map = AllPreds[G1[source(Ex,G1)].label];

        // Traverse from sink to source
        
        Vertex v = G1[target(Ex, G1)].label;
        cout << "Target: " << G1[target(Ex,G1)].label << "\n";
        cout << "Going from: " << v << " -> " << predecessor_map[v] << "\n";
        bool edge_flag = true;
        for(Vertex u = predecessor_map[v];          // Start by setting 'u' to the destintaion node's predecessor
                   u != v;                          // Keep tracking the path until we get to the source
                   v = u, u = predecessor_map[v])   // Set the current vertex to the current predecessor, and the predecessor to one level up 
        {
            // Get the edge from the Network Graph
            Edge e; bool f = false;
            tie(e, f) = edge(u, v, Network);
            assert(f && "Could not find edge in original graph!");           
            cout << "Found " << source(e, Network) << " - " << target(e, Network) << "\n";
            double wt = Network[e].weight;

            if(addedLabels.count(u) == 0)
            {
                // Add a vertex and thus also add an edge
                VertexQ Vx = add_vertex(G1);
                G1[Vx].label = u;
                labelVertexQmap.insert(make_pair(u,Vx));

                VertexQ Wx = labelVertexQmap[v];
                EdgeQ eq; bool fq = false;
                tie(eq, fq) = add_edge(Vx, Wx, G1);
                G1[eq].weight = wt;
            }
            else
            {
                // Label (and thus vertex) already added 
                // check if edge exists, if not add edge
                VertexQ Vx = labelVertexQmap[u], Wx = labelVertexQmap[v];
                EdgeQ eq; bool fq = false;
                tie(eq, fq) = edge(Vx, Wx, G1);
                if(!fq)
                {
                    // Edge not found
                    tie(eq, fq) = add_edge(Vx, Wx, G1);
                    G1[eq].weight = wt;
                }
                else
                    edge_flag = false;
            }
        }
        if(edge_flag)
            remove_edge(Ex, G1); // ??
    }
    
    write_graphviz_dp(dbg, G1, dp_q);

    //try
    //{
        //write_graphviz_dp(OutputFile, Network, dp);
    //}
    //catch(std::exception &err)
    //{
        //cerr << err.what() << endl;
        //cerr << "write_graphviz failed for " << OutputFileName << "\n";
        //return 1;
    //}

    //return 0;
}
