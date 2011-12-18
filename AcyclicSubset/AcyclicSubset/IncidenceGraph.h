/*
 * File:   IncidenceGraph.h
 * Author: Piotr Brendel
 */

#ifndef INCIDENCEGRAPH_H
#define INCIDENCEGRAPH_H

#include "Simplex.h"
#include "ConfigurationsFlags.hpp"
#include "AccTest.hpp"
#include "IntersectionFlagsBitSet.hpp"
#include "AccInfoFlags.hpp"
#include "IntersectionInfoFlags.hpp"

#include <list>
#include <queue>
#include <set>

////////////////////////////////////////////////////////////////////////////////

enum AccSubAlgorithm
{
    ASA_AccSub = 0,
    ASA_AccSubIG,
    ASA_AccSubST,
};

////////////////////////////////////////////////////////////////////////////////

class IncidenceGraph
{
    
public:

    // to na parametry szablonu
    typedef unsigned int IntersectionFlags;
    // typedef IntersectionFlagsBitSet<4> IntersectionFlags;

    struct Edge;
    typedef std::vector<Edge *> Edges;

    struct Node
    {
        typedef unsigned short int PropertiesFlags;

        enum  // properties flags
        {
            IGNPF_IN_ACC_SUB                = 0x0001,
            IGNPF_ADDED_TO_QUEUE            = 0x0002,
            IGNPF_ADDED_TO_GRAPH            = 0x0004,
            IGNPF_ADDED_TO_OUTPUT           = 0x0008,
            IGNPF_ON_BORDER                 = 0x0010,
            IGNPF_HELPER_FLAG_1             = 0x1000,
            IGNPF_HELPER_FLAG_2             = 0x2000,
            IGNPF_HELPER_FLAG_3             = 0x4000,
            IGNPF_HELPER_FLAG_4             = 0x8000,
        };
        
#define GET_SET(FUNC, FLAG) inline bool Is##FUNC() const { return (propertiesFlags & IGNPF_##FLAG) == IGNPF_##FLAG; } \
                            inline void Is##FUNC(bool f) { if (f) propertiesFlags |= IGNPF_##FLAG; else propertiesFlags &= ~(IGNPF_##FLAG); }

        GET_SET(InAccSub, IN_ACC_SUB)
        GET_SET(AddedToQueue, ADDED_TO_QUEUE)
        GET_SET(AddedToGraph, ADDED_TO_GRAPH)
        GET_SET(AddedToOutput, ADDED_TO_OUTPUT)
        GET_SET(OnBorder, ON_BORDER)
        GET_SET(HelperFlag1, HELPER_FLAG_1)
        GET_SET(HelperFlag2, HELPER_FLAG_2)
        GET_SET(HelperFlag3, HELPER_FLAG_3)
        GET_SET(HelperFlag4, HELPER_FLAG_4)

#undef GET_SET

        Node(IncidenceGraph *graph, Simplex *simplex, int index) : accInfo(this)
        {
            this->graph = graph;
            this->simplex = simplex;
            this->index = index;
            this->propertiesFlags = 0;

            for (int i = 0; i < simplex->size(); i++)
            {
                v2i[(*simplex)[i]] = i;
            }
        }

        void AddEdge(Edge *edge)
        {
            // mozemy w ramach debugowania sprawdzic, czy krawedz nie zostala juz dodana
            // w rzeczywistosci taka sytuacja nie moze miec miejsca (sprawdzamy to
            // przed wywolaniem AddNeighbour)
            edges.push_back(edge);
        }

        bool HasNeighbour(Node *neighbour)
        {
            for (Edges::iterator i = edges.begin(); i != edges.end(); i++)
            {
                if ((*i)->GetNeighbour(this) == neighbour) return true;
            }
            return false;
        }

        void RemoveNeighbour(Node *neighbour)
        {
            for (Edges::iterator i = edges.begin(); i != edges.end(); i++)
            {
                if ((*i)->GetNeighbour(this) == neighbour)
                {
                    edges.erase(i);
                    return;
                }
            }
        }

