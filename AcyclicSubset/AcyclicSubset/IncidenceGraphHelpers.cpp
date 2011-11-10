/* 
 * File:   IncidenceGraphHelpers.cpp
 * Author: Piotr Brendel
 */

#include "IncidenceGraphHelpers.h"
#include "PartitionGraph.h"

#ifdef USE_HELPERS
#include "../Helpers/Utils.h"
#endif
////////////////////////////////////////////////////////////////////////////////

IncidenceGraph *IncidenceGraphHelpers::Create(SimplexList& simplexList)
{
#ifdef USE_HELPERS
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
#ifdef USE_HELPERS
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
#ifdef USE_HELPERS
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

IncidenceGraph *IncidenceGraphHelpers::CreateAndCalculateAcyclicSubset(SimplexList& simplexList, AcyclicTest<IncidenceGraph::IntersectionFlags>* test)
{
#ifdef USE_HELPERS
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexList);
    ig->CreateGraph();
    Timer::Update("incidence graph created");
    MemoryInfo::Print();
    ig->CalculateAcyclicSubset(test);
    Timer::Update("acyclic subset calculated");
    MemoryInfo::Print();
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexList);
    ig->CreateGraph();
    ig->CalculateAcyclicSubset(test);
#endif
    return ig;
}

IncidenceGraph *IncidenceGraphHelpers::CreateAndCalculateAcyclicSubsetWithBorder(SimplexList& simplexList, const VertsSet &borderVerts, AcyclicTest<IncidenceGraph::IntersectionFlags>* test)
{
#ifdef USE_HELPERS
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexList);
    ig->borderVerts = borderVerts;
    ig->CreateGraphWithBorder();
    Timer::Update("incidence graph created");
    MemoryInfo::Print();
    ig->CalculateAcyclicSubsetWithBorder(test);
    Timer::Update("acyclic subset calculated");
    MemoryInfo::Print();
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexList);
    ig->borderVerts = borderVerts;
    ig->CreateGraphWithBorder();
    ig->CalculateAcyclicSubsetWithBorder(test);
#endif
    return ig;
}

////////////////////////////////////////////////////////////////////////////////

IncidenceGraph *IncidenceGraphHelpers::CreateAndCalculateAcyclicSubsetOnline(SimplexList& simplexList, AcyclicTest<IncidenceGraph::IntersectionFlags>* test)
{
#ifdef USE_HELPERS
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexList);
    ig->CreateGraphAndCalculateAcyclicSubset(test);
    Timer::Update("incidence graph created and acyclic subset calculated");
    MemoryInfo::Print();
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexList);
    ig->CreateGraphAndCalculateAcyclicSubset(test);
#endif
    return ig;
}

IncidenceGraph *IncidenceGraphHelpers::CreateAndCalculateAcyclicSubsetOnlineWithBorder(SimplexList& simplexList, const VertsSet &borderVerts, AcyclicTest<IncidenceGraph::IntersectionFlags>* test)
{
#ifdef USE_HELPERS
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexList);
    ig->borderVerts = borderVerts;
    ig->CreateGraphAndCalculateAcyclicSubsetWithBorder(test);
    Timer::Update("incidence graph created and acyclic subset calculated");
    MemoryInfo::Print();
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexList);
    ig->borderVerts = borderVerts;
    ig->CreateGraphAndCalculateAcyclicSubsetWithBorder(test);
#endif
    return ig;
}

IncidenceGraph *IncidenceGraphHelpers::CreateAndCalculateAcyclicSubsetOnlineWithBorder(SimplexPtrList& simplexPtrList, const VertsSet &borderVerts, AcyclicTest<IncidenceGraph::IntersectionFlags>* test)
{
#ifdef USE_HELPERS
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexPtrList);
    ig->borderVerts = borderVerts;
    ig->CreateGraphAndCalculateAcyclicSubsetWithBorder(test);
    Timer::Update("incidence graph created and acyclic subset calculated");
    MemoryInfo::Print();
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexPtrList);
    ig->borderVerts = borderVerts;
    ig->CreateGraphAndCalculateAcyclicSubsetWithBorder(test);
