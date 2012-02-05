/* 
 * File:   AccInfoSimplexMF.hpp
 * Author: Piotr Brendel
 */

#ifndef ACCINFOSIMPLEXMF_HPP
#define	ACCINFOSIMPLEXMF_HPP

#include <algorithm>
#include <cassert>

template <typename IncidenceGraphType>
class AccInfoSimplexMF
{
    typedef IncidenceGraphType IncidenceGraph;
    typedef typename IncidenceGraph::Vertex Vertex;
    typedef typename IncidenceGraph::VertsSet VertsSet;
    typedef typename IncidenceGraph::Simplex Simplex;
    typedef typename IncidenceGraph::SimplexList SimplexList;
    typedef typename IncidenceGraph::Node Node;
    typedef typename IncidenceGraph::Edges Edges;
    typedef typename IncidenceGraph::IntersectionFlags IntersectionFlags;
    typedef typename IncidenceGraph::AccTest AccTest;

    Node                *node;
    // list of NORMALIZED maximal faces
    SimplexList         intersectionMF;
    unsigned char       accSubID;

public:

    AccInfoSimplexMF(Node *n)
    {
        node = n;
        accSubID = 0;
    }

    bool IsAccIntersectionAcyclic(AccTest *accTest)
    {
        accTest->IsAcyclic(*node->simplex, intersectionMF);
    }

    bool IsInsideAccIntersection(const IntersectionFlags &flags)
    {
        return ((flags & GetIntersectionFlags()) == flags);
    }

    bool IsDisjointWithAccIntersection(const IntersectionFlags &flags)
    {
        return ((flags & GetIntersectionFlags()) == 0);
    }

    bool IsVertexInAccIntersection(Vertex vertex)
    {
        Vertex normalizedVertex = node->NormalizeVertex(vertex);
        for (typename SimplexList::iterator s = intersectionMF.begin(); s != intersectionMF.end(); s++)
        {
            if (std::find(s->begin(), s->end(), normalizedVertex) != s->end())
            {
                return true;
            }
        }
        return false;
    }

    Vertex FindAccVertex()
    {
        if (intersectionMF.size() == 0)
        {
            return Vertex(-1);
        }
        for (typename Simplex::iterator v = node->simplex->begin(); v != node->simplex->end(); v++)
        {
            if (IsVertexInAccIntersection(*v))
            {
                return *v;
            }
        }
        return Vertex(-1);
    }

    Vertex FindAccVertexNotEqual(Vertex vertex)
    {
        if (intersectionMF.size() == 0)
        {
            return Vertex(-1);
        }
        for (typename Simplex::iterator v = node->simplex->begin(); v != node->simplex->end(); v++)
        {
            if (*v == vertex)
            {
                continue;
            }
            if (IsVertexInAccIntersection(*v))
            {
                return *v;
            }
        }
        return Vertex(-1);
    }

    Vertex FindAccVertexNotIn(const VertsSet &vertsSet)
    {
        if (intersectionMF.size() == 0)
        {
            return Vertex(-1);
        }
        for (typename Simplex::iterator v = node->simplex->begin(); v != node->simplex->end(); v++)
        {
            if (std::find(vertsSet.begin(), vertsSet.end(), *v) != vertsSet.end())
            {
                continue;
            }
            if (IsVertexInAccIntersection(*v))
            {
                return *v;
            }
        }
        return Vertex(-1);
    }

    void UpdateAccIntersectionWithSimplex(const Simplex &simplex)
    {
        Simplex normalizedSimplex = node->Normalize(simplex);
        // if not added it means that simplex was already present in
        // the intersection (or is face of another simplex present there)
        // so there is no need to update neighours AccInfo
        if (!AddNormalizedSimplex(normalizedSimplex))
        {
            return;
        }
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
                neighbour->GetAccInfo().AddNormalizedSimplex(neighbour->Normalize(s));
            }
        }
    }

    void UpdateAccIntersectionWithVertex(Vertex v)
    {
        UpdateAccIntersectionWithSimplex(Simplex::FromVertex(v));
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
            if (intersection.size() > 0)
            {
                neighbour->GetAccInfo().AddNormalizedSimplex(neighbour->Normalize(intersection));
            }
        }
    }

    void UpdateAccIntersection(const Simplex &intersection)
    {
        AddNormalizedSimplex(node->Normalize(intersection));
    }

    bool HasIntersectionWithAccSub()
    {
        return (intersectionMF.size() > 0);
    }

    int BufferSize()
    {
        return 1;
    }

    void ReadFromBuffer(int *buffer, int &index)
    {
    //    intersectionFlags = buffer[index++];
    }

    void WriteToBuffer(int *buffer, int &index)
    {
    //    buffer[index++] = intersectionFlags;
    }

    int GetAccSubID() const { return accSubID; }
    void SetAccSubID(int id) { accSubID = id; }

private:

    IntersectionFlags GetIntersectionFlags()
    {
        IntersectionFlags flags = 0;
        for (typename SimplexList::iterator s = intersectionMF.begin(); s != intersectionMF.end(); s++)
        {
            flags |= node->GetSubconfigurationsFlags(*s);
        }
        return flags;
    }

    bool IsFaceOf(const Simplex &face, const Simplex &simplex)
    {
        typename Simplex::const_iterator itf = face.begin();
        typename Simplex::const_iterator its = simplex.begin();
        while (itf != face.end() && its != simplex.end())
        {
            if (*itf == *its)
            {
                itf++;
                its++;
            }
            else if (*its < *itf)
            {
                its++;
            }
            else
            {
                return false;
            }
        }
        return (itf == face.end());
    }

    bool AddNormalizedSimplex(const Simplex &simplex)
    {
        typename SimplexList::iterator s = intersectionMF.begin();
        while (s != intersectionMF.end())
        {
            // if given simplex is a face of a simplex already present
            // in intrsection then it's not maximal face -> we're done
            if (IsFaceOf(simplex, *s))
            {
                return false;
            }
            // if another simplex in intersection is a face of given simplex
            // we remove it from the list of maximal faces
            else if (IsFaceOf(*s, simplex))
            {
                s = intersectionMF.erase(s);
            }
            else
            {
                s++;
            }
        }
        intersectionMF.push_back(simplex);
        return true;
    }
};

#endif	/* ACCINFOSIMPLEXMF_HPP */

