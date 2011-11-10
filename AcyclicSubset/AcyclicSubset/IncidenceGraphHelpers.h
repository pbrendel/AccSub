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
    static IncidenceGraph *CreateAndCalculateAcyclicSubset(SimplexList &simplexList, AcyclicTest<IncidenceGraph::IntersectionFlags> *test);
    static IncidenceGraph *CreateAndCalculateAcyclicSubsetWithBorder(SimplexList &simplexList, const VertsSet &borderVerts, AcyclicTest<IncidenceGraph::IntersectionFlags> *test);
    static IncidenceGraph *CreateAndCalculateAcyclicSubsetOnline(SimplexList &simplexList, AcyclicTest<IncidenceGraph::IntersectionFlags> *test);
    static IncidenceGraph *CreateAndCalculateAcyclicSubsetOnlineWithBorder(SimplexList &simplexList, const VertsSet &borderVerts, AcyclicTest<IncidenceGraph::IntersectionFlags> *test);
    static IncidenceGraph *CreateAndCalculateAcyclicSubsetOnlineWithBorder(SimplexPtrList &simplexPtrList, const VertsSet &borderVerts, AcyclicTest<IncidenceGraph::IntersectionFlags> *test);
    static IncidenceGraph *CreateAndCalculateAcyclicSubsetSpanningTree(SimplexList &simplexList, AcyclicTest<IncidenceGraph::IntersectionFlags> *test);
    static IncidenceGraph *CreateAndCalculateAcyclicSubsetSpanningTree(SimplexPtrList &simplexPtrList, AcyclicTest<IncidenceGraph::IntersectionFlags> *test);
    static IncidenceGraph *CreateAndCalculateAcyclicSubsetSpanningTreeWithBorder(SimplexList &simplexList, const VertsSet &borderVerts, AcyclicTest<IncidenceGraph::IntersectionFlags> *test);
    static IncidenceGraph *CreateAndCalculateAcyclicSubsetSpanningTreeWithBorder(SimplexPtrList &simplexPtrList, const VertsSet &borderVerts, AcyclicTest<IncidenceGraph::IntersectionFlags> *test);
    static IncidenceGraph *CreateAndCalculateAcyclicSubsetParallel(SimplexList &simplexList, int packsCount, AccSubAlgorithm accSubAlgorithm, AcyclicTest<IncidenceGraph::IntersectionFlags> *test);        
};

#endif	/* INCIDENCEGRAPHHELPERS_H */

