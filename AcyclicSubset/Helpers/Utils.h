#ifndef _UTILS_H_
#define _UTILS_H_

#include <fstream>
#include <ostream>
#include <iostream>
#include <ctime>

#include "../AcyclicSubset/Simplex.h"
#include "../AcyclicSubset/IncidenceGraph.h"
#include "../AcyclicSubset/OutputGraph.h"

////////////////////////////////////////////////////////////////////////////////
// Debug

class Debug
{

public:

    static void Print(std::ostream &str, Simplex &simplex);
    static void Print(std::ostream &str, Simplex *simplex);
    static void Print(std::ostream &str, IncidenceGraph::VertsSet vertsSet);
    static void Print(std::ostream &str, SimplexList &simplexList);
    static void Print(std::ostream &str, IncidenceGraph::Nodes &nodes);
    static void Print(std::ostream &str, IncidenceGraph::Path &path);
    static void Print(std::ostream &str, IncidenceGraph::IntNodesMap &m);
    static void PrintAcyclicSet(std::ostream &str, IncidenceGraph::Nodes &nodes);
    static void PrintGraph(std::ostream &str, IncidenceGraph::Nodes &nodes);
    static void Print(std::ostream &str, std::map<Simplex, IncidenceGraph::IntersectionFlags> &flags);
    static void Print(std::ostream &str, OutputGraph &graph);
    static void PrintIntersectionFromFlags(std::ostream &str, std::map<Simplex, IncidenceGraph::IntersectionFlags> &flagsMap, IncidenceGraph::IntersectionFlags flags);
};

////////////////////////////////////////////////////////////////////////////////
// Log

#define USE_LOG

class Log
{
  
public: 
    
    static std::fstream stream;    
    static void Init(const char *);
    static void Close();
            
};

////////////////////////////////////////////////////////////////////////////////

class Timer
{

    static clock_t now;

public:

    static void Init();
    static float Update(const char *);
    static float Update();
    static clock_t Now();
    static float TimeFrom(clock_t);
    static float TimeFrom(clock_t, const char *);
    
};

////////////////////////////////////////////////////////////////////////////////

#endif // _UTILS_H_
