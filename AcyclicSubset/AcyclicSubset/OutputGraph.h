#ifndef OUTPUTGRAPH_H
#define OUTPUTGRAPH_H

#include "Simplex.h"
#include "IncidenceGraph.h"

class IncidenceGraph;

class OutputGraph
{

public:

    struct Node
    {        
        int                 index;
        Simplex             simplex;
        std::vector<Node *> subnodes;    
        std::vector<int>    kappa;
    
        Node(int index, const Simplex &s);
 
        void AddSubnode(Node *subnode, int kappa);
        void GetSubnodes(std::vector<Node *> &nodes);
        
        Node *FindNodeWithSimplex(const Simplex &s);
        static Node *FindNodeWithSimplex(std::vector<Node *> nodes, const Simplex &s);

        static bool Sorter(const Node *a, const Node *b);
    };

    typedef std::vector<Node *> Nodes;
    Nodes nodes;

    OutputGraph(IncidenceGraph *ig);
    OutputGraph(const SimplexList &simplexList);
    ~OutputGraph();

private:

    int nodeIndex;
    IncidenceGraph *incidenceGraph;

    Node *GenerateNode(IncidenceGraph::Node *baseNode, Simplex &baseSimplex, Nodes &generatedSubnodes, IncidenceGraph::IntersectionFlags &subnodesFlags);
    Node *AddNode(const Simplex &s);
    Node *FindOrAddNode(const Simplex &simplex);

};

#endif /* OUTPUTGRAPH_H */
