/* 
 * File:   ComputationsParallelOMP.cpp
 * Author: Piotr Brendel
 */

#include "ComputationsParallelOMP.h"
#include "ComputationsLocal.h"

#ifdef ACCSUB_TRACE
#include "../Helpers/Utils.h"
#endif

////////////////////////////////////////////////////////////////////////////////

void ComputationsParallelOMP::Compute(PartitionGraph::Nodes &nodes, AccSubAlgorithm accSubAlgorithm, AcyclicTest<IncidenceGraph::IntersectionFlags> *acyclicTest)
{
#ifdef ACCSUB_TRACE
    Timer::TimeStamp("***** ComputationsParallelOMP start");
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
    #pragma omp parallel for
    for (PartitionGraph::Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        ComputationsLocal::CreateIncidenceGraph(*i, accSubAlgorithm, acyclicTest);
#ifdef ACCSUB_TRACE
        Timer::TimeStamp("***** incidence graph calculated");
#endif    
    }
#ifdef ACCSUB_TRACE
    Timer::TimeStamp("***** ComputationsParallelOMP end");
    Timer::TimeFrom(start, "total parallel computations");
#endif
}

////////////////////////////////////////////////////////////////////////////////
// eof
