#ifndef _INCIDENCE_GRAPH_H_
#define _INCIDENCE_GRAPH_H_

#include "Simplex.h"
#include "AcyclicTest.hpp"
#include "IntersectionFlagsBitSet.hpp"

#include <list>
#include <queue>
#include <set>

class IncidenceGraph;

////////////////////////////////////////////////////////////////////////////////

class IncidenceGraph
{
    
public:

    typedef unsigned int IntersectionFlags;
    // typedef IntersectionFlagsBitSet<4> IntersectionFlags;
    
    struct Params
    {
        int dim;
        bool minimizeSimplices;
        bool sortNodes;

    public:

        Params(int dim)
        {
            this->dim = dim;
            this->minimizeSimplices = false;
            this->sortNodes = true;
        }

        Params(int dim, bool minimizeSimplices)
        {
            this->dim = dim;
            this->minimizeSimplices = minimizeSimplices;
            this->sortNodes = true;
        }

        Params(int dim, bool minimizeSimplices, bool sortNodes)
        {
            this->dim = dim;
            this->minimizeSimplices = minimizeSimplices;
            this->sortNodes = sortNodes;
        }

    };

    struct ParallelParams
    {
        int packSize;
        
    public:
        
        ParallelParams(int packSize)
        {
            this->packSize = packSize;
        }
    };

    struct Node;

    struct Edge
    {
        Node                *node;
        Simplex             intersection;
        IntersectionFlags   intersectionFlags;

        Edge(Node *node)
        {
            this->node = node;
            this->intersectionFlags = 0;
        }

        bool IntersectionCalculated()
        {
            return (intersectionFlags != 0);
        }
    };

    typedef std::vector<Edge> Edges;
    // listy dzialaja szybciej dla algorytmu online
    // typedef std::list<Edge> Edges;

    struct Node
    {
        typedef unsigned short int Flags;

        Simplex             *simplex;
        Edges               edges;
        void                *outputData; // tu bedzie zapisywany OutputNode

        enum  // properties flags
        {
            IGNPF_ACYCLIC                   = 0x0001,
            IGNPF_ADDED_TO_LIST             = 0x0002,
            IGNPF_ADDED_TO_GRAPH            = 0x0004,
            IGNPF_ADDED_TO_OUTPUT           = 0x0008,
            IGNPF_ON_BORDER                 = 0x0010,
            IGNPF_HELPER_FLAG_1             = 0x1000,
            IGNPF_HELPER_FLAG_2             = 0x2000,
            IGNPF_HELPER_FLAG_3             = 0x4000,
            IGNPF_HELPER_FLAG_4             = 0x8000,
        };
        
        Node(IncidenceGraph *graph, Simplex *simplex);

        void AddNeighbour(Node *neighbour);
        bool HasNeighbour(Node *neighbour);
        void RemoveNeighbour(Node *neighbour);
        void SetIntersection(Node *neighbour, const Simplex &intersection);
        bool HasAcyclicIntersection(AcyclicTest<IntersectionFlags> *test);
        void UpdateAcyclicIntersectionWithVertex(Vertex v);
        void UpdateAcyclicIntersectionWithEdge(Vertex v1, Vertex v2);
        void UpdateNeighboursAcyclicIntersection();
        void UpdateAcyclicIntersectionFlags(IntersectionFlags flags, IntersectionFlags flagsMaximalFaces);

        Simplex Normalize(const Simplex &simplex);
        int NormalizeVertex(int v);
        bool operator==(const Node &node);
        static bool Sorter(const Node *a, const Node *b);

#define GET_SET(FUNC, FLAG) inline bool Is##FUNC() const { return (propertiesFlags & IGNPF_##FLAG) == IGNPF_##FLAG; } \
                            inline void Is##FUNC(bool f) { if (f) propertiesFlags |= IGNPF_##FLAG; else propertiesFlags &= ~(IGNPF_##FLAG); }

