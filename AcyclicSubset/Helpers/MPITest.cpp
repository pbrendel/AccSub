/* 
 * File:   MPITest.cpp
 * Author: Piotr Brendel
 */

#include "MPITest.h"
#include "../AcyclicSubset/ParallelGraph.h"
#include "../Helpers/Utils.h"

#ifdef USE_MPI
#include <mpi.h>
#include "../AcyclicSubset/MPIData.h"
#include "Tests.h"
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
            sprintf(buff, "random set of size %d, randomly removed %d simplices", (int)simplexList.size(), Tests::simplicesCount);
            break;
        case 2:
            sprintf(buff, "testing %s", Tests::inputFilename.c_str());
            break;
        case 3:
            sprintf(buff, "acyclic tree of %d nodes %d simplices each, total: %d", Tests::nodesCount, Tests::nodeSimplicesCount, (int)simplexList.size());
            break;
        default:
            break;
    }

    std::cout<<buff<<std::endl;
    Tests::log<<"<input_size>"<<simplexList.size()<<"</input_size>"<<std::endl;
    Tests::log<<std::endl<<"\t<description>"<<buff<<"</description>"<<std::endl<<std::endl;

    Timer::Time start = Timer::Now();
    IncidenceGraph *ig = IncidenceGraph::CreateAndCalculateAcyclicSubsetParallel(simplexList, Tests::incidenceGraphParams, Tests::parallelParams, 0, false);
    float totalTime = Timer::TimeFrom(start, "parallel computations");

    Tests::Test(ig, RT_AcyclicSubset, totalTime);

    delete ig;

    MemoryInfo::Print();

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

    Tests::OpenLog();

    switch (Tests::testType)
    {
        case 0:
        case 1:
        case 2:
        case 3:
            StandardTest();
            break;
        case 4:
            TestFromList();
            break;
        default:
            break;
    }

    ParallelGraph::KillMPISlaves();
    Tests::CloseLog();

    MemoryInfo::Print();

#endif
}

////////////////////////////////////////////////////////////////////////////////

void MPITest::Slave(int processRank)
{
#ifdef USE_MPI

    ParallelGraph::MPISlave(processRank);

    MemoryInfo::Print();

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

    if (rank == 0)
    {
        Master(argc, argv);
    }
    else
    {
        Slave(rank);
    }

    std::cout<<"terminating process: "<<rank<<std::endl;
    MPI_Finalize();

#endif
}

////////////////////////////////////////////////////////////////////////////////
// eof
