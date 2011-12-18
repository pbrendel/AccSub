/*
 * File:   Utils.h
 * Author: Piotr Brendel
 */

#ifndef UTILS_H
#define UTILS_H

#include <fstream>
#include <ostream>
#include <iostream>
#include <ctime>

#include "../AcyclicSubset/Simplex.h"
#include "../AcyclicSubset/IncidenceGraph.h"
#include "../AcyclicSubset/OutputGraph.h"

#ifdef USE_MPI
#include <mpi.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// Debug

class Debug
{

public:

    static void Print(std::ostream &str, Simplex &simplex);
    static void Print(std::ostream &str, Simplex *simplex);
    static void Print(std::ostream &str, VertsSet &vertsSet);
    static void Print(std::ostream &str, SimplexList &simplexList);
    static void Print(std::ostream &str, SimplexPtrList &simplexPtrList);
    static void Print(std::ostream &str, IncidenceGraph::Nodes &nodes);
    static void Print(std::ostream &str, IncidenceGraph::Path &path);
    static void Print(std::ostream &str, IncidenceGraph::VertexHash &m);
    static void PrintAccSub(std::ostream &str, IncidenceGraph::Nodes &nodes);
    static void PrintGraph(std::ostream &str, IncidenceGraph::Nodes &nodes);
    static void Print(std::ostream &str, std::map<Simplex, IncidenceGraph::IntersectionFlags> &flags);
    static void Print(std::ostream &str, OutputGraph &graph);
    static void PrintIntersectionFromFlags(std::ostream &str, std::map<Simplex, IncidenceGraph::IntersectionFlags> &flagsMap, IncidenceGraph::IntersectionFlags flags);
};

////////////////////////////////////////////////////////////////////////////////

class Timer
{

public:

#ifdef USE_MPI
    typedef double Time;
#else
    typedef clock_t Time;
#endif

private:
    
    static Time now;
    static Time timeStart;

public:

    static void Init();
    static float Update(const char *);
    static float Update();
    static Time Now();
    static float TimeFrom(Time);
    static float TimeFrom(Time, const char *);
    static void TimeStamp(const char *);
    
};

////////////////////////////////////////////////////////////////////////////////

class MemoryInfo
{

public:

    static void Print();
    static int GetUsage();

};

////////////////////////////////////////////////////////////////////////////////

#endif /* UTILS_H */
