/*
 * File:   Utils.cpp
 * Author: Piotr Brendel
 */

#include "Utils.h"

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
    std::cout<<msg<<" : "<<s<<std::endl;
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
    std::cout<<msg<<" : "<<s<<std::endl;
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
    std::cout<<msg<<" : "<<(float(clock() - timeStart) / CLOCKS_PER_SEC)<<std::endl;
#endif
}

////////////////////////////////////////////////////////////////////////////////

void MemoryInfo::Print()
{
    std::cout<<"memory usage: "<<GetUsage()<<" MB"<<std::endl;
}

int MemoryInfo::GetUsage()
{
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return (usage.ru_maxrss >> 10);

}

////////////////////////////////////////////////////////////////////////////////
// eof
