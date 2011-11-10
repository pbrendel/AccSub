/* 
 * File:   ComputationsParallelOMP.h
 * Author: Piotr Brendel
 */

#ifndef COMPUTATIONSPARALLELOMP_H
#define	COMPUTATIONSPARALLELOMP_H

#include "PartitionGraph.h"

class ComputationsParallelOMP
{

public:

    static void Compute(PartitionGraph::Nodes &nodes, AccSubAlgorithm accSubAlgorithm, AcyclicTest<IncidenceGraph::IntersectionFlags> *test);

};

#endif	/* COMPUTATIONSPARALLELOMP_H */

