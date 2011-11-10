/* 
 * File:   ComputationsParallelMPI.cpp
 * Author: Piotr Brendel
 */

#include "ComputationsParallelMPI.h"
#include "../Helpers/Utils.h"
#include <map>

#ifdef USE_MPI
#include <mpi.h>
#include "MPIData.h"
#endif

#define MPI_MY_WORK_TAG        1
#define MPI_MY_DIE_TAG         2
#define MPI_MY_DATASIZE_TAG    3
#define MPI_MY_DATA_TAG        4
#define MPI_MY_MEMORY_INFO_TAG 5

////////////////////////////////////////////////////////////////////////////////

void ComputationsParallelMPI::Compute(ParallelGraph::DataNodes &nodes, AccSubAlgorithm accSubAlgorithm, AcyclicTest<IncidenceGraph::IntersectionFlags> *acyclicTest)
{
#ifdef USE_MPI
    int nodesCount = nodes.size();
    int currentNode = 0;
    int tasksCount;
    int dataSize;
    MPI_Status status;
    std::map<int, ParallelGraph::DataNode*> rankToNode;

    MPI_Comm_size(MPI_COMM_WORLD, &tasksCount);

    int size = (tasksCount < (nodesCount + 1)) ? tasksCount : (nodesCount + 1);

    // dopoki starczy nam node'ow wysylamy paczki
    for (int rank = 1; rank < size; rank++)
    {
        std::cout<<"sending node "<<currentNode<<" to process: "<<rank<<std::endl;
        rankToNode[rank] = nodes[currentNode];
        SendMPISimplexData(nodes[currentNode++], accSubAlgorithm, rank);
    }

    // potem czekamy na dane i wysylamy kolejne
    while (currentNode < nodesCount)
    {
        // najpierw pobieramy rozmiar danych
        MPI_Recv(&dataSize, 1, MPI_INT, MPI_ANY_SOURCE, MPI_MY_DATASIZE_TAG, MPI_COMM_WORLD, &status);
        int *buffer = new int[dataSize];

        // teraz dane od node'a od ktorego dostalismy info o rozmiarze danych
        MPI_Recv(buffer, dataSize, MPI_INT, status.MPI_SOURCE, MPI_MY_DATA_TAG, MPI_COMM_WORLD, &status);
#ifdef DEBUG_MPI
        std::cout<<"process 0 ";
        Timer::TimeStamp("received data");
#endif
        SetMPIIncidenceGraphData(rankToNode[status.MPI_SOURCE], buffer, size);
        std::cout<<"sending node "<<currentNode<<" to process: "<<status.MPI_SOURCE<<std::endl;
        rankToNode[status.MPI_SOURCE] = nodes[currentNode];
        SendMPISimplexData(nodes[currentNode++], accSubAlgorithm, status.MPI_SOURCE);
    }

    // na koncu odbieramy to co jeszcze jest liczone
    for (int rank = 1; rank < size; ++rank)
    {
        // najpierw pobieramy rozmiar danych
        MPI_Recv(&dataSize, 1, MPI_INT, MPI_ANY_SOURCE, MPI_MY_DATASIZE_TAG, MPI_COMM_WORLD, &status);
        int *buffer = new int[dataSize];
        // teraz dane od node'a od ktorego dostalismy info o rozmiarze danych
        MPI_Recv(buffer, dataSize, MPI_INT, status.MPI_SOURCE, MPI_MY_DATA_TAG, MPI_COMM_WORLD, &status);
#ifdef DEBUG_MPI
        std::cout<<"process 0 ";
        Timer::TimeStamp("received data");
#endif
        SetMPIIncidenceGraphData(rankToNode[status.MPI_SOURCE], buffer, size);
    }
    std::cout<<"parallel computing done"<<std::endl;
#endif
}

////////////////////////////////////////////////////////////////////////////////

void ComputationsParallelMPI::SendMPISimplexData(ParallelGraph::DataNode *node, AccSubAlgorithm accSubAlgorithm, int processRank)
{
#ifdef USE_MPI
#ifdef DEBUG_MPI
        std::cout<<"process 0 ";
        Timer::TimeStamp("packing data");
#endif
    MPIData::SimplexData *data = new MPIData::SimplexData(node->simplexPtrList, node->borderVerts, accSubAlgorithm, node->GetConstantSimplexSize());
    int dataSize = data->GetSize();
    MPI_Send(&dataSize, 1, MPI_INT, processRank, MPI_MY_DATASIZE_TAG, MPI_COMM_WORLD);
#ifdef DEBUG_MPI
        std::cout<<"process 0 ";
        Timer::TimeStamp("sending data");
#endif
    MPI_Send(data->GetBuffer(), dataSize, MPI_INT, processRank, MPI_MY_WORK_TAG, MPI_COMM_WORLD);
    delete data;
#endif
}

