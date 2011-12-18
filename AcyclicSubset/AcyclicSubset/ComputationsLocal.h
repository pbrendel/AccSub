/* 
 * File:   ComputationsLocal.h
 * Author: Piotr Brendel
 */

#ifndef COMPUTATIONSLOCAL_H
#define	COMPUTATIONSLOCAL_H

#include "PartitionGraph.h"

class ComputationsLocal
{

public:

    static void Compute(PartitionGraph::Nodes &nodes, AccSubAlgorithm accSubAlgorithm, AccTest<IncidenceGraph::IntersectionFlags> *test);
    static void CreateIncidenceGraph(PartitionGraph::Node *node, AccSubAlgorithm accSubAlgorithm, AccTest<IncidenceGraph::IntersectionFlags> *test);
    
};

#endif	/* COMPUTATIONSLOCAL_H */

