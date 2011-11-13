/*
 * File:   Simplex.h
 * Author: Piotr Brendel
 */

#ifndef SIMPLEX_H
#define SIMPLEX_H

#include "SimplexT.hpp"
#include <set>

typedef unsigned int Vertex;
typedef std::set<Vertex> VertsSet;
//typedef std::vector<Vertex> Simplex;
typedef SimplexT<Vertex> Simplex;
typedef std::vector<Simplex> SimplexList;
typedef Simplex* SimplexPtr;
typedef std::vector<SimplexPtr> SimplexPtrList;

#endif /* SIMPLEX_H */
