/*
 * File:   ComputationsParallelOMP.hpp
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 */

#ifndef COMPUTATIONSPARALLELOMP_HPP
#define	COMPUTATIONSPARALLELOMP_HPP

#include "ComputationsLocal.hpp"

#ifdef ACCSUB_TRACE
#include "Utils.hpp"
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
        Timer::TimeStamp("ComputationsParallelOMP start");
        Timer::Time start = Timer::Now();
        if (accSubAlgorithm == AccSubAlgorithm::AccSubIG)
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
            Timer::TimeStamp("incidence graph calculated");
#endif
        }
#ifdef ACCSUB_TRACE
        Timer::TimeStamp("ComputationsParallelOMP end");
        Timer::TimeFrom(start, "total distributed computations");
#endif
    }
};

#endif	/* COMPUTATIONSPARALLELOMP_HPP */

