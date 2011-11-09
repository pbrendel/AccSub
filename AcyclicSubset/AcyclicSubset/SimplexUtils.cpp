/* 
 * File:   SimplexUtils.cpp
 * Author: Piotr Brendel
 */

#include "SimplexUtils.h"
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <algorithm>
//#include <cstdlib>
//#include <cstdlib>
//#include <time.h>
//

////////////////////////////////////////////////////////////////////////////////

void ReadSimplexList(SimplexList &simplexList, const char *filename, bool sortVerts)
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

////////////////////////////////////////////////////////////////////////////////

void GenerateSimplexList(SimplexList &simplexList, int simplicesCount, int vertsCount, int dim)
{
    srand(time(0));

    while (simplicesCount--)
    {
        //generujeny nowy sympleks:
        Simplex s;
        while (s.size() != dim + 1)
        {
            int r = rand() % vertsCount + 1;
            // zabezpieczenie przed powtarzajacymi sie wierzcholkami
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

////////////////////////////////////////////////////////////////////////////////

void GenerateReverseSimplexList(SimplexList &simplexList, Simplex &simplex, int firstVert, int vertsCount, int currentDim, int maxDim, int &index, std::set<int> excludedIndices)
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
            // nie znaleziono indeksu w pomijanych indeksach wiec dodajemy
            if (excludedIndices.find(index) == excludedIndices.end())
            {
                simplexList.push_back(simplex);
            }
            index++;
        }
    }
}

int GenerateReverseSimplexList(SimplexList &simplexList, int simplicesCount, int vertsCount, int dim)
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
    for (int i = 0; i < dim; i++) simplex.push_back(0); // dodajemy dim elementow
    GenerateReverseSimplexList(simplexList, simplex, 1, vertsCount, 0, dim, index, excludedIndices);
    // std::cout<<"total simplices: "<<totalSimplices<<" generated: "<<simplexList.size()<<std::endl;
    return totalSimplices;
}

////////////////////////////////////////////////////////////////////////////////

void GenerateAcyclicTree(SimplexList &simplexList, int simplicesCount, int nodesCount)
{
    int n = (int)ceil(sqrt(simplicesCount / nodesCount * 0.5f));

    int nodeBase = 1;
    int nodeBaseOffset = (n + 1) * (n + 1) + nodesCount * 2 + 2;
    int nc = nodesCount;
    Simplex s(3);

    // najpierw generujemy wszystkie "node'y"
    while (nc > 0)
    {

        int vertex = nodeBase;
        nodeBase += nodeBaseOffset;
        bool up = true;
        int count = simplicesCount / nodesCount;
        while (count > 0)
        {
            if (up)
            {
                s[0] = vertex;
                s[1] = vertex + 1;
                s[2] = vertex + n + 2;
                up = false;
            }
            else
            {
                s[0] = vertex;
                s[1] = vertex + n + 1;
                s[2] = vertex + n + 2;
                up = true;
                vertex++;
                if ((vertex % (n + 1)) == 0)
                {
                    vertex++;
                }
            }
            simplexList.push_back(s);
            count--;
        }

        nc--;
    }

    // potem losowo laczymy je tak, zeby powstaly "nieacykliczne" polaczenia
    for (int i = 0; i < 1; i++)
    {
        for (int j = i + 1; j < nodesCount; j++)
        {
            int fromMin = 1 + nodeBaseOffset * i;
            int fromMax = fromMin + n * n - 2;
//            int fromMax = fromMin + n - 2;
            int from = fromMin + (rand() % (fromMax - fromMin + 1));
            int toMin = 1 + nodeBaseOffset * j;
//            int toMax = toMin + n * n - 1;
            int toMax = toMin + n - 1;
            int to = toMin + (rand() % (toMax - toMin + 1));

//            int n1 = fromMin + (n + 1) * (n + 1) + newVert;
//            int n2 = n1 + 1;
//            newVert += 2;
//            s[0] = from;
//            s[1] = from + 1;
//            s[2] = n1;
//            simplexList.push_back(s);
//            s[0] = from + 1;
//            s[1] = from + 2;
//            s[2] = n2;
//            simplexList.push_back(s);
//            s[0] = n1;
//            s[1] = n2;
//            s[2] = to;
//            simplexList.push_back(s);

            s[0] = from;
            s[1] = from + 1;
            s[2] = to;
            simplexList.push_back(s);
            s[0] = from + 1;
            s[1] = from + 2;
            s[2] = to + 1;
            simplexList.push_back(s);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

bool FindDuplicates(SimplexList &simplexList)
{
    for (SimplexList::iterator i = simplexList.begin(); i != simplexList.end(); i++)
    {
        if (std::find(i + 1, simplexList.end(), *i) != simplexList.end())
        {
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
// eof
