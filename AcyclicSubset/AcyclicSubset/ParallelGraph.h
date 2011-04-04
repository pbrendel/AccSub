#ifndef _PARALLEL_GRAPH_H_
#define _PARALLEL_GRAPH_H_

#include "IncidenceGraph.h"

class ParallelGraph
{
public:

    struct DataNode;
    
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
        SimplexList simplexList;
        std::set<Vertex> verts;
        std::set<Vertex> borderVerts;
        std::vector<DataEdge *> edges;

        IncidenceGraph *ig;
        IncidenceGraph::IntNodesMap H;

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

        void CreateIncidenceGraph(const IncidenceGraph::Params &params, AcyclicTest<IncidenceGraph::IntersectionFlags> *test);
        void CreateIntNodesMapWithBorderNodes();
    };

    typedef std::vector<DataNode *> DataNodes;
    typedef std::vector<DataEdge *> DataEdges;

    struct AcyclicTreeNode;

    struct AcyclicTreeEdge
    {
        AcyclicTreeNode *nodeA;
        AcyclicTreeNode *nodeB;
        IncidenceGraph::Path pathToA;
        IncidenceGraph::Path pathToB;
        Vertex intersectionVertex;
        bool isAcyclic;

        AcyclicTreeEdge(AcyclicTreeNode *na, AcyclicTreeNode *nb, Vertex intersectionVertex)
        {
            nodeA = na;
            nodeB = nb;
            this->intersectionVertex = intersectionVertex;
            isAcyclic = false;
        }

        void FindAcyclicConnections();
        void UpdateAcyclicConnections();
    };

    struct AcyclicTreeNode
    {
        int acyclicID;
        std::set<Vertex> borderVerts;
        int acyclicSubsetSize;
        std::vector<AcyclicTreeEdge *> edges;
        IncidenceGraph::ConnectedComponent connectedComponent;
        std::vector<Vertex> singleBorderVerts;
        bool isConnectedToAcyclicSubset;

        AcyclicTreeNode(int id, IncidenceGraph::ConnectedComponent connectedComponent, std::set<Vertex> &borderVerts, int acyclicSubsetSize)
        {
            this->acyclicID = id;
            this->connectedComponent = connectedComponent;
            this->borderVerts = borderVerts;
            this->acyclicSubsetSize = acyclicSubsetSize;
            this->isConnectedToAcyclicSubset = false;
        }

        ~AcyclicTreeNode()
        {
        }

        void AddEdge(AcyclicTreeEdge *edge)
        {
            edges.push_back(edge);
        }

        void UpdateBorderVerts();
        void FindAcyclicSubsetToBorderConnection(Vertex borderVertex, IncidenceGraph::Path &path);
        void UpdateAcyclicSubsetToBorderConnection(Vertex borderVertex, IncidenceGraph::Path &path);
        void UpdatePathFromBorderToAcyclicSubset(Vertex borderVertex, IncidenceGraph::Path &path);
        void UpdatePathFromAcyclicSubsetToBorder(Vertex borderVertex, IncidenceGraph::Path &path);
    };

    typedef std::vector<AcyclicTreeNode *> AcyclicTreeNodes;
    typedef std::vector<AcyclicTreeEdge *> AcyclicTreeEdges;

    void GetIntersection(std::vector<Vertex> &intersection, std::set<Vertex> &setA, std::set<Vertex> &setB);
    
    ParallelGraph(IncidenceGraph *ig, const SimplexList &simplexList, IncidenceGraph::Params params, IncidenceGraph::ParallelParams parallelParams, AcyclicTest<IncidenceGraph::IntersectionFlags> *test);
    ~ParallelGraph();

private:

    IncidenceGraph *incidenceGraph;

    DataNodes dataNodes;
    DataEdges dataEdges;
    AcyclicTreeNodes acyclicTreeNodes;
    AcyclicTreeEdges acyclicTreeEdges;

    void PrepareData(SimplexList &simplexList, int packSize);
    void DivideData(const SimplexList &simplexList, int packSize);
    void CreateDataEdges();
    void CreateAcyclicTree();
    void CombineGraphs();

};

#endif // _PARALLEL_GRAPH_H_
