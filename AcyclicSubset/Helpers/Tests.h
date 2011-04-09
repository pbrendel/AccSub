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

    static int GetBetti(SimplexList &, int);

    static void TestRandom(int, int, int);
    static void TestReverseRandom(int, int, int);
    static void TestAcyclicTree(int, int);
    static void TestFromFile(const char *);
    static void TestFromList(const char *);
    static void TestFromCommandLine(int, char **);

private:

    static void TestAndCompareMinimalization(SimplexList &, std::string);
    static void TestMinimalization(const char *);
    static void TestAndCompareOutputCreation(SimplexList &, std::string);
    static void TestOutputCreation(const char *);
    
    static AcyclicTest<IncidenceGraph::IntersectionFlags> *GetAcyclicTest();
    static bool IsAcyclicSubsetReduction(ReductionType);

    static float ComputeHomology(OutputGraph *, bool);
    static void Test(SimplexList &, ReductionType);
    static void TestAndCompare(SimplexList &, std::string);

    // testType:
    // 0 - random
    // 1 - "reverse" random
    // 2 - single
    // 3 = list
    // 4 - minimalization
    // 5 - output generation
    // 6 - acyclic tree
    static int              testType; 
    static std::string      inputFilename;
    static int              acyclicTestNumber;
    static int              simplicesCount;
    static int              vertsCount;
    static int              nodeSimplicesCount;
    static int              nodesCount;
    static int              sortVerts;
    static std::string      logFilename;
    static std::ofstream    log;
    static int              useAlgebraic;
    static int              useCoreduction;
    static int              useAcyclicSubset;
    static int              useAcyclicSubsetOnline;
    static int              useAcyclicSubsetSpanningTree;
    static int              useAcyclicSubsetParallel;
    static IncidenceGraph::Params           incidenceGraphParams;
    static IncidenceGraph::ParallelParams   parallelParams;

};

#endif	/* TESTS_H */

