/*
 * File:   PartitionGraph.hpp
 * Author: Piotr Brendel
 */

#ifndef PARTITIONGRAPH_HPP
#define PARTITIONGRAPH_HPP

#include "AccSpanningTree.hpp"
#include <cmath> // ceil()

#ifdef ACCSUB_TRACE
#include "../Helpers/Utils.hpp"
#endif

template <typename IncidenceGraphType, template <typename> class PrepareDataPolicy, template <typename> class ComputationsPolicy>
class PartitionGraphT
{
public:
   
    typedef IncidenceGraphType IncidenceGraph;
    typedef typename IncidenceGraph::Vertex Vertex;
    typedef typename IncidenceGraph::Simplex Simplex;
    typedef typename IncidenceGraph::SimplexList SimplexList;
    typedef typename IncidenceGraph::SimplexPtrList SimplexPtrList;
    typedef typename IncidenceGraph::IntersectionFlags IntersectionFlags;
    typedef typename IncidenceGraph::VertexHash VertexHash;
    typedef typename IncidenceGraph::AccTest AccTest;
    typedef typename IncidenceGraph::AccSubAlgorithm AccSubAlgorithm;

    struct Edge;

    struct Node
    {
        SimplexPtrList      simplexPtrList;
        std::set<Vertex>    verts;
        std::set<Vertex>    borderVerts;
        std::vector<Edge *> edges;
        IncidenceGraph      *ig;
        VertexHash          H;
        int                 processRank;

        Node()
        {
            ig = 0;
        }

        Node(SimplexPtrList &simplexPtrList)
        {
            ig = 0;
            this->simplexPtrList = simplexPtrList;
        }
        
        ~Node()
        {
            delete ig;
        }

        void AddEdge(Edge *edge, Simplex &edgeVerts)
        {
            edges.push_back(edge);
            AddToBorder(edgeVerts);
        }

        void AddToBorder(std::vector<Vertex> verts)
        {
            borderVerts.insert(verts.begin(), verts.end());
        }

        void CreateVertexHashForBorderNodes()
        {
            if (H.size() > 0)
            {
                // juz zbudowany
                return;
            }
            for (typename IncidenceGraph::Nodes::iterator node = ig->nodes.begin(); node != ig->nodes.end(); node++)
            {
                if ((*node)->IsOnBorder())
                {
                    for (typename Simplex::iterator v = (*node)->simplex->begin(); v != (*node)->simplex->end(); v++)
                    {
                        H[*v].push_back(*node);
                    }
                }
            }
        }
    };

    struct Edge
    {
        Node *nodeA;
        Node *nodeB;

        Edge(Node *na, Node *nb)
        {
            nodeA = na;
            nodeB = nb;
        }
    };
    
    typedef std::vector<Node *> Nodes;
    typedef std::vector<Edge *> Edges;
    
    PartitionGraphT(SimplexList &simplexList, int packsCount, AccSubAlgorithm accSubAlgorithm, AccTest *accTest)
    {
        this->incidenceGraph = new IncidenceGraph(Simplex::GetSimplexListDimension(simplexList));
        this->initialSize = simplexList.size();
        this->accSubAlgorithm = accSubAlgorithm;
        this->accTest = accTest;
        int packSize = (int)ceil(float(simplexList.size()) / packsCount);
#ifdef ACCSUB_TRACE
        std::cout<<"pack size: "<<packSize<<std::endl;
#endif
        PrepareDataPolicy<PartitionGraphT>::Prepare(simplexList, packSize);
#ifdef ACCSUB_TRACE
        Timer::Update("preparing data");
#endif
        DivideData(simplexList, packSize);
#ifdef ACCSUB_TRACE
        Timer::Update("dividing data");
#endif
        CreateDataEdges();
#ifdef ACCSUB_TRACE
        Timer::Update("creating data connections");
#endif
        CalculateIncidenceGraphs(nodes);
#ifdef ACCSUB_TRACE
        Timer::Update("creating incidence graphs");
#endif
        AccSpanningTreeT<PartitionGraphT> *ast = new AccSpanningTreeT<PartitionGraphT>(this);
        CalculateIncidenceGraphs(isolatedNodes);
#ifdef ACCSUB_TRACE
        Timer::Update("creating isolated incidence graphs");
#endif
        CombineGraphs();
        ast->JoinAccSubsets();
        delete ast;
    }

    ~PartitionGraphT()
    {
        for (typename Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            delete (*i);
        }
        for (typename Edges::iterator i = edges.begin(); i != edges.end(); i++)
        {
            delete (*i);
        }
        for (typename Nodes::iterator i = isolatedNodes.begin(); i != isolatedNodes.end(); i++)
        {
            delete (*i);
        }
    }

    IncidenceGraph *GetIncidenceGraph() { return incidenceGraph; }

    Nodes nodes;
    Edges edges;
    Nodes isolatedNodes;
    
private:

    IncidenceGraph              *incidenceGraph;
    int                         initialSize;
    AccSubAlgorithm             accSubAlgorithm;
    AccTest                     *accTest;
    
    void DivideData(SimplexList &simplexList, int packSize)
    {
        Node *currentNode = new Node();
        int simplicesLeft = packSize;
        typename SimplexList::iterator it = simplexList.begin();
        while (it != simplexList.end())
        {
            currentNode->simplexPtrList.push_back(&(*it));
            for (typename Simplex::const_iterator i = it->begin(); i != it->end(); i++)
            {
                currentNode->verts.insert(*i);
            }
            simplicesLeft--;
            if (simplicesLeft == 0)
            {
                nodes.push_back(currentNode);
                currentNode = new Node();
                simplicesLeft = packSize;
            }
            it++;
        }
        if (currentNode->simplexPtrList.size() > 0)
        {
            nodes.push_back(currentNode);
        }
        else
        {
            delete currentNode;
        }
    }

    void CreateDataEdges()
    {
        for (typename Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            for (typename Nodes::iterator j = i + 1; j != nodes.end(); j++)
            {
                Simplex intersection;
                GetSortedIntersectionOfUnsortedSets(intersection, (*i)->verts, (*j)->verts);
                if (intersection.size() > 0)
                {
                    Edge *edge = new Edge(*i, *j);
                    edges.push_back(edge);
                    (*i)->AddEdge(edge, intersection);
                    (*j)->AddEdge(edge, intersection);
                }
            }
        }
    }

    void CalculateIncidenceGraphs(Nodes &sourceNodes)
    {
        ComputationsPolicy<PartitionGraphT>::Compute(sourceNodes, accSubAlgorithm, accTest);
    }

    void CombineGraphs()
    {
        // laczymy sympleksy
        for (typename Edges::iterator edge = edges.begin(); edge != edges.end(); edge++)
        {
            typename IncidenceGraph::Nodes nodesA = (*edge)->nodeA->ig->nodes;
            (*edge)->nodeB->CreateVertexHashForBorderNodes();
            VertexHash HB = (*edge)->nodeB->H;

            for (typename IncidenceGraph::Nodes::iterator node = nodesA.begin(); node != nodesA.end(); node++)
            {
                if (!(*node)->IsOnBorder())
                {
                    continue;
                }
                for (typename Simplex::iterator v = (*node)->simplex->begin(); v != (*node)->simplex->end(); v++)
                {
                    typename IncidenceGraph::Nodes neighbours = HB[*v];
                    for (typename IncidenceGraph::Nodes::iterator neighbour = neighbours.begin(); neighbour != neighbours.end(); neighbour++)
                    {
                        if (!(*node)->HasNeighbour(*neighbour))
                        {
                            typename IncidenceGraph::Edge *edge = new typename IncidenceGraph::Edge(*node, *neighbour);
                            incidenceGraph->edges.push_back(edge);
                            (*node)->AddEdge(edge);
                            (*neighbour)->AddEdge(edge);
                        }
                    }
                }
            }
        }

#ifdef ACCSUB_TRACE
        Timer::Update("connecting simplices on border");
#endif

        // przenosimy wszystkie sympleksy do jednego grafu
        for (typename Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            incidenceGraph->nodes.insert(incidenceGraph->nodes.end(), (*i)->ig->nodes.begin(), (*i)->ig->nodes.end());
            (*i)->ig->nodes.clear();
            incidenceGraph->edges.insert(incidenceGraph->edges.end(), (*i)->ig->edges.begin(), (*i)->ig->edges.end());
            (*i)->ig->edges.clear();
        }

        // to samo ze spojnymi skladowymi z drugiej fazy obliczen
        for (typename Nodes::iterator i = isolatedNodes.begin(); i != isolatedNodes.end(); i++)
        {
            incidenceGraph->nodes.insert(incidenceGraph->nodes.end(), (*i)->ig->nodes.begin(), (*i)->ig->nodes.end());
            (*i)->ig->nodes.clear();
            incidenceGraph->edges.insert(incidenceGraph->edges.end(), (*i)->ig->edges.begin(), (*i)->ig->edges.end());
            (*i)->ig->edges.clear();
        }

        // przypisywanie nowego grafu do nodow
        for (typename IncidenceGraph::Nodes::iterator i = incidenceGraph->nodes.begin(); i != incidenceGraph->nodes.end(); i++)
        {
            (*i)->SetParentGraph(incidenceGraph);
        }

#ifdef ACCSUB_TRACE
        std::cout<<"total simplices after connecting graphs: "<<incidenceGraph->nodes.size()<<std::endl;
        std::cout<<"reduced acyclic subset size: "<<(initialSize - incidenceGraph->nodes.size())<<" ("<<((initialSize - incidenceGraph->nodes.size()) * 100 / initialSize)<<"%)"<<std::endl;
        Timer::Update("moving simplices to single incidence graph");
#endif
    }
};

#endif /* PARTITIONGRAPH_HPP */