        GET_SET(Acyclic, ACYCLIC)
        GET_SET(AddedToList, ADDED_TO_LIST)
        GET_SET(AddedToGraph, ADDED_TO_GRAPH)
        GET_SET(AddedToOutput, ADDED_TO_OUTPUT)
        GET_SET(OnBorder, ON_BORDER)
        GET_SET(HelperFlag1, HELPER_FLAG_1)
        GET_SET(HelperFlag2, HELPER_FLAG_2)
        GET_SET(HelperFlag3, HELPER_FLAG_3)
        GET_SET(HelperFlag4, HELPER_FLAG_4)

#undef GET_SET

        IntersectionFlags GetAcyclicIntersectionFlags() const { return acyclicIntersectionFlags; }
        IntersectionFlags GetAcyclicIntersectionFlagsMaximalFaces() const { return acyclicIntersectionFlagsMaximalFaces; }
        int GetAcyclicSubsetID() const { return acyclicSubsetID; }
        void SetAcyclicSubsetID(int id) { acyclicSubsetID = id; }

    private:

        IncidenceGraph      *graph;
        Flags               propertiesFlags;
        IntersectionFlags   acyclicIntersectionFlags;
        IntersectionFlags   acyclicIntersectionFlagsMaximalFaces;
        int                 acyclicSubsetID;
        std::map<int, int>  v2i;    // vertex to index map

    };

private:

    IncidenceGraph(const Params &params);
    IncidenceGraph(const SimplexList &simplexList, const Params &params);

public:

    typedef std::vector<Node *> Nodes;
    typedef std::list<Node *> Path;
    typedef Node *ConnectedComponent;
    typedef std::vector<ConnectedComponent> ConnectedComponents;
    typedef std::map<int, Nodes> IntNodesMap;
    typedef std::set<Vertex> VertsSet;

    std::map<Simplex, IntersectionFlags> configurationsFlags;
    std::map<Simplex, IntersectionFlags> subconfigurationsFlags;

    static IncidenceGraph *Create(const SimplexList &simplexList, const Params &params);
    static IncidenceGraph *CreateWithBorderVerts(const SimplexList &simplexList, const VertsSet &borderVerts, const Params &params);
    static IncidenceGraph *CreateAndCalculateAcyclicSubset(const SimplexList &simplexList, const Params &params, AcyclicTest<IntersectionFlags> *test);
    static IncidenceGraph *CreateAndCalculateAcyclicSubsetOnline(const SimplexList &simplexList, const Params &params, AcyclicTest<IntersectionFlags> *test);
    static IncidenceGraph *CreateAndCalculateAcyclicSubsetWithSpanningTree(const SimplexList &simplexList, const Params &params, AcyclicTest<IntersectionFlags> *test);
    static IncidenceGraph *CreateAndCalculateAcyclicSubsetParallel(const SimplexList &simplexList, const Params &params, const ParallelParams &parallelParams, AcyclicTest<IntersectionFlags> *test);

    ~IncidenceGraph();

    void CalculateAcyclicSubset(AcyclicTest<IntersectionFlags> *test);
    void CalculateAcyclicSubsetWithSpanningTree(AcyclicTest<IntersectionFlags> *test);
    
    void GetAcyclicSubset(SimplexList &simplexList);
    int GetAcyclicSubsetSize();
    
    void CalculateNodesIntersection(Node *a, Node *b, Edge &edgeAtoB);
    
    Params params;
    Nodes nodes;
    ConnectedComponents connectedComponents;
    VertsSet borderVerts;
    std::vector<VertsSet> connectedComponentsBorders;
    std::vector<int> connectedComponentsAcyclicSubsetSize;

private:

    void CreateGraph(bool minimizeSimplices);
    
    void CreateGraphAndCalculateAcyclicSubset(AcyclicTest<IntersectionFlags> *test);
    void AddNeighboursToListAndUpdateAcyclicIntersection(Node *node, IntNodesMap &H, std::queue<Node *> &L);
    void AddNodeToGraphAndNeighboursToList(Node *node, IntNodesMap &H, std::queue<Node *> &L);
    void RemoveNodeFromGraph(Node *node);

    void CreateAcyclicSpanningTree(std::vector<Path> &paths, int maxAcyclicSubsetID);

    void RemoveAcyclicSubset();
    
};

////////////////////////////////////////////////////////////////////////////////

#endif // _INCIDENCE_GRAPH_H_
