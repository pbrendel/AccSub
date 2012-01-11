/* 
 * File:   ComputationsParallelOMP.h
 * Author: Piotr Brendel
 */

#ifndef COMPUTATIONSPARALLELOMP_H
#define	COMPUTATIONSPARALLELOMP_H

#include "ComputationsLocal.h"

#ifdef ACCSUB_TRACE
#include "../Helpers/Utils.h"
#endif

template <typename PartitionGraph>
class ComputationsParallelOMP
{

    typedef typename PartitionGraph::Nodes Nodes;
    typedef typename PartitionGraph::AccTest AccTest;
    typedef typename PartitionGraph::AccSubAlgorithm AccSubAlgorithm;

public:

    static void Compute(Nodes &nodes, AccSubAlgorithm accSubAlgorithm, AccTest *accTest)
    {
#ifdef ACCSUB_TRACE
        Timer::TimeStamp("***** ComputationsParallelOMP start");
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
        #pragma omp parallel for
        for (typename Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            ComputationsLocal<PartitionGraph>::CreateIncidenceGraph(*i, accSubAlgorithm, accTest);
#ifdef ACCSUB_TRACE
            Timer::TimeStamp("***** incidence graph calculated");
#endif
        }
#ifdef ACCSUB_TRACE
        Timer::TimeStamp("***** ComputationsParallelOMP end");
        Timer::TimeFrom(start, "total parallel computations");
#endif
    }
};

#endif	/* COMPUTATIONSPARALLELOMP_H */

