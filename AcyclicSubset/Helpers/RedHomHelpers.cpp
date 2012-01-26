/* 
 * File:   RedHomHelpers.cpp
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 */

////////////////////////////////////////////////////////////////////////////////

#include "RedHomHelpers.hpp"

ofstreamcout fcout;

////////////////////////////////////////////////////////////////////////////////
// code from redHom/complex/simplicial/SimplexSComplex.cpp

SimplexSComplex::Simplex* SimplexSComplex::createSimplexHierarchy(vertex_set &s)
{
    assert(s.size() > 0);

    if (s.size() == 1)
    {
        return makeBaseSimplex(s);
    }

    Simplex* rootSimplex = new Simplex(s.begin(), s.end(), nextId++);
    simplexAddedEvent(*rootSimplex);

    // set<int> probably can't be used as a drop-in replacement...
    for (vertex_set::iterator it = s.begin(), end = s.end(); it != end; )
    {
        const int &val = *it;
        vertex_set::iterator new_it = it;
        ++new_it;

        s.erase(it);
        Simplex *subSimplex = addSimplex(s);
        s.insert(it, val);

        rootSimplex->addToBorder(*subSimplex);
        subSimplex->addToCoborder(*rootSimplex, val);
        // {val} = root \ sub

        it = new_it;
    }

    return rootSimplex;
}

////////////////////////////////////////////////////////////////////////////////
// eof
