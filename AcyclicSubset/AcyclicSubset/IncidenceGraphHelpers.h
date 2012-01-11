/* 
 * File:   IncidenceGraphHelpers.h
 * Author: Piotr Brendel
 */

#ifndef INCIDENCEGRAPHHELPERS_H
#define	INCIDENCEGRAPHHELPERS_H

//#include "PartitionGraph.h"

#ifdef ACCSUB_TRACE
#include "../Helpers/Utils.h"
#endif

template <typename IncidenceGraph>
class IncidenceGraphHelpers
{
    typedef typename IncidenceGraph::Vertex Vertex;
    typedef typename IncidenceGraph::VertsSet VertsSet;
    typedef typename IncidenceGraph::Simplex Simplex;
    typedef typename IncidenceGraph::SimplexList SimplexList;
    typedef typename IncidenceGraph::SimplexPtrList SimplexPtrList;
    typedef typename IncidenceGraph::AccTest AccTest;
    typedef typename IncidenceGraph::AccSubAlgorithm AccSubAlgorithm;

public:
    
    static IncidenceGraph *Create(SimplexList &simplexList)
    {
#ifdef ACCSUB_TRACE
        Timer::Update();
        IncidenceGraph *ig = new IncidenceGraph(simplexList);
        ig->CreateGraph();
        Timer::Update("incidence graph created");
        MemoryInfo::Print();
#else
        IncidenceGraph *ig = new IncidenceGraph(simplexList);
        ig->CreateGraph();
#endif
        return ig;
    }

    static IncidenceGraph *CreateWithBorder(SimplexList &simplexList, const VertsSet &borderVerts)
    {
#ifdef ACCSUB_TRACE
        Timer::Update();
        IncidenceGraph *ig = new IncidenceGraph(simplexList);
        ig->borderVerts = borderVerts;
        ig->CreateGraphWithBorder();
        Timer::Update("incidence graph created");
        MemoryInfo::Print();
#else
        IncidenceGraph *ig = new IncidenceGraph(simplexList);
        ig->borderVerts = borderVerts;
        ig->CreateGraphWithBorder();
#endif
        return ig;
    }
    
    static IncidenceGraph *CreateWithBorder(SimplexPtrList &simplexPtrList, const VertsSet &borderVerts)
    {
#ifdef ACCSUB_TRACE
        Timer::Update();
        IncidenceGraph *ig = new IncidenceGraph(simplexPtrList);
        ig->borderVerts = borderVerts;
        ig->CreateGraphWithBorder();
        Timer::Update("incidence graph created");
        MemoryInfo::Print();
#else
        IncidenceGraph *ig = new IncidenceGraph(simplexPtrList);
        ig->borderVerts = borderVerts;
        ig->CreateGraphWithBorder();
#endif
        return ig;
    }

    static IncidenceGraph *CreateAndCalculateAccSub(SimplexList &simplexList, AccTest *accTest)
    {
#ifdef ACCSUB_TRACE
        Timer::Update();
        IncidenceGraph *ig = new IncidenceGraph(simplexList);
        ig->CreateGraph();
        Timer::Update("incidence graph created");
        MemoryInfo::Print();
        ig->CalculateAccSub(accTest);
        Timer::Update("acyclic subset calculated");
        MemoryInfo::Print();
#else
        IncidenceGraph *ig = new IncidenceGraph(simplexList);
        ig->CreateGraph();
        ig->CalculateAccSub(accTest);
#endif
        return ig;
    }

    static IncidenceGraph *CreateAndCalculateAccSubWithBorder(SimplexList &simplexList, const VertsSet &borderVerts, AccTest *accTest)
    {
#ifdef ACCSUB_TRACE
        Timer::Update();
        IncidenceGraph *ig = new IncidenceGraph(simplexList);
        ig->borderVerts = borderVerts;
        ig->CreateGraphWithBorder();
        Timer::Update("incidence graph created");
        MemoryInfo::Print();
        ig->CalculateAccSubWithBorder(accTest);
        Timer::Update("acyclic subset calculated");
        MemoryInfo::Print();
#else
        IncidenceGraph *ig = new IncidenceGraph(simplexList);
        ig->borderVerts = borderVerts;
        ig->CreateGraphWithBorder();
        ig->CalculateAccSubWithBorder(accTest);
#endif
        return ig;
    }

    static IncidenceGraph *CreateAndCalculateAccSubIG(SimplexList &simplexList, AccTest *accTest)
    {
#ifdef ACCSUB_TRACE
        Timer::Update();
        IncidenceGraph *ig = new IncidenceGraph(simplexList);
        ig->CreateGraphAndCalculateAccSub(accTest);
        Timer::Update("incidence graph created and acyclic subset calculated");
        MemoryInfo::Print();
#else
        IncidenceGraph *ig = new IncidenceGraph(simplexList);
        ig->CreateGraphAndCalculateAccSub(accTest);
#endif
        return ig;
    }
    
