/* 
 * File:   ComputationsParallelOMP.cpp
 * Author: Piotr Brendel
 */

#include "ComputationsParallelOMP.h"
#include "ComputationsLocal.h"
#include "../Helpers/Utils.h"

////////////////////////////////////////////////////////////////////////////////

void ComputationsParallelOMP::Compute(ParallelGraph::DataNodes &nodes, AccSubAlgorithm accSubAlgorithm, AcyclicTest<IncidenceGraph::IntersectionFlags> *acyclicTest)
{
    Timer::TimeStamp("***** ComputationsParallelOMP start");
    Timer::Time start = Timer::Now();
    #pragma omp parallel for
    for (ParallelGraph::DataNodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        ComputationsLocal::CreateIncidenceGraph(*i, accSubAlgorithm, acyclicTest);
        Timer::TimeStamp("***** incidence graph calculated");
    }
    Timer::TimeStamp("***** ComputationsParallelOMP end");
    Timer::TimeFrom(start, "total parallel computations");
}

////////////////////////////////////////////////////////////////////////////////
// eof
