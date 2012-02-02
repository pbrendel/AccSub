/* 
 * File:   IncidenceGraphDefaultTraits.h
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 */

#ifndef INCIDENCEGRAPHTRAITS_H
#define	INCIDENCEGRAPHTRAITS_H

#include "SimplexT.hpp"
#include "IntersectionInfoFlags.hpp"
#include "IntersectionInfoFlagsSimplex.hpp"
#include "AccInfoFlags.hpp"
#include "AccSubAlgorithmType.h"
#include "IntersectionFlagsBitSet.hpp"
#include "SimplexNormalization.hpp"

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
    typedef SimplexNormalizationNone<Vertex, Simplex> SimplexNormalization;
    typedef AccSubAlgorithmType             AccSubAlgorithm;

};

template <int D>
class IncidenceGraphTraitsDim
{
public:

    typedef int                             Vertex;
    typedef std::set<Vertex>                VertsSet;
    typedef SimplexT<Vertex>                Simplex;
    typedef std::vector<Simplex>            SimplexList;
    typedef Simplex *                       SimplexPtr;
    typedef std::vector<SimplexPtr>         SimplexPtrList;
    typedef IntersectionFlagsBitSet<D>      IntersectionFlags;
    typedef SimplexNormalizationNone<Vertex, Simplex> SimplexNormalization;
    typedef AccSubAlgorithmType             AccSubAlgorithm;

};

#endif	/* INCIDENCEGRAPHTRAITS_H */

