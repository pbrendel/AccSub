/*
 * File:   AccSpanningTree.h
 * Author: Piotr Brendel
 */

#ifndef ACCSPANNINGTREE_H
#define ACCSPANNINGTREE_H

#include "Simplex.h"
#include "PartitionGraph.h"

class AccSpanningTree
{
    
public:
    
    struct Edge;
    
    struct Node
    {
        PartitionGraph::Node *parent;
        int subtreeID;
        std::set<Vertex> borderVerts;
        int accSubSize;
        std::vector<Edge *> edges;
        IncidenceGraph::ConnectedComponent connectedComponent;
        std::vector<Vertex> boundaryVertsToConnect;
        std::vector<IncidenceGraph::Path> boundaryVertsConnectingPaths;
        bool isConnectedToAccSub;

        Node(PartitionGraph::Node *parent, int id, IncidenceGraph::ConnectedComponent connectedComponent, std::set<Vertex> &borderVerts, int accSubSize)
        {
            this->parent = parent;
            this->subtreeID = id;
            this->connectedComponent = connectedComponent;
            this->borderVerts = borderVerts;
            this->accSubSize = accSubSize;
            this->isConnectedToAccSub = false;
        }

        void AddEdge(Edge *edge)
        {
            edges.push_back(edge);
        }
        
        void FindAccSubToBorderConnection(Vertex borderVertex, IncidenceGraph::Path &path);
        void UpdateAccSubToBorderConnection(Vertex borderVertex, IncidenceGraph::Path &path);
        void UpdatePathFromBorderToAccSub(Vertex borderVertex, IncidenceGraph::Path &path);
        void UpdatePathFromAccSubToBorder(Vertex borderVertex, IncidenceGraph::Path &path);
        void FindBoundaryVertsConnectingPaths();
        void UpdateBoundaryVertsConnectingPaths();
    };
    
    struct Edge
    {
        Node *nodeA;
        Node *nodeB;
        IncidenceGraph::Path pathToA;
        IncidenceGraph::Path pathToB;
        Vertex intersectionVertex;
        bool isInSpanningTree;

        Edge(Node *na, Node *nb, Vertex iv)
        {
            nodeA = na;
            nodeB = nb;
            intersectionVertex = iv;
            isInSpanningTree = false;
        }

        void FindAccConnections();
        void UpdateAccConnections();
    };

    typedef std::vector<Node *> Nodes;
    typedef std::vector<Edge *> Edges;
    
    Nodes nodes;
    Edges edges;    

    AccSpanningTree(PartitionGraph *pg);
    ~AccSpanningTree();
    
    void JoinAccSubsets();
 
};

#endif /* ACCSPANNINGTREE_H */
