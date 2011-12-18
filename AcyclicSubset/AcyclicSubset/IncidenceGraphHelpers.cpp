/* 
 * File:   IncidenceGraphHelpers.cpp
 * Author: Piotr Brendel
 */

#include "IncidenceGraphHelpers.h"
#include "PartitionGraph.h"

#ifdef ACCSUB_TRACE
#include "../Helpers/Utils.h"
#endif
////////////////////////////////////////////////////////////////////////////////

IncidenceGraph *IncidenceGraphHelpers::Create(SimplexList& simplexList)
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

IncidenceGraph *IncidenceGraphHelpers::CreateWithBorder(SimplexList& simplexList, const VertsSet& borderVerts)
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

IncidenceGraph *IncidenceGraphHelpers::CreateWithBorder(SimplexPtrList& simplexPtrList, const VertsSet& borderVerts)
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

////////////////////////////////////////////////////////////////////////////////

IncidenceGraph *IncidenceGraphHelpers::CreateAndCalculateAccSub(SimplexList& simplexList, AccTest<IncidenceGraph::IntersectionFlags>* test)
{
#ifdef ACCSUB_TRACE
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexList);
    ig->CreateGraph();
    Timer::Update("incidence graph created");
    MemoryInfo::Print();
    ig->CalculateAccSub(test);
    Timer::Update("acyclic subset calculated");
    MemoryInfo::Print();
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexList);
    ig->CreateGraph();
    ig->CalculateAccSub(test);
#endif
    return ig;
}

IncidenceGraph *IncidenceGraphHelpers::CreateAndCalculateAccSubWithBorder(SimplexList& simplexList, const VertsSet &borderVerts, AccTest<IncidenceGraph::IntersectionFlags>* test)
{
#ifdef ACCSUB_TRACE
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexList);
    ig->borderVerts = borderVerts;
    ig->CreateGraphWithBorder();
    Timer::Update("incidence graph created");
    MemoryInfo::Print();
    ig->CalculateAccSubWithBorder(test);
    Timer::Update("acyclic subset calculated");
    MemoryInfo::Print();
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexList);
    ig->borderVerts = borderVerts;
    ig->CreateGraphWithBorder();
    ig->CalculateAccSubWithBorder(test);
#endif
    return ig;
}

////////////////////////////////////////////////////////////////////////////////

IncidenceGraph *IncidenceGraphHelpers::CreateAndCalculateAccSubIG(SimplexList& simplexList, AccTest<IncidenceGraph::IntersectionFlags>* test)
{
#ifdef ACCSUB_TRACE
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexList);
    ig->CreateGraphAndCalculateAccSub(test);
    Timer::Update("incidence graph created and acyclic subset calculated");
    MemoryInfo::Print();
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexList);
    ig->CreateGraphAndCalculateAccSub(test);
#endif
    return ig;
}

IncidenceGraph *IncidenceGraphHelpers::CreateAndCalculateAccSubIGWithBorder(SimplexList& simplexList, const VertsSet &borderVerts, AccTest<IncidenceGraph::IntersectionFlags>* test)
{
#ifdef ACCSUB_TRACE
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexList);
    ig->borderVerts = borderVerts;
    ig->CreateGraphAndCalculateAccSubWithBorder(test);
    Timer::Update("incidence graph created and acyclic subset calculated");
    MemoryInfo::Print();
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexList);
    ig->borderVerts = borderVerts;
    ig->CreateGraphAndCalculateAccSubWithBorder(test);
#endif
    return ig;
}

IncidenceGraph *IncidenceGraphHelpers::CreateAndCalculateAccSubIGWithBorder(SimplexPtrList& simplexPtrList, const VertsSet &borderVerts, AccTest<IncidenceGraph::IntersectionFlags>* test)
{
#ifdef ACCSUB_TRACE
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexPtrList);
    ig->borderVerts = borderVerts;
    ig->CreateGraphAndCalculateAccSubWithBorder(test);
    Timer::Update("incidence graph created and acyclic subset calculated");
    MemoryInfo::Print();
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexPtrList);
    ig->borderVerts = borderVerts;
    ig->CreateGraphAndCalculateAccSubWithBorder(test);
#endif
    return ig;
}

////////////////////////////////////////////////////////////////////////////////

IncidenceGraph *IncidenceGraphHelpers::CreateAndCalculateAccSubST(SimplexList& simplexList, AccTest<IncidenceGraph::IntersectionFlags>* test)
{
#ifdef ACCSUB_TRACE
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexList);
    ig->CreateGraph();
    Timer::Update("incidence graph created");
    MemoryInfo::Print();
    ig->CalculateAccSubST(test);
    Timer::Update("acyclic subset calculated");
    MemoryInfo::Print();
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexList);
    ig->CreateGraph();
    ig->CalculateAccSubSpanningTree(test);
#endif
    return ig;
}

IncidenceGraph *IncidenceGraphHelpers::CreateAndCalculateAccSubST(SimplexPtrList& simplexPtrList, AccTest<IncidenceGraph::IntersectionFlags>* test)
{
#ifdef ACCSUB_TRACE
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexPtrList);
    ig->CreateGraph();
    Timer::Update("incidence graph created");
    MemoryInfo::Print();
    ig->CalculateAccSubST(test);
    Timer::Update("acyclic subset calculated");
    MemoryInfo::Print();
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexPtrList);
    ig->CreateGraph();
    ig->CalculateAccSubSpanningTree(test);
#endif
    return ig;
}

IncidenceGraph *IncidenceGraphHelpers::CreateAndCalculateAccSubSTWithBorder(SimplexList& simplexList, const VertsSet &borderVerts, AccTest<IncidenceGraph::IntersectionFlags>* test)
{
#ifdef ACCSUB_TRACE
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexList);
    ig->borderVerts = borderVerts;
    ig->CreateGraphWithBorder();
    Timer::Update("incidence graph created");
    MemoryInfo::Print();
    ig->CalculateAccSubSTWithBorder(test);
    Timer::Update("acyclic subset calculated");
    MemoryInfo::Print();
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexList);
    ig->borderVerts = borderVerts;
    ig->CreateGraphWithBorder();
    ig->CalculateAccSubSpanningTreeWithBorder(test);
#endif
    return ig;
}

IncidenceGraph *IncidenceGraphHelpers::CreateAndCalculateAccSubSTWithBorder(SimplexPtrList& simplexPtrList, const VertsSet &borderVerts, AccTest<IncidenceGraph::IntersectionFlags>* test)
{
#ifdef ACCSUB_TRACE
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexPtrList);
    ig->borderVerts = borderVerts;
    ig->CreateGraphWithBorder();
    Timer::Update("incidence graph created");
    MemoryInfo::Print();
    ig->CalculateAccSubSTWithBorder(test);
    Timer::Update("acyclic subset calculated");
    MemoryInfo::Print();
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexPtrList);
    ig->borderVerts = borderVerts;
    ig->CreateGraphWithBorder();
    ig->CalculateAccSubSpanningTreeWithBorder(test);
#endif
    return ig;
}

////////////////////////////////////////////////////////////////////////////////

IncidenceGraph *IncidenceGraphHelpers::CreateAndCalculateAccSubParallel(SimplexList& simplexList, int packsCount, AccSubAlgorithm accSubAlgorithm, AccTest<IncidenceGraph::IntersectionFlags>* test)
{
    PartitionGraph *pg = new PartitionGraph(simplexList, packsCount, accSubAlgorithm, test);
    IncidenceGraph *ig = pg->GetIncidenceGraph();
    delete pg;
    return ig;
}

////////////////////////////////////////////////////////////////////////////////
// eof
