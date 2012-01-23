/*
 * File:   IncidenceGraphAlgorithms.hpp
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 */

#ifndef INCIDENCEGRAPHALGORITHMS_HPP
#define INCIDENCEGRAPHALGORITHMS_HPP

#include <queue>
#include <map>
#include <algorithm>

////////////////////////////////////////////////////////////////////////////////

template <typename IncidenceGraph>
class FindNodeNotOnBorder
{
    typedef typename IncidenceGraph::Node Node;

public:

    bool FoundNode(Node *node)
    {
        return !node->IsOnBorder();
    }
};

template <typename IncidenceGraph>
class FindNodeNotInAccSub
{
    typedef typename IncidenceGraph::Node Node;

public:

    bool FoundNode(Node *node)
    {
        return !node->IsInAccSub();
    }
};

template <typename IncidenceGraph>
class FindNodeWithVertex
{
    typedef typename IncidenceGraph::Vertex Vertex;
    typedef typename IncidenceGraph::Node Node;

    Vertex vertex;

public:

    FindNodeWithVertex(Vertex v) : vertex(v) { }

    bool FoundNode(Node *node)
    {
        return (std::find(node->simplex->begin(), node->simplex->end(), vertex) != node->simplex->end());
    }
};

////////////////////////////////////////////////////////////////////////////////

template <typename IncidenceGraph, template<typename> class FindOptions>
typename IncidenceGraph::Node *FindNode(typename IncidenceGraph::ConnectedComponent connectedComponent, FindOptions<IncidenceGraph> findOptions)
{
    typedef typename IncidenceGraph::Node Node;
    typedef typename IncidenceGraph::Nodes Nodes;
    typedef typename IncidenceGraph::Edges Edges;

    Nodes tmp;
    std::queue<Node *> Q;
    tmp.push_back(connectedComponent);
    connectedComponent->IsHelperFlag1(true);
    Q.push(connectedComponent);
    Node *foundNode = 0;
    while (foundNode == 0 && !Q.empty())
    {
        Node *currentNode = Q.front();
        Q.pop();
        if (findOptions.FoundNode(currentNode))
        {
            foundNode = currentNode;
        }
        else
        {
            for (typename Edges::iterator edge = currentNode->edges.begin(); edge != currentNode->edges.end(); edge++)
            {
                Node *neighbour = (*edge)->GetNeighbour(currentNode);
                if (!neighbour->IsHelperFlag1())
                {
                    tmp.push_back(neighbour);
                    neighbour->IsHelperFlag1(true);
                    Q.push(neighbour);
                }
            }
        }
    }
    for (typename Nodes::iterator i = tmp.begin(); i != tmp.end(); i++)
    {
        (*i)->IsHelperFlag1(false);
    }
    return foundNode;
}

////////////////////////////////////////////////////////////////////////////////

