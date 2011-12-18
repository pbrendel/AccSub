/* 
 * File:   IncidenceGraphHelpers.h
 * Author: Piotr Brendel
 */

#ifndef INCIDENCEGRAPHHELPERS_H
#define	INCIDENCEGRAPHHELPERS_H

#include "IncidenceGraph.h"

class IncidenceGraphHelpers
{
public:
    
    static IncidenceGraph *Create(SimplexList &simplexList);
    static IncidenceGraph *CreateWithBorder(SimplexList &simplexList, const VertsSet &borderVerts);
    static IncidenceGraph *CreateWithBorder(SimplexPtrList &simplexPtrList, const VertsSet &borderVerts);
    static IncidenceGraph *CreateAndCalculateAccSub(SimplexList &simplexList, AccTest<IncidenceGraph::IntersectionFlags> *test);
    static IncidenceGraph *CreateAndCalculateAccSubWithBorder(SimplexList &simplexList, const VertsSet &borderVerts, AccTest<IncidenceGraph::IntersectionFlags> *test);
    static IncidenceGraph *CreateAndCalculateAccSubIG(SimplexList &simplexList, AccTest<IncidenceGraph::IntersectionFlags> *test);
    static IncidenceGraph *CreateAndCalculateAccSubIGWithBorder(SimplexList &simplexList, const VertsSet &borderVerts, AccTest<IncidenceGraph::IntersectionFlags> *test);
    static IncidenceGraph *CreateAndCalculateAccSubIGWithBorder(SimplexPtrList &simplexPtrList, const VertsSet &borderVerts, AccTest<IncidenceGraph::IntersectionFlags> *test);
    static IncidenceGraph *CreateAndCalculateAccSubST(SimplexList &simplexList, AccTest<IncidenceGraph::IntersectionFlags> *test);
    static IncidenceGraph *CreateAndCalculateAccSubST(SimplexPtrList &simplexPtrList, AccTest<IncidenceGraph::IntersectionFlags> *test);
    static IncidenceGraph *CreateAndCalculateAccSubSTWithBorder(SimplexList &simplexList, const VertsSet &borderVerts, AccTest<IncidenceGraph::IntersectionFlags> *test);
    static IncidenceGraph *CreateAndCalculateAccSubSTWithBorder(SimplexPtrList &simplexPtrList, const VertsSet &borderVerts, AccTest<IncidenceGraph::IntersectionFlags> *test);
    static IncidenceGraph *CreateAndCalculateAccSubParallel(SimplexList &simplexList, int packsCount, AccSubAlgorithm accSubAlgorithm, AccTest<IncidenceGraph::IntersectionFlags> *test);
};

#endif	/* INCIDENCEGRAPHHELPERS_H */

