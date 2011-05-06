#ifndef INCIDENCEGRAPHUTILS_H
#define INCIDENCEGRAPHUTILS_H

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
                if (!edge->node->IsHelperFlag1())
                {
                    tmp.push_back(edge->node);
                    edge->node->IsHelperFlag1(true);
                    L.push(edge->node);
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
                if (!edge->node->IsHelperFlag1() && findOptions.IsValidNeighbour(edge->node))
                {
                    L.push(edge->node);
                    tmp.push_back(edge->node);
                    edge->node->IsHelperFlag1(true);
                    pred[edge->node] = node;
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

void CreateConfigurationsFlags(int maxDim, std::map<Simplex, IncidenceGraph::IntersectionFlags> &configurationsFlags, std::map<Simplex, IncidenceGraph::IntersectionFlags> &subconfigurationsFlags);
void GetSortedIntersectionOfUnsortedSets(std::vector<Vertex> &intersection, const std::set<Vertex> &setA, const std::set<Vertex> &setB);

////////////////////////////////////////////////////////////////////////////////

#endif /* INCIDENCEGRAPHUTILS_H */

