/* 
 * File:   SimplexUtils.hpp
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 */

#ifndef SIMPLEXUTILS_HPP
#define	SIMPLEXUTILS_HPP

#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

template <typename Simplex>
class SimplexUtils
{
    typedef std::vector<Simplex> SimplexList;

public:
    
    static void ReadSimplexList(SimplexList &simplexList, const char *filename, bool sortVerts)
    {
        std::ifstream input(filename);
        if (!input.is_open())
        {
            throw std::string("Can't open file ") + filename + "|";
        }

        simplexList.clear();
        Simplex simplex;
        std::string line;
        while (getline(input, line))
        {
            if (line.find_first_of("#", 0) != std::string::npos)
            {
                continue;
            }
            simplex.clear();
            int token;
            std::istringstream tokens(line);
            while(tokens >> token)
            {
                simplex.push_back(token);
            }
            if (sortVerts)
            {
                std::sort(simplex.begin(), simplex.end());
            }
            simplexList.push_back(simplex);
        }
        input.close();
    }

    static void GenerateSimplexList(SimplexList &simplexList, int simplicesCount, int vertsCount, int dim)
    {
        srand(time(0));
        while (simplicesCount--)
        {
            Simplex s;
            while (s.size() != dim + 1)
            {
                int r = rand() % vertsCount + 1;
                if (std::find(s.begin(), s.end(), r) == s.end())
                {
                    s.push_back(r);
                }
            }
            std::sort(s.begin(), s.end());
            if (std::find(simplexList.begin(), simplexList.end(), s) == simplexList.end())
            {
                simplexList.push_back(s);
            }
        }
    }

private:
    
    static void GenerateReverseSimplexList(SimplexList &simplexList, Simplex &simplex, int firstVert, int vertsCount, int currentDim, int maxDim, int &index, std::set<int> excludedIndices)
    {
        for (int i = firstVert; i <= vertsCount; i++)
        {
            simplex[currentDim] = i;
            if (currentDim < maxDim - 1)
            {
                GenerateReverseSimplexList(simplexList, simplex, i + 1, vertsCount, currentDim + 1, maxDim, index, excludedIndices);
            }
            else
            {
                if (excludedIndices.find(index) == excludedIndices.end())
                {
                    simplexList.push_back(simplex);
                }
                index++;
            }
        }
    }

public:

    static int GenerateReverseSimplexList(SimplexList &simplexList, int simplicesCount, int vertsCount, int dim)
    {
        srand(time(0));

        int totalSimplices = 1;
        for (int i = 0; i < dim; i++)
        {
            totalSimplices *= (vertsCount - i);
        }
        for (int i = 2; i <= dim; i++)
        {
            totalSimplices /= i;
        }
        std::set<int> excludedIndices;
        for (int i = 0; i < simplicesCount; i++)
        {
            excludedIndices.insert(rand() % totalSimplices);
        }

        int index = 0;
        Simplex simplex;
        for (int i = 0; i < dim; i++) simplex.push_back(0);
        GenerateReverseSimplexList(simplexList, simplex, 1, vertsCount, 0, dim, index, excludedIndices);
        return totalSimplices;
    }

    static bool FindDuplicates(SimplexList &simplexList)
    {
        for (typename SimplexList::iterator i = simplexList.begin(); i != simplexList.end(); i++)
        {
            if (std::find(i + 1, simplexList.end(), *i) != simplexList.end())
            {
                return true;
            }
        }
        return false;
    }
};

#endif	/* SIMPLEXUTILS_HPP */