template <typename IncidenceGraph, template<typename> class FindOptions>
typename IncidenceGraph::Node *FindNode(typename IncidenceGraph::Nodes &nodes, FindOptions<IncidenceGraph> findOptions)
{
    for (typename IncidenceGraph::Nodes::iterator node = nodes.begin(); node != nodes.end(); node++)
    {
        if (findOptions.FoundNode(*node))
        {
            return *node;
        }
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

template <typename IncidenceGraph>
class FindPathBase
{
    typedef typename IncidenceGraph::Node Node;

public:
    bool IsValidNeighbour(Node *n)
    {
        return true;
    }

    bool EndOfPath(Node *n)
    {
        return false;
    }
};

template <typename IncidenceGraph>
class FindPathToNode : public FindPathBase<IncidenceGraph>
{
    typedef typename IncidenceGraph::Node Node;

    Node *node;
    
public:
    
    FindPathToNode(Node *n)
    {
        node = n;
    }
            
    bool FoundNode(Node *n)
    {
        return (node == n);
    }
};

template <typename IncidenceGraph>
class FindPathToVertex : public FindPathBase<IncidenceGraph>
{
    typedef typename IncidenceGraph::Vertex Vertex;
    typedef typename IncidenceGraph::Node Node;

    Vertex vertex;

public:

    FindPathToVertex(Vertex v)
    {
        vertex = v;
    }

    bool FoundNode(Node *n)
    {
        return (std::find(n->simplex->begin(), n->simplex->end(), vertex) != n->simplex->end());
    }
};

template <typename IncidenceGraph>
class FindPathToNodeInAccSub : public FindPathBase<IncidenceGraph>
{
    typedef typename IncidenceGraph::Node Node;

public:
    bool FoundNode(Node *n)
    {
        return n->IsInAccSub();
    }
};

template <typename IncidenceGraph>
class FindPathToNodeWithAccIntersection : public FindPathBase<IncidenceGraph>
{
    typedef typename IncidenceGraph::Node Node;

public:
    bool FoundNode(Node *n)
    {
        return (n->GetAccInfo().HasIntersectionWithAccSub());
    }
};

template <typename IncidenceGraph>
class FindPathToNodeNotInAccSub : public FindPathBase<IncidenceGraph>
{
    typedef typename IncidenceGraph::Node Node;

public:
    bool FoundNode(Node *n)
    {
        return (!n->GetAccInfo().HasIntersectionWithAccSub() && !n->IsInAccSub());
    }

    bool EndOfPath(Node *n)
    {
        return n->IsInAccSub();
    }
};

template <typename IncidenceGraph>
class FindPathToNodeNotInAccSubNorOnBorder : public FindPathBase<IncidenceGraph>
{
    typedef typename IncidenceGraph::Node Node;

public:
    bool FoundNode(Node *n)
    {
        return (!n->GetAccInfo().HasIntersectionWithAccSub() && !n->IsInAccSub() && !n->IsOnBorder());
    }

    bool IsValidNeighbour(Node *n)
    {
        return !n->IsOnBorder();
    }

    bool EndOfPath(Node *n)
    {
        return n->IsInAccSub();
    }
};

////////////////////////////////////////////////////////////////////////////////

template <typename IncidenceGraph, template<typename> class FindOptions>
typename IncidenceGraph::Path FindPath(typename IncidenceGraph::Node *firstNode, FindOptions<IncidenceGraph> findOptions)
{
    typedef typename IncidenceGraph::Node Node;
    typedef typename IncidenceGraph::Nodes Nodes;
    typedef typename IncidenceGraph::Edges Edges;
    typedef typename IncidenceGraph::Path Path;

    std::queue<Node *> Q;
    Nodes tmp;
    firstNode->IsHelperFlag1(true);
    Q.push(firstNode);
    tmp.push_back(firstNode);
    Node *foundNode = 0;
    std::map<Node *, Node *> pred;
    while (!Q.empty() && foundNode == 0)
    {
        Node *node = Q.front();
        Q.pop();
        if (findOptions.FoundNode(node))
        {
            foundNode = node;
        }
        else
        {
            for (typename Edges::iterator edge = node->edges.begin(); edge != node->edges.end(); edge++)
            {
                Node *neighbour = (*edge)->GetNeighbour(node);
                if (!neighbour->IsHelperFlag1() && findOptions.IsValidNeighbour(neighbour))
                {
                    Q.push(neighbour);
                    tmp.push_back(neighbour);
                    neighbour->IsHelperFlag1(true);
                    pred[neighbour] = node;
                }
            }
        }
    }

    for (typename Nodes::iterator i = tmp.begin(); i != tmp.end(); i++)
    {
        (*i)->IsHelperFlag1(false);
    }

    Path path;
    if (foundNode != 0)
    {
        path.push_front(foundNode);
        Node *n = foundNode;
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

template <typename IncidenceGraph>
class RemoveNodesWithFlags
{
    typedef typename IncidenceGraph::Node Node;
    typedef typename IncidenceGraph::Node::PropertiesFlags PropertiesFlags;

    PropertiesFlags flags;

public:

    RemoveNodesWithFlags(PropertiesFlags f)
    {
        flags = f;
    }

    bool IsTrue(Node *node)
    {
        return (node->GetPropertiesFlags() & flags) != 0;
    }
};

template <typename IncidenceGraph, template<typename> class RemovePredicate>
void RemoveNodesWithPredicate(IncidenceGraph *graph, RemovePredicate<IncidenceGraph> predicate)
{
    typedef typename IncidenceGraph::Nodes Nodes;
    typedef typename IncidenceGraph::Edges Edges;

    Edges newEdges;
    for (typename Edges::iterator i = graph->edges.begin(); i != graph->edges.end(); i++)
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
    Nodes newNodes;
    for (typename Nodes::iterator i = graph->nodes.begin(); i != graph->nodes.end(); i++)
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

template <typename VertexVector, typename VertexSet>
void GetSortedIntersectionOfUnsortedSets(VertexVector &intersection, const VertexSet &setA, const VertexSet &setB)
{
    VertexVector va;
    va.assign(setA.begin(), setA.end());
    std::sort(va.begin(), va.end());
    VertexVector vb;
    vb.assign(setB.begin(), setB.end());
    std::sort(vb.begin(), vb.end());

    if (intersection.size() > 0)
    {
        intersection.clear();
    }

    if (va.size() == 0 || vb.size() == 0) return;
    if (va.front() > vb.back()) return;
    if (va.back() < vb.front()) return;

    typename VertexVector::iterator ia = va.begin();
    typename VertexVector::iterator ib = vb.begin();
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

template <typename VertexVector, typename VertexSet>
void GetIntersectionOfUnsortedSetAndSortedVector(VertexSet &intersection, const VertexSet &setA, const VertexVector &vb)
{
    VertexVector va;
    va.assign(setA.begin(), setA.end());
    std::sort(va.begin(), va.end());

    if (intersection.size() > 0)
    {
        intersection.clear();
    }

    if (va.size() == 0 || vb.size() == 0) return;
    if (va.front() > vb.back()) return;
    if (va.back() < vb.front()) return;

    typename VertexVector::iterator ia = va.begin();
    typename VertexVector::const_iterator ib = vb.begin();
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

