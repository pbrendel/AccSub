/* 
 * File:   DebugMemory.h
 * Author: Piotr Brendel
 */

#ifndef DEBUGMEMORY_H
#define	DEBUGMEMORY_H

class MemoryInfo
{
    static long memoryAllocated;
    static long maxMemoryAllocated;
    static long totalAllocations;
    static long totalDeallocations;

public:

    static void Alloc(int);
    static void Dealloc(int);

    static void PrintInfo(bool = false);
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

