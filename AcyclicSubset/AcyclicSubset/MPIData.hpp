/*
 * File:   MPIData.hpp
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 */

#ifndef MPIDATA_HPP
#define	MPIDATA_HPP

#include <set>

////////////////////////////////////////////////////////////////////////////////

template<typename IncidenceGraph>
class MPISimplexData
{
    typedef typename IncidenceGraph::Vertex Vertex;
    typedef typename IncidenceGraph::Simplex Simplex;
    typedef typename IncidenceGraph::SimplexList SimplexList;
    typedef typename IncidenceGraph::SimplexPtrList SimplexPtrList;

    int *buffer;
    int size;

    int CalcBufferSize(const SimplexPtrList &simplexPtrList, int borderVertsCount, int simplexSize)
    {
        int size = 0;
        if (simplexSize == 0)
        {
            for (typename SimplexPtrList::const_iterator i = simplexPtrList.begin(); i != simplexPtrList.end(); i++)
            {
                size += ((*i)->size() + 1); // ilosc wierzcholkow + ich numery
            }
        }
        else
        {
            size = simplexSize * simplexPtrList.size();
        }
        // additional ints are:
        // - simplexSize
        // - simplexList size
        // - borderVerts size
        // - accSubAlgorithm
        // - accTest id
        return size + borderVertsCount + 5;
    }

public:

    MPISimplexData(int *buffer, int size)
    {
        this->buffer = buffer;
        this->size = size;
    }

    MPISimplexData(const SimplexPtrList &simplexPtrList, const std::set<Vertex> &borderVerts, int accSubAlgorithm, int accTestNumber, int simplexSize)
    {
        size = CalcBufferSize(simplexPtrList, borderVerts.size(), simplexSize);
        buffer = new int[size];

        int index = 0;

        // number of vertices in each simplex
        // if == 0 then we write this value for each simplex
        // if != 0 then all simplices share the same value
        buffer[index++] = simplexSize;
        buffer[index++] = simplexPtrList.size();
        buffer[index++] = accSubAlgorithm;
        buffer[index++] = accTestNumber;
        if (simplexSize == 0)
        {
            for (typename SimplexPtrList::const_iterator i = simplexPtrList.begin(); i != simplexPtrList.end(); i++)
            {
                buffer[index++] = (*i)->size();
                for (typename Simplex::const_iterator v = (*i)->begin(); v != (*i)->end(); v++)
                {
                    buffer[index++] = (*v);
                }
            }
        }
        else
        {
            for (typename SimplexPtrList::const_iterator i = simplexPtrList.begin(); i != simplexPtrList.end(); i++)
            {
                for (typename Simplex::const_iterator v = (*i)->begin(); v != (*i)->end(); v++)
                {
                    buffer[index++] = (*v);
                }
            }
        }
        buffer[index++] = borderVerts.size();
        for (typename std::set<Vertex>::const_iterator v = borderVerts.begin(); v != borderVerts.end(); v++)
        {
            buffer[index++] = (*v);
        }
    }

    ~MPISimplexData()
    {
        delete [] buffer;
    }
    
    int *GetBuffer() const { return buffer; }
    int GetSize() const { return size; }

    void GetSimplexData(SimplexList &simplexList, std::set<Vertex> &borderVerts, int &accSubAlgorithm, int &accTestNumber)
    {
        int index = 0;
        int simplexSize = buffer[index++];
        int simplexCount = buffer[index++];
        accSubAlgorithm = buffer[index++];
        accTestNumber = buffer[index++];
        if (simplexSize == 0)
        {
            for (int i = 0; i < simplexCount; i++)
            {
                int size = buffer[index++];
                Simplex simplex;
                for (int j = 0; j < size; j++)
                {
                    simplex.push_back(buffer[index++]);
                }
                simplexList.push_back(simplex);
            }
        }
        else
        {
            for (int i = 0; i < simplexCount; i++)
            {
                Simplex simplex;
                for (int j = 0; j < simplexSize; j++)
                {
                    simplex.push_back(buffer[index++]);
                }
                simplexList.push_back(simplex);
            }
        }
        int borderVertsCount = buffer[index++];
        for (int i = 0; i < borderVertsCount; i++)
        {
            borderVerts.insert(buffer[index++]);
        }
    }

};

////////////////////////////////////////////////////////////////////////////////

template<typename IncidenceGraph>
class MPIIncidenceGraphData
{
    typedef typename IncidenceGraph::Vertex Vertex;
    typedef typename IncidenceGraph::VertsSet VertsSet;
    typedef typename IncidenceGraph::Simplex Simplex;
    typedef typename IncidenceGraph::SimplexPtrList SimplexPtrList;

    int *buffer;
    int size;

    int CalcBufferSize(const IncidenceGraph *ig)
    {
        int size = 0;
        for (typename IncidenceGraph::Nodes::const_iterator node = ig->nodes.begin(); node != ig->nodes.end(); node++)
        {
            if ((*node)->IsInAccSub())
            {
                continue;
            }
            if ((*node)->IsOnBorder())
            {
                size++;
            }
            size += (*node)->GetAccInfo().GetBufferSize();
            // additional 2 ints are:
            // - index
            // - newIndex
            size += 2;
        }
        for (typename IncidenceGraph::Edges::const_iterator edge = ig->edges.begin(); edge != ig->edges.end(); edge++)
        {
            if (!(*edge)->nodeA->IsInAccSub() && !(*edge)->nodeB->IsInAccSub())
            {
                size += 2;
            }
        }
        int connectedComponentsCount = ig->connectedComponents.size();
        for (typename std::vector<VertsSet>::const_iterator i = ig->connectedComponentsBorders.begin(); i != ig->connectedComponentsBorders.end(); i++)
        {
            // number of vertices + vertices
            size += (i->size() + 1);
        }
        // additional ints are:
        // - dim
        // - graph nodes count
        // - graph edges count
        // - number of boundary simplices
        // - size of acylic subset
        // - number of connected components
        // connected components are multiplied by two because for each
        // connected component we write its id and size of acyclic subset
        return size + connectedComponentsCount * 2 + 6;
    }

public:

