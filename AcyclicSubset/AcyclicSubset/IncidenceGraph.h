/*
 * File:   IncidenceGraph.h
 * Author: Piotr Brendel
 */

#ifndef INCIDENCEGRAPH_H
#define INCIDENCEGRAPH_H

#include "Simplex.h"
#include "AcyclicTest.hpp"
#include "IntersectionFlagsBitSet.hpp"

#include <list>
#include <queue>
#include <set>

class IncidenceGraph;

////////////////////////////////////////////////////////////////////////////////

enum AccSubAlgorithm
{
    ASA_Acc = 0,
    ASA_AccIG,
    ASA_AccST,
};

////////////////////////////////////////////////////////////////////////////////

class IncidenceGraph
{
    
public:

    // to na parametry szablonu
    typedef unsigned int IntersectionFlags;
    // typedef IntersectionFlagsBitSet<4> IntersectionFlags;

    static int counter;

    struct Edge;
    typedef std::vector<Edge *> Edges;

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

        void AddEdge(Edge *edge);
        bool HasNeighbour(Node *neighbour);
        void RemoveNeighbour(Node *neighbour);
        IntersectionFlags GetNormalizedIntersectionFlags(const Simplex &intersection);
        bool HasAcyclicIntersection(AcyclicTest<IntersectionFlags> *test);
        Vertex FindAcyclicVertex();
        Vertex FindAcyclicVertexNotEqual(Vertex vertex);
        Vertex FindAcyclicVertexNotIn(const VertsSet &vertsSet);
        void UpdateAcyclicIntersectionWithSimplex(const Simplex &simplex);
        void UpdateAcyclicIntersectionWithVertex(Vertex v);
        void UpdateAcyclicIntersectionWithEdge(Vertex v1, Vertex v2);
        void UpdateNeighboursAcyclicIntersection();
        void UpdateAcyclicIntersectionFlags(IntersectionFlags flags, IntersectionFlags flagsMaximalFaces);

        Simplex Normalize(const Simplex &simplex);
        int NormalizeVertex(Vertex v);
        bool operator==(const Node &node);

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
        void SetParentGraph(IncidenceGraph *g) { graph = g; }
        Flags GetPropertiesFlags() { return propertiesFlags; }

    private:

        IncidenceGraph      *graph;
        Flags               propertiesFlags;
        IntersectionFlags   acyclicIntersectionFlags;
        IntersectionFlags   acyclicIntersectionFlagsMaximalFaces;
        int                 acyclicSubsetID;
        std::map<Vertex, int>  v2i;    // vertex to index map

    };

    struct Edge
    {
        Node                *nodeA;
        Node                *nodeB;
        Simplex             intersection;
        IntersectionFlags   intersectionFlagsA;
        IntersectionFlags   intersectionFlagsB;

        Edge(Node *na, Node *nb)
        {
            IncidenceGraph::counter++;
            this->nodeA = na;
            this->nodeB = nb;
            this->intersectionFlagsA = 0;
            this->intersectionFlagsB = 0;
        }

        Node *GetNeighbour(Node *node)
        {
            return (node == nodeA) ? nodeB : nodeA;
        }

        bool Contains(Node *node)
        {
            return (node == nodeA || node == nodeB);
        }

        bool IntersectionCalculated()
        {
            return (intersectionFlagsA != 0 && intersectionFlagsB != 0);
        }

        void CalculateIntersection()
        {
            if (Simplex::GetIntersection(nodeA->simplex, nodeB->simplex, intersection))
            {
                intersectionFlagsA = nodeA->GetNormalizedIntersectionFlags(intersection);
                intersectionFlagsB = nodeB->GetNormalizedIntersectionFlags(intersection);
            }
        }

        IntersectionFlags GetIntersectionFlags(Node *node)
        {
            return (node == nodeA) ? intersectionFlagsA : intersectionFlagsB;
        }
    };

public:

    IncidenceGraph(int dim);
    IncidenceGraph(SimplexList &simplexList);
    IncidenceGraph(SimplexPtrList &simplexPtrList);

public:

    typedef std::vector<Node *> Nodes;
   //typedef std::list<Edge *> Edges;
    typedef std::list<Node *> Path;
    typedef Node *ConnectedComponent;
    typedef std::vector<ConnectedComponent> ConnectedComponents;
    typedef std::map<Vertex, Nodes> VertexHash;

    std::map<Simplex, IntersectionFlags> configurationsFlags;
    std::map<Simplex, IntersectionFlags> subconfigurationsFlags;

    ~IncidenceGraph();

    void CalculateAcyclicSubset(AcyclicTest<IntersectionFlags> *test);
    void CalculateAcyclicSubsetWithBorder(AcyclicTest<IntersectionFlags> *test);
    void CalculateAcyclicSubsetSpanningTree(AcyclicTest<IntersectionFlags> *test);
    void CalculateAcyclicSubsetSpanningTreeWithBorder(AcyclicTest<IntersectionFlags> *test);

    void UpdateConnectedComponents();
    void RemoveAcyclicSubset();
    void AssignNewIndices(bool checkAcyclicity);
    void RemoveConnectedComponentAndCopySimplexList(ConnectedComponent cc, SimplexPtrList &simplexPtrList);
    
    void GetAcyclicSubset(SimplexList &simplexList);
    int GetAcyclicSubsetSize();
    
    int dim;
    Nodes nodes;
    Edges edges;
    ConnectedComponents connectedComponents;
    VertsSet borderVerts;
    std::vector<VertsSet> connectedComponentsBorders;
    std::vector<int> connectedComponentsAcyclicSubsetSize;

public:

    void CreateVertexHash(VertexHash &H);

    void CreateGraph();
    void CreateGraphWithBorder();
    
    void CreateGraphAndCalculateAcyclicSubset(AcyclicTest<IntersectionFlags> *test);
    void CreateGraphAndCalculateAcyclicSubsetWithBorder(AcyclicTest<IntersectionFlags> *test);
    void EnqNeighboursAndUpdateAcyclicIntersection(Node *node, VertexHash &H, std::queue<Node *> &L);
    void AddToGraphAndEnqNeighbours(Node *node, VertexHash &H, std::queue<Node *> &L);
    void RemoveAcyclicEdges();

    void CreateAcyclicSpanningTree(std::vector<Path> &paths, int maxAcyclicSubsetID);
    
};

////////////////////////////////////////////////////////////////////////////////

#endif /* INCIDENCEGRAPH_H */
