/* 
 * File:   IncidenceGraphDefaultTraits.h
 * Author: Piotr Brendel
 */

#ifndef INCIDENCEGRAPHTRAITS_H
#define	INCIDENCEGRAPHTRAITS_H

#include "SimplexT.hpp"
#include "IntersectionInfoFlags.hpp"
#include "AccInfoFlags.hpp"

#include <vector>
#include <set>

class IncidenceGraphDefaultTraits
{
public:
 
    typedef int                             Vertex;
    typedef std::set<Vertex>                VertsSet;
    typedef SimplexT<Vertex>                Simplex;
    typedef std::vector<Simplex>            SimplexList;
    typedef Simplex *                       SimplexPtr;
    typedef std::vector<SimplexPtr>         SimplexPtrList;
    typedef unsigned int                    IntersectionFlags;
    // typedef IntersectionInfoFlags           IntersectionInfo;
    // typedef AccInfoFlags                    AccInfo;
};

#endif	/* INCIDENCEGRAPHTRAITS_H */