    MPIIncidenceGraphData(int *buffer, int size)
    {
        this->buffer = buffer;
        this->size = size;
    }
    
    MPIIncidenceGraphData(const IncidenceGraph *ig)
    {
        size = CalcBufferSize(ig);
        std::vector<int> simplicesOnBorder;
        buffer = new int[size];
        int index = 0;
        buffer[index++] = ig->dim;
        int nodesCount = 0;
        for (typename IncidenceGraph::Nodes::const_iterator node = ig->nodes.begin(); node != ig->nodes.end(); node++)
        {
            if (!(*node)->IsInAccSub())
            {
                nodesCount++;
            }
        }
        buffer[index++] = nodesCount;
        for (typename IncidenceGraph::Nodes::const_iterator node = ig->nodes.begin(); node != ig->nodes.end(); node++)
        {
            if ((*node)->IsInAccSub())
            {
                continue;
            }
            if ((*node)->IsOnBorder())
            {
                simplicesOnBorder.push_back((*node)->index);
            }
            buffer[index++] = (*node)->helpers.i;
            buffer[index++] = (*node)->index;
            (*node)->GetAccInfo().WriteToBuffer(buffer, index);
        }
        int edgesCount = 0;
        for (typename IncidenceGraph::Edges::const_iterator edge = ig->edges.begin(); edge != ig->edges.end(); edge++)
        {
            if (!(*edge)->nodeA->IsInAccSub() && !(*edge)->nodeB->IsInAccSub())
            {
                edgesCount++;
            }
        }
        buffer[index++] = edgesCount;
        for (typename IncidenceGraph::Edges::const_iterator edge = ig->edges.begin(); edge != ig->edges.end(); edge++)
        {
            if ((*edge)->nodeA->IsInAccSub() || (*edge)->nodeB->IsInAccSub())
            {
                continue;
            }
            buffer[index++] = (*edge)->nodeA->index;
            buffer[index++] = (*edge)->nodeB->index;
        }
        buffer[index++] = simplicesOnBorder.size();
        for (std::vector<int>::iterator i = simplicesOnBorder.begin(); i != simplicesOnBorder.end(); i++)
        {
            buffer[index++] = (*i);
        }
        buffer[index++] = ig->connectedComponents.size();
        for (typename IncidenceGraph::ConnectedComponents::const_iterator i = ig->connectedComponents.begin(); i != ig->connectedComponents.end(); i++)
        {
            buffer[index++] = (*i)->index;
        }
        for (typename std::vector<VertsSet>::const_iterator i = ig->connectedComponentsBorders.begin(); i != ig->connectedComponentsBorders.end(); i++)
        {
            buffer[index++] = i->size();
            for (typename VertsSet::const_iterator v = i->begin(); v != i->end(); v++)
            {
                buffer[index++] = (*v);
            }
        }
        for (std::vector<int>::const_iterator i = ig->connectedComponentsAccSubSize.begin(); i != ig->connectedComponentsAccSubSize.end(); i++)
        {
            buffer[index++] = (*i);
        }
    }

    ~MPIIncidenceGraphData()
    {
        delete [] buffer;
    }

    int *GetBuffer() const { return buffer; }
    int GetSize() const { return size; }

    IncidenceGraph *GetIncidenceGraph(const SimplexPtrList &simplexPtrList)
    {
        int index = 0;

        int dim = buffer[index++];
        IncidenceGraph *ig = new IncidenceGraph(dim);

        int nodesCount = buffer[index++];
        for (int i = 0; i < nodesCount; i++)
        {
            int simplexIndex = buffer[index++];
            int nodeIndex = buffer[index++];
            typename IncidenceGraph::Node *node = new typename IncidenceGraph::Node(ig, const_cast<Simplex *>(simplexPtrList.at(simplexIndex)), nodeIndex);
            node->GetAccInfo().ReadFromBuffer(buffer, index);
            ig->nodes.push_back(node);
        }
        int edgesCount = buffer[index++];
        for (int i = 0; i < edgesCount; i++)
        {
            typename IncidenceGraph::Node *nodeA = ig->nodes[buffer[index++]];
            typename IncidenceGraph::Node *nodeB = ig->nodes[buffer[index++]];
            typename IncidenceGraph::Edge *edge = new typename IncidenceGraph::Edge(nodeA, nodeB);
            ig->edges.push_back(edge);
            nodeA->AddEdge(edge);
            nodeB->AddEdge(edge);
        }
        int simplicesOnBorderCount = buffer[index++];
        for (int i = 0; i < simplicesOnBorderCount; i++)
        {
            ig->nodes.at(buffer[index++])->IsOnBorder(true);
        }
        int connectedComponentsCount = buffer[index++];
        for (int i = 0; i < connectedComponentsCount; i++)
        {
            ig->connectedComponents.push_back(ig->nodes.at(buffer[index++]));
        }
        for (int i = 0; i < connectedComponentsCount; i++)
        {
            int size = buffer[index++];
            VertsSet vs;
            for (int j = 0; j < size; j++)
            {
                vs.insert(buffer[index++]);
            }
            ig->connectedComponentsBorders.push_back(vs);
        }
        for (int i = 0; i < connectedComponentsCount; i++)
        {
            ig->connectedComponentsAccSubSize.push_back(buffer[index++]);
        }
        return ig;
    }
};

////////////////////////////////////////////////////////////////////////////////

#endif	/* MPIDATA_HPP */

