/*
 * File:   Utils.hpp
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <fstream>
#include <ostream>
#include <iostream>
#include <ctime>
#include <algorithm>
#include <map>

#ifdef USE_MPI
#include <mpi.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// Debug

class Debug
{

public:

    template <typename Simplex>
    static void PrintSimplex(std::ostream &str, const Simplex &simplex)
    {
        for (typename Simplex::const_iterator i = simplex.begin(); i != simplex.end(); i++)
        {
            str<<(*i)<<" ";
        }
        str<<std::endl;
    }

    template <typename Simplex>
    static void PrintSimplex(std::ostream &str, const Simplex *simplex)
    {
        for (typename Simplex::const_iterator i = simplex->begin(); i != simplex->end(); i++)
        {
            str<<(*i)<<" ";
        }
        str<<std::endl;
    }

    template <typename VertsSet>
    static void PrintVertsSet(std::ostream &str, const VertsSet &vertsSet)
    {
        for (typename VertsSet::const_iterator i = vertsSet.begin(); i != vertsSet.end(); i++)
        {
            str<<(*i)<<" ";
        }
        str<<std::endl;
    }

    template <typename SimplexList>
    static void PrintSimplexList(std::ostream &str, const SimplexList &simplexList)
    {
        int index = 0;
        for (typename SimplexList::const_iterator i = simplexList.begin(); i != simplexList.end(); i++)
        {
            str<<index++<<" : ";
            PrintSimplex(str, *i);
        }
    }

    template <typename SimplexPtrList>
    static void PrintSimplexPtrList(std::ostream &str, const SimplexPtrList &simplexPtrList)
    {
        int index = 0;
        for (typename SimplexPtrList::const_iterator i = simplexPtrList.begin(); i != simplexPtrList.end(); i++)
        {
            str<<index++<<" : ";
            PrintSimplex(str, *(*i));
        }
    }

    template <typename Nodes>
    static void PrintNodes(std::ostream &str, const Nodes &nodes)
    {
        for (typename Nodes::const_iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            PrintSimplex(str, (*i)->simplex);
        }
    }

    template <typename Path>
    static void PrintPath(std::ostream &str, const Path &path)
    {
        for (typename Path::const_iterator i = path.begin(); i != path.end(); i++)
        {
            PrintSimplex(str, (*i)->simplex);
        }
    }

    template <typename VertexHash>
    static void PrintVertexHash(std::ostream &str, const VertexHash &m)
    {
        for (typename VertexHash::const_iterator i = m.begin(); i != m.end(); i++)
        {
            str<<i->first<<":"<<std::endl;
            PrintNodes(str, i->second);
            str<<std::endl;
        }
    }

    template <typename Nodes>
    static void PrintAccSub(std::ostream &str, const Nodes &nodes)
    {
        for (typename Nodes::const_iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            if ((*i)->IsInAccSub())
            {
                PrintSimplex(str, *((*i)->simplex));
            }
        }
        str<<std::endl;
    }

    template <typename IncidenceGraph>
    static void PrintGraph(std::ostream &str, IncidenceGraph *graph)
    {
        for (typename IncidenceGraph::Nodes::iterator i = graph->nodes.begin(); i != graph->nodes.end(); i++)
        {
            typename IncidenceGraph::Node *n = (*i);
            str<<"vertex: "<<n->index<<":"<<std::endl;
            str<<"neighbours: ";
            for (typename IncidenceGraph::Edges::iterator j = n->edges.begin(); j != n->edges.end(); j++)
            {
                str<<j->node->index<<", ";
            }
           str<<std::endl;
        }
    }

    template <typename Simplex, typename IntersectionFlags>
    static void PrintIntersectionFlags(std::ostream &str, const std::map<Simplex, IntersectionFlags> &flags)
    {
        for (typename std::map<Simplex, IntersectionFlags>::const_iterator i = flags.begin(); i != flags.end(); i++)
        {
            str<<"configuration: ";
            PrintSimplex(str, i->first);
            str<<"flags: "<<std::hex<<i->second<<std::endl;
        }
    }

    template <typename OutputGraph>
    static void PrintOutputGraph(std::ostream &str, OutputGraph &graph)
    {
        std::sort(graph.nodes.begin(), graph.nodes.end(), OutputGraph::Node::Sorter);
        for (typename OutputGraph::Nodes::iterator i = graph.nodes.begin(); i != graph.nodes.end(); i++)
        {
            str<<"node "<<(*i)->index<<" : ";
            PrintSimplex(str, (*i)->simplex);

            for (typename OutputGraph::Nodes::iterator j = (*i)->subnodes.begin(); j != (*i)->subnodes.end(); j++)
            {
                str<<"    "<<(*j)->index<<" : ";
                Print(str, (*j)->simplex);
            }
        }
    }

    template <typename Simplex, typename IntersectionFlags>
    static void PrintIntersectionFromFlags(std::ostream &str, const std::map<Simplex, IntersectionFlags> &flagsMap, const IntersectionFlags flags)
    {
        for (typename std::map<Simplex, IntersectionFlags>::const_iterator i = flagsMap.begin(); i != flagsMap.end(); i++)
        {
            if (i->second & flags)
            {
                str<<"configuration: ";
                PrintSimplex(str, i->first);
                str<<"flags: "<<std::dec<<i->second<<std::endl<<std::endl;
            }
        }
    }
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

    static std::map<int, int> slavesMemoryInfo;
    static int maxUsage;

public:

    static void Print();
    static int GetUsage();
    static int GetMaxUsage();

    static void AddSlavesMemoryInfo(int rank, int mem);
    static void PrintSlavesMemoryInfo();
    static void ClearSlavesMemoryInfo();

};

////////////////////////////////////////////////////////////////////////////////

#endif /* UTILS_HPP */
