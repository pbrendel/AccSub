#ifndef SIMPLEX_H
#define SIMPLEX_H

#include <vector>
#include <set>

typedef int Vertex;
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
