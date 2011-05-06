/* 
 * File:   DebugMemory.cpp
 * Author: Piotr Brendel
 */

#include "DebugMemory.h"
#include <iostream>
#include <map>

#ifdef USE_MPI
#include "MPITest.h"
#endif

////////////////////////////////////////////////////////////////////////////////

long MemoryInfo::memoryAllocated = 0;
long MemoryInfo::maxMemoryAllocated = 0;
long MemoryInfo::totalAllocations = 0;
long MemoryInfo::totalDeallocations = 0;

////////////////////////////////////////////////////////////////////////////////

#include "../AcyclicSubset/IncidenceGraph.h"
#include "../AcyclicSubset/OutputGraph.h"

std::map<int, int> allocationsCounters;
std::map<int, int> deallocationsCounters;

bool CountSize(int size)
{
    if (size == sizeof(IncidenceGraph)) return true;
    if (size == sizeof(IncidenceGraph::Node)) return true;
    if (size == sizeof(IncidenceGraph::Edge)) return true;
    if (size == sizeof(OutputGraph)) return true;
    if (size == sizeof(OutputGraph::Node)) return true;
#ifdef DEBUG_MEMORY_VERTEX
    if (size == sizeof(DebugMemoryVertex)) return true;
#endif
    return false;
}

void CountAllocation(int size)
{
    if (CountSize(size))
    {
        allocationsCounters[size]++;
    }
}

void CountDeallocation(int size)
{
    if (CountSize(size))
    {
        deallocationsCounters[size]++;
    }
}

void PrintCounters()
{
#define PC(a) std::cout<<#a<<" allocated: "<<allocationsCounters[sizeof(a)]<<" deallocated: "<<deallocationsCounters[sizeof(a)]<<std::endl;
    PC(IncidenceGraph)
    PC(IncidenceGraph::Node)
    PC(IncidenceGraph::Edge)
    PC(OutputGraph)
    PC(OutputGraph::Node)
#ifdef DEBUG_MEMORY_VERTEX
    PC(DebugMemoryVertex)
#endif
#undef PC
}

////////////////////////////////////////////////////////////////////////////////

void MemoryInfo::Alloc(int size)
{
 //   std::cout<<"allocating: "<<size<<std::endl;
#ifdef DEBUG_MEMORY
    memoryAllocated += size;
    totalAllocations += size;
    if (memoryAllocated > maxMemoryAllocated)
    {
        maxMemoryAllocated = memoryAllocated;
    }
    CountAllocation(size);
#endif
}

void MemoryInfo::Dealloc(int size)
{
//    std::cout<<"deallocating: "<<size<<std::endl;
#ifdef DEBUG_MEMORY
    memoryAllocated -= size;
    totalDeallocations += size;
    CountDeallocation(size);
#endif
}

void MemoryInfo::PrintInfo(bool full)
{
#ifdef DEBUG_MEMORY
#define MINF(a) (a / (1024 * 1024))
#ifdef USE_MPI
    std::cout<<"process "<<MPITest::GetProcessRank();
#endif
    std::cout<<"memory: "<<MINF(memoryAllocated)<<" max: "<<MINF(maxMemoryAllocated);
    std::cout<<" alloc: "<<MINF(totalAllocations)<<" dealloc: "<<MINF(totalDeallocations)<<std::endl;
    if (full)
    {
        PrintCounters();
    }
#undef MINF
#endif
}

////////////////////////////////////////////////////////////////////////////////
// eof
