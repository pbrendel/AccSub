/* 
 * File:   ComputationsLocal.h
 * Author: Piotr Brendel
 */

#ifndef COMPUTATIONSLOCAL_H
#define	COMPUTATIONSLOCAL_H

#include "ParallelGraph.h"

class ComputationsLocal
{

public:

    static void Compute(ParallelGraph::DataNodes &nodes, AccSubAlgorithm accSubAlgorithm, AcyclicTest<IncidenceGraph::IntersectionFlags> *test);
    static void CreateIncidenceGraph(ParallelGraph::DataNode *node, AccSubAlgorithm accSubAlgorithm, AcyclicTest<IncidenceGraph::IntersectionFlags> *test);
    
};

#endif	/* COMPUTATIONSLOCAL_H */