void ComputationsParallelMPI::SetMPIIncidenceGraphData(ParallelGraph::DataNode *node, int* buffer, int size)
{
#ifdef USE_MPI
    MPIData::IncidenceGraphData *data = new MPIData::IncidenceGraphData(buffer, size);
    node->ig = data->GetIncidenceGraph(node->simplexPtrList);
#ifdef DEBUG_MPI
        std::cout<<"process 0 ";
        Timer::TimeStamp("unpacked data");
#endif
    delete data;
#endif
}

////////////////////////////////////////////////////////////////////////////////

void ComputationsParallelMPI::Slave(int processRank)
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

        if (status.MPI_TAG == MPI_MY_MEMORY_INFO_TAG)
        {
            int memory = MemoryInfo::GetUsage();
            MPI_Send(&memory, 1, MPI_INT, 0, MPI_MY_MEMORY_INFO_TAG, MPI_COMM_WORLD);
            MemoryInfo::Print();
            continue;
        }

        // wpp. musi to byc MPI_MY_DATASIZE_TAG
        assert(status.MPI_TAG == MPI_MY_DATASIZE_TAG);

        // pobieramy bufor z danymi
        int *buffer = new int[dataSize];
        MPI_Recv(buffer, dataSize, MPI_INT, 0, MPI_MY_WORK_TAG, MPI_COMM_WORLD, &status);
#ifdef DEBUG_MPI
        std::cout<<"process "<<processRank<<" ";
        Timer::TimeStamp("received data");
#endif

        // z pobranego bufora budujemy dane wejsciowe
        MPIData::SimplexData *data = new MPIData::SimplexData(buffer, dataSize);
        SimplexList simplexList;
        std::set<Vertex> borderVerts;
        int acyclicTestNumber;
        int accSubAlgorithm;
        data->GetSimplexData(simplexList, borderVerts, acyclicTestNumber, accSubAlgorithm, 0);
#ifdef DEBUG_MPI
        std::cout<<"process "<<processRank<<" ";
        Timer::TimeStamp("upacked data");
#endif
        AcyclicTest<IncidenceGraph::IntersectionFlags> *test = AcyclicTest<IncidenceGraph::IntersectionFlags>::Create(acyclicTestNumber, GetDimension(simplexList));

        // tworzymy graf incydencji z policzonym podzbiorem acyklicznym
        IncidenceGraph *ig = 0;
        if (accSubAlgorithm == ASA_AccIG)
        {
            ig = IncidenceGraph::CreateAndCalculateAcyclicSubsetOnlineWithBorder(simplexList, borderVerts, test);
        }
        else
        {
            ig = IncidenceGraph::CreateAndCalculateAcyclicSubsetSpanningTreeWithBorder(simplexList, borderVerts, test);
        }
        ig->UpdateConnectedComponents();
        ig->AssignNewIndices(true);

        delete data;
        delete test;

        // zamieniamy na bufor danych
        MPIData::IncidenceGraphData *igData = new MPIData::IncidenceGraphData(ig);
#ifdef DEBUG_MPI
        std::cout<<"process "<<processRank<<" ";
        Timer::TimeStamp("packing data");
#endif
        dataSize = igData->GetSize();

#ifdef DEBUG_MPI
        std::cout<<"process "<<processRank<<" ";
        Timer::TimeStamp("sending data");
#endif
        // i odsylamy do mastera
        MPI_Send(&dataSize, 1, MPI_INT, 0, MPI_MY_DATASIZE_TAG, MPI_COMM_WORLD);
        MPI_Send(igData->GetBuffer(), dataSize, MPI_INT,0, MPI_MY_DATA_TAG, MPI_COMM_WORLD);

        delete igData;
        delete ig;
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////

void ComputationsParallelMPI::KillSlaves()
{
#ifdef USE_MPI
    int tasksCount;
    MPI_Comm_size(MPI_COMM_WORLD, &tasksCount);
    for (int rank = 1; rank < tasksCount; ++rank)
    {
        MPI_Send(0, 0, MPI_INT, rank, MPI_MY_DIE_TAG, MPI_COMM_WORLD);
    }
    std::cout<<"slaves killed"<<std::endl;
#endif
}

void ComputationsParallelMPI::CollectDebugMemoryInfo()
{
#ifdef USE_MPI
    MPI_Status status;
    int tasksCount;
    int mem = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &tasksCount);
    for (int rank = 1; rank < tasksCount; ++rank)
    {
        MPI_Send(0, 0, MPI_INT, rank, MPI_MY_MEMORY_INFO_TAG, MPI_COMM_WORLD);
        MPI_Recv(&mem, 1, MPI_INT, rank, MPI_MY_MEMORY_INFO_TAG, MPI_COMM_WORLD, &status);
        // todo!!!
        // MemoryInfo::AddSlaveMemoryInfo(rank, mem);
    }
    std::cout<<"slaves killed"<<std::endl;
#endif
}

////////////////////////////////////////////////////////////////////////////////
// eof