        Simplex Normalize(const Simplex &simplex)
        {
            Simplex s = Simplex::WithSize(simplex.size());
            int index = 0;
            for (Simplex::const_iterator i = simplex.begin(); i != simplex.end(); i++)
            {
                s[index++] = v2i[(*i)];
            }
            return s;
        }

        int NormalizeVertex(Vertex v)
        {
            return v2i[v];    
        }

        IntersectionFlags GetNormalizedIntersectionFlags(const Simplex &intersection)
        {
            return graph->subconfigurationsFlags[Normalize(intersection)];
        }

        void SetParentGraph(IncidenceGraph *g) { graph = g; }
        PropertiesFlags GetPropertiesFlags() { return propertiesFlags; }
        AccInfoFlags<IncidenceGraph> &GetAccInfo() { return accInfo; }

        IntersectionFlags GetConfigurationsFlags(const Simplex &s) { return graph->configurationsFlags[s]; }
        IntersectionFlags GetSubconfigurationsFlags(const Simplex &s) { return graph->subconfigurationsFlags[s]; }

        bool operator==(const Node &node)
        {
            return (this->simplex == node.simplex);
        }

        Simplex             *simplex;
        Edges               edges;
        int                 index;

        union
        {
            void *ptr;
            int  i;
        } helpers;

    private:

        IncidenceGraph          *graph;
        PropertiesFlags         propertiesFlags;
        AccInfoFlags<IncidenceGraph> accInfo;
        std::map<Vertex, int>   v2i;    // vertex to index map

    };

    struct Edge
    {
        Node                *nodeA;
        Node                *nodeB;
        IntersectionInfoFlags<IncidenceGraph> intersection;

        Edge(Node *na, Node *nb) : intersection(this)
        {
            nodeA = na;
            nodeB = nb;
        }

        Node *GetNeighbour(Node *node)
        {
            return (node == nodeA) ? nodeB : nodeA;
        }

        bool Contains(Node *node)
        {
            return (node == nodeA || node == nodeB);
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

    ConfigurationsFlags<Simplex, IntersectionFlags> configurationsFlags;
    ConfigurationsFlags<Simplex, IntersectionFlags> subconfigurationsFlags;

    ~IncidenceGraph();

    void CalculateAccSub(AccTest<IntersectionFlags> *test);
    void CalculateAccSubWithBorder(AccTest<IntersectionFlags> *test);
    void CalculateAccSubSpanningTree(AccTest<IntersectionFlags> *test);
    void CalculateAccSubSpanningTreeWithBorder(AccTest<IntersectionFlags> *test);

    void UpdateConnectedComponents();
    void RemoveAccSub();
    void AssignNewIndices(bool checkAcyclicity);
    void RemoveConnectedComponentAndCopySimplexList(ConnectedComponent cc, SimplexPtrList &simplexPtrList);
    
    void GetAccSub(SimplexList &simplexList);
    int GetAccSubSize();
    
    int                 dim;
    Nodes               nodes;
    Edges               edges;
    ConnectedComponents connectedComponents;
    VertsSet            borderVerts;
    std::vector<VertsSet> connectedComponentsBorders;
    std::vector<int>    connectedComponentsAccSubSize;

public:

    void CreateVertexHash(VertexHash &H);

    void CreateGraph();
    void CreateGraphWithBorder();
    
    void CreateGraphAndCalculateAccSub(AccTest<IntersectionFlags> *test);
    void CreateGraphAndCalculateAccSubWithBorder(AccTest<IntersectionFlags> *test);
    void EnqNeighboursAndUpdateAccIntersection(Node *node, VertexHash &H, std::queue<Node *> &Q);
    void AddToGraphAndEnqNeighbours(Node *node, VertexHash &H, std::queue<Node *> &Q);
    void RemoveEdgesWithAccSub();

    void CreateAccSpanningTree(std::vector<Path> &paths, int maxAccSubID);
    
};

////////////////////////////////////////////////////////////////////////////////

#endif /* INCIDENCEGRAPH_H */
