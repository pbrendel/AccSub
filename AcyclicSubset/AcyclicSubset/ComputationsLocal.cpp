/* 
 * File:   ComputationsLocal.cpp
 * Author: Piotr Brendel
 */

#include "ComputationsLocal.h"
#include "../Helpers/Utils.h"

////////////////////////////////////////////////////////////////////////////////

void ComputationsLocal::Compute(ParallelGraph::DataNodes &nodes, const IncidenceGraph::ParallelParams &parallelParams, AcyclicTest<IncidenceGraph::IntersectionFlags> *test)
{
    Timer::TimeStamp("***** ComputationsLocal start");
    Timer::Time start = Timer::Now();
    for (ParallelGraph::DataNodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        CreateIncidenceGraph(*i, parallelParams, test);
        Timer::TimeStamp("***** incidence graph calculated");
    }
    Timer::TimeStamp("***** ComputationsLocal end");
    Timer::TimeFrom(start, "total parallel computations");
}

void ComputationsLocal::CreateIncidenceGraph(ParallelGraph::DataNode *node, const IncidenceGraph::ParallelParams &parallelParams, AcyclicTest<IncidenceGraph::IntersectionFlags> *test)
{
    if (parallelParams.useAcyclicSubsetOnlineAlgorithm)
    {
        std::cout<<"using online algorihm"<<std::endl;
        node->ig = IncidenceGraph::CreateAndCalculateAcyclicSubsetOnlineWithBorder(node->simplexPtrList, node->borderVerts, test);
        node->ig->UpdateConnectedComponents();
        node->ig->RemoveAcyclicSubset();
        node->ig->AssignNewIndices(false);
        Timer::Update("acyclic subset removed");
    }
    else
    {
        std::cout<<"using spanning tree algorihm"<<std::endl;
        node->ig = IncidenceGraph::CreateAndCalculateAcyclicSubsetSpanningTreeWithBorder(node->simplexPtrList, node->borderVerts, test);
        node->ig->UpdateConnectedComponents();
        node->ig->RemoveAcyclicSubset();
        node->ig->AssignNewIndices(false);
        Timer::Update("acyclic subset removed");
    }
}

////////////////////////////////////////////////////////////////////////////////
// eof
