/* 
 * File:   DebugMemory.h
 * Author: Piotr Brendel
 */

#ifndef DEBUGMEMORY_H
#define	DEBUGMEMORY_H

#include <map>

class MemoryInfo
{
    static long memoryAllocated;
    static long maxMemoryAllocated;
    static long totalAllocations;
    static long totalDeallocations;

    static std::map<int, int> slavesMemoryInfo;
    static int slavesMinMemory;
    static int slavesMaxMemory;

public:

    static void Alloc(int);
    static void Dealloc(int);

    static void PrintInfo(bool = false);
    static void Reset(bool = false);

    static int GetMaxMemoryAllocated() { return maxMemoryAllocated; }

    static void AddSlaveMemoryInfo(int, int);
    static void PrintSlavesMemoryInfo();
};

template <typename T>
class DebugMemory
{
public:

    DebugMemory()
    {
        MemoryInfo::Alloc(sizeof(T));
    }

    DebugMemory(const DebugMemory &)
    {
        MemoryInfo::Alloc(sizeof(T));
    }

    ~DebugMemory()
    {
        MemoryInfo::Dealloc(sizeof(T));
    }

};

#endif	/* DEBUGMEMORY_H */

