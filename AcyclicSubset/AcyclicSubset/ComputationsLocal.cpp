/* 
 * File:   ComputationsLocal.cpp
 * Author: Piotr Brendel
 */

#include "ComputationsLocal.h"
#include "IncidenceGraphHelpers.h"

#ifdef ACCSUB_TRACE
#include "../Helpers/Utils.h"
#endif

////////////////////////////////////////////////////////////////////////////////

void ComputationsLocal::Compute(PartitionGraph::Nodes &nodes, AccSubAlgorithm accSubAlgorithm, AcyclicTest<IncidenceGraph::IntersectionFlags> *test)
{
#ifdef ACCSUB_TRACE
    Timer::TimeStamp("***** ComputationsLocal start");
    Timer::Time start = Timer::Now();
    if (accSubAlgorithm == ASA_AccIG)
    {    
        std::cout<<"using AccIG"<<std::endl;
    }
    else
    {
        std::cout<<"using AccST"<<std::endl;            
    }
#endif    
    for (PartitionGraph::Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        CreateIncidenceGraph(*i, accSubAlgorithm, test);
#ifdef ACCSUB_TRACE
        Timer::TimeStamp("***** incidence graph calculated");
#endif    
    }
#ifdef ACCSUB_TRACE
    Timer::TimeStamp("***** ComputationsLocal end");
    Timer::TimeFrom(start, "total parallel computations");
#endif
}

void ComputationsLocal::CreateIncidenceGraph(PartitionGraph::Node *node, AccSubAlgorithm accSubAlgorithm, AcyclicTest<IncidenceGraph::IntersectionFlags> *test)
{
    if (accSubAlgorithm == ASA_AccIG)
    {
        node->ig = IncidenceGraphHelpers::CreateAndCalculateAcyclicSubsetOnlineWithBorder(node->simplexPtrList, node->borderVerts, test);
        node->ig->UpdateConnectedComponents();
        node->ig->RemoveAcyclicSubset();
        node->ig->AssignNewIndices(false);
    }
    else
    {
        node->ig = IncidenceGraphHelpers::CreateAndCalculateAcyclicSubsetSpanningTreeWithBorder(node->simplexPtrList, node->borderVerts, test);
        node->ig->UpdateConnectedComponents();
        node->ig->RemoveAcyclicSubset();
        node->ig->AssignNewIndices(false);
    }
}

////////////////////////////////////////////////////////////////////////////////
// eof
