/* 
 * File:   ComputationsLocalMPITest.h
 * Author: Piotr Brendel
 */

#ifndef COMPUTATIONSLOCALMPITEST_H
#define	COMPUTATIONSLOCALMPITEST_H

#include "ParallelGraph.h"

class ComputationsLocalMPITest
{

public:

    static void Compute(ParallelGraph::DataNodes &nodes, AccSubAlgorithm accSubAlgorithm, AcyclicTest<IncidenceGraph::IntersectionFlags> *test);

};

#endif	/* COMPUTATIONSLOCALMPITEST_H */

