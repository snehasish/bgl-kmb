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

typedef adjacency_list <listS, vecS, undirectedS, vertex_p, edge_p, no_property > graph_p;

typedef graph_traits<graph_p>::vertex_descriptor vd_p;
typedef graph_traits<graph_p>::edge_descriptor ed_p;

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

typedef adjacency_list <listS, vecS, undirectedS, vertex_q, edge_q, no_property > graph_q;
typedef graph_traits<graph_q>::vertex_descriptor vd_q;
typedef graph_traits<graph_q>::edge_descriptor ed_q;

// Global Variables
string input_filename, output_filename;
float multicast_fraction = 0.0;
unsigned vertex_count = 0;
set<unsigned> multicast_vertices;


int main(int argc, char* argv[])
{
    if(argc != 4)
    {
        cerr << "Usage: " << argv[0] << " <Multicast Fraction> <Input.dot> <Output.dot>\n";
        return 1; 
    }

    try
    {
        multicast_fraction = stof(string(argv[1]));
        if(multicast_fraction < 0 || multicast_fraction > 1)
            throw invalid_argument("Fraction should be > 0 and < 1.");
    }
    catch(std::exception &err)
    {
        cerr << err.what() << endl;
        cerr << "Invalid fraction argument\n";
        return 1;
    }

    input_filename.assign(argv[2]);
    output_filename.assign(argv[3]);

#ifdef DEBUG
    cout << "Running in debug mode\n";
    ifstream input_file("steiner.dot", ios::in);
    ofstream output_file("debug.dot", ios::out);
#else
    ifstream input_file(input_filename.c_str(), ios::in);
    ofstream output_file(output_filename.c_str(), ios::out);
#endif

    assert(output_file.is_open() && input_file.is_open() && "Error opening files");

    graph_p network(0);
    dynamic_properties dp;
    auto name = get(&vertex_p::name, network);
    dp.property("node_id", name);
    auto fillcolor = get(&vertex_p::fillcolor, network);
    dp.property("fillcolor", fillcolor);
    auto weight = get(&edge_p::weight, network);
    dp.property("weight", weight); 
    auto penwidth = get(&edge_p::penwidth, network);
    dp.property("penwidth", penwidth); 

    try
    {
        read_graphviz(input_file, network, dp, "node_id");
    }
    catch(std::exception &err)
    {
        cerr << err.what() << endl;
        cerr << "read_graphviz failed for " << input_filename << "\n";
        return 1;
    }

    // Initialize all dynamic properties

    BGL_FORALL_VERTICES(v, network, graph_p)
    {
        fillcolor[v] = "white";
    }

    BGL_FORALL_EDGES(e, network, graph_p)
    {
        penwidth[e] = 1;
    }

    // TODO: Add timing code

    vertex_count = num_vertices(network);
    unsigned multicast_count = unsigned(multicast_fraction * vertex_count);

#ifndef DEBUG
    // Assume that vertices are numbered 0 -- N-1 (generated by BRITE)
    // Randomly select multicast nodes using uniform distribution

    std::default_random_engine generator;
    std::uniform_int_distribution<unsigned> distribution(0, multicast_count - 1);    

    multicast_count = multicast_count == 0 ? 1 : multicast_count;

    do
    {
        // Insert vd_p Names ( == vd_p descriptors )
        unsigned value = distribution(generator) % multicast_count;
        multicast_vertices.insert(value);
    } while( multicast_vertices.size() < multicast_count);

#else
    // debug for steiner.dot
    multicast_count = 4;
    cout << "Num vd_p: " << vertex_count << endl;
    cout << "Num MCast: " << multicast_count << endl;
    multicast_vertices.insert(0);
    multicast_vertices.insert(1);
    multicast_vertices.insert(2);
    multicast_vertices.insert(3);

    // Debug
    ostream dbg(std::cout.rdbuf());
    write_graphviz_dp(dbg, network, dp);
#endif

    graph_q G1(0);
    map<unsigned, vd_q> vd_p_to_vd_q_map;

    for(auto &V : multicast_vertices)
    {
        vd_q Vx = add_vertex(G1); 
        //cout << Vx << " " << V << "\n";
        G1[Vx].label = V;
        G1[Vx].name = Vx;
        vd_p_to_vd_q_map.insert(make_pair(V,Vx));
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

    map<vd_p, vector<vd_p> > AllPreds;
    map<vd_p, vector<unsigned> > AllDistances;

    auto start_step_1 = chrono::steady_clock::now();

    // Step 1 -- Construct undirected distance graph G1, G and S.
    for(auto V = multicast_vertices.begin(), E = multicast_vertices.end(); V != E; V++)
    {
        vd_p Vx = *V;
        // Get the shortest path between Vx,All 
        vector<unsigned> D(vertex_count);

        AllPreds.insert(pair<vd_p,vector<vd_p> >(Vx, vector<vd_p>(vertex_count)));
        AllDistances.insert(pair<vd_p,vector<unsigned> >(Vx, vector<unsigned>(vertex_count)));

        dijkstra_shortest_paths(network, Vx,
                weight_map(get(&edge_p::weight,network))
                .distance_map(make_iterator_property_map(AllDistances[Vx].begin(), get(vertex_index, network)))
                .predecessor_map(make_iterator_property_map(AllPreds[Vx].begin(), get(vertex_index, network))));

        for(auto W = std::next(V); W != E; W++)
        {
            vd_p Wx = *W; 
            //// Debug
            //cout << "Adding ed_p (Vx, Wx, D[Wx]): " << Vx << "," << Wx << "," << D[Wx] << endl;
            ed_q e; bool found;
            tie(e,found) = add_edge(vd_p_to_vd_q_map[*V], vd_p_to_vd_q_map[*W], G1);
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

    vector<ed_q> st;
    kruskal_minimum_spanning_tree(G1, back_inserter(st), 
            weight_map(get(&edge_q::weight, G1)));

    // Step 2b -- Trim G1 based on MST

    set<vd_q> vKeep, vRemove;
    set<ed_q> eKeep, eRemove;

    for(auto &e : st)
    {
        vKeep.insert(source(e, G1)); vKeep.insert(target(e, G1));
        eKeep.insert(e);
    }

    auto end_step_2 = chrono::steady_clock::now();
    cout << "[KMB] Step2: "<< chrono::duration <double, milli> (end_step_2-start_step_2).count() << " ms" << endl;

#ifdef DEBUG
    BGL_FORALL_VERTICES(v, G1, graph_q) 
    {
        if(vKeep.count(v) == 0) 
            vRemove.insert(v);
    }

    BGL_FORALL_EDGES(e, G1, graph_q)
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

    graph_q G2(0);

    // For each edge in the MST, get the path we saved earlier
    // For each vertex in path, if vertex is not added, add vertex, add edge
    // else check if edge is added else add edge. Set weight

    set<unsigned> added_labels;
    map<unsigned, vd_q> label_map;

    for(auto &Ex : eKeep)
    {

#ifdef DEBUG
        cout << "Source: " << G1[source(Ex,G1)].label << "\n";
        cout << "Target: " << G1[target(Ex,G1)].label << "\n";
#endif

        // Traverse from sink to source

        auto predecessor_map = AllPreds[G1[source(Ex,G1)].label];
        vd_p v = G1[target(Ex, G1)].label;
        for(vd_p u = predecessor_map[v];          // Start by setting 'u' to the destintaion node's predecessor
                u != v;                          // Keep tracking the path until we get to the source
                v = u, u = predecessor_map[v])   // Set the current vertex to the current predecessor, and the predecessor to one level up 
        {   
            vd_q Vx, Ux;
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

            ed_q eq; bool fq = false;
            tie(eq, fq) = edge(Vx, Ux, G2);
            if(!fq)
            {
                ed_p e; bool f = false;
                tie(e, f) = edge(u, v, network);
                assert(f && "Could not find edge in original graph!");           
                double wt = network[e].weight;

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

    vector<ed_q> st2;
    kruskal_minimum_spanning_tree(G2, back_inserter(st2), 
            weight_map(get(&edge_q::weight, G2)));

    // Step 4b -- Trim G2 based on MST

    set<vd_q> vKeep2, vRemove2;
    set<ed_q> eKeep2, eRemove2;

    for(auto &e : st2)
    {
        vKeep2.insert(source(e, G2)); vKeep2.insert(target(e, G2));
        eKeep2.insert(e);
    }

    BGL_FORALL_VERTICES(v, G2, graph_q) 
    {
        if(vKeep2.count(v) == 0) 
            vRemove2.insert(v);
    }

    BGL_FORALL_EDGES(e, G2, graph_q)
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

    set<vd_q> to_remove;
    do
    {
        for(auto v : to_remove)  
        {
            clear_vertex(v, G2);
            remove_vertex(v, G2);
        }

        to_remove.clear();

        BGL_FORALL_VERTICES(v, G2, graph_q)      
        {
            if(multicast_vertices.count(G2[v].label) == 0 && degree(v, G2) == 1)
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

    for(auto &m : multicast_vertices)
    {
        network[m].fillcolor = string("black");
    }

    BGL_FORALL_EDGES(ed, G2, graph_q)
    {
        vd_p vx = G2[source(ed, G2)].label;
        vd_p vy = G2[target(ed, G2)].label;

        //cout << vx << " " << vy << "\n";
        ed_p e; bool f = false;
        tie(e,f) = edge(vx, vy, network);
        assert(f && "ed_p not found in original graph!");
        network[e].penwidth = 5;
    }

    try
    {
        write_graphviz_dp(output_file, network, dp);
    }
    catch(std::exception &err)
    {
        cerr << err.what() << endl;
        cerr << "write_graphviz failed for " << output_filename << "\n";
        return 1;
    }

    return 0;
}


