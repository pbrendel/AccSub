/* 
 * File:   ComputationsLocalMPITest.hpp
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 */

#ifndef COMPUTATIONSLOCALMPITEST_HPP
#define	COMPUTATIONSLOCALMPITEST_HPP

#include "MPIData.hpp"
#include "IncidenceGraphHelpers.hpp"

#ifdef ACCSUB_TRACE
#include "../Helpers/Utils.hpp"
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
        Timer::TimeStamp("ComputationsLocalMPITest start");
        Timer::Time start = Timer::Now();
        if (accSubAlgorithm == AccSubAlgorithm::AccSubIG)
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

            MPISimplexData<IncidenceGraph> *simplexData = new MPISimplexData<IncidenceGraph>(node->simplexPtrList, node->borderVerts, accSubAlgorithm, accTest->GetID(), Simplex::GetSimplexListConstantSize(node->simplexPtrList));
            SimplexList simplexList;
            std::set<Vertex> borderVerts;
            int accTestNumber;
            int accSubAlg;
            simplexData->GetSimplexData(simplexList, borderVerts, accSubAlg, accTestNumber);
            AccTest *accTest = AccTest::Create(accTestNumber, Simplex::GetSimplexListDimension(simplexList));

            IncidenceGraph *ig = 0;
            if (accSubAlg == AccSubAlgorithm::AccSubIG)
            {
               ig = IncidenceGraphHelpers<IncidenceGraph>::CreateAndCalculateAccSubIGWithBorder(simplexList, borderVerts, accTest);
            }
            else
            {
                ig = IncidenceGraphHelpers<IncidenceGraph>::CreateAndCalculateAccSubSTWithBorder(simplexList, borderVerts, accTest);
            }
            ig->UpdateConnectedComponents();
            ig->RemoveAccSub();
            ig->AssignNewIndices(false);

            MPIIncidenceGraphData<IncidenceGraph> *igData = new MPIIncidenceGraphData<IncidenceGraph>(ig);
            node->ig = igData->GetIncidenceGraph(node->simplexPtrList);

            delete igData;
            delete simplexData;
            delete accTest;

#ifdef ACCSUB_TRACE
            Timer::TimeStamp("incidence graph calculated");
#endif
        }
#ifdef ACCSUB_TRACE
        Timer::TimeStamp("ComputationsLocalMPITest end");
        Timer::TimeFrom(start, "total parallel computations");
#endif
    }
};

#endif	/* COMPUTATIONSLOCALMPITEST_HPP */

