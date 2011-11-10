/*
 * File:   Tests.h
 * Author: Piotr Brendel
 */

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
    RT_Acc,
    RT_AccIG,
    RT_AccST,
    RT_AccParallel
};

class Tests
{

public:

    static void PrintHelp();
    static void ProcessArguments(int, char **);
    
    static void GenerateData(SimplexList &);

    static void Test(SimplexList &, ReductionType);
    static void TestAndCompare(SimplexList &);

    static void StandardTest();
    static void TestFromList();

    static void TestFromCommandLine(int, char **);

private:
    
    static bool IsAcyclicSubsetReduction(ReductionType);

    // testType:
    // 0 - random
    // 1 - single file
    // 2 - list
    static int              testType;
    static int              acyclicTestNumber;
    static std::string      inputFilename;
    static int              simplicesCount;
    static int              simplicesDim;
    static int              vertsCount;
    static int              sortSimplices;
    static int              sortVerts;
    static int              useAlgebraic;
    static int              useCoreduction;
    static int              useAcc;
    static int              useAccIG;
    static int              useAccST;
    static int              useParallel;
    static int              packsCount;
    static int              parallelAccSubAlgorithm;
    static int              prepareData;

    friend class MPITest;
};

#endif	/* TESTS_H */

