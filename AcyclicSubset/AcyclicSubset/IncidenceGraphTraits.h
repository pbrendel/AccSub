/* 
 * File:   IncidenceGraphDefaultTraits.h
 * Author: Piotr Brendel
 */

#ifndef INCIDENCEGRAPHTRAITS_H
#define	INCIDENCEGRAPHTRAITS_H

#include "SimplexT.hpp"
#include "IntersectionInfoFlags.hpp"
#include "AccInfoFlags.hpp"
#include "AccSubAlgorithmType.h"

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
    typedef AccSubAlgorithmType             AccSubAlgorithm;

};

#endif	/* INCIDENCEGRAPHTRAITS_H */

