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

void ComputationsLocal::Compute(PartitionGraph::Nodes &nodes, AccSubAlgorithm accSubAlgorithm, AccTest<IncidenceGraph::IntersectionFlags> *test)
{
#ifdef ACCSUB_TRACE
    Timer::TimeStamp("***** ComputationsLocal start");
    Timer::Time start = Timer::Now();
    if (accSubAlgorithm == ASA_AccSubIG)
    {    
        std::cout<<"using AccSubIG"<<std::endl;
    }
    else
    {
        std::cout<<"using AccSubST"<<std::endl;
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

void ComputationsLocal::CreateIncidenceGraph(PartitionGraph::Node *node, AccSubAlgorithm accSubAlgorithm, AccTest<IncidenceGraph::IntersectionFlags> *test)
{
    if (accSubAlgorithm == ASA_AccSubIG)
    {
        node->ig = IncidenceGraphHelpers::CreateAndCalculateAccSubIGWithBorder(node->simplexPtrList, node->borderVerts, test);
    }
    else
    {
        node->ig = IncidenceGraphHelpers::CreateAndCalculateAccSubSTWithBorder(node->simplexPtrList, node->borderVerts, test);
    }
    node->ig->UpdateConnectedComponents();
    node->ig->RemoveAccSub();
    node->ig->AssignNewIndices(false);
}

////////////////////////////////////////////////////////////////////////////////
// eof
