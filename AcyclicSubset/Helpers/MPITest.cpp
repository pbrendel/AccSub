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

void MPITest::Master(int argc, char **argv)
{
#ifdef USE_MPI

    SimplexList simplexList;
    Tests::ProcessArguments(argc, argv);
    Tests::GenerateData(simplexList);
    std::cout<<"data size: "<<simplexList.size()<<std::endl;

    Timer::Time start = Timer::Now();
    IncidenceGraph *ig = IncidenceGraph::CreateAndCalculateAcyclicSubsetParallel(simplexList, Tests::incidenceGraphParams, Tests::parallelParams, 0, false);
    float totalTime = Timer::TimeFrom(start, "parallel computations");

    Tests::Test(ig, RT_AcyclicSubset, totalTime);

    delete ig;
    
#endif
}

////////////////////////////////////////////////////////////////////////////////

void MPITest::Slave(int processRank)
{
#ifdef USE_MPI

    ParallelGraph::MPISlave(processRank);
    
#endif
}

////////////////////////////////////////////////////////////////////////////////

void MPITest::Test(int argc, char **argv)
{
#ifdef USE_MPI

    MPI_Init(&argc, &argv);
    
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
