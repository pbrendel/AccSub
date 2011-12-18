/*
 * File:   Utils.cpp
 * Author: Piotr Brendel
 */

#include "Utils.h"

#include <iostream>
#include <algorithm>

#include <sys/resource.h>

#ifdef USE_MPI
#include <mpi.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// Debug

void Debug::Print(std::ostream &str, Simplex &simplex)
{
    for (Simplex::iterator i = simplex.begin(); i != simplex.end(); i++)
    {
        str<<(*i)<<" ";
    }
    str<<std::endl;
}

void Debug::Print(std::ostream &str, Simplex *simplex)
{
    for (Simplex::iterator i = simplex->begin(); i != simplex->end(); i++)
    {
        str<<(*i)<<" ";
    }
    str<<std::endl;
}

void Debug::Print(std::ostream &str, VertsSet &vertsSet)
{
    for (VertsSet::iterator i = vertsSet.begin(); i != vertsSet.end(); i++)
    {
        str<<(*i)<<" ";
    }
    str<<std::endl;
}

void Debug::Print(std::ostream &str, SimplexList &simplexList)
{
    int index = 0;
    for (SimplexList::iterator i = simplexList.begin(); i != simplexList.end(); i++)
    {
        str<<index++<<" : ";
        for (Simplex::iterator j = (*i).begin(); j < (*i).end(); j++)
        {
            str<<(*j)<<" ";
        }        
        str<<std::endl;
    }
}

void Debug::Print(std::ostream &str, SimplexPtrList &simplexPtrList)
{
    int index = 0;
    for (SimplexPtrList::iterator i = simplexPtrList.begin(); i != simplexPtrList.end(); i++)
    {
        str<<index++<<" : ";
        for (Simplex::iterator j = (*i)->begin(); j < (*i)->end(); j++)
        {
            str<<(*j)<<" ";
        }
        str<<std::endl;
    }
}

void Debug::Print(std::ostream &str, IncidenceGraph::Nodes &nodes)
{
    for (IncidenceGraph::Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        Print(str, (*i)->simplex);
    }
}

void Debug::Print(std::ostream &str, IncidenceGraph::Path &path)
{
    for (IncidenceGraph::Path::iterator i = path.begin(); i != path.end(); i++)
    {
        Print(str, (*i)->simplex);
    }
}

void Debug::Print(std::ostream &str, IncidenceGraph::VertexHash &m)
{
#ifdef DEBUG
    for (IncidenceGraph::VertexNodesMap::iterator i = m.begin(); i != m.end(); i++)
    {
        str<<i->first<<":"<<std::endl;
        for (IncidenceGraph::Nodes::iterator j = i->second.begin(); j != i->second.end(); j++)
        {
            str<<(*j)->index<<", ";
        }   
        str<<std::endl;
    }
#endif
}

void Debug::PrintAccSub(std::ostream &str, IncidenceGraph::Nodes &nodes)
{
    for (IncidenceGraph::Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        if ((*i)->IsInAccSub())
        {
            Print(str, *((*i)->simplex));
        }
    }   
    str<<std::endl;
}

void Debug::PrintGraph(std::ostream &str, IncidenceGraph::Nodes &nodes)
{
//    for (IncidenceGraph::Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
//    {
//        IncidenceGraph::Node *n = (*i);
//        str<<"vertex: "<<n->index<<":"<<std::endl;
//        str<<"neighbours: ";
//        for (IncidenceGraph::Edges::iterator j = n->edges.begin(); j != n->edges.end(); j++)
//        {
//            str<<j->node->index<<", ";
//        }
//       str<<std::endl;
//    }
}

void Debug::Print(std::ostream &str, std::map<Simplex, IncidenceGraph::IntersectionFlags> &flags)
{
    for (std::map<Simplex, IncidenceGraph::IntersectionFlags>::iterator i = flags.begin(); i != flags.end(); i++)
    {
        str<<"configuration: ";
        Print(str, const_cast<Simplex &>(i->first));
        str<<"flags: "<<std::hex<<i->second<<std::endl;
    }
}      

void Debug::Print(std::ostream &str, OutputGraph &graph)
{
    std::sort(graph.nodes.begin(), graph.nodes.end(), OutputGraph::Node::Sorter);
    for (OutputGraph::Nodes::iterator i = graph.nodes.begin(); i != graph.nodes.end(); i++)
    {
        str<<"node "<<(*i)->index<<" : ";
        Print(str, (*i)->simplex);    
        
        for (OutputGraph::Nodes::iterator j = (*i)->subnodes.begin(); j != (*i)->subnodes.end(); j++)
        {
            str<<"    "<<(*j)->index<<" : ";    
            Print(str, (*j)->simplex);
        }        
    }        
}

void Debug::PrintIntersectionFromFlags(std::ostream &str, std::map<Simplex, IncidenceGraph::IntersectionFlags> &flagsMap, IncidenceGraph::IntersectionFlags flags)
{
    for (std::map<Simplex, IncidenceGraph::IntersectionFlags>::iterator i = flagsMap.begin(); i != flagsMap.end(); i++)
    {
        if (i->second & flags)
        {
            str<<"configuration: ";
            Print(str, const_cast<Simplex &>(i->first));
            str<<"flags: "<<std::dec<<i->second<<std::endl<<std::endl;
        }
    }
}

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
