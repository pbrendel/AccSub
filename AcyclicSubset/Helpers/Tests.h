/*
 * File:   Tests.h
 * Author: Piotr Brendel
 */

#ifndef TESTS_H
#define	TESTS_H

#include <fstream>
#include <string>

#include "../AcyclicSubset/IncidenceGraphTraits.h"
#include "../AcyclicSubset/IncidenceGraph.h"
#include "../AcyclicSubset/OutputGraph.h"

class OutputGraph;

enum ReductionType
{
    RT_None,
    RT_Coreduction,
    RT_AccSub,
    RT_AccSubIG,
    RT_AccSubST,
    RT_AccSubParallel
};

class Tests
{
    typedef IncidenceGraphT<IncidenceGraphDefaultTraits, IntersectionInfoFlags, AccInfoFlags> IncidenceGraph;
    typedef IncidenceGraph::Simplex Simplex;
    typedef IncidenceGraph::SimplexList SimplexList;
    typedef IncidenceGraph::AccTest AccTest;
    typedef IncidenceGraph::AccSubAlgorithm AccSubAlgorithm;

    typedef OutputGraphT<IncidenceGraph> OutputGraph;

public:

    static void TestFromCommandLine(int, char **);
    static void MPITestFromCommandLine(int, char**);

private:
    
    // testType:
    // 0 - random
    // 1 - single file
    // 2 - list
    static int              testType;
    static int              accTestNumber;
    static std::string      inputFilename;
    static int              simplicesCount;
    static int              simplicesDim;
    static int              vertsCount;
    static int              sortSimplices;
    static int              sortVerts;
    static int              useAlgebraic;
    static int              useCoreduction;
    static int              useAccSub;
    static int              useAccSubIG;
    static int              useAccSubST;
    static int              useAccSubParallel;
    static int              packsCount;
    static int              parallelAccSubAlgorithm;
    static int              prepareData;
    static int              processRank;

    static void PrintHelp();
    static void ProcessArguments(int, char **);

    static void GenerateData(SimplexList &);

    static bool IsAccSubReduction(ReductionType);

    static void Test(SimplexList &, ReductionType);
    static void TestAndCompare(SimplexList &);

    static void StandardTest();
    static void TestFromList();

    static void MPIMaster(int argc, char **argv);
    static void MPISlave(int processRank);

    static int GetProcessRank() { return processRank; }

};

#endif	/* TESTS_H */

