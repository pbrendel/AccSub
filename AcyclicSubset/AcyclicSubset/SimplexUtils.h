/* 
 * File:   SimplexUtils.h
 * Author: Piotr Brendel
 */

#ifndef SIMPLEXUTILS_H
#define	SIMPLEXUTILS_H

#include "Simplex.h"

void ReadSimplexList(SimplexList &simplexList, const char *filename, bool sortVerts);
void GenerateSimplexList(SimplexList &simplexList, int simplicesCount, int vertsCount, int dim);
int GenerateReverseSimplexList(SimplexList &simplexList, int simplicesCount, int vertsCount, int dim);
void GenerateAccTree(SimplexList &simplexList, int simplicesCount, int nodesCount);

bool FindDuplicates(SimplexList &simplexList);

#endif	/* SIMPLEXUTILS_H */

