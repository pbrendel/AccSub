/* 
 * File:   ComputationsLocalMPITest.cpp
 * Author: Piotr Brendel
 */

#include "ComputationsLocalMPITest.h"
#include "ComputationsLocal.h"
#include "MPIData.h"
#include "IncidenceGraphHelpers.h"

#ifdef USE_HELPERS
#include "../Helpers/Utils.h"
#endif

////////////////////////////////////////////////////////////////////////////////

void ComputationsLocalMPITest::Compute(PartitionGraph::Nodes &nodes, AccSubAlgorithm accSubAlgorithm, AcyclicTest<IncidenceGraph::IntersectionFlags> *test)
{
#ifdef USE_HELPERS
    Timer::TimeStamp("***** ComputationsLocalMPITest start");
    Timer::Time start = Timer::Now();
    if (accSubAlgorithm == ASA_AccIG)
    {    
        std::cout<<"using AccIG"<<std::endl;
    }
    else
    {
        std::cout<<"using AccST"<<std::endl;            
    }
#endif
    for (PartitionGraph::Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        PartitionGraph::Node *node = *i;

        MPIData::SimplexData *simplexData = new MPIData::SimplexData(node->simplexPtrList, node->borderVerts, accSubAlgorithm, test->GetID(), GetConstantSimplexSize(node->simplexPtrList));
        SimplexList simplexList;
        std::set<Vertex> borderVerts;
        int acyclicTestNumber;
        int accSubAlg;
        simplexData->GetSimplexData(simplexList, borderVerts, accSubAlg, acyclicTestNumber);
        AcyclicTest<IncidenceGraph::IntersectionFlags> *test = AcyclicTest<IncidenceGraph::IntersectionFlags>::Create(acyclicTestNumber, GetDimension(simplexList));

        IncidenceGraph *ig = 0;
        if (accSubAlg == ASA_AccIG)
        {
           ig = IncidenceGraphHelpers::CreateAndCalculateAcyclicSubsetOnlineWithBorder(simplexList, borderVerts, test);
           ig->UpdateConnectedComponents();
           ig->RemoveAcyclicSubset();
           ig->AssignNewIndices(false);
        }
        else
        {
            ig = IncidenceGraphHelpers::CreateAndCalculateAcyclicSubsetSpanningTreeWithBorder(simplexList, borderVerts, test);
            ig->UpdateConnectedComponents();
            ig->RemoveAcyclicSubset();
            ig->AssignNewIndices(false);
        }

        MPIData::IncidenceGraphData *igData = new MPIData::IncidenceGraphData(ig);
        node->ig = igData->GetIncidenceGraph(node->simplexPtrList);

        delete igData;
        delete simplexData;

#ifdef USE_HELPERS
        Timer::TimeStamp("***** incidence graph calculated");
#endif        
    }
#ifdef USE_HELPERS
    Timer::TimeStamp("***** ComputationsLocalMPITest end");
    Timer::TimeFrom(start, "total parallel computations");
#endif
}

////////////////////////////////////////////////////////////////////////////////
// eof
