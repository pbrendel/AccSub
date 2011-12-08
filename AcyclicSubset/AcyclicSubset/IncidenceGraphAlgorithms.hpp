/*
 * File:   IncidenceGraphAlgorithms.hpp
 * Author: Piotr Brendel
 */

#ifndef INCIDENCEGRAPHALGORITHMS_HPP
#define INCIDENCEGRAPHALGORITHMS_HPP

#include "IncidenceGraph.h"
#include <algorithm>

////////////////////////////////////////////////////////////////////////////////

class FindNodeNotOnBorder
{
public:

    bool FoundNode(IncidenceGraph::Node *node)
    {
        return !node->IsOnBorder();
    }
};

class FindNotAcyclicNode
{
public:

    bool FoundNode(IncidenceGraph::Node *node)
    {
        return !node->IsAcyclic();
    }
};

class FindNodeWithVertex
{
    Vertex vertex;

public:

    FindNodeWithVertex(Vertex v) : vertex(v) { }

    bool FoundNode(IncidenceGraph::Node *node)
    {
        return (std::find(node->simplex->begin(), node->simplex->end(), vertex) != node->simplex->end());
    }
};

////////////////////////////////////////////////////////////////////////////////

template <typename FindOptions>
IncidenceGraph::Node *FindNode(IncidenceGraph::ConnectedComponent connectedComponent, FindOptions findOptions)
{
    IncidenceGraph::Nodes tmp;
    std::queue<IncidenceGraph::Node *> L;
    tmp.push_back(connectedComponent);
    connectedComponent->IsHelperFlag1(true);
    L.push(connectedComponent);
    IncidenceGraph::Node *foundNode = 0;
    while (foundNode == 0 && !L.empty())
    {
        IncidenceGraph::Node *currentNode = L.front();
        L.pop();
        if (findOptions.FoundNode(currentNode))
        {
            foundNode = currentNode;
        }
        else
        {
            for (IncidenceGraph::Edges::iterator edge = currentNode->edges.begin(); edge != currentNode->edges.end(); edge++)
            {
                IncidenceGraph::Node *neighbour = (*edge)->GetNeighbour(currentNode);
                if (!neighbour->IsHelperFlag1())
                {
                    tmp.push_back(neighbour);
                    neighbour->IsHelperFlag1(true);
                    L.push(neighbour);
                }
            }
        }
    }
    for (IncidenceGraph::Nodes::iterator i = tmp.begin(); i != tmp.end(); i++)
    {
        (*i)->IsHelperFlag1(false);
    }
    return foundNode;
}

////////////////////////////////////////////////////////////////////////////////

