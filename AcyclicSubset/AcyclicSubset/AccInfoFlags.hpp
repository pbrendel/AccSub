/* 
 * File:   AccInfoFlags.hpp
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 */

#ifndef ACCINFOFLAGS_HPP
#define	ACCINFOFLAGS_HPP

#include "IntersectionFlagsHelpers.hpp"
#include <cassert>

template <typename IncidenceGraphType>
class AccInfoFlags
{
    typedef IncidenceGraphType IncidenceGraph;
    typedef typename IncidenceGraph::Vertex Vertex;
    typedef typename IncidenceGraph::VertsSet VertsSet;
    typedef typename IncidenceGraph::Simplex Simplex;
    typedef typename IncidenceGraph::Node Node;
    typedef typename IncidenceGraph::Edges Edges;
    typedef typename IncidenceGraph::IntersectionFlags IntersectionFlags;
    typedef typename IncidenceGraph::AccTest AccTest;

    Node                *node;
    IntersectionFlags   intersectionFlags;
    IntersectionFlags   intersectionFlagsMF;
    unsigned char       accSubID;

public:

    AccInfoFlags(Node *n)
    {
        node = n;
        intersectionFlags = 0;
        intersectionFlagsMF = 0;
        accSubID = 0;
    }

    bool IsAccIntersectionAcyclic(AccTest *accTest)
    {
        return accTest->IsAcyclic(*node->simplex, intersectionFlags, intersectionFlagsMF);
    }

    bool IsInsideAccIntersection(const IntersectionFlags &flags)
    {
        return ((flags & intersectionFlags) == flags);
    }

    bool IsDisjointWithAccIntersection(const IntersectionFlags &flags)
    {
        return ((flags & intersectionFlags) == 0);
    }

    bool IsVertexInAccIntersection(Vertex vertex)
    {
        return (intersectionFlags & (1 << node->NormalizeVertex(vertex)));
    }

    Vertex FindAccVertex()
    {
        if (intersectionFlags == 0)
        {
            return Vertex(-1);
        }
        for (typename Simplex::iterator v = node->simplex->begin(); v != node->simplex->end(); v++)
        {
            if (intersectionFlags & (1 << node->NormalizeVertex(*v)))
            {
                return *v;
            }
        }
        return Vertex(-1);
    }
    
    Vertex FindAccVertexNotEqual(Vertex vertex)
    {
        if (intersectionFlags == 0)
        {
            return Vertex(-1);
        }
        for (typename Simplex::iterator v = node->simplex->begin(); v != node->simplex->end(); v++)
        {
            if (*v == vertex)
            {
                continue;
            }
            if (intersectionFlags & (1 << node->NormalizeVertex(*v)))
            {
                return *v;
            }
        }
        return Vertex(-1);
    }

    Vertex FindAccVertexNotIn(const VertsSet &vertsSet)
    {
        if (intersectionFlags == 0)
        {
            return Vertex(-1);
        }
        for (typename Simplex::iterator v = node->simplex->begin(); v != node->simplex->end(); v++)
        {
            if (std::find(vertsSet.begin(), vertsSet.end(), *v) != vertsSet.end())
            {
                continue;
            }
            if (intersectionFlags & (1 << node->NormalizeVertex(*v)))
            {
                return *v;
            }
        }
        return Vertex(-1);
    }
    
    void UpdateAccIntersectionWithSimplex(const Simplex &simplex)
    {
        for (typename Edges::iterator edge = node->edges.begin(); edge != node->edges.end(); edge++)
        {
            if ((*edge)->GetNeighbour(node)->IsInAccSub())
            {
                continue;
            }
            Simplex s;
            Simplex::GetIntersection((*edge)->GetIntersection(), simplex, s);
            if (s.size() > 0)
            {
                Node *neighbour = (*edge)->GetNeighbour(node);
                s = neighbour->Normalize(s);
                neighbour->GetAccInfo().UpdateAccIntersectionFlags(neighbour->GetSubconfigurationsFlags(s), neighbour->GetConfigurationsFlags(s));
            }
        }
        Simplex normalizedSimplex = node->Normalize(simplex);
        UpdateAccIntersectionFlags(node->GetSubconfigurationsFlags(normalizedSimplex), node->GetConfigurationsFlags(normalizedSimplex));
    }

    void UpdateAccIntersectionWithVertex(Vertex v)
    {
        Simplex s = Simplex::FromVertex(v);
        for (typename Edges::iterator edge = node->edges.begin(); edge != node->edges.end(); edge++)
        {
            if ((*edge)->GetNeighbour(node)->IsInAccSub())
            {
                continue;
            }
            if ((*edge)->GetIntersection().ContainsVertex(v))
            {
                Node *neighbour = (*edge)->GetNeighbour(node);
                Simplex s1 = neighbour->Normalize(s);
                neighbour->GetAccInfo().UpdateAccIntersectionFlags(node->GetSubconfigurationsFlags(s1), node->GetConfigurationsFlags(s1));
            }
        }
        s = node->Normalize(s);
        UpdateAccIntersectionFlags(node->GetSubconfigurationsFlags(s), node->GetConfigurationsFlags(s));
    }

    void UpdateAccIntersectionWithEdge(Vertex v1, Vertex v2)
    {
        assert (v1 != v2);
        UpdateAccIntersectionWithSimplex(Simplex::FromVertices(v1, v2));
    }

    void UpdateNeighboursAccIntersection()
    {
        Simplex intersection;
        for (typename Edges::iterator edge = node->edges.begin(); edge != node->edges.end(); edge++)
        {
            Node *neighbour = (*edge)->GetNeighbour(node);
            if (neighbour->IsInAccSub())
            {
                continue;
            }
            Simplex::GetIntersection(node->simplex, neighbour->simplex, intersection);
            intersection = neighbour->Normalize(intersection);
            neighbour->GetAccInfo().UpdateAccIntersectionFlags(node->GetSubconfigurationsFlags(intersection), node->GetConfigurationsFlags(intersection));
        }
    }

    void UpdateAccIntersection(const Simplex &intersection)
    {
        Simplex normalizedIntersection = node->Normalize(intersection);
        UpdateAccIntersectionFlags(node->GetSubconfigurationsFlags(normalizedIntersection), node->GetConfigurationsFlags(normalizedIntersection));
    }

    bool HasIntersectionWithAccSub()
    {
        return (intersectionFlags != 0);
    }

    int GetBufferSize()
    {
        return IntersectionFlagsHelpers<IntersectionFlags>::GetBufferSize(intersectionFlags);
    }

    void ReadFromBuffer(int *buffer, int &index)
    {
        IntersectionFlagsHelpers<IntersectionFlags>::ReadFromBuffer(intersectionFlags, buffer, index);
    }

    void WriteToBuffer(int *buffer, int &index)
    {
        IntersectionFlagsHelpers<IntersectionFlags>::WriteToBuffer(intersectionFlags, buffer, index);
    }

    int GetAccSubID() const { return accSubID; }
    void SetAccSubID(int id) { accSubID = id; }

private:

    void UpdateAccIntersectionFlags(const IntersectionFlags &flags, const IntersectionFlags &flagsMF)
    {
        // if flags are already set then we're done
        if ((intersectionFlags & flags) == flags)
        {
            return;
        }
        intersectionFlags |= flags;
        intersectionFlagsMF |= flagsMF;
        IntersectionFlags flagsSubfaces = flags & (~flagsMF);
        intersectionFlagsMF &= (~flagsSubfaces);
    }

};

#endif	/*ACCINFOFLAGS_HPP */

