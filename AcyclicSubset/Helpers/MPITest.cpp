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

////////////////////////////////////////////////////////////////////////////////

void MPITest::Master(int argc, char **argv)
{
#ifdef USE_MPI

    SimplexList simplexList;
    Tests::ProcessArguments(argc, argv);
    Tests::GenerateData(simplexList);
    std::cout<<"data size: "<<simplexList.size()<<std::endl;

    clock_t start = Timer::Now();
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

    MPI_Status status;
    int dataSize;

    while (1)
    {

        // pobieramy pierwszy komunikat
        MPI_Recv(&dataSize, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        // jezeli polecenie zakonczenia pracy to konczymy
        if (status.MPI_TAG == MPI_MY_DIE_TAG)
        {
            return;
        }

        // wpp. musi to byc MPI_MY_DATASIZE_TAG
        assert(status.MPI_TAG == MPI_MY_DATASIZE_TAG);

        // pobieramy bufor z danymi
        int *buffer = new int[dataSize];
        MPI_Recv(buffer, dataSize, MPI_INT, 0, MPI_MY_WORK_TAG, MPI_COMM_WORLD, &status);

        // z pobranego bufora budujemy dane wejsciowe
        MPIData::SimplexData *data = new MPIData::SimplexData(buffer, dataSize);
        SimplexList simplexList;
        std::set<Vertex> borderVerts;
        data->GetSimplexData(simplexList, borderVerts, Tests::incidenceGraphParams.dim, Tests::acyclicTestNumber);
        AcyclicTest<IncidenceGraph::IntersectionFlags> *test = Tests::GetAcyclicTest();

        // tworzymy graf incydencji z policzonym podzbiorem acyklicznym
        IncidenceGraph *ig = IncidenceGraph::CreateWithBorderVerts(simplexList, borderVerts, Tests::incidenceGraphParams);
        ig->CalculateAcyclicSubsetWithSpanningTree(test);

        delete data;
        delete test;

        // zamieniamy na bufor danych
        MPIData::IncidenceGraphData *igData = new MPIData::IncidenceGraphData(ig);
        dataSize = igData->GetSize();

        // i odsylamy do mastera
        MPI_Send(&dataSize, 1, MPI_INT, 0, MPI_MY_DATASIZE_TAG, MPI_COMM_WORLD);
        MPI_Send(igData->GetBuffer(), dataSize, MPI_INT,0, MPI_MY_DATA_TAG, MPI_COMM_WORLD);

        delete igData;
        delete ig;
    }
    
#endif
}

////////////////////////////////////////////////////////////////////////////////

void MPITest::Test(int argc, char **argv)
{
#ifdef USE_MPI

    int rank;

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
