/* 
 * File:   ComputationsParallelMPI.h
 * Author: Piotr Brendel
 */

#ifndef COMPUTATIONSPARALLELMPI_H
#define	COMPUTATIONSPARALLELMPI_H

#include "PartitionGraph.h"

class ComputationsParallelMPI
{

    static void SendMPISimplexData(PartitionGraph::Node *node, AccSubAlgorithm accSubAlgorithm, int processRank);
    static void SetMPIIncidenceGraphData(PartitionGraph::Node *node, int *buffer, int size);

public:

    static void Compute(PartitionGraph::Nodes &nodes, AccSubAlgorithm accSubAlgorithm, AcyclicTest<IncidenceGraph::IntersectionFlags> *test);

    static void Slave(int processRank);

    static void KillSlaves();
    static void CollectDebugMemoryInfo();
};

#endif	/* COMPUTATIONSPARALLELMPI_H */

