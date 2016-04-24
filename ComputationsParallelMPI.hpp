/*
 * File:   ComputationsParallelMPI.hpp
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 */

#ifndef COMPUTATIONSPARALLELMPI_HPP
#define	COMPUTATIONSPARALLELMPI_HPP

#include "IncidenceGraphHelpers.hpp"
#include <map>

#ifdef ACCSUB_TRACE
#include "Utils.hpp"
#endif

#ifdef USE_MPI
#include <mpi.h>
#include "MPIData.hpp"
#ifdef DEBUG_MPI
#include "Utils.hpp"
#endif
#endif

#define MPI_MY_WORK_TAG        1
#define MPI_MY_DIE_TAG         2
#define MPI_MY_DATASIZE_TAG    3
#define MPI_MY_DATA_TAG        4
#define MPI_MY_MEMORY_INFO_TAG 5

////////////////////////////////////////////////////////////////////////////////

template <typename PartitionGraph>
class ComputationsParallelMPI
{
    typedef typename PartitionGraph::IncidenceGraph IncidenceGraph;
    typedef typename PartitionGraph::Vertex Vertex;
    typedef typename PartitionGraph::Simplex Simplex;
    typedef typename PartitionGraph::SimplexList SimplexList;
    typedef typename PartitionGraph::SimplexPtrList SimplexPtrList;
    typedef typename PartitionGraph::Node Node;
    typedef typename PartitionGraph::Nodes Nodes;
    typedef typename PartitionGraph::AccTest AccTest;
    typedef typename PartitionGraph::AccSubAlgorithm AccSubAlgorithm;

    static AccSubAlgorithm accSubAlgorithm;
    static AccTest *accTest;

