/*
 * File:   Utils.cpp
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 */

#include "Utils.hpp"

#include <limits>
#include <sys/resource.h>

#ifdef USE_MPI
#include <mpi.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// Timer

Timer::Time Timer::now = 0;
Timer::Time Timer::timeStart = 0;

void Timer::Init()
{
#ifdef USE_MPI
    now = MPI_Wtime();
    timeStart = MPI_Wtime();
#else
    now = clock();
    timeStart = clock();
#endif
}

float Timer::Update(const char *msg)
{
#ifdef USE_MPI
    double t = MPI_Wtime();
    float s = float(t - now);
#else
    clock_t t = clock();
    float s = float(t - now) / CLOCKS_PER_SEC;
#endif
    std::cout<<msg<<" : "<<s<<"s"<<std::endl;
    now = t;
    return s;
}

float Timer::Update()
{
#ifdef USE_MPI
    double t = MPI_Wtime();
    float s = float(t - now);
#else
    clock_t t = clock();
    float s = float(t - now) / CLOCKS_PER_SEC;
#endif
    now = t;
    return s;
}

Timer::Time Timer::Now()
{
    return now;
}

float Timer::TimeFrom(Timer::Time t, const char *msg)
{
#ifdef USE_MPI
    float s = float(MPI_Wtime() - t);
#else
    float s = float(clock() - t) / CLOCKS_PER_SEC;
#endif
    std::cout<<msg<<" : "<<s<<"s"<<std::endl;
    return s;
}

float Timer::TimeFrom(Timer::Time t)
{
#ifdef USE_MPI
    float s = float(MPI_Wtime() - t);
#else
    float s = float(clock() - t) / CLOCKS_PER_SEC;
#endif
    return s;
}

void Timer::TimeStamp(const char* msg)
{
#ifdef USE_MPI
    std::cout<<msg<<" : "<<float(MPI_Wtime() - timeStart)<<std::endl;
#else
    std::cout<<msg<<" : "<<(float(clock() - timeStart) / CLOCKS_PER_SEC)<<"s"<<std::endl;
#endif
}

////////////////////////////////////////////////////////////////////////////////

std::map<int, int> MemoryInfo::slavesMemoryInfo;
int MemoryInfo::maxUsage = 0;

void MemoryInfo::Print()
{
    int usage = GetUsage();
    std::cout<<"memory usage: "<<usage<<" MB    max: "<<GetMaxUsage()<<std::endl;
}

int MemoryInfo::GetUsage()
{
#ifdef LINUX
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    int mu = usage.ru_maxrss >> 10;
    maxUsage = (mu > maxUsage) ? mu : maxUsage;
    return mu;
#else
    return 0;
#endif
}

int MemoryInfo::GetMaxUsage()
{
    return maxUsage;
}

void MemoryInfo::AddSlavesMemoryInfo(int rank, int mem)
{
    slavesMemoryInfo[rank] = mem;
}

void MemoryInfo::PrintSlavesMemoryInfo()
{
    int count = 0;
    int total = 0;
    int min = std::numeric_limits<int>::max();
    int max = 0;

    for (std::map<int, int>::iterator i = slavesMemoryInfo.begin(); i != slavesMemoryInfo.end(); i++)
    {
        count++;
        std::cout<<"process: "<<i->first<<" memory: "<<i->second<<std::endl;
        int mem = i->second;
        total += mem;
        if (mem < min) min = mem;
        if (mem > max) max = mem;
    }

    if (count > 0)
    {
        std::cout<<"min: "<<min<<std::endl;
        std::cout<<"max: "<<max<<std::endl;
        std::cout<<"avg: "<<(total / count)<<std::endl;
    }
}

void MemoryInfo::ClearSlavesMemoryInfo()
{
    slavesMemoryInfo.clear();
}

////////////////////////////////////////////////////////////////////////////////
// eof
