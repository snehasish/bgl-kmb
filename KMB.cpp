#include <iostream>
#include <cassert>
#include <string>
#include <fstream>
#include <set>
#include <vector>
#include <map>
#include <random>
#include <chrono>
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

#ifdef DEBUG
    cout << "Running in debug mode\n";
    ifstream InputFile("steiner.dot", ios::in);
    ofstream OutputFile("debug.dot", ios::out);
#else
    ifstream InputFile(InputFileName.c_str(), ios::in);
    ofstream OutputFile(OutputFileName.c_str(), ios::out);
#endif

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
    unsigned NumMulticast = unsigned(MulticastFraction * NumVertices);

#ifndef DEBUG
    // Assume that vertices are numbered 0 -- N-1 (generated by BRITE)
    // Randomly select multicast nodes using uniform distribution

    std::default_random_engine generator;
    std::uniform_int_distribution<unsigned> distribution(0,NumMulticast - 1);    

    NumMulticast = NumMulticast == 0 ? 1 : NumMulticast;

    do
    {
        // Insert Vertex Names ( == Vertex descriptors )
        unsigned value = distribution(generator) % NumMulticast;
        MulticastVertices.insert(value);
    } while( MulticastVertices.size() < NumMulticast);

#else
    // debug for steiner.dot
    NumMulticast = 4;
    cout << "Num Vertex: " << NumVertices << endl;
    cout << "Num MCast: " << NumMulticast << endl;
    MulticastVertices.insert(0);
    MulticastVertices.insert(1);
    MulticastVertices.insert(2);
    MulticastVertices.insert(3);

    // Debug
    ostream dbg(std::cout.rdbuf());
    write_graphviz_dp(dbg, Network, dp);
#endif

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

#ifdef DEBUG
    // Debug
    dynamic_properties dp_q;
    auto name_q = get(&vertex_q::name, G1);
    dp_q.property("node_id", name_q);
    auto label_q = get(&vertex_q::label, G1);
    dp_q.property("label", label_q);
    auto weight_q = get(&edge_q::weight, G1);
    dp_q.property("weight", weight_q);
    write_graphviz_dp(dbg, G1, dp_q);
    cout << "Step1-----------------------------------\n";
#endif

    map<Vertex, vector<Vertex> > AllPreds;
    map<Vertex, vector<unsigned> > AllDistances;

    auto start_step_1 = chrono::steady_clock::now();

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

    auto end_step_1 = chrono::steady_clock::now();
    cout << "[KMB] Step1: "<< chrono::duration <double, milli> (end_step_1-start_step_1).count() << " ms" << endl;

#ifdef DEBUG
    // Debug
    write_graphviz_dp(dbg, G1, dp_q);
    cout << "Step2-----------------------------------\n";
#endif

    // Step 2a -- Construct Minimum Spanning Tree from G1

    auto start_step_2 = chrono::steady_clock::now();

    vector<EdgeQ> st;
    kruskal_minimum_spanning_tree(G1, back_inserter(st), 
            weight_map(get(&edge_q::weight, G1)));

    // Step 2b -- Trim G1 based on MST

    set<VertexQ> vKeep, vRemove;
    set<EdgeQ> eKeep, eRemove;

    for(auto &e : st)
    {
        vKeep.insert(source(e, G1)); vKeep.insert(target(e, G1));
        eKeep.insert(e);
    }

    auto end_step_2 = chrono::steady_clock::now();
    cout << "[KMB] Step2: "<< chrono::duration <double, milli> (end_step_2-start_step_2).count() << " ms" << endl;

#ifdef DEBUG
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
    cout << "Step3-----------------------------------\n";
#endif

    auto start_step_3 = chrono::steady_clock::now();
    // Step 3 -- Add the shortest paths back to G1 (expand each edge)

    graph_q_t G2(0);

    // For each edge in the MST, get the path we saved earlier
    // For each vertex in path, if vertex is not added, add vertex, add edge
    // else check if edge is added else add edge. Set weight

    set<unsigned> added_labels;
    map<unsigned, VertexQ> label_map;

    for(auto &Ex : eKeep)
    {

#ifdef DEBUG
        cout << "Source: " << G1[source(Ex,G1)].label << "\n";
        cout << "Target: " << G1[target(Ex,G1)].label << "\n";
#endif

        // Traverse from sink to source

        auto predecessor_map = AllPreds[G1[source(Ex,G1)].label];
        Vertex v = G1[target(Ex, G1)].label;
        for(Vertex u = predecessor_map[v];          // Start by setting 'u' to the destintaion node's predecessor
                u != v;                          // Keep tracking the path until we get to the source
                v = u, u = predecessor_map[v])   // Set the current vertex to the current predecessor, and the predecessor to one level up 
        {   
            VertexQ Vx, Ux;
            if(added_labels.count(v) == 0) 
            {
                Vx = add_vertex(G2);
                G2[Vx].label = v;
                G2[Vx].name = Vx;
                added_labels.insert(v);
                label_map.insert(make_pair(v,Vx));
            }
            else
            {
                Vx = label_map[v];
            }


            if(added_labels.count(u) == 0)
            {
                Ux = add_vertex(G2);
                G2[Ux].label = u;
                G2[Ux].name = Ux;
                added_labels.insert(u);
                label_map.insert(make_pair(u,Ux));
            }
            else
            {
                Ux = label_map[u];
            }

            EdgeQ eq; bool fq = false;
            tie(eq, fq) = edge(Vx, Ux, G2);
            if(!fq)
            {
                Edge e; bool f = false;
                tie(e, f) = edge(u, v, Network);
                assert(f && "Could not find edge in original graph!");           
                double wt = Network[e].weight;

                tie(eq, fq) = add_edge(Vx, Ux, G2);
                G2[eq].weight = wt;
            }
        }
    }

    auto end_step_3 = chrono::steady_clock::now();
    cout << "[KMB] Step3: "<< chrono::duration <double, milli> (end_step_3-start_step_3).count() << " ms" << endl;

