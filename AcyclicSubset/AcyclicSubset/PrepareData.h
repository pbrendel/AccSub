/* 
 * File:   PrepareData.h
 * Author: Piotr Brendel
 */

#ifndef PREPAREDATA_H
#define	PREPAREDATA_H

#include <cstdlib>

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
        Simplex simplex;
        bool added;

        SimplexDescriptor()
        {
            added = false;
        }
    };

public:

    static void Prepare(SimplexList &simplexList, int packSize)
    {
        int count = simplexList.size();
        SimplexDescriptor *descriptors = new SimplexDescriptor[count];
        int index = 0;
        for (typename SimplexList::iterator i = simplexList.begin(); i != simplexList.end(); i++)
        {
            descriptors[index++].simplex = *i;
        }
        simplexList.clear();

        std::map<Vertex, std::vector<SimplexDescriptor *> > H;
        for (int i = 0; i < count; i++)
        {
            Simplex &s = descriptors[i].simplex;
            for (typename Simplex::iterator v = s.begin(); v != s.end(); v++)
            {
                H[*v].push_back(&descriptors[i]);
            }
        }

        index = 0;
        for (int i = 0; i < count; i++)
        {
            if (descriptors[i].added)
            {
                continue;
            }
            std::queue<SimplexDescriptor *> Q;
            Q.push(&descriptors[i]);
            descriptors[i].added = true;
            while (!Q.empty())
            {
                SimplexDescriptor *sd = Q.front();
                Q.pop();
                simplexList.push_back(sd->simplex);
                for (typename Simplex::iterator v = sd->simplex.begin(); v != sd->simplex.end(); v++)
                {
                    std::vector<SimplexDescriptor *> neighbours = H[*v];
                    for (typename std::vector<SimplexDescriptor *>::iterator n = neighbours.begin(); n != neighbours.end(); n++)
                    {
                        if (!(*n)->added)
                        {
                            Q.push(*n);
                            (*n)->added = true;
                        }
                    }
                }
            }
        }
        delete [] descriptors;
    }
};

////////////////////////////////////////////////////////////////////////////////

#endif	/* PREPAREDATA_H */

