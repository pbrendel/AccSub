#ifndef MPIDATA_H
#define	MPIDATA_H

#include "Simplex.h"

#include <set>

class IncidenceGraph;

#ifdef DEBUG_MEMORY
#include "../Helpers/DebugMemory.h"
#endif

namespace MPIData
{

#ifdef DEBUG_MEMORY
class SimplexData : public DebugMemory<SimplexData>
#else
class SimplexData
#endif
{
    
    int *buffer;
    int size;

    int CalcBufferSize(const SimplexPtrList &simplexPtrList, int borderVertsCount, int simplexSize);

public:

    SimplexData(int *buffer, int size);
    SimplexData(const SimplexPtrList &simplexPtrList, const std::set<Vertex> &borderVerts, int dim, int acyclicTestNumber, int simplexSize = 0);
    ~SimplexData();
    
    int *GetBuffer() const { return buffer; }
    int GetSize() const { return size; }

    void GetSimplexData(SimplexList &simplexList, std::set<Vertex> &borderVerts, int &dim, int &acyclicTestNumber);

};

#ifdef DEBUG_MEMORY
class IncidenceGraphData : public DebugMemory<IncidenceGraphData>
#else
class IncidenceGraphData
#endif
{
    int *buffer;
    int size;

    int CalcBufferSize(const IncidenceGraph *ig);

public:

    IncidenceGraphData(int *buffer, int size);
    IncidenceGraphData(const IncidenceGraph *ig);
    ~IncidenceGraphData();

    int *GetBuffer() const { return buffer; }
    int GetSize() const { return size; }

    IncidenceGraph *GetIncidenceGraph(const SimplexPtrList &simplexPtrList);
};

}

#endif	/* MPIDATA_H */

