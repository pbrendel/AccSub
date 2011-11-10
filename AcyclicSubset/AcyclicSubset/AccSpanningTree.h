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
        int acyclicSubsetSize;
        std::vector<Edge *> edges;
        IncidenceGraph::ConnectedComponent connectedComponent;
        std::vector<Vertex> singleBorderVerts;
        bool isConnectedToAcyclicSubset;

        Node(PartitionGraph::Node *parent, int id, IncidenceGraph::ConnectedComponent connectedComponent, std::set<Vertex> &borderVerts, int acyclicSubsetSize)
        {
            this->parent = parent;
            this->subtreeID = id;
            this->connectedComponent = connectedComponent;
            this->borderVerts = borderVerts;
            this->acyclicSubsetSize = acyclicSubsetSize;
            this->isConnectedToAcyclicSubset = false;
        }

        void AddEdge(Edge *edge)
        {
            edges.push_back(edge);
        }
        
        void FindAcyclicSubsetToBorderConnection(Vertex borderVertex, IncidenceGraph::Path &path);
        void UpdateAcyclicSubsetToBorderConnection(Vertex borderVertex, IncidenceGraph::Path &path);
        void UpdatePathFromBorderToAcyclicSubset(Vertex borderVertex, IncidenceGraph::Path &path);
        void UpdatePathFromAcyclicSubsetToBorder(Vertex borderVertex, IncidenceGraph::Path &path);        
        void UpdateBorderVerts();
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

        void FindAcyclicConnections();
        void UpdateAcyclicConnections();
    };

    typedef std::vector<Node *> Nodes;
    typedef std::vector<Edge *> Edges;
    
    Nodes nodes;
    Edges edges;    

    AccSpanningTree(PartitionGraph *pg);
    ~AccSpanningTree();
    
    void JoinAcyclicSubsets();
 
};

#endif /* ACCSPANNINGTREE_H */