template <typename FindOptions>
IncidenceGraph::Node *FindNode(IncidenceGraph::Nodes &nodes, FindOptions findOptions)
{
    for (IncidenceGraph::Nodes::iterator node = nodes.begin(); node != nodes.end(); node++)
    {
        if (findOptions.FoundNode(*node))
        {
            return *node;
        }
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

class FindPathBase
{
public:
    bool IsValidNeighbour(IncidenceGraph::Node *n)
    {
        return true;
    }

    bool EndOfPath(IncidenceGraph::Node *n)
    {
        return false;
    }
};

class FindPathToNode : public FindPathBase
{
    IncidenceGraph::Node *node;
    
public:
    
    FindPathToNode(IncidenceGraph::Node *n)
    {
        node = n;
    }
            
    bool FoundNode(IncidenceGraph::Node *n)
    {
        return (node == n);
    }
};

class FindPathToVertex : public FindPathBase
{
    Vertex vertex;

public:

    FindPathToVertex(Vertex v)
    {
        vertex = v;
    }

    bool FoundNode(IncidenceGraph::Node *n)
    {
        return (std::find(n->simplex->begin(), n->simplex->end(), vertex) != n->simplex->end());
    }
};

class FindPathToAcyclicNode : public FindPathBase
{
public:
    bool FoundNode(IncidenceGraph::Node *n)
    {
        return n->IsAcyclic();
    }
};

class FindPathToNodeWithAcyclicIntersection : public FindPathBase
{
public:
    bool FoundNode(IncidenceGraph::Node *n)
    {
        return (n->GetAcyclicIntersectionFlags() != 0);
    }
};

class FindPathToNodeNotInAcyclicSubset : public FindPathBase
{
public:
    bool FoundNode(IncidenceGraph::Node *n)
    {
        return (!n->GetAcyclicIntersectionFlags() && !n->IsAcyclic());
    }

    bool EndOfPath(IncidenceGraph::Node *n)
    {
        return n->IsAcyclic();
    }
};

class FindPathToNodeNotInAcyclicSubsetNorOnBorder : public FindPathBase
{
public:
    bool FoundNode(IncidenceGraph::Node *n)
    {
        return (!n->GetAcyclicIntersectionFlags() && !n->IsAcyclic() && !n->IsOnBorder());
    }

    bool IsValidNeighbour(IncidenceGraph::Node *n)
    {
        return !n->IsOnBorder();
    }

    bool EndOfPath(IncidenceGraph::Node *n)
    {
        return n->IsAcyclic();
    }
};

////////////////////////////////////////////////////////////////////////////////

template <typename FindOptions>
IncidenceGraph::Path FindPath(IncidenceGraph::Node *firstNode, FindOptions findOptions)
{
    std::queue<IncidenceGraph::Node *> L;
    IncidenceGraph::Nodes tmp;
    firstNode->IsHelperFlag1(true);
    L.push(firstNode);
    tmp.push_back(firstNode);
    IncidenceGraph::Node *foundNode = 0;
    std::map<IncidenceGraph::Node *, IncidenceGraph::Node *> pred;
    while (!L.empty() && foundNode == 0)
    {
        IncidenceGraph::Node *node = L.front();
        L.pop();
        if (findOptions.FoundNode(node))
        {
            foundNode = node;
        }
        else
        {
            for (IncidenceGraph::Edges::iterator edge = node->edges.begin(); edge != node->edges.end(); edge++)
            {
                IncidenceGraph::Node *neighbour = (*edge)->GetNeighbour(node);
                if (!neighbour->IsHelperFlag1() && findOptions.IsValidNeighbour(neighbour))
                {
                    L.push(neighbour);
                    tmp.push_back(neighbour);
                    neighbour->IsHelperFlag1(true);
                    pred[neighbour] = node;
                }
            }
        }
    }

    // zerujemy ustawione flagi pomocnicze
    for (IncidenceGraph::Nodes::iterator i = tmp.begin(); i != tmp.end(); i++)
    {
        (*i)->IsHelperFlag1(false);
    }

    IncidenceGraph::Path path;
    if (foundNode != 0)
    {
        path.push_front(foundNode);
        IncidenceGraph::Node *n = foundNode;
        while (n != firstNode && !findOptions.EndOfPath(n))
        {
            n = pred[n];
            path.push_front(n);
            assert(n != 0);
        }
    }
    return path;
}

////////////////////////////////////////////////////////////////////////////////

class RemoveNodesWithFlags
{
    IncidenceGraph::Node::Flags flags;

public:

    RemoveNodesWithFlags(IncidenceGraph::Node::Flags f)
    {
        flags = f;
    }

    bool IsTrue(IncidenceGraph::Node *node)
    {
        return (node->GetPropertiesFlags() & flags) != 0;
    }
};

template <typename RemovePredicate>
void RemoveNodesWithPredicate(IncidenceGraph *graph, RemovePredicate predicate)
{
    // na koncu wszystkie zaznaczone node'y i krawedzie usuwamy z grafu
    IncidenceGraph::Edges newEdges;
    for (IncidenceGraph::Edges::iterator i = graph->edges.begin(); i != graph->edges.end(); i++)
    {
        if (predicate.IsTrue((*i)->nodeA) || predicate.IsTrue((*i)->nodeB))
        {
            delete *i;
        }
        else
        {
            newEdges.push_back(*i);
        }
    }
    graph->edges = newEdges;
    IncidenceGraph::Nodes newNodes;
    for (IncidenceGraph::Nodes::iterator i = graph->nodes.begin(); i != graph->nodes.end(); i++)
    {
        if (predicate.IsTrue(*i))
        {
            delete *i;
        }
        else
        {
            newNodes.push_back(*i);
        }
    }
    graph->nodes = newNodes;
}

////////////////////////////////////////////////////////////////////////////////

template <typename VertexT>
void GetSortedIntersectionOfUnsortedSets(std::vector<VertexT> &intersection, const std::set<VertexT> &setA, const std::set<VertexT> &setB)
{
    std::vector<VertexT> va;
    va.assign(setA.begin(), setA.end());
    std::sort(va.begin(), va.end());
    std::vector<VertexT> vb;
    vb.assign(setB.begin(), setB.end());
    std::sort(vb.begin(), vb.end());

    if (intersection.size() > 0)
    {
        intersection.clear();
    }

    if (va.size() == 0 || vb.size() == 0) return;
    if (va.front() > vb.back()) return;
    if (va.back() < vb.front()) return;

    typename std::vector<VertexT>::iterator ia = va.begin();
    typename std::vector<VertexT>::iterator ib = vb.begin();
    while (ia != va.end() && ib != vb.end())
    {
        if (*ia < *ib) ia++;
        else if (*ib < *ia) ib++;
        else
        {
            intersection.push_back(*ia);
            ia++;
            ib++;
        }
    }
}

template <typename VertexT>
void GetIntersectionOfUnsortedSetAndSortedVector(std::set<VertexT> &intersection, const std::set<VertexT> &setA, const std::vector<VertexT> &vb)
{
    std::vector<VertexT> va;
    va.assign(setA.begin(), setA.end());
    std::sort(va.begin(), va.end());

    if (intersection.size() > 0)
    {
        intersection.clear();
    }

    if (va.size() == 0 || vb.size() == 0) return;
    if (va.front() > vb.back()) return;
    if (va.back() < vb.front()) return;

    typename std::vector<VertexT>::iterator ia = va.begin();
    typename std::vector<VertexT>::const_iterator ib = vb.begin();
    while (ia != va.end() && ib != vb.end())
    {
        if (*ia < *ib) ia++;
        else if (*ib < *ia) ib++;
        else
        {
            intersection.insert(*ia);
            ia++;
            ib++;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

#endif /* INCIDENCEGRAPHALGORITHMS_HPP */

