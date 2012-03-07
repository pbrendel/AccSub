/*
 * File:   Tests.h
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 */

#ifndef TESTS_H
#define	TESTS_H

#include <fstream>
#include <string>

#include "../AcyclicSubset/IncidenceGraphTraits.h"
#include "../AcyclicSubset/IncidenceGraph.hpp"
#include "../AcyclicSubset/OutputGraph.hpp"
#include "../AcyclicSubset/PartitionGraph.hpp"
#include "../AcyclicSubset/PrepareData.hpp"
#include "../AcyclicSubset/ComputationsLocal.hpp"
#include "../AcyclicSubset/ComputationsLocalMPITest.hpp"
#include "../AcyclicSubset/ComputationsParallelMPI.hpp"
#include "../AcyclicSubset/ComputationsParallelOMP.hpp"

enum ReductionType
{
    RT_None,
    RT_Coreduction,
    RT_AccSub,
    RT_AccSubIG,
    RT_AccSubST,
    RT_AccSubDist
};

class Tests
{
    typedef IncidenceGraphT<IncidenceGraphDefaultTraits, IntersectionInfoFlags, AccInfoFlags> IncidenceGraph;
    typedef IncidenceGraph::Simplex Simplex;
    typedef IncidenceGraph::SimplexList SimplexList;
    typedef IncidenceGraph::AccTest AccTest;
    typedef IncidenceGraph::AccSubAlgorithm AccSubAlgorithm;

    typedef OutputGraphT<IncidenceGraph> OutputGraph;
    
#ifdef USE_MPI
    typedef PartitionGraphT<IncidenceGraph, PrepareDataBFS, ComputationsParallelMPI> PartitionGraph;
#else
#ifdef USE_OMP
    typedef PartitionGraphT<IncidenceGraph, PrepareDataBFS, ComputationsParallelOMP> PartitionGraph;
#else
    typedef PartitionGraphT<IncidenceGraph, PrepareDataBFS, ComputationsLocalMPITest> PartitionGraph;
#endif
#endif

public:

    static void TestFromCommandLine(int, char **);
    static void MPITestFromCommandLine(int, char**);

private:
    
    // inputType:
    // 0 - single file
    // 1 - list
    // 2 - random rips complex
    static int              inputType;
    static int              accTestNumber;
    static std::string      inputFilename;
    static int              sortVerts;
    static int              useAlgebraic;
    static int              useCoreductions;
    static int              useAccSub;
    static int              useAccSubIG;
    static int              useAccSubST;
    static int              useAccSubDist;
    static int              packsCount;
    static int              distAccSubAlgorithm;
    static int              processRank;
    static int              randomPointsCount;
    static float            randomPointsDiam;
    static int              randomPointsDim;
    static float            ripsComplexEpsilon;
    static int              ripsComplexDim;

    static void PrintHelp();
    static void ProcessArgument(std::vector<std::string> &args);
    static void ProcessArguments(int, char **);

    static bool IsAccSubReduction(ReductionType);

    static void Test(SimplexList &, ReductionType);
    static void TestAndCompare(SimplexList &);

    static void TestSingleFile();
    static void TestFromList();
    static void TestRandomRipsComplex();

    static void MPIMaster(int argc, char **argv);
    static void MPISlave(int processRank);

    static int GetProcessRank() { return processRank; }

};

#endif	/* TESTS_H */

