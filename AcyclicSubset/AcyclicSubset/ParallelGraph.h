/*
 * File:   ParallelGraph.h
 * Author: Piotr Brendel
 */

#ifndef PARALLELGRAPH_H
#define PARALLELGRAPH_H

#include "IncidenceGraph.h"

// szablon
// PrepareDataPolicy
// ComputationsPolicy

class ParallelGraph
{
public:

    struct DataNode;
    struct SpanningTreeNode;

    struct DataEdge
    {
        DataNode *nodeA;
        DataNode *nodeB;

        DataEdge(DataNode *na, DataNode *nb)
        {
            nodeA = na;
            nodeB = nb;
        }
    };

    struct DataNode
    {
        SimplexPtrList simplexPtrList;
        std::set<Vertex> verts;
        std::set<Vertex> borderVerts;
        std::vector<DataEdge *> edges;

        std::vector<SpanningTreeNode *> spanningTreeNodes;

        IncidenceGraph *ig;
        IncidenceGraph::VertexNodesMap H;

        int processRank;

        DataNode()
        {
            ig = 0;
        }

        ~DataNode()
        {
            delete ig;
        }

        void AddEdge(DataEdge *edge, Simplex &edgeVerts)
        {
            edges.push_back(edge);
            AddToBorder(edgeVerts);
        }

        void AddToBorder(std::vector<Vertex> verts)
        {
            borderVerts.insert(verts.begin(), verts.end());
        }

        int GetConstantSimplexSize();
        void CreateIntNodesMapWithBorderNodes();
        void RemoveChildAndCopySimplexPtrList(SpanningTreeNode *node, SimplexPtrList &simplexPtrList);
    };

    typedef std::vector<DataNode *> DataNodes;
    typedef std::vector<DataEdge *> DataEdges;

    struct SpanningTreeEdge
    {
        SpanningTreeNode *nodeA;
        SpanningTreeNode *nodeB;
        IncidenceGraph::Path pathToA;
        IncidenceGraph::Path pathToB;
        Vertex intersectionVertex;
        bool isInSpanningTree;

        SpanningTreeEdge(SpanningTreeNode *na, SpanningTreeNode *nb, Vertex intersectionVertex)
        {
            nodeA = na;
            nodeB = nb;
            this->intersectionVertex = intersectionVertex;
            isInSpanningTree = false;
        }

        void FindAcyclicConnections();
        void UpdateAcyclicConnections();
    };

    struct SpanningTreeNode
    {
        DataNode *parent;
        int subtreeID;
        std::set<Vertex> borderVerts;
        int acyclicSubsetSize;
        std::vector<SpanningTreeEdge *> edges;
        IncidenceGraph::ConnectedComponent connectedComponent;
        std::vector<Vertex> singleBorderVerts;
        bool isConnectedToAcyclicSubset;

        SpanningTreeNode(DataNode *parent, int id, IncidenceGraph::ConnectedComponent connectedComponent, std::set<Vertex> &borderVerts, int acyclicSubsetSize)
        {
            this->parent = parent;
            this->subtreeID = id;
            this->connectedComponent = connectedComponent;
            this->borderVerts = borderVerts;
            this->acyclicSubsetSize = acyclicSubsetSize;
            this->isConnectedToAcyclicSubset = false;
        }

        ~SpanningTreeNode()
        {
        }

        void AddEdge(SpanningTreeEdge *edge)
        {
            edges.push_back(edge);
        }

        void UpdateBorderVerts();
        void FindAcyclicSubsetToBorderConnection(Vertex borderVertex, IncidenceGraph::Path &path);
        void UpdateAcyclicSubsetToBorderConnection(Vertex borderVertex, IncidenceGraph::Path &path);
        void UpdatePathFromBorderToAcyclicSubset(Vertex borderVertex, IncidenceGraph::Path &path);
        void UpdatePathFromAcyclicSubsetToBorder(Vertex borderVertex, IncidenceGraph::Path &path);
    };

    typedef std::vector<SpanningTreeNode *> SpanningTreeNodes;
    typedef std::vector<SpanningTreeEdge *> SpanningTreeEdges;

    void GetIntersection(std::vector<Vertex> &intersection, std::set<Vertex> &setA, std::set<Vertex> &setB);
    
    ParallelGraph(SimplexList &simplexList, int packsCount, AccSubAlgorithm accSubAlgorithm, AcyclicTest<IncidenceGraph::IntersectionFlags> *acyclicTest);
    ~ParallelGraph();

    IncidenceGraph *GetIncidenceGraph() { return incidenceGraph; }

private:

    IncidenceGraph *incidenceGraph;
    int initialSize;

    AccSubAlgorithm accSubAlgorithm;
    AcyclicTest<IncidenceGraph::IntersectionFlags> *acyclicTest;

    DataNodes dataNodes;
    DataEdges dataEdges;
    DataNodes secondPhaseDataNodes;

    SpanningTreeNodes spanningTreeNodes;
    SpanningTreeEdges spanningTreeEdges;

    void DivideData(SimplexList &simplexList, int packSize);
    void CreateDataEdges();
    void CalculateIncidenceGraphs(DataNodes &sourceNodes);
    void CreateSpanningTree();
    void CombineGraphs();
};

#endif /* PARALLELGRAPH_H */
