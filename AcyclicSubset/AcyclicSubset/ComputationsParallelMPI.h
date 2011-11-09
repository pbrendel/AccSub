/* 
 * File:   ComputationsParallelMPI.h
 * Author: Piotr Brendel
 */

#ifndef COMPUTATIONSPARALLELMPI_H
#define	COMPUTATIONSPARALLELMPI_H

#include "ParallelGraph.h"

class ComputationsParallelMPI
{

    static void SendMPISimplexData(ParallelGraph::DataNode *node, const IncidenceGraph::ParallelParams &parallelParams, int processRank);
    static void SetMPIIncidenceGraphData(ParallelGraph::DataNode *node, int *buffer, int size);

public:

    static void Compute(ParallelGraph::DataNodes &nodes, const IncidenceGraph::ParallelParams &parallelParams, AcyclicTest<IncidenceGraph::IntersectionFlags> *test);

    static void Slave(int processRank);

    static void KillSlaves();
    static void CollectDebugMemoryInfo();
};

#endif	/* COMPUTATIONSPARALLELMPI_H */

