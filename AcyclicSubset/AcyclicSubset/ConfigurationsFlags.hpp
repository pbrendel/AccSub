/* 
 * File:   ConfigurationsFlags.hpp
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 */

#ifndef CONFIGURATIONSFLAGS_HPP
#define	CONFIGURATIONSFLAGS_HPP

#include <vector>
#include <map>

template <typename SimplexType, typename FlagsType>
class ConfigurationsFlags
{
    std::map<SimplexType, FlagsType> flags;

public:

    ConfigurationsFlags()
    {
        // do nothing
    }

    ConfigurationsFlags(int dim, bool subconfigurations, bool addMaximalSimplex)
    {
        Create(dim, subconfigurations, addMaximalSimplex);
    }

    // for a given d-dimensional simplex represented by "normalized"
    // vertex labels (from 0 to d+1) we create a hash in which keys
    // are proper faces of such simplex and values are unique flags for
    // each face. moreover, if subconfigurations == true flag also contain
    // flags or all its faces.
    void Create(int dim, bool subconfigurations, bool addMaximalSimplex)
    {
        // "base" simplex
        SimplexType s;
        for (int i = 0; i <= dim; i++)
        {
            s.push_back(i);
        }
        std::vector<SimplexType> faces;
        s.GenerateProperFaces(faces);
        if (addMaximalSimplex)
        {
            faces.push_back(s);
        }
        FlagsType flag = 1;
        for (typename std::vector<SimplexType>::iterator i = faces.begin(); i != faces.end(); i++)
        {
            if (!subconfigurations)
            {
                flags[*i] = flag;
            }
            else
            {
                FlagsType subFlags = flag;
                // generating all proper faces of given face
                std::vector<SimplexType> subconfigurations;
                i->GenerateProperFaces(subconfigurations);
                for (typename std::vector<SimplexType>::iterator j = subconfigurations.begin(); j != subconfigurations.end(); j++)
                {
                    subFlags |= flags[(*j)];
                }
                flags[*i] = subFlags;
            }            
            flag = flag << 1;
        }
    }

    FlagsType operator[] (const SimplexType &s)
    {
        return flags[s];
    }

    bool GetSimplex(FlagsType f, SimplexType &simplex)
    {
        for (typename std::map<SimplexType, FlagsType>::iterator i = flags.begin(); i != flags.end(); i++)
        {
            if (i->second == f)
            {
                simplex = i->first;
                return true;
            }
        }
        return false;
    }
};

#endif	/* CONFIGURATIONSFLAGS_HPP */

