#include "Simplex.h"
#include <algorithm>

////////////////////////////////////////////////////////////////////////////////

int GetDimension(const SimplexList &simplexList)
{
    return (simplexList.size() > 0) ? (int)simplexList[0].size() - 1 : 0;
}

int GetDimension(const SimplexPtrList &simplexPtrList)
{
    return (simplexPtrList.size() > 0) ? (int)simplexPtrList[0]->size() - 1 : 0;
}

////////////////////////////////////////////////////////////////////////////////

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
    return Vertex(-1);
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
    return Vertex(-1);
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
    return Vertex(-1);
}

bool ContainsVertex(Simplex *s, Vertex v)
{
    return (find(s->begin(), s->end(), v) != s->end());
}

////////////////////////////////////////////////////////////////////////////////

void GenerateProperFaces(Simplex &simplex, int index, int dim, int first, int count, Simplex current, SimplexList &faces)
{
    current.push_back(simplex[first]);
    if (index == dim)
    {        
        faces.push_back(current);
    }
    else
    {
        for (int i = first + 1; i < count; i++)
        {        
            GenerateProperFaces(simplex, index + 1, dim, i, count, current, faces);
        }    
    }            
}

void GenerateProperFaces(Simplex &simplex, SimplexList &faces)
{
    Simplex s;    
    for (int d = 0; d < simplex.size() - 1; d++)
    {
        for (int i = 0; i < simplex.size(); i++)
        {
            GenerateProperFaces(simplex, 0, d, i, simplex.size(), s, faces);
        }
    }         
}

void AddProperFaces(Simplex &simplex, SimplexList &simplexList)
{    
    // generujemy wszystkie podsympleksy
    SimplexList faces;
    GenerateProperFaces(simplex, faces);
    
    // dodajemy tylko te, ktorych jeszcze nie ma
    for (SimplexList::iterator i = faces.begin(); i != faces.end(); i++)
    {
        if (std::find(simplexList.begin(), simplexList.end(), (*i)) == simplexList.end())
        {
            simplexList.push_back(*i);    
        }    
    } 
}

////////////////////////////////////////////////////////////////////////////////

bool SimplexSorterSize(const Simplex &a, const Simplex &b)
{
    return (a.size() < b.size());
}

////////////////////////////////////////////////////////////////////////////////

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
