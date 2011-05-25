#include "Simplex.h"

#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <time.h>
#include <algorithm>
#include <set>
#include <cmath>

// test!!!
#include <iostream>

////////////////////////////////////////////////////////////////////////////////
// ReadSimplexVector

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
// GenerateSimplexList

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
        else
        {
            std::cout<<"duplicate found"<<std::endl;
        }
    } 
}

////////////////////////////////////////////////////////////////////////////////
// GenerateReverseSimplexList

void GenerateReverseSimplexList(SimplexList &simplexList, Simplex &simplex, int firstVert, int vertsCount, int currentDim, int maxDim, int &index, std::set<int> excludedIndices)
{
    for (int i = firstVert; i <= vertsCount; i++)
    {
        if (currentDim == 0)
        {
            std::cout<<i<<std::endl;
        }
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

    std::cout<<"total simplices: "<<totalSimplices<<" generated: "<<simplexList.size()<<std::endl;
    return totalSimplices;
}

////////////////////////////////////////////////////////////////////////////////
// GenerateSimplexListAcyclicTree

void GenerateAcyclicTree(SimplexList &simplexList, int nodeSimplicesCount, int nodesCount)
{
    int n = (int)ceil(sqrt(nodeSimplicesCount * 0.5f));

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
        int count = nodeSimplicesCount;
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

    int newVert = 1;

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

//    Debug::Print(Log::stream, simplexList);

}

////////////////////////////////////////////////////////////////////////////////
// GetIntersection

bool GetIntersection(Simplex *a, Simplex *b, Simplex &intersection)
{
    if (intersection.size() > 0)
    {
        intersection.clear();
    }
    Simplex::iterator i = a->begin();
    Simplex::iterator j = b->begin();
    while (i != a->end() && j != b->end())
    {
        if ((*i) < (*j)) i++;
        else if ((*i) > (*j)) j++;
        else
        {
            intersection.push_back(*i);
            i++;
            j++;
        }
    }
    return (intersection.size() > 0);
}

////////////////////////////////////////////////////////////////////////////////
// GetVertexFromIntersection

Vertex GetVertexFromIntersection(Simplex *a, Simplex *b)
{
    Simplex::iterator i = a->begin();
    Simplex::iterator j = b->begin();
    while (i != a->end() && j != b->end())
    {
        if ((*i) < (*j)) i++;
        else if ((*i) > (*j)) j++;
        else
        {
            return (*i);
        }
    }
    return -1;
}

Vertex GetVertexFromIntersectionNotEqual(Simplex *a, Simplex *b, Vertex vertex)
{
    Simplex::iterator i = a->begin();
    Simplex::iterator j = b->begin();
    while (i != a->end() && j != b->end())
    {
        if ((*i) < (*j)) i++;
        else if ((*i) > (*j)) j++;
        else if ((*i) == vertex)
        {
            i++;
            j++;
        }
        else
        {
            return (*i);
        }
    }
    return -1;
}

Vertex GetVertexFromIntersectionNotIn(Simplex *a, Simplex *b, const VertsSet &vertsSet)
{
    Simplex::iterator i = a->begin();
    Simplex::iterator j = b->begin();
    while (i != a->end() && j != b->end())
    {
        if ((*i) < (*j)) i++;
        else if ((*i) > (*j)) j++;
        else if (std::find(vertsSet.begin(), vertsSet.end(), *i) != vertsSet.end())
        {
            i++;
            j++;
        }
        else
        {
            return (*i);
        }
    }
    return -1;
}

////////////////////////////////////////////////////////////////////////////////
// ContainsVertex

bool ContainsVertex(Simplex *s, Vertex v)
{
    return (find(s->begin(), s->end(), v) != s->end());
}

////////////////////////////////////////////////////////////////////////////////
//  GenerateSubsimplices

void GenerateSubsimplices(Simplex &simplex, int index, int dim, int first, int count, Simplex current, SimplexList &subsimplices)
{
    current.push_back(simplex[first]);
    if (index == dim)
    {        
        subsimplices.push_back(current);
    }
    else
    {
        for (int i = first + 1; i < count; i++)
        {        
            GenerateSubsimplices(simplex, index + 1, dim, i, count, current, subsimplices);
        }    
    }            
}

////////////////////////////////////////////////////////////////////////////////
//  GenerateSubsimplices

void GenerateSubsimplices(Simplex &simplex, SimplexList &sumbsimplices)
{
    Simplex s;    
    for (int d = 0; d < simplex.size() - 1; d++)
    {
        for (int i = 0; i < simplex.size(); i++)
        {
            GenerateSubsimplices(simplex, 0, d, i, simplex.size(), s, sumbsimplices);
        }
    }         
}

////////////////////////////////////////////////////////////////////////////////
//  AddSubsimplices

void AddSubsimplices(Simplex &simplex, SimplexList &simplexList)
{    
    // generujemy wszystkie podsympleksy
    SimplexList simplices;
    GenerateSubsimplices(simplex, simplices);
    
    // dodajemy tylko te, ktorych jeszcze nie ma
    for (SimplexList::iterator i = simplices.begin(); i != simplices.end(); i++)
    {
        if (std::find(simplexList.begin(), simplexList.end(), (*i)) == simplexList.end())
        {
            simplexList.push_back(*i);    
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
// SimplexSorter

bool SimplexSorter(Simplex a, Simplex b)
{
    return (a.size() < b.size());
}

////////////////////////////////////////////////////////////////////////////////
// operator==

bool operator==(const Simplex &a, const Simplex &b)
{
    if (a.size() != b.size())
    {
        return false;
    }
    Simplex::const_iterator i = a.begin();
    Simplex::const_iterator j = b.begin();
    while (i != a.end())
    {
        if ((*i) != (*j)) return false;
        i++;
        j++;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// eof
