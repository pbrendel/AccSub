/* 
 * File:   ComputationsParallelOMP.h
 * Author: Piotr Brendel
 */

#ifndef COMPUTATIONSPARALLELOMP_H
#define	COMPUTATIONSPARALLELOMP_H

#include "ParallelGraph.h"

class ComputationsParallelOMP
{

public:

    static void Compute(ParallelGraph::DataNodes &nodes, const IncidenceGraph::ParallelParams &parallelParams, AcyclicTest<IncidenceGraph::IntersectionFlags> *test);

};

#endif	/* COMPUTATIONSPARALLELOMP_H */

