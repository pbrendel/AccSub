/* 
 * File:   ComputationsLocalMPITest.h
 * Author: Piotr Brendel
 */

#ifndef COMPUTATIONSLOCALMPITEST_H
#define	COMPUTATIONSLOCALMPITEST_H

#include "PartitionGraph.h"

class ComputationsLocalMPITest
{

public:

    static void Compute(PartitionGraph::Nodes &nodes, AccSubAlgorithm accSubAlgorithm, AccTest<IncidenceGraph::IntersectionFlags> *test);

};

#endif	/* COMPUTATIONSLOCALMPITEST_H */

