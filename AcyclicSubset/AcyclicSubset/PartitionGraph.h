/*
 * File:   PartitionGraph.h
 * Author: Piotr Brendel
 */

#ifndef PARTITIONGRAPH_H
#define PARTITIONGRAPH_H

#include "IncidenceGraph.h"

// szablon
// PrepareDataPolicy
// ComputationsPolicy

class PartitionGraph
{
public:

    struct Edge;

    struct Node
    {
        SimplexPtrList simplexPtrList;
        std::set<Vertex> verts;
        std::set<Vertex> borderVerts;
        std::vector<Edge *> edges;

        IncidenceGraph *ig;
        IncidenceGraph::VertexNodesMap H;

        int processRank;

        Node()
        {
            ig = 0;
        }

        Node(SimplexPtrList &simplexPtrList)
        {
            ig = 0;
            this->simplexPtrList = simplexPtrList;
        }
        
        ~Node()
        {
            delete ig;
        }

        void AddEdge(Edge *edge, Simplex &edgeVerts)
        {
            edges.push_back(edge);
            AddToBorder(edgeVerts);
        }

        void AddToBorder(std::vector<Vertex> verts)
        {
            borderVerts.insert(verts.begin(), verts.end());
        }

        void CreateIntNodesMapWithBorderNodes();
    };

    struct Edge
    {
        Node *nodeA;
        Node *nodeB;

        Edge(Node *na, Node *nb)
        {
            nodeA = na;
            nodeB = nb;
        }
    };
    
    typedef std::vector<Node *> Nodes;
    typedef std::vector<Edge *> Edges;

    void GetIntersection(std::vector<Vertex> &intersection, std::set<Vertex> &setA, std::set<Vertex> &setB);
    
    PartitionGraph(SimplexList &simplexList, int packsCount, AccSubAlgorithm accSubAlgorithm, AcyclicTest<IncidenceGraph::IntersectionFlags> *acyclicTest);
    ~PartitionGraph();

    IncidenceGraph *GetIncidenceGraph() { return incidenceGraph; }

    Nodes nodes;
    Edges edges;
    Nodes isolatedNodes;
    
private:

    IncidenceGraph *incidenceGraph;
    int initialSize;

    AccSubAlgorithm accSubAlgorithm;
    AcyclicTest<IncidenceGraph::IntersectionFlags> *acyclicTest; 
    
    void DivideData(SimplexList &simplexList, int packSize);
    void CreateDataEdges();
    void CalculateIncidenceGraphs(Nodes &sourceNodes);
    void CombineGraphs();
};

#endif /* PARTITIONGRAPH_H */
