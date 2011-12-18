/* 
 * File:   AccInfoFlags.hpp
 * Author: Piotr Brendel
 */

#ifndef ACCINFOFLAGS_HPP
#define	ACCINFOFLAGS_HPP

#include <cassert>

template <typename IncidenceGraphT>
class AccInfoFlags
{
    typedef typename IncidenceGraphT::Node Node;
    typedef typename IncidenceGraphT::Edges Edges;
    typedef typename IncidenceGraphT::IntersectionFlags IntersectionFlags;

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

    bool IsAccIntersectionAcyclic(AccTest<IntersectionFlags> *test)
    {
        test->IsAcyclic(*node->simplex, intersectionFlags, intersectionFlagsMF);
    }

    bool IsInsideAccIntersection(IntersectionFlags flags)
    {
        return ((flags & intersectionFlags) == flags);
    }

    bool IsDisjointWithAccIntersection(IntersectionFlags flags)
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
        for (Simplex::iterator v = node->simplex->begin(); v != node->simplex->end(); v++)
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
        for (Simplex::iterator v = node->simplex->begin(); v != node->simplex->end(); v++)
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
        for (Simplex::iterator v = node->simplex->begin(); v != node->simplex->end(); v++)
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
            if (!(*edge)->IntersectionCalculated())
            {
                (*edge)->CalculateIntersection();
            }
            // obliczamy czesc wspolna (przeciecie) krawedzi i przeciecia z sasiadem
            Simplex s;
            Simplex::GetIntersection((*edge)->intersection, simplex, s);
            // jezeli przeciecie to jest niepuste (maja wspolne punkty)
            // to aktualizujemy jego acyclic flags
            if (s.size() > 0)
            {
                Node *neighbour = (*edge)->GetNeighbour(node);
                s = neighbour->Normalize(s);
                neighbour->GetAccInfo().UpdateAccIntersectionFlags(node->GetSubconfigurationsFlags(s), node->GetConfigurationsFlags(s));
            }
        }
        intersectionFlags |= node->GetSubconfigurationsFlags(node->Normalize(simplex));
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
            if (!(*edge)->IntersectionCalculated())
            {
                (*edge)->CalculateIntersection();
            }
            if ((*edge)->intersection.ContainsVertex(v))
            {
                Node *neighbour = (*edge)->GetNeighbour(node);
                Simplex s1 = neighbour->Normalize(s);
                neighbour->GetAccInfo().UpdateAccIntersectionFlags(node->GetSubconfigurationsFlags(s1), node->GetConfigurationsFlags(s1));
            }
        }
        intersectionFlags |= node->GetSubconfigurationsFlags(node->Normalize(s));
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
        UpdateAccIntersectionFlags(node->GetSubconfigurationsFlags(intersection), node->GetConfigurationsFlags(intersection));
    }

    bool HasIntersectionWithAccSub()
    {
        return (intersectionFlags != 0);
    }

    int BufferSize()
    {
        return 1;
    }

    void ReadFromBuffer(int *buffer, int &index)
    {
        intersectionFlags = buffer[index++];
    }

    void WriteToBuffer(int *buffer, int &index)
    {
        buffer[index++] = intersectionFlags;
    }

    // IntersectionFlags GetAccIntersectionFlags() const { return intersectionFlags; }
    // IntersectionFlags GetAccIntersectionFlagsMF() const { return intersectionFlagsMF; }
    int GetAccSubID() const { return accSubID; }
    void SetAccSubID(int id) { accSubID = id; }

private:

    void UpdateAccIntersectionFlags(IntersectionFlags flags, IntersectionFlags flagsMF)
    {
        // jezeli flagi juz sa ustawione to nic nie robimy
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
