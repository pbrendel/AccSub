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
#include "../Helpers/Rips/rips.hpp"

template <typename Simplex>
class SimplexUtils
{
    typedef std::vector<Simplex> SimplexList;
    typedef std::set<Simplex> SimplexSet;

public:
    
    static void ReadSimplexList(SimplexList &simplexList, const char *filename, bool sortVerts)
    {
        std::ifstream input(filename);
        if (!input.is_open())
        {
            throw std::string("Can't open file ") + filename;
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

    static void WriteSimplexList(SimplexList &simplexList, const char *filename, bool sortVerts)
    {
        std::ofstream output(filename);
        if (!output.is_open())
        {
            throw std::string("Can't create file ") + filename;
        }

        for (typename SimplexList::iterator s = simplexList.begin(); s != simplexList.end(); s++)
        {
            if (sortVerts)
            {
                std::sort(s->begin(), s->end());
            }
            bool space = false;
            for (typename Simplex::iterator v = s->begin(); v != s->end(); v++)
            {
                if (space)
                {
                    output<<" ";
                }
                space = true;
                output<<(*v);
            }
            output<<std::endl;
        }
        output.close();
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

    static bool GenerateRandomRipsComplex(SimplexList &simplexList, int pointsCount, float pointsDiam, int pointsDim, float ripsComplexEpsilon, float ripsComplexDim)
    {
        typedef rips::euclidanPoint EuclideanPoint;
        typedef rips::simplicialComplex<EuclideanPoint> RipsComplex;

        srand(time(0));
        float radius = pointsDiam * 0.5f;
        std::vector<EuclideanPoint> points;
        for (int i = 0; i < pointsCount; i++)
        {
            std::vector<double> coords;
            for (int j = 0; j < pointsDim; j++)
            {
                float c = ((rand() % RAND_MAX) * pointsDiam - radius) / RAND_MAX;
                coords.push_back(c);
            }
            points.push_back(EuclideanPoint(coords));
        }
        RipsComplex complex(points, ripsComplexEpsilon, ripsComplexDim);
        std::vector<std::set<int> > tempSimplexList;
        complex.outputMaxSimplices(tempSimplexList);
        for (std::vector<std::set<int> >::iterator i = tempSimplexList.begin(); i != tempSimplexList.end(); i++)
        {
            Simplex s;
            for (std::set<int>::iterator j = i->begin(); j != i->end(); j++)
            {
                s.push_back(*j);
            }
            simplexList.push_back(s);
        }
        std::sort(simplexList.begin(), simplexList.end(), Simplex::SortBySizeDesc);
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

private:

    void AddWithBorder(Simplex baseSimplex, SimplexSet &configuration)
    {
        configuration.insert(baseSimplex);
        if (baseSimplex.size() > 1)
        {
            typename Simplex::iterator i = baseSimplex.begin();
            while (i != baseSimplex.end())
            {
                int v = *i;
                i = baseSimplex.erase(i);
                AddWithBorder(baseSimplex, configuration);
                i = baseSimplex.insert(i, v);
                i++;
            }
        }
    }

public:

    void GenerateAllConfigurations(Simplex baseSimplex, SimplexSet &configuration, std::set<SimplexSet> &allConfigurations)
    {
        std::vector<Simplex> border;
        if (baseSimplex.size() > 1)
        {
            typename Simplex::iterator i = baseSimplex.begin();
            while (i != baseSimplex.end())
            {
                int v = *i;
                i = baseSimplex.erase(i);
                border.push_back(baseSimplex);
                i = baseSimplex.insert(i, v);
                i++;
            }
        }
        int last = (1 << (border.size())) + 1;
        for (int i = 0; i < last; i++)
        {
            SimplexSet tmpConf;
            for (int j = 0; j < border.size(); j++)
            {
                if (i & (1 << j))
                {
                    AddWithBorder(border[j], tmpConf);
                }
            }
            allConfigurations.insert(tmpConf);
            for (int j = 0; j < border.size(); j++)
            {
                if (!(i & (1 << j)))
                {
                    std::set<SimplexSet> subconfigurations;
                    GenerateAllConfigurations(border[j], tmpConf, subconfigurations);
                    std::set<SimplexSet> tmpConfigurations(allConfigurations.begin(), allConfigurations.end());
                    allConfigurations.clear();
                    for (typename std::set<SimplexSet>::iterator c1 = subconfigurations.begin(); c1 != subconfigurations.end(); c1++)
                    {
                        for (typename std::set<SimplexSet>::iterator c2 = tmpConfigurations.begin(); c2 != tmpConfigurations.end(); c2++)
                        {
                            SimplexSet newConf;
                            std::set_union(c1->begin(), c1->end(), c2->begin(), c2->end(), std::inserter(newConf, newConf.begin()));
                            allConfigurations.insert(newConf);
                        }
                    }
                }
            }
        }
    }

private:

    int _1pow(int exp)
    {
        if (exp % 2)
        {
            return -1;
        }
        return 1;
    }

public:

    int ComputeEulerCharacteristic(const SimplexSet &configuration)
    {
        std::map<int, int> facesCount;
        for (typename SimplexSet::iterator s = configuration.begin(); s != configuration.end(); s++)
        {
            facesCount[s->size() - 1]++;
        }
        int ec = 0;
        for (std::map<int, int>::iterator i = facesCount.begin(); i != facesCount.end(); i++)
        {
            ec = ec + _1pow(i->first) * i->second;
        }
        return ec;
    }

};

#endif	/* SIMPLEXUTILS_HPP */

