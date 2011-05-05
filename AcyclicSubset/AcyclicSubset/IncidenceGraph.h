#ifndef INCIDENCEGRAPH_H
#define INCIDENCEGRAPH_H

#include "Simplex.h"
#include "AcyclicTest.hpp"
#include "IntersectionFlagsBitSet.hpp"
#include "MPIData.h"

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
        int acyclicTestNumber;
        bool minimizeSimplices;
        bool sortNodes;

    public:

        Params(int dim = 3, int acyclicTestNumber = 0, bool minimizeSimplices = false, bool sortNodes = false)
        {
            this->dim = dim;
            this->acyclicTestNumber = acyclicTestNumber;
            this->minimizeSimplices = minimizeSimplices;
            this->sortNodes = sortNodes;
        }
    };

    struct ParallelParams
    {
        int packSize;
        int packsCount;
        bool prepareData;
        
    public:
        
        ParallelParams(int packSize, int packsCount = -1, bool prepareData = false)
        {
            this->packSize = packSize;
            this->packsCount = packsCount;
            this->prepareData = prepareData;
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
        
        // potrzebne do rownoleglych obliczen
        int index;
        int newIndex;

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
        
        Node(IncidenceGraph *graph, Simplex *simplex, int index);

        void AddNeighbour(Node *neighbour);
        bool HasNeighbour(Node *neighbour);
        void RemoveNeighbour(Node *neighbour);
        void SetIntersection(Node *neighbour, const Simplex &intersection);
        bool HasAcyclicIntersection(AcyclicTest<IntersectionFlags> *test);
        Vertex FindAcyclicVertex();
        Vertex FindAcyclicVertexNotEqual(Vertex vertex);
        Vertex FindAcyclicVertexNotIn(const VertsSet &vertsSet);
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

public:

    IncidenceGraph(const Params &params);
    IncidenceGraph(SimplexList &simplexList, const Params &params);
    IncidenceGraph(SimplexPtrList &simplexPtrList, const Params &params);

public:

    typedef std::vector<Node *> Nodes;
    typedef std::list<Node *> Path;
    typedef Node *ConnectedComponent;
    typedef std::vector<ConnectedComponent> ConnectedComponents;
    typedef std::map<int, Nodes> IntNodesMap;

    std::map<Simplex, IntersectionFlags> configurationsFlags;
    std::map<Simplex, IntersectionFlags> subconfigurationsFlags;

    static IncidenceGraph *Create(SimplexList &simplexList, const Params &params);
    static IncidenceGraph *CreateWithBorderVerts(SimplexList &simplexList, const VertsSet &borderVerts, const Params &params);
    static IncidenceGraph *CreateWithBorderVerts(SimplexPtrList &simplexPtrList, const VertsSet &borderVerts, const Params &params);
    static IncidenceGraph *CreateAndCalculateAcyclicSubset(SimplexList &simplexList, const Params &params, AcyclicTest<IntersectionFlags> *test);
    static IncidenceGraph *CreateAndCalculateAcyclicSubsetOnline(SimplexList &simplexList, const Params &params, AcyclicTest<IntersectionFlags> *test);
    static IncidenceGraph *CreateAndCalculateAcyclicSubsetWithSpanningTree(SimplexList &simplexList, const Params &params, AcyclicTest<IntersectionFlags> *test);
    static IncidenceGraph *CreateAndCalculateAcyclicSubsetParallel(SimplexList &simplexList, const Params &params, const ParallelParams &parallelParams, AcyclicTest<IntersectionFlags> *test, bool local);

    ~IncidenceGraph();

    void CalculateAcyclicSubset(AcyclicTest<IntersectionFlags> *test);
    void CalculateAcyclicSubsetWithSpanningTree(AcyclicTest<IntersectionFlags> *test);

    void RemoveAcyclicSubset();
    
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

    void AssignNewIndices();

    friend class MPIData::IncidenceGraphData;
    
};

////////////////////////////////////////////////////////////////////////////////

#endif /* INCIDENCEGRAPH_H */
