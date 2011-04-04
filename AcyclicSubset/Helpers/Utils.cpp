#include "Utils.h"

#include <iostream>
#include <algorithm>

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

void Debug::Print(std::ostream &str, IncidenceGraph::VertsSet vertsSet)
{
    for (IncidenceGraph::VertsSet::iterator i = vertsSet.begin(); i != vertsSet.end(); i++)
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

void Debug::Print(std::ostream &str, IncidenceGraph::IntNodesMap &m)
{
#ifdef DEBUG
    for (IncidenceGraph::IntNodesMap::iterator i = m.begin(); i != m.end(); i++)
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

void Debug::PrintAcyclicSet(std::ostream &str, IncidenceGraph::Nodes &nodes)
{
    for (IncidenceGraph::Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        if ((*i)->IsAcyclic())
        {
            Print(str, *((*i)->simplex));
        }
    }   
    str<<std::endl;
}

void Debug::PrintGraph(std::ostream &str, IncidenceGraph::Nodes &nodes)
{
#ifdef DEBUG
    for (IncidenceGraph::Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        IncidenceGraph::Node *n = (*i);
        str<<"vertex: "<<n->index<<":"<<std::endl;
        str<<"neighbours: ";
        for (IncidenceGraph::Edges::iterator j = n->edges.begin(); j != n->edges.end(); j++)
        {
            str<<j->node->index<<", ";
        }        
       str<<std::endl;
    }
#endif
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
// Log

std::fstream Log::stream;

void Log::Init(const char *filename)
{
    stream.open(filename, std::ios::out);
    stream<<" --- Log started --- "<<std::endl;
}

void Log::Close()
{
    stream<<" --- Log finished --- "<<std::endl;        
}

////////////////////////////////////////////////////////////////////////////////
// Timer

clock_t Timer::now = 0;

void Timer::Init()
{
    now = clock();
}

float Timer::Update(const char *msg)
{
    clock_t t = clock();
    float s = float(t - now)/ CLOCKS_PER_SEC;
    std::cout<<msg<<" : "<<s<<std::endl;
    now = t;
    return s;
}

float Timer::Update()
{
    clock_t t = clock();
    float s = float(t - now)/ CLOCKS_PER_SEC;
    now = t;
    return s;
}

clock_t Timer::Now()
{
    return now;
}

float Timer::TimeFrom(clock_t t, const char *msg)
{
    float s = float(clock() - t)/ CLOCKS_PER_SEC;
    std::cout<<msg<<" : "<<s<<std::endl;
    return s;
}

float Timer::TimeFrom(clock_t t)
{
    float s = float(clock() - t)/ CLOCKS_PER_SEC;
    return s;
}

////////////////////////////////////////////////////////////////////////////////
// eof
