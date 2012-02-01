/* 
 * File:   IntersectionInfoFlagsSimplex.hpp
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 */

#ifndef INTERSECTIONINFOFLAGSSIMPLEX_HPP
#define	INTERSECTIONINFOFLAGSSIMPLEX_HPP

template <typename IncidenceGraph>
class IntersectionInfoFlags
{
    typedef typename IncidenceGraph::Simplex Simplex;
    typedef typename IncidenceGraph::IntersectionFlags IntersectionFlags;
    typedef typename IncidenceGraph::Node Node;
    typedef typename IncidenceGraph::Edge Edge;

    Simplex             intersection;
    IntersectionFlags   intersectionFlagsA;
    IntersectionFlags   intersectionFlagsB;

    bool IsCalculated()
    {
        return (intersectionFlagsA != 0 && intersectionFlagsB != 0);
    }

    void Calculate(Edge *edge)
    {
        if (Simplex::GetIntersection(edge->nodeA->simplex, edge->nodeB->simplex, intersection))
        {
            intersectionFlagsA = edge->nodeA->GetNormalizedIntersectionFlags(intersection);
            intersectionFlagsB = edge->nodeB->GetNormalizedIntersectionFlags(intersection);
        }
    }

public:

    IntersectionInfoFlags()
    {
        this->intersectionFlagsA = 0;
        this->intersectionFlagsB = 0;
    }

    Simplex &Get(Edge *edge)
    {
        if (!IsCalculated())
        {
            Calculate(edge);
        }
        return intersection;
    }

    IntersectionFlags &GetFlags(Edge *edge, Node *node)
    {
        if (!IsCalculated())
        {
            Calculate(edge);
        }
        return (node == edge->nodeA) ? intersectionFlagsA : intersectionFlagsB;
    }
};

#endif	/* INTERSECTIONINFOFLAGSSIMPLEX_HPP */

