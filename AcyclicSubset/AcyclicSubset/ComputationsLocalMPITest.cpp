/* 
 * File:   ComputationsLocalMPITest.cpp
 * Author: Piotr Brendel
 */

#include "ComputationsLocalMPITest.h"
#include "ComputationsLocal.h"
#include "MPIData.h"
#include "../Helpers/Utils.h"

////////////////////////////////////////////////////////////////////////////////

void ComputationsLocalMPITest::Compute(ParallelGraph::DataNodes &nodes, const IncidenceGraph::ParallelParams &parallelParams, AcyclicTest<IncidenceGraph::IntersectionFlags> *test)
{
    Timer::TimeStamp("***** ComputationsLocalMPITest start");
    Timer::Time start = Timer::Now();
    for (ParallelGraph::DataNodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        ParallelGraph::DataNode *node = *i;

        MPIData::SimplexData *simplexData = new MPIData::SimplexData(node->simplexPtrList, node->borderVerts, parallelParams.useAcyclicSubsetOnlineAlgorithm ? ASA_Online : ASA_SpanningTree, node->GetConstantSimplexSize());
        SimplexList simplexList;
        std::set<Vertex> borderVerts;
        int acyclicTestNumber;
        int useAcyclicSubsetOnlineAlgorithm;
        simplexData->GetSimplexData(simplexList, borderVerts, acyclicTestNumber, useAcyclicSubsetOnlineAlgorithm);
        AcyclicTest<IncidenceGraph::IntersectionFlags> *test = AcyclicTest<IncidenceGraph::IntersectionFlags>::Create(acyclicTestNumber, GetDimension(simplexList));

        IncidenceGraph *ig = 0;
        if (useAcyclicSubsetOnlineAlgorithm)
        {
           std::cout<<"using online algorihm"<<std::endl;
           ig = IncidenceGraph::CreateAndCalculateAcyclicSubsetOnlineWithBorder(simplexList, borderVerts, test);
           ig->UpdateConnectedComponents();
           ig->RemoveAcyclicSubset();
           ig->AssignNewIndices(false);
           Timer::Update("acyclic subset removed");
        }
        else
        {
            std::cout<<"using spanning tree algorihm"<<std::endl;
            ig = IncidenceGraph::CreateAndCalculateAcyclicSubsetSpanningTreeWithBorder(simplexList, borderVerts, test);
            ig->UpdateConnectedComponents();
            ig->RemoveAcyclicSubset();
            ig->AssignNewIndices(false);
            Timer::Update("acyclic subset removed");
        }

        MPIData::IncidenceGraphData *igData = new MPIData::IncidenceGraphData(ig);
        node->ig = igData->GetIncidenceGraph(node->simplexPtrList);

        delete igData;
        delete simplexData;

        Timer::TimeStamp("***** incidence graph calculated");
    }
    Timer::TimeStamp("***** ComputationsLocalMPITest end");
    Timer::TimeFrom(start, "total parallel computations");
}

////////////////////////////////////////////////////////////////////////////////
// eof