#endif
    return ig;
}

////////////////////////////////////////////////////////////////////////////////

IncidenceGraph *IncidenceGraphHelpers::CreateAndCalculateAcyclicSubsetSpanningTree(SimplexList& simplexList, AcyclicTest<IncidenceGraph::IntersectionFlags>* test)
{
#ifdef USE_HELPERS
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexList);
    ig->CreateGraph();
    Timer::Update("incidence graph created");
    MemoryInfo::Print();
    ig->CalculateAcyclicSubsetSpanningTree(test);
    Timer::Update("acyclic subset calculated");
    MemoryInfo::Print();
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexList);
    ig->CreateGraph();
    ig->CalculateAcyclicSubsetSpanningTree(test);
#endif
    return ig;
}

IncidenceGraph *IncidenceGraphHelpers::CreateAndCalculateAcyclicSubsetSpanningTree(SimplexPtrList& simplexPtrList, AcyclicTest<IncidenceGraph::IntersectionFlags>* test)
{
#ifdef USE_HELPERS
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexPtrList);
    ig->CreateGraph();
    Timer::Update("incidence graph created");
    MemoryInfo::Print();
    ig->CalculateAcyclicSubsetSpanningTree(test);
    Timer::Update("acyclic subset calculated");
    MemoryInfo::Print();
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexPtrList);
    ig->CreateGraph();
    ig->CalculateAcyclicSubsetSpanningTree(test);
#endif
    return ig;
}

IncidenceGraph *IncidenceGraphHelpers::CreateAndCalculateAcyclicSubsetSpanningTreeWithBorder(SimplexList& simplexList, const VertsSet &borderVerts, AcyclicTest<IncidenceGraph::IntersectionFlags>* test)
{
#ifdef USE_HELPERS
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexList);
    ig->borderVerts = borderVerts;
    ig->CreateGraphWithBorder();
    Timer::Update("incidence graph created");
    MemoryInfo::Print();
    ig->CalculateAcyclicSubsetSpanningTreeWithBorder(test);
    Timer::Update("acyclic subset calculated");
    MemoryInfo::Print();
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexList);
    ig->borderVerts = borderVerts;
    ig->CreateGraphWithBorder();
    ig->CalculateAcyclicSubsetSpanningTreeWithBorder(test);
#endif
    return ig;
}

IncidenceGraph *IncidenceGraphHelpers::CreateAndCalculateAcyclicSubsetSpanningTreeWithBorder(SimplexPtrList& simplexPtrList, const VertsSet &borderVerts, AcyclicTest<IncidenceGraph::IntersectionFlags>* test)
{
#ifdef USE_HELPERS
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexPtrList);
    ig->borderVerts = borderVerts;
    ig->CreateGraphWithBorder();
    Timer::Update("incidence graph created");
    MemoryInfo::Print();
    ig->CalculateAcyclicSubsetSpanningTreeWithBorder(test);
    Timer::Update("acyclic subset calculated");
    MemoryInfo::Print();
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexPtrList);
    ig->borderVerts = borderVerts;
    ig->CreateGraphWithBorder();
    ig->CalculateAcyclicSubsetSpanningTreeWithBorder(test);
#endif
    return ig;
}

////////////////////////////////////////////////////////////////////////////////

IncidenceGraph *IncidenceGraphHelpers::CreateAndCalculateAcyclicSubsetParallel(SimplexList& simplexList, int packsCount, AccSubAlgorithm accSubAlgorithm, AcyclicTest<IncidenceGraph::IntersectionFlags>* test)
{
    PartitionGraph *pg = new PartitionGraph(simplexList, packsCount, accSubAlgorithm, test);
    IncidenceGraph *ig = pg->GetIncidenceGraph();
    delete pg;
    return ig;
}

////////////////////////////////////////////////////////////////////////////////
// eof
