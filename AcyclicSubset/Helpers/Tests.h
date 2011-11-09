#ifndef TESTS_H
#define	TESTS_H

#include <fstream>
#include <string>

#include "../AcyclicSubset/IncidenceGraph.h"
#include "../AcyclicSubset/Simplex.h"
#include "../AcyclicSubset/AcyclicTest.hpp"

class OutputGraph;

enum ReductionType
{
    RT_None,
    RT_Coreduction,
    RT_AcyclicSubset,
    RT_AcyclicSubsetOnline,
    RT_AcyclicSubsetSpanningTree,
    RT_AcyclicSubsetParallel,
};

class Tests
{

public:

    static void PrintHelp();
    static void ProcessArguments(int, char **);
    
    static void GenerateData(SimplexList &);

    static void Test(SimplexList &, ReductionType);
    static void Test(IncidenceGraph *, ReductionType, float);
    static void TestAndCompare(SimplexList &);

    static void StandardTest();
    static void TestFromList();

    static void TestFromCommandLine(int, char **);

    static int GetBetti(SimplexList &, int);
    static float ComputeHomology(OutputGraph *, bool);

    static void OpenLog();
    static void CloseLog();

private:
    
    static bool IsAcyclicSubsetReduction(ReductionType);

    // testType:
    // 0 - random
    // 1 - single file
    // 2 - list
    static int              testType;
    static std::string      inputFilename;
    static int              simplicesCount;
    static int              vertsCount;
    static int              simplicesDim;
    static int              sortSimplices;
    static int              sortVerts;
    static std::string      logFilename;
    static std::ofstream    log;
    static int              useAlgebraic;
    static int              useCoreduction;
    static int              useAcyclicSubset;
    static int              useAcyclicSubsetOnline;
    static int              useAcyclicSubsetSpanningTree;
    static int              useAcyclicSubsetParallel;
    static int              acyclicTestNumber;
    static IncidenceGraph::ParallelParams   parallelParams;

    friend class MPITest;
};

#endif	/* TESTS_H */

