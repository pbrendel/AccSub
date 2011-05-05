#ifndef PARALLELGRAPH_H
#define PARALLELGRAPH_H

#include "IncidenceGraph.h"

#define MPI_MY_WORK_TAG        1
#define MPI_MY_DIE_TAG         2
#define MPI_MY_DATASIZE_TAG    3
#define MPI_MY_DATA_TAG        4

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
        IncidenceGraph::IntNodesMap H;

        int processRank;

        DataNode()
        {
            ig = 0;
        }

        ~DataNode()
        {
         //   delete ig;
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
        int GetConstantSimplexSize();
        void SendMPIData(const IncidenceGraph::Params &params, int processRank);
        void SetMPIIncidenceGraphData(int *buffer, int size);
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
    
    ParallelGraph(IncidenceGraph *ig, SimplexList &simplexList, IncidenceGraph::Params params, IncidenceGraph::ParallelParams parallelParams, AcyclicTest<IncidenceGraph::IntersectionFlags> *test, bool local);
    ~ParallelGraph();

private:

    IncidenceGraph *incidenceGraph;
    bool local;

    DataNodes dataNodes;
    DataEdges dataEdges;
    DataNodes secondPhaseDataNodes;
    SpanningTreeNodes spanningTreeNodes;
    SpanningTreeEdges spanningTreeEdges;

    void PrepareData(SimplexList &simplexList, int packSize);
    void DivideData(SimplexList &simplexList, int packSize);
    void CreateDataEdges();
    DataNode *GetNodeWithProcessRank(DataNodes &sourceNodes, int processRank);
    void CalculateIncidenceGraphs(DataNodes &sourceNodes, const IncidenceGraph::Params &params, AcyclicTest<IncidenceGraph::IntersectionFlags> *test);
    void CreateSpanningTree();
    void CombineGraphs();

public:
    
    static void MPISlave(int processRank);

};

#endif /* PARALLELGRAPH_H */
