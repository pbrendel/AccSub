/* 
 * File:   PrepareDataBFS.cpp
 * Author: Piotr Brendel
 */

#include "PrepareDataBFS.h"
#include <map>
#include <queue>

////////////////////////////////////////////////////////////////////////////////

struct SimplexDescriptor
{
    Simplex simplex;
    bool added;

    SimplexDescriptor()
    {
        added = false;
    }
};

void PrepareDataBFS::Prepare(SimplexList &simplexList, int)
{
    int count = simplexList.size();
    SimplexDescriptor *descriptors = new SimplexDescriptor[count];
    int index = 0;
    for (SimplexList::iterator i = simplexList.begin(); i != simplexList.end(); i++)
    {
        descriptors[index++].simplex = *i;
    }
    simplexList.clear();

    std::map<Vertex, std::vector<SimplexDescriptor *> > H;
    for (int i = 0; i < count; i++)
    {
        Simplex &s = descriptors[i].simplex;
        for (Simplex::iterator v = s.begin(); v != s.end(); v++)
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
        std::queue<SimplexDescriptor *> L;
        L.push(&descriptors[i]);
        descriptors[i].added = true;
        while (!L.empty())
        {
            SimplexDescriptor *sd = L.front();
            L.pop();
            simplexList.push_back(sd->simplex);
            for (Simplex::iterator v = sd->simplex.begin(); v != sd->simplex.end(); v++)
            {
                std::vector<SimplexDescriptor *> neighbours = H[*v];
//                if (neighbours.size() > 20)
//                {
//                    continue;
//                }
                for (std::vector<SimplexDescriptor *>::iterator n = neighbours.begin(); n != neighbours.end(); n++)
                {
                    if (!(*n)->added)
                    {
                        L.push(*n);
                        (*n)->added = true;
                    }
                }
            }
        }
    }
    delete [] descriptors;
}

////////////////////////////////////////////////////////////////////////////////
// eof