#ifdef DEBUG
    dynamic_properties dp_q2;
    auto name_q2 = get(&vertex_q::name, G2);
    dp_q2.property("node_id", name_q2);
    auto label_q2 = get(&vertex_q::label, G2);
    dp_q2.property("label", label_q2);
    auto weight_q2 = get(&edge_q::weight, G2);
    dp_q2.property("weight", weight_q2);

    write_graphviz_dp(dbg, G2, dp_q2);

    cout << "Step4-----------------------------------\n";
#endif

    auto start_step_4 = chrono::steady_clock::now();
    // Step 4a -- Construct Minimum Spanning Tree from G2

    vector<EdgeQ> st2;
    kruskal_minimum_spanning_tree(G2, back_inserter(st2), 
            weight_map(get(&edge_q::weight, G2)));

    // Step 4b -- Trim G2 based on MST

    set<VertexQ> vKeep2, vRemove2;
    set<EdgeQ> eKeep2, eRemove2;

    for(auto &e : st2)
    {
        vKeep2.insert(source(e, G2)); vKeep2.insert(target(e, G2));
        eKeep2.insert(e);
    }

    BGL_FORALL_VERTICES(v, G2, graph_q_t) 
    {
        if(vKeep2.count(v) == 0) 
            vRemove2.insert(v);
    }

    BGL_FORALL_EDGES(e, G2, graph_q_t)
    {
        if(eKeep2.count(e) == 0)
            eRemove2.insert(e);
    }

    for(auto Ex : eRemove2) remove_edge(Ex, G2);
    for(auto Vx : vRemove2) remove_vertex(Vx, G2);

    auto end_step_4 = chrono::steady_clock::now();
    cout << "[KMB] Step4: "<< chrono::duration <double, milli> (end_step_4-start_step_4).count() << " ms" << endl;

#ifdef DEBUG
    write_graphviz_dp(dbg, G2, dp_q2);
    cout << "Step5-----------------------------------\n";
#endif

    auto start_step_5 = chrono::steady_clock::now();

    // Remove non-steiner leaf nodes
    // Possible optimization : Start from leaf node and delete upwards instead 
    // of iterating over and over the graph and examining each time.

    set<VertexQ> to_remove;
    do
    {
        for(auto v : to_remove)  
        {
            clear_vertex(v, G2);
            remove_vertex(v, G2);
        }

        to_remove.clear();

        BGL_FORALL_VERTICES(v, G2, graph_q_t)      
        {
            if(MulticastVertices.count(G2[v].label) == 0 && degree(v, G2) == 1)
            {
                //cout << "Removing: " << v << " " << G2[v].label << "\n";
                to_remove.insert(v);
            }
        }
    } while(to_remove.size() > 0);

    auto end_step_5 = chrono::steady_clock::now();
    cout << "[KMB] Step5: "<< chrono::duration <double, milli> (end_step_5-start_step_5).count() << " ms" << endl;

#ifdef DEBUG
    write_graphviz_dp(dbg, G2, dp_q2);
    cout << "Step6-----------------------------------\n";
#endif

    // Update the fillcolor and penwidth in the original graph and print it out. 

    for(auto &m : MulticastVertices)
    {
        Network[m].fillcolor = string("black");
    }

    BGL_FORALL_EDGES(ed, G2, graph_q_t)
    {
        Vertex vx = G2[source(ed, G2)].label;
        Vertex vy = G2[target(ed, G2)].label;

        //cout << vx << " " << vy << "\n";
        Edge e; bool f = false;
        tie(e,f) = edge(vx, vy, Network);
        assert(f && "Edge not found in original graph!");
        Network[e].penwidth = 5;
    }

    try
    {
        write_graphviz_dp(OutputFile, Network, dp);
    }
    catch(std::exception &err)
    {
        cerr << err.what() << endl;
        cerr << "write_graphviz failed for " << OutputFileName << "\n";
        return 1;
    }

    return 0;
}