    static IncidenceGraph *CreateAndCalculateAccSubIGWithBorder(SimplexList &simplexList, const VertsSet &borderVerts, AccTest *accTest)
    {
#ifdef ACCSUB_TRACE
        Timer::Update();
        IncidenceGraph *ig = new IncidenceGraph(simplexList);
        ig->borderVerts = borderVerts;
        ig->CreateGraphAndCalculateAccSubWithBorder(accTest);
        Timer::Update("incidence graph created and acyclic subset calculated");
        MemoryInfo::Print();
#else
        IncidenceGraph *ig = new IncidenceGraph(simplexList);
        ig->borderVerts = borderVerts;
        ig->CreateGraphAndCalculateAccSubWithBorder(accTest);
#endif
        return ig;
    }

    static IncidenceGraph *CreateAndCalculateAccSubIGWithBorder(SimplexPtrList &simplexPtrList, const VertsSet &borderVerts, AccTest *accTest)
    {
#ifdef ACCSUB_TRACE
        Timer::Update();
        IncidenceGraph *ig = new IncidenceGraph(simplexPtrList);
        ig->borderVerts = borderVerts;
        ig->CreateGraphAndCalculateAccSubWithBorder(accTest);
        Timer::Update("incidence graph created and acyclic subset calculated");
        MemoryInfo::Print();
#else
        IncidenceGraph *ig = new IncidenceGraph(simplexPtrList);
        ig->borderVerts = borderVerts;
        ig->CreateGraphAndCalculateAccSubWithBorder(accTest);
#endif
        return ig;
    }
    
    static IncidenceGraph *CreateAndCalculateAccSubST(SimplexList &simplexList, AccTest *accTest)
    {
#ifdef ACCSUB_TRACE
        Timer::Update();
        IncidenceGraph *ig = new IncidenceGraph(simplexList);
        ig->CreateGraph();
        Timer::Update("incidence graph created");
        MemoryInfo::Print();
        ig->CalculateAccSubST(accTest);
        Timer::Update("acyclic subset calculated");
        MemoryInfo::Print();
#else
        IncidenceGraph *ig = new IncidenceGraph(simplexList);
        ig->CreateGraph();
        ig->CalculateAccSubSpanningTree(accTest);
#endif
        return ig;
    }

    static IncidenceGraph *CreateAndCalculateAccSubST(SimplexPtrList &simplexPtrList, AccTest *accTest)
    {
#ifdef ACCSUB_TRACE
        Timer::Update();
        IncidenceGraph *ig = new IncidenceGraph(simplexPtrList);
        ig->CreateGraph();
        Timer::Update("incidence graph created");
        MemoryInfo::Print();
        ig->CalculateAccSubST(accTest);
        Timer::Update("acyclic subset calculated");
        MemoryInfo::Print();
#else
        IncidenceGraph *ig = new IncidenceGraph(simplexPtrList);
        ig->CreateGraph();
        ig->CalculateAccSubSpanningTree(accTest);
#endif
        return ig;
    }
    
    static IncidenceGraph *CreateAndCalculateAccSubSTWithBorder(SimplexList &simplexList, const VertsSet &borderVerts, AccTest *accTest)
    {
#ifdef ACCSUB_TRACE
        Timer::Update();
        IncidenceGraph *ig = new IncidenceGraph(simplexList);
        ig->borderVerts = borderVerts;
        ig->CreateGraphWithBorder();
        Timer::Update("incidence graph created");
        MemoryInfo::Print();
        ig->CalculateAccSubSTWithBorder(accTest);
        Timer::Update("acyclic subset calculated");
        MemoryInfo::Print();
#else
        IncidenceGraph *ig = new IncidenceGraph(simplexList);
        ig->borderVerts = borderVerts;
        ig->CreateGraphWithBorder();
        ig->CalculateAccSubSpanningTreeWithBorder(accTest);
#endif
        return ig;
    }
    
    static IncidenceGraph *CreateAndCalculateAccSubSTWithBorder(SimplexPtrList &simplexPtrList, const VertsSet &borderVerts, AccTest *accTest)
    {
#ifdef ACCSUB_TRACE
        Timer::Update();
        IncidenceGraph *ig = new IncidenceGraph(simplexPtrList);
        ig->borderVerts = borderVerts;
        ig->CreateGraphWithBorder();
        Timer::Update("incidence graph created");
        MemoryInfo::Print();
        ig->CalculateAccSubSTWithBorder(accTest);
        Timer::Update("acyclic subset calculated");
        MemoryInfo::Print();
#else
        IncidenceGraph *ig = new IncidenceGraph(simplexPtrList);
        ig->borderVerts = borderVerts;
        ig->CreateGraphWithBorder();
        ig->CalculateAccSubSpanningTreeWithBorder(accTest);
#endif
        return ig;
    }
/*
    static IncidenceGraph *CreateAndCalculateAccSubParallel(SimplexList &simplexList, int packsCount, AccSubAlgorithm accSubAlgorithm, AccTest *accTest)
    {
        PartitionGraphT<IncidenceGraph> *pg = new PartitionGraphT<IncidenceGraph>(simplexList, packsCount, accSubAlgorithm, accTest);
        IncidenceGraph *ig = pg->GetIncidenceGraph();
        delete pg;
        return ig;
    }*/
};

#endif	/* INCIDENCEGRAPHHELPERS_H */

