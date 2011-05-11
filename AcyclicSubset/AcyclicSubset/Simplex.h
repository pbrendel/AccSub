#ifndef SIMPLEX_H
#define SIMPLEX_H

#include <vector>
#include <set>

#ifdef DEBUG_MEMORY_VERTEX

#include "../Helpers/DebugMemory.h"

class DebugMemoryVertex : DebugMemory<DebugMemoryVertex>
{
public:
    int vertex;

    DebugMemoryVertex() { vertex = 0; }
    DebugMemoryVertex(int v) { vertex = v; }
    DebugMemoryVertex(const DebugMemoryVertex &v) { vertex = v.vertex; }
    // operator int() { return vertex; }
    bool operator==(DebugMemoryVertex v) const { return vertex == v.vertex; }
    bool operator!=(DebugMemoryVertex v) const { return vertex != v.vertex; }
    bool operator>=(DebugMemoryVertex v) const { return vertex >= v.vertex; }
    bool operator>(DebugMemoryVertex v) const { return vertex > v.vertex; }
    bool operator<=(DebugMemoryVertex v) const { return vertex <= v.vertex; }
    bool operator<(DebugMemoryVertex v) const { return vertex < v.vertex; }
    bool operator++() { ++vertex; }
    bool operator++(int) { int val = vertex; vertex++; return val; }

    friend std::ostream& operator<<(std::ostream &str, DebugMemoryVertex v)
    {
        str<<v.vertex;
        return str;
    }
};

//int operator<<(int i , DebugMemoryVertex v) { return i << v.vertex; }

typedef DebugMemoryVertex Vertex;
#else

typedef long Vertex;

#endif

typedef std::set<Vertex> VertsSet;
typedef std::vector<Vertex> Simplex;
typedef std::vector<Simplex> SimplexList;
typedef Simplex* SimplexPtr;
typedef std::vector<SimplexPtr> SimplexPtrList;

void ReadSimplexList(SimplexList &simplexList, const char *filename, bool sortVerts);
void GenerateSimplexList(SimplexList &simplexList, int simplicesCount, int vertsCount, int dim);
int GenerateReverseSimplexList(SimplexList &simplexList, int simplicesCount, int vertsCount, int dim);
void GenerateAcyclicTree(SimplexList &simplexList, int nodeSimplicesCount, int nodesCount);

bool GetIntersection(Simplex *a, Simplex *b, Simplex &intersection);
Vertex GetVertexFromIntersection(Simplex *a, Simplex *b);
Vertex GetVertexFromIntersectionNotEqual(Simplex *a, Simplex *b, Vertex vertex);
Vertex GetVertexFromIntersectionNotIn(Simplex *a, Simplex *b, const VertsSet &vertsSet);
bool ContainsVertex(Simplex *a, Vertex vertex);

void GenerateSubsimplices(Simplex &simplex, SimplexList &subsimplices);
void AddSubsimplices(Simplex &simplex, SimplexList &simplexList);

bool FindDuplicates(SimplexList &simplexList);

bool operator==(Simplex &a, Simplex &b);

#endif /* SIMPLEX_H */
