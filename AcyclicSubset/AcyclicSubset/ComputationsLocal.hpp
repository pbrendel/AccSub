/* 
 * File:   ComputationsLocal.hpp
 * Author: Piotr Brendel
 */

#ifndef COMPUTATIONSLOCAL_HPP
#define	COMPUTATIONSLOCAL_HPP

#include "IncidenceGraphHelpers.hpp"

#ifdef ACCSUB_TRACE
#include "../Helpers/Utils.hpp"
#endif

template <typename PartitionGraph>
class ComputationsLocal
{
    typedef typename PartitionGraph::IncidenceGraph IncidenceGraph;
    typedef typename PartitionGraph::Node Node;
    typedef typename PartitionGraph::Nodes Nodes;
    typedef typename PartitionGraph::AccTest AccTest;
    typedef typename PartitionGraph::AccSubAlgorithm AccSubAlgorithm;

public:

    static void Compute(Nodes &nodes, AccSubAlgorithm accSubAlgorithm, AccTest *accTest)
    {
#ifdef ACCSUB_TRACE
        Timer::TimeStamp("ComputationsLocal start");
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
            CreateIncidenceGraph(*i, accSubAlgorithm, accTest);
#ifdef ACCSUB_TRACE
            Timer::TimeStamp("incidence graph calculated");
#endif    
        }
#ifdef ACCSUB_TRACE
        Timer::TimeStamp("ComputationsLocal end");
        Timer::TimeFrom(start, "total parallel computations");
#endif
    }

    static void CreateIncidenceGraph(Node *node, AccSubAlgorithm accSubAlgorithm, AccTest *accTest)
    {
        if (accSubAlgorithm == AccSubAlgorithm::AccSubIG)
        {
            node->ig = IncidenceGraphHelpers<IncidenceGraph>::CreateAndCalculateAccSubIGWithBorder(node->simplexPtrList, node->borderVerts, accTest);
        }
        else
        {
            node->ig = IncidenceGraphHelpers<IncidenceGraph>::CreateAndCalculateAccSubSTWithBorder(node->simplexPtrList, node->borderVerts, accTest);
        }
        node->ig->UpdateConnectedComponents();
        node->ig->RemoveAccSub();
        node->ig->AssignNewIndices(false);
    }
};

#endif	/* COMPUTATIONSLOCAL_HPP */

