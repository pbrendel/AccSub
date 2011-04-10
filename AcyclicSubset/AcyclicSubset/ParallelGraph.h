#ifndef PARALLELGRAPH_H
#define PARALLELGRAPH_H

#include "IncidenceGraph.h"

#define USE_MPI
#define MPI_MY_WORK_TAG        1
#define MPI_MY_DIE_TAG         2
#define MPI_MY_DATASIZE_TAG    3
#define MPI_MY_DATA_TAG        4

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
        SimplexPtrList simplexPtrList;
        SimplexList localSimplexList;
        std::set<Vertex> verts;
        std::set<Vertex> borderVerts;
        std::vector<DataEdge *> edges;

        IncidenceGraph *ig;
        IncidenceGraph::IntNodesMap H;

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

        void CreateIncidenceGraphLocally(const IncidenceGraph::Params &params, AcyclicTest<IncidenceGraph::IntersectionFlags> *test);
        void SendMPIData(const IncidenceGraph::Params &params, int processRank);
        void SetMPIIncidenceGraphData(int *buffer, int size);
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
    
    ParallelGraph(IncidenceGraph *ig, SimplexList &simplexList, IncidenceGraph::Params params, IncidenceGraph::ParallelParams parallelParams, AcyclicTest<IncidenceGraph::IntersectionFlags> *test, bool local);
    ~ParallelGraph();

private:

    IncidenceGraph *incidenceGraph;
    bool local;

    DataNodes dataNodes;
    DataEdges dataEdges;
    AcyclicTreeNodes acyclicTreeNodes;
    AcyclicTreeEdges acyclicTreeEdges;

    void PrepareData(SimplexList &simplexList, int packSize);
    void DivideData(SimplexList &simplexList, int packSize);
    void CreateDataEdges();
    DataNode *GetNodeWithProcessRank(int processRank);
    void CalculateIncidenceGraphs(const IncidenceGraph::Params &params, AcyclicTest<IncidenceGraph::IntersectionFlags> *test);
    void CreateAcyclicTree();
    void CombineGraphs();

};

#endif /* PARALLELGRAPH_H */
