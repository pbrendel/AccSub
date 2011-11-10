/*
 * File:   PartitionGraph.cpp
 * Author: Piotr Brendel
 */

#include "PartitionGraph.h"
#include "IncidenceGraph.h"
#include "IncidenceGraphAlgorithms.h"
#include "AccSpanningTree.h"
#include "ComputationsLocal.h"
#include "ComputationsLocalMPITest.h"
#include "PrepareDataBFS.h"

#include <cmath> // ceil()

#ifdef USE_HELPERS
#include "../Helpers/Utils.h"
#endif

////////////////////////////////////////////////////////////////////////////////

void PartitionGraph::Node::CreateIntNodesMapWithBorderNodes()
{
    if (H.size() > 0)
    {
        // juz zbudowany
        return;
    }
    for (IncidenceGraph::Nodes::iterator node = ig->nodes.begin(); node != ig->nodes.end(); node++)
    {
        if ((*node)->IsOnBorder())
        {
            for (Simplex::iterator v = (*node)->simplex->begin(); v != (*node)->simplex->end(); v++)
            {
                H[*v].push_back(*node);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

PartitionGraph::PartitionGraph(SimplexList &simplexList, int packsCount, AccSubAlgorithm accSubAlgorithm, AcyclicTest<IncidenceGraph::IntersectionFlags> *acyclicTest)
{
    this->incidenceGraph = new IncidenceGraph(GetDimension(simplexList));
    this->initialSize = simplexList.size();
    this->accSubAlgorithm = accSubAlgorithm;
    this->acyclicTest = acyclicTest;
    int packSize = (int)ceil(float(simplexList.size()) / packsCount);
#ifdef USE_HELPERS
    std::cout<<"pack size: "<<packSize<<std::endl;
#endif
    PrepareDataBFS::Prepare(simplexList, packSize);
#ifdef USE_HELPERS
    Timer::Update("preparing data");
#endif
    DivideData(simplexList, packSize);
#ifdef USE_HELPERS
    Timer::Update("dividing data");
#endif
    CreateDataEdges();
#ifdef USE_HELPERS
    Timer::Update("creating data connections");
#endif
    CalculateIncidenceGraphs(nodes);
#ifdef USE_HELPERS
    Timer::Update("creating incidence graphs");
#endif
    AccSpanningTree *ast = new AccSpanningTree(this);
    CalculateIncidenceGraphs(isolatedNodes);
#ifdef USE_HELPERS
    Timer::Update("creating isolated incidence graphs");
#endif
    CombineGraphs();
    ast->JoinAcyclicSubsets();
    delete ast;
}

////////////////////////////////////////////////////////////////////////////////

PartitionGraph::~PartitionGraph()
{
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        delete (*i);
    }
    for (Edges::iterator i = edges.begin(); i != edges.end(); i++)
    {
        delete (*i);
    }
    for (Nodes::iterator i = isolatedNodes.begin(); i != isolatedNodes.end(); i++)
    {
        delete (*i);
    }
}

////////////////////////////////////////////////////////////////////////////////

void PartitionGraph::DivideData(SimplexList& simplexList, int packSize)
{
    Node *currentNode = new Node();
    int simplicesLeft = packSize;
    SimplexList::iterator it = simplexList.begin();

    while (it != simplexList.end())
    {
        // tempSimplexList.push_back(*it);

        currentNode->simplexPtrList.push_back(&(*it));
        for (Simplex::const_iterator i = it->begin(); i != it->end(); i++)
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

        // test!!!
    //    if (tempSimplexList.size() > 27500) break;
    }
    if (currentNode->simplexPtrList.size() > 0)
    {
        nodes.push_back(currentNode);
    }
    else
    {
        delete currentNode;
    }

     // std::cout<<"temp simplex list ("<<tempSimplexList.size()<<") homology:"<<std::endl;
     // Tests::Test(tempSimplexList, RT_Coreduction);
}

////////////////////////////////////////////////////////////////////////////////

void PartitionGraph::CreateDataEdges()
{
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        for (Nodes::iterator j = i + 1; j != nodes.end(); j++)
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

//    for (DataNodes::iterator i = dataNodes.begin(); i != dataNodes.end(); i++)
//    {
//        DataNode *node = *i;
//        std::cout<<"border verts: "<<node->borderVerts.size()<<" verts size: "<<node->verts.size()<<std::endl;
//    }
}

////////////////////////////////////////////////////////////////////////////////

void PartitionGraph::CalculateIncidenceGraphs(Nodes &sourceNodes)
{
    ComputationsLocalMPITest::Compute(sourceNodes, accSubAlgorithm, acyclicTest);
}

////////////////////////////////////////////////////////////////////////////////

void PartitionGraph::CombineGraphs()
{
    // laczymy sympleksy
    for (Edges::iterator edge = edges.begin(); edge != edges.end(); edge++)
    {
        IncidenceGraph::Nodes nodesA = (*edge)->nodeA->ig->nodes;
        (*edge)->nodeB->CreateIntNodesMapWithBorderNodes();
        IncidenceGraph::VertexNodesMap HB = (*edge)->nodeB->H;

        for (IncidenceGraph::Nodes::iterator node = nodesA.begin(); node != nodesA.end(); node++)
        {
            if (!(*node)->IsOnBorder())
            {
                continue;
            }
            for (Simplex::iterator v = (*node)->simplex->begin(); v != (*node)->simplex->end(); v++)
            {
                IncidenceGraph::Nodes neighbours = HB[*v];
                if (neighbours.size() == 0)
                {
                    continue;
                }
                for (IncidenceGraph::Nodes::iterator neighbour = neighbours.begin(); neighbour != neighbours.end(); neighbour++)
                {                    
                    if (!(*node)->HasNeighbour(*neighbour))
                    {
                        (*node)->AddNeighbour(*neighbour);
                        (*neighbour)->AddNeighbour(*node);
                    }
                }
            }
        }
    }

#ifdef USE_HELPERS
    Timer::Update("connecting simplices on border");
#endif
    
    // przenosimy wszystkie sympleksy do jednego grafu
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
         incidenceGraph->nodes.insert(incidenceGraph->nodes.end(), (*i)->ig->nodes.begin(), (*i)->ig->nodes.end());
        (*i)->ig->nodes.clear();
    }

    // to samo ze spojnymi skladowymi z drugiej fazy obliczen
    for (Nodes::iterator i = isolatedNodes.begin(); i != isolatedNodes.end(); i++)
    {
         incidenceGraph->nodes.insert(incidenceGraph->nodes.end(), (*i)->ig->nodes.begin(), (*i)->ig->nodes.end());
        (*i)->ig->nodes.clear();
    }

    // przypisywanie nowego grafu do nodow
    for (IncidenceGraph::Nodes::iterator i = incidenceGraph->nodes.begin(); i != incidenceGraph->nodes.end(); i++)
    {
        (*i)->SetParentGraph(incidenceGraph);
    }

#ifdef USE_HELPERS
    std::cout<<"total simplices after connecting graphs: "<<incidenceGraph->nodes.size()<<std::endl;
    std::cout<<"reduced acyclic subset size: "<<(initialSize - incidenceGraph->nodes.size())<<" ("<<((initialSize - incidenceGraph->nodes.size()) * 100 / initialSize)<<"%)"<<std::endl;
    Timer::Update("moving simplices to single incidence graph");
#endif
    
}

////////////////////////////////////////////////////////////////////////////////
// eof
