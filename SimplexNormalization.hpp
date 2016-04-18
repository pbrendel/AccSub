/* 
 * File:   SimplexNormalization.hpp
 * Author: Piotr Brendel
 */

#ifndef SIMPLEXNORMALIZATION_HPP
#define	SIMPLEXNORMALIZATION_HPP

#include <map>

template <typename VertexT, typename SimplexT>
class SimplexNormalizationNone
{
public:

    SimplexNormalizationNone(SimplexT *simplex)
    {
        // done
    }
    
    SimplexT Normalize(SimplexT *baseSimplex, const SimplexT &simplex)
    {
        int index = 0;
        SimplexT s = SimplexT::WithSize(simplex.size());
        for (typename SimplexT::const_iterator i = simplex.begin(); i != simplex.end(); i++)
        {
            s[index++] = (VertexT)NormalizeVertex(baseSimplex, *i);
        }
        return s;
    }

    int NormalizeVertex(SimplexT *baseSimplex, VertexT v)
    {
        for (int i = 0; i < baseSimplex->size(); i++)
        {
            if (baseSimplex->at(i) == v)
            {
                return i;
            }
        }
        return 0;
    }
};

template <typename VertexT, typename SimplexT>
class SimplexNormalizationMap
{

    std::map<VertexT, int> v2i;
    
public:

    SimplexNormalizationMap(SimplexT *simplex)
    {
        for (int i = 0; i < simplex->size(); i++)
        {
            v2i[(*simplex)[i]] = i;
        }        
    }

    SimplexT Normalize(SimplexT *baseSimplex, const SimplexT &simplex)
    {
        SimplexT s = SimplexT::WithSize(simplex.size());
        int index = 0;
        for (typename SimplexT::const_iterator i = simplex.begin(); i != simplex.end(); i++)
        {
            s[index++] = v2i[(*i)];
        }
        return s;
    }

    int NormalizeVertex(SimplexT *baseSimplex, VertexT v)
    {
        return v2i[v];
    } 
};

#endif	/* SIMPLEXNORMALIZATION_HPP */

