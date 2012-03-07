/* 
 * File:   PrepareData.hpp
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 */

#ifndef PREPAREDATA_HPP
#define	PREPAREDATA_HPP

#include <cstdlib>
#include <cstring> // memset

////////////////////////////////////////////////////////////////////////////////

template <typename PartitionGraph>
class PrepareDataNone
{
    typedef typename PartitionGraph::SimplexList SimplexList;

public:

    static void Prepare(SimplexList &simplexList, int packSize) { }

};

////////////////////////////////////////////////////////////////////////////////

template <typename PartitionGraph>
class PrepareDataRandom
{
    typedef typename PartitionGraph::Simplex Simplex;
    typedef typename PartitionGraph::SimplexList SimplexList;

public:

    static void Prepare(SimplexList &simplexList, int packSize)
    {
        srand(clock());
        for (int i = simplexList.size() - 1; i > 1; i--)
        {
            int index = rand() % i;
            Simplex s = simplexList[index];
            simplexList[index] = simplexList[i];
            simplexList[i] = s;
        }
    }
};

////////////////////////////////////////////////////////////////////////////////

template <typename PartitionGraph>
class PrepareDataBFS
{
    typedef typename PartitionGraph::Vertex Vertex;
    typedef typename PartitionGraph::Simplex Simplex;
    typedef typename PartitionGraph::SimplexList SimplexList;
    
    struct SimplexDescriptor
    {
        Simplex &simplex;
        bool added;

        SimplexDescriptor(Simplex s) : simplex(s)
        {
            added = false;
        }
    };

public:

    static void Prepare(SimplexList &simplexList, int packSize)
    {
        int count = simplexList.size();
        int *indicies = new int[count];
        char *added = new char[count];
        memset(added, 0, sizeof(char) * count);

        std::map<Vertex, std::vector<int> > H;
        for (int i = 0; i < count; i++)
        {
            Simplex &s = simplexList[i];
            for (typename Simplex::iterator v = s.begin(); v != s.end(); v++)
            {
                H[*v].push_back(i);
            }
        }

        int index = 0;
        for (int i = 0; i < count; i++)
        {
            if (added[i])
            {
                continue;
            }
            std::queue<int> Q;
            Q.push(i);
            added[i] = 1;
            while (!Q.empty())
            {
                int current = Q.front();
                Q.pop();
                indicies[index++] = current;
                Simplex &s = simplexList[current];
                for (typename Simplex::iterator v = s.begin(); v != s.end(); v++)
                {
                    std::vector<int> neighbours = H[*v];
                    for (typename std::vector<int>::iterator n = neighbours.begin(); n != neighbours.end(); n++)
                    {
                        if (added[*n] == 0)
                        {
                            Q.push(*n);
                            added[*n] = 1;
                        }
                    }
                }
            }
        }
        for (int i = 0; i < count; i++)
        {
            if (indicies[i] == -1)
            {
                continue;
            }
            if (indicies[i] == i)
            {
                indicies[i] = -1;
                continue;
            }
            Simplex s = simplexList[i];
            int current = i;
            while (indicies[current] != i)
            {
                simplexList[current] = simplexList[indicies[current]];
                int next = indicies[current];
                indicies[current] = -1;
                current = next;
            }
            simplexList[current] = s;
            indicies[current] = -1;
        }
        delete [] indicies;
        delete [] added;
    }
};

////////////////////////////////////////////////////////////////////////////////

#endif	/* PREPAREDATA_HPP */

