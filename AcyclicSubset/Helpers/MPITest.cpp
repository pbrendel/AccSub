/* 
 * File:   MPITest.cpp
 * Author: Piotr Brendel
 */

#include "MPITest.h"
#include "Tests.h"
#include "Utils.h"
#include "../AcyclicSubset/PartitionGraph.h"
#include "../AcyclicSubset/ComputationsParallelMPI.h"
#include "../AcyclicSubset/AcyclicTest.hpp"
#include "../AcyclicSubset/IncidenceGraphHelpers.h"
#include "../AcyclicSubset/HomologyHelpers.h"

#ifdef USE_MPI
#include <mpi.h>
#include "../AcyclicSubset/MPIData.h"
#endif

#include <assert.h>

int MPITest::rank = 0;

////////////////////////////////////////////////////////////////////////////////

void MPITest::StandardTest()
{
#ifdef USE_MPI

    SimplexList simplexList;
    Tests::GenerateData(simplexList);

    char buff[100] = { 0 };
    switch (Tests::testType)
    {
        case 0:
            sprintf(buff, "random set of size: %d", (int)simplexList.size());
            break;
        case 1:
            sprintf(buff, "testing %s", Tests::inputFilename.c_str());
            break;
        default:
            break;
    }

    std::cout<<buff<<std::endl;

    AcyclicTest<IncidenceGraph::IntersectionFlags> *test = AcyclicTest<IncidenceGraph::IntersectionFlags>::Create(Tests::acyclicTestNumber, Simplex::GetSimplexListDimension(simplexList));
    Timer::Time start = Timer::Now();

    IncidenceGraph *ig = IncidenceGraphHelpers::CreateAndCalculateAcyclicSubsetParallel(simplexList, Tests::packsCount, (AccSubAlgorithm)Tests::parallelAccSubAlgorithm, test);
    delete test;

    Timer::TimeFrom(start, "parallel computations");

    std::cout<<"acyclic subset size: "<<ig->GetAcyclicSubsetSize()<<std::endl;

    Timer::Update();
    OutputGraph *og = new OutputGraph(ig);
    Timer::Update("creating output");
    MemoryInfo::Print();

    start = Timer::Now();
    HomologyHelpers::ComputeHomology(og, false);
    Timer::TimeFrom(start, "computing homology");
    MemoryInfo::Print();

    delete og;
    delete ig;
      
#endif
}

void MPITest::TestFromList()
{
#ifdef USE_MPI

    std::fstream input(Tests::inputFilename.c_str(), std::ios::in);
    if (!input.is_open())
    {
        throw std::string("Can't open file ") + Tests::inputFilename;
    }

    Tests::testType = 2; // dane z pliku
    while (!input.eof())
    {
        input>>Tests::inputFilename;
        if (Tests::inputFilename != "")
        {
            StandardTest();
        }
        else
        {
            break;
        }
    }
    input.close();
    
#endif
}

void MPITest::Master(int argc, char **argv)
{
#ifdef USE_MPI

    Tests::ProcessArguments(argc, argv);

    std::cout<<"Aby uzyskac wiecej informacji uruchom z parametrem -help"<<std::endl;

    switch (Tests::testType)
    {
        case 0:
        case 1:
            StandardTest();
            break;
        case 2:
            TestFromList();
            break;
        default:
            break;
    }

    ComputationsParallelMPI::KillSlaves();

    MemoryInfo::Print();

#endif
}

////////////////////////////////////////////////////////////////////////////////

void MPITest::Slave(int processRank)
{
#ifdef USE_MPI

    ComputationsParallelMPI::Slave(processRank);

#endif
}

////////////////////////////////////////////////////////////////////////////////

void MPITest::Test(int argc, char **argv)
{
#ifdef USE_MPI

    MPI_Init(&argc, &argv);
    Timer::Init();
    
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    std::cout<<"starting process: "<<rank<<std::endl;
    Timer::Time now = Timer::Now();

    if (rank == 0)
    {
        Master(argc, argv);
    }
    else
    {
        Slave(rank);
    }

    std::cout<<"terminating process: "<<rank<<std::endl;
    Timer::TimeFrom(now, "total");
    MemoryInfo::Print();
    MPI_Finalize();

#endif
}

////////////////////////////////////////////////////////////////////////////////
// eof
