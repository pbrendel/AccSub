/* 
 * File:   IntersectionInfoFlags.hpp
 * Author: Piotr Brendel
 */

#ifndef INTERSECTIONINFOFLAGS_HPP
#define	INTERSECTIONINFOFLAGS_HPP

template <typename IncidenceGraph>
class IntersectionInfoFlags
{
    typedef typename IncidenceGraph::Simplex Simplex;
    typedef typename IncidenceGraph::IntersectionFlags IntersectionFlags;
    typedef typename IncidenceGraph::Node Node;
    typedef typename IncidenceGraph::Edge Edge;

    Edge                *edge;
    Simplex             intersection;
    IntersectionFlags   intersectionFlagsA;
    IntersectionFlags   intersectionFlagsB;

    bool IsCalculated()
    {
        return (intersectionFlagsA != 0 && intersectionFlagsB != 0);
    }

    void Calculate()
    {
        if (Simplex::GetIntersection(edge->nodeA->simplex, edge->nodeB->simplex, intersection))
        {
            intersectionFlagsA = edge->nodeA->GetNormalizedIntersectionFlags(intersection);
            intersectionFlagsB = edge->nodeB->GetNormalizedIntersectionFlags(intersection);
        }
    }

public:

    IntersectionInfoFlags(Edge *edge)
    {
        this->edge = edge;
        this->intersectionFlagsA = 0;
        this->intersectionFlagsB = 0;
    }

    Simplex &Get()
    {
        if (!IsCalculated())
        {
            Calculate();
        }
        return intersection;
    }

    IntersectionFlags GetFlags(Node *node)
    {
        if (!IsCalculated())
        {
            Calculate();
        }
        return (node == edge->nodeA) ? intersectionFlagsA : intersectionFlagsB;
    }
};

#endif	/* INTERSECTIONINFOFLAGS_HPP */

