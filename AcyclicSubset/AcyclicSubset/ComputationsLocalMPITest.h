/* 
 * File:   ComputationsLocalMPITest.h
 * Author: Piotr Brendel
 */

#ifndef COMPUTATIONSLOCALMPITEST_H
#define	COMPUTATIONSLOCALMPITEST_H

#include "MPIData.h"
#include "IncidenceGraphHelpers.h"

#ifdef ACCSUB_TRACE
#include "../Helpers/Utils.h"
#endif

template <typename PartitionGraph>
class ComputationsLocalMPITest
{
    typedef typename PartitionGraph::IncidenceGraph IncidenceGraph;
    typedef typename PartitionGraph::Vertex Vertex;
    typedef typename PartitionGraph::Simplex Simplex;
    typedef typename PartitionGraph::SimplexList SimplexList;
    typedef typename PartitionGraph::SimplexPtrList SimplexPtrList;
    typedef typename PartitionGraph::Node Node;
    typedef typename PartitionGraph::Nodes Nodes;
    typedef typename PartitionGraph::AccTest AccTest;
    typedef typename PartitionGraph::AccSubAlgorithm AccSubAlgorithm;

public:

    static void Compute(Nodes &nodes, AccSubAlgorithm accSubAlgorithm, AccTest *accTest)
    {
    #ifdef ACCSUB_TRACE
        Timer::TimeStamp("***** ComputationsLocalMPITest start");
        Timer::Time start = Timer::Now();
        if (accSubAlgorithm == ASA_AccSubIG)
        {
            std::cout<<"using AccSubIG"<<std::endl;
        }
        else
        {
            std::cout<<"using AccSubST"<<std::endl;
        }
    #endif
        for (typename Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            Node *node = *i;

            MPIData::SimplexData<IncidenceGraph> *simplexData = new MPIData::SimplexData<IncidenceGraph>(node->simplexPtrList, node->borderVerts, accSubAlgorithm, accTest->GetID(), Simplex::GetSimplexListConstantSize(node->simplexPtrList));
            SimplexList simplexList;
            std::set<Vertex> borderVerts;
            int accTestNumber;
            int accSubAlg;
            simplexData->GetSimplexData(simplexList, borderVerts, accSubAlg, accTestNumber);
            AccTest *test = AccTest::Create(accTestNumber, Simplex::GetSimplexListDimension(simplexList));

            IncidenceGraph *ig = 0;
            if (accSubAlg == AccSubAlgorithm::ASA_AccSubIG)
            {
               ig = IncidenceGraphHelpers<IncidenceGraph>::CreateAndCalculateAccSubIGWithBorder(simplexList, borderVerts, test);
            }
            else
            {
                ig = IncidenceGraphHelpers<IncidenceGraph>::CreateAndCalculateAccSubSTWithBorder(simplexList, borderVerts, test);
            }
            ig->UpdateConnectedComponents();
            ig->RemoveAccSub();
            ig->AssignNewIndices(false);

            MPIData::IncidenceGraphData<IncidenceGraph> *igData = new MPIData::IncidenceGraphData<IncidenceGraph>(ig);
            node->ig = igData->GetIncidenceGraph(node->simplexPtrList);

            delete igData;
            delete simplexData;

    #ifdef ACCSUB_TRACE
            Timer::TimeStamp("***** incidence graph calculated");
    #endif
        }
    #ifdef ACCSUB_TRACE
        Timer::TimeStamp("***** ComputationsLocalMPITest end");
        Timer::TimeFrom(start, "total parallel computations");
    #endif
    }
};

#endif	/* COMPUTATIONSLOCALMPITEST_H */

