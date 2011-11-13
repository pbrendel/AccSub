/*
 * File:   SimplexUtils.h
 * Author: Piotr Brendel
 */

#ifndef SIMPLEX_H
#define SIMPLEX_H

#include <vector>
#include <set>

typedef unsigned int Vertex;
typedef std::set<Vertex> VertsSet;
typedef std::vector<Vertex> Simplex;
typedef std::vector<Simplex> SimplexList;
typedef Simplex* SimplexPtr;
typedef std::vector<SimplexPtr> SimplexPtrList;

int GetDimension(const SimplexList &simplexList);
int GetDimension(const SimplexPtrList &simplexPtrList);
int GetConstantSimplexSize(const SimplexList &simplexList);
int GetConstantSimplexSize(const SimplexPtrList &simplexPtrList);

bool GetIntersection(const Simplex *a, const Simplex *b, Simplex &intersection);
Vertex GetVertexFromIntersection(Simplex *a, Simplex *b);
Vertex GetVertexFromIntersectionNotEqual(Simplex *a, Simplex *b, Vertex vertex);
Vertex GetVertexFromIntersectionNotIn(Simplex *a, Simplex *b, const VertsSet &vertsSet);
bool ContainsVertex(Simplex *a, Vertex vertex);

void GenerateProperFaces(Simplex &simplex, SimplexList &subsimplices);
void AddProperFaces(Simplex &simplex, SimplexList &simplexList);

bool SimplexSorterSize(const Simplex &a, const Simplex &b);

bool operator==(const Simplex &a, const Simplex &b);

#endif /* SIMPLEX_H */