    static void SendMPISimplexData(Node *node, int processRank)
    {
#ifdef USE_MPI
#ifdef DEBUG_MPI
        std::cout<<"process 0 ";
        Timer::TimeStamp("packing data");
#endif
        MPISimplexData<IncidenceGraph> *data = new MPISimplexData<IncidenceGraph>(node->simplexPtrList, node->borderVerts, accSubAlgorithm, accTest->GetID(), Simplex::GetSimplexListConstantSize(node->simplexPtrList));
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

    static void SetMPIIncidenceGraphData(Node *node, int *buffer, int size)
    {
#ifdef USE_MPI
        MPIIncidenceGraphData<IncidenceGraph> *data = new MPIIncidenceGraphData<IncidenceGraph>(buffer, size);
        node->ig = data->GetIncidenceGraph(node->simplexPtrList);
#ifdef DEBUG_MPI
        std::cout<<"process 0 ";
        Timer::TimeStamp("unpacked data");
#endif
        delete data;
#endif
    }

public:

    static void Compute(Nodes &nodes, AccSubAlgorithm accSubAlgorithm, AccTest *accTest)
    {
#ifdef ACCSUB_TRACE
        Timer::TimeStamp("ComputationsParallelMPI start");
        Timer::Time start = Timer::Now();
        if (accSubAlgorithm == AccSubAlgorithm::AccSubIG)
        {
            std::cout<<"using AccSubIG"<<std::endl;
        }
        else
        {
            std::cout<<"using AccSubST"<<std::endl;
        }
#endif
#ifdef USE_MPI
        ComputationsParallelMPI::accSubAlgorithm = accSubAlgorithm;
        ComputationsParallelMPI::accTest = accTest;
        int nodesCount = nodes.size();
        int currentNode = 0;
        int tasksCount;
        int dataSize;
        MPI_Status status;
        std::map<int, Node *> rankToNode;

        MPI_Comm_size(MPI_COMM_WORLD, &tasksCount);

        int size = (tasksCount < (nodesCount + 1)) ? tasksCount : (nodesCount + 1);

        // as long as we have free nodes we send data to them
        for (int rank = 1; rank < size; rank++)
        {
            std::cout<<"sending node "<<currentNode<<" to process: "<<rank<<std::endl;
            rankToNode[rank] = nodes[currentNode];
            SendMPISimplexData(nodes[currentNode++], rank);
        }

        // then we find for a node to finish
        while (currentNode < nodesCount)
        {
            // getting size of the data
            MPI_Recv(&dataSize, 1, MPI_INT, MPI_ANY_SOURCE, MPI_MY_DATASIZE_TAG, MPI_COMM_WORLD, &status);
            int *buffer = new int[dataSize];

            // getting data
            MPI_Recv(buffer, dataSize, MPI_INT, status.MPI_SOURCE, MPI_MY_DATA_TAG, MPI_COMM_WORLD, &status);
#ifdef DEBUG_MPI
            std::cout<<"process 0 ";
            Timer::TimeStamp("received data");
#endif
            SetMPIIncidenceGraphData(rankToNode[status.MPI_SOURCE], buffer, size);
            std::cout<<"sending node "<<currentNode<<" to process: "<<status.MPI_SOURCE<<std::endl;
            rankToNode[status.MPI_SOURCE] = nodes[currentNode];
            SendMPISimplexData(nodes[currentNode++], status.MPI_SOURCE);
        }

        // finally we wait for all nodes that sill compute
        for (int rank = 1; rank < size; ++rank)
        {
            // getting size of the data
            MPI_Recv(&dataSize, 1, MPI_INT, MPI_ANY_SOURCE, MPI_MY_DATASIZE_TAG, MPI_COMM_WORLD, &status);
            int *buffer = new int[dataSize];
            // getting data
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

    static void Slave(int processRank)
    {
#ifdef USE_MPI
        MPI_Status status;
        int dataSize;

        while (1)
        {
            // first message
            MPI_Recv(&dataSize, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            // if it is die command
            if (status.MPI_TAG == MPI_MY_DIE_TAG)
            {
                return;
            }

            if (status.MPI_TAG == MPI_MY_MEMORY_INFO_TAG)
            {
                std::cout<<"process "<<processRank<<" ";
                MemoryInfo::Print();
                int memory = MemoryInfo::GetMaxUsage();
                MPI_Send(&memory, 1, MPI_INT, 0, MPI_MY_MEMORY_INFO_TAG, MPI_COMM_WORLD);
                continue;
            }

            assert(status.MPI_TAG == MPI_MY_DATASIZE_TAG);

            // getting data to compute
            int *buffer = new int[dataSize];
            MPI_Recv(buffer, dataSize, MPI_INT, 0, MPI_MY_WORK_TAG, MPI_COMM_WORLD, &status);
#ifdef DEBUG_MPI
            std::cout<<"process "<<processRank<<" ";
            Timer::TimeStamp("received data");
#endif

            // formatting buffer into input data
            MPISimplexData<IncidenceGraph> *data = new MPISimplexData<IncidenceGraph>(buffer, dataSize);
            SimplexList simplexList;
            std::set<Vertex> borderVerts;
            int accSubAlgorithm;
            int accTestNumber;
            data->GetSimplexData(simplexList, borderVerts, accSubAlgorithm, accTestNumber);
#ifdef DEBUG_MPI
            std::cout<<"process "<<processRank<<" ";
            Timer::TimeStamp("upacked data");
#endif
            AccTest *accTest = AccTest::Create(accTestNumber, Simplex::GetSimplexListDimension(simplexList));

            // main computations
            IncidenceGraph *ig = 0;
            if (accSubAlgorithm == AccSubAlgorithm::AccSubIG)
            {
                ig = IncidenceGraphHelpers<IncidenceGraph>::CreateAndCalculateAccSubIGWithBorder(simplexList, borderVerts, accTest);
            }
            else
            {
                ig = IncidenceGraphHelpers<IncidenceGraph>::CreateAndCalculateAccSubSTWithBorder(simplexList, borderVerts, accTest);
            }
            ig->UpdateConnectedComponents();
            ig->AssignNewIndices(true);

            delete data;
            delete accTest;

            // writting results into buffer
            MPIIncidenceGraphData<IncidenceGraph> *igData = new MPIIncidenceGraphData<IncidenceGraph>(ig);
#ifdef DEBUG_MPI
            std::cout<<"process "<<processRank<<" ";
            Timer::TimeStamp("packing data");
#endif
            dataSize = igData->GetSize();

#ifdef DEBUG_MPI
            std::cout<<"process "<<processRank<<" ";
            Timer::TimeStamp("sending data");
#endif
            // sending to master node
            MPI_Send(&dataSize, 1, MPI_INT, 0, MPI_MY_DATASIZE_TAG, MPI_COMM_WORLD);
            MPI_Send(igData->GetBuffer(), dataSize, MPI_INT,0, MPI_MY_DATA_TAG, MPI_COMM_WORLD);

            delete igData;
            delete ig;
        }
#endif
    }

    static void KillSlaves()
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

    static void CollectDebugMemoryInfo()
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
            MemoryInfo::AddSlavesMemoryInfo(rank, mem);
        }
        MemoryInfo::PrintSlavesMemoryInfo();
#endif
    }
};

template <typename PartitionGraph>
typename ComputationsParallelMPI<PartitionGraph>::AccSubAlgorithm ComputationsParallelMPI<PartitionGraph>::accSubAlgorithm = ComputationsParallelMPI<PartitionGraph>::AccSubAlgorithm::AccSubST;
template <typename PartitionGraph>
typename ComputationsParallelMPI<PartitionGraph>::AccTest *ComputationsParallelMPI<PartitionGraph>::accTest = 0;

#endif	/* COMPUTATIONSPARALLELMPI_HPP */

