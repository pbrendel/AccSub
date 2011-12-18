/* 
 * File:   IntersectionInfoFlags.hpp
 * Author: Piotr Brendel
 */

#ifndef INTERSECTIONINFOFLAGS_HPP
#define	INTERSECTIONINFOFLAGS_HPP

template <typename IncidenceGraphT>
class IntersectionInfoFlags
{
    typedef typename IncidenceGraphT::IntersectionFlags IntersectionFlags;
    typedef typename IncidenceGraphT::Node Node;
    typedef typename IncidenceGraphT::Edge Edge;

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

