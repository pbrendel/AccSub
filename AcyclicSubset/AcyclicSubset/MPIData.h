/*
 * File:   MPIData.h
 * Author: Piotr Brendel
 */

#ifndef MPIDATA_H
#define	MPIDATA_H

#include "IncidenceGraph.h"

#include <set>

namespace MPIData
{

template<typename IncidenceGraph>
class SimplexData
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
        // dodatkowe inty to:
        // - wartosc simplexSize
        // - rozmiar simplexList
        // - rozmiar borderVerts
        // - rodzaj testu acyklicznosci
        // - rodzaj algorytmu
        return size + borderVertsCount + 5;
    }

public:

    SimplexData(int *buffer, int size)
    {
        this->buffer = buffer;
        this->size = size;
    }

    SimplexData(const SimplexPtrList &simplexPtrList, const std::set<Vertex> &borderVerts, int accSubAlgorithm, int accTestNumber, int simplexSize)
    {
        size = CalcBufferSize(simplexPtrList, borderVerts.size(), simplexSize);
        buffer = new int[size];

        int index = 0;

        // ilosc punktow sympleksu
        // jezeli == 0 to dla kazdego sympleksu zapisujemy odpowiednia wartosc
        // jezeli != 0 to wszystkie sympleksy maja taka sama ilosc punktow
        buffer[index++] = simplexSize;
        // ilosc sympleksow
        buffer[index++] = simplexPtrList.size();
        // typ algorytmu obliczania podzbioru acyklicznego
        buffer[index++] = accSubAlgorithm;
        // numer testu acyklicznosci
        buffer[index++] = accTestNumber;
        // dane sympleksow
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

    ~SimplexData()
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

template<typename IncidenceGraph>
class IncidenceGraphData
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
            // nie zapisujemy sympleksow ze zbioru acyklicznego
            if ((*node)->IsInAccSub())
            {
                continue;
            }
            // jezeli jest na brzegu to bedziemy zapisywali jego indeks
            if ((*node)->IsOnBorder())
            {
                size++;
            }
            size += (*node)->GetAccInfo().BufferSize();
            // 2 inty to:
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
            // ilosc wierzcholkow + same wierzcholki
            size += (i->size() + 1);
        }
        // dodatkowe inty to:
        // - wymiar
        // - ilosc wierzcholkow grafu
        // - ilosc krawedzi grafu
        // - ilosc sympleksow na brzegu
        // - ilosc sympleksow w zbiorze acyklicznym
        // - ilosc skladowych spojnych
        // connectedComponentsCount mnozymy razy 2 bo bedziemy zapisywali
        // dla kazdej skladowej indeks jej reprezentanta i rozmiar podzbioru
        // acyklicznego
        return size + connectedComponentsCount * 2 + 6;
    }

public:

    IncidenceGraphData(int *buffer, int size)
    {
        this->buffer = buffer;
        this->size = size;
    }
    
    IncidenceGraphData(const IncidenceGraph *ig)
    {
        size = CalcBufferSize(ig);
        std::vector<int> simplicesOnBorder;
        buffer = new int[size];
        int index = 0;
        // wymiar
        buffer[index++] = ig->dim;
        // ilosc wierzcholkow grafu
        int nodesCount = 0;
        for (typename IncidenceGraph::Nodes::const_iterator node = ig->nodes.begin(); node != ig->nodes.end(); node++)
        {
            if (!(*node)->IsInAccSub())
            {
                nodesCount++;
            }
        }
        buffer[index++] = nodesCount;
        // wierzcholki grafu
        for (typename IncidenceGraph::Nodes::const_iterator node = ig->nodes.begin(); node != ig->nodes.end(); node++)
        {
             // nie zapisujemy sympleksow ze zbioru acyklicznego
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
        // ilosc krawedzi grafu
        int edgesCount = 0;
        for (typename IncidenceGraph::Edges::const_iterator edge = ig->edges.begin(); edge != ig->edges.end(); edge++)
        {
            if (!(*edge)->nodeA->IsInAccSub() && !(*edge)->nodeB->IsInAccSub())
            {
                edgesCount++;
            }
        }
        buffer[index++] = edgesCount;
        // krawedzie grafu
        for (typename IncidenceGraph::Edges::const_iterator edge = ig->edges.begin(); edge != ig->edges.end(); edge++)
        {
            // nie zapisujemy krawedzi pomiedzy wierzcholkami
            // ktore znajduja sie w zbiorze acyklicznym
            if ((*edge)->nodeA->IsInAccSub() || (*edge)->nodeB->IsInAccSub())
            {
                continue;
            }
            buffer[index++] = (*edge)->nodeA->index;
            buffer[index++] = (*edge)->nodeB->index;
        }
        // sympleksy brzegowe
        buffer[index++] = simplicesOnBorder.size();
        for (std::vector<int>::iterator i = simplicesOnBorder.begin(); i != simplicesOnBorder.end(); i++)
        {
            buffer[index++] = (*i);
        }
        // spojne skladowe
        buffer[index++] = ig->connectedComponents.size();
        for (typename IncidenceGraph::ConnectedComponents::const_iterator i = ig->connectedComponents.begin(); i != ig->connectedComponents.end(); i++)
        {
            buffer[index++] = (*i)->index;
        }
        // wierzcholki brzegowe dla spojnych skladowych
        for (typename std::vector<VertsSet>::const_iterator i = ig->connectedComponentsBorders.begin(); i != ig->connectedComponentsBorders.end(); i++)
        {
            buffer[index++] = i->size();
            for (typename VertsSet::const_iterator v = i->begin(); v != i->end(); v++)
            {
                buffer[index++] = (*v);
            }
        }
        // rozmiary podzbiorow acyklicznych w spojnych skladowych
        for (std::vector<int>::const_iterator i = ig->connectedComponentsAccSubSize.begin(); i != ig->connectedComponentsAccSubSize.end(); i++)
        {
            buffer[index++] = (*i);
        }
    }

    ~IncidenceGraphData()
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

        // tworzymy node'y
        int nodesCount = buffer[index++];
        for (int i = 0; i < nodesCount; i++)
        {
            int simplexIndex = buffer[index++];
            int nodeIndex = buffer[index++];
            typename IncidenceGraph::Node *node = new IncidenceGraph::Node(ig, const_cast<Simplex *>(simplexPtrList.at(simplexIndex)), nodeIndex);
            node->GetAccInfo().ReadFromBuffer(buffer, index);
            ig->nodes.push_back(node);
        }
        // tworzymy krawedzie
        int edgesCount = buffer[index++];
        for (int i = 0; i < edgesCount; i++)
        {
            typename IncidenceGraph::Node *nodeA = ig->nodes[buffer[index++]];
            typename IncidenceGraph::Node *nodeB = ig->nodes[buffer[index++]];
            typename IncidenceGraph::Edge *edge = new IncidenceGraph::Edge(nodeA, nodeB);
            ig->edges.push_back(edge);
            nodeA->AddEdge(edge);
            nodeB->AddEdge(edge);
        }
        // zaznaczamy sympleksy brzegu
        int simplicesOnBorderCount = buffer[index++];
        for (int i = 0; i < simplicesOnBorderCount; i++)
        {
            ig->nodes.at(buffer[index++])->IsOnBorder(true);
        }
        // dane poszczegolnych skladowych spojnych
        int connectedComponentsCount = buffer[index++];
        // reprezentanci
        for (int i = 0; i < connectedComponentsCount; i++)
        {
            ig->connectedComponents.push_back(ig->nodes.at(buffer[index++]));
        }
        // wierzcholki w brzegu
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
        // rozmiar podzbioru acyklicznego
        for (int i = 0; i < connectedComponentsCount; i++)
        {
            ig->connectedComponentsAccSubSize.push_back(buffer[index++]);
        }
        return ig;
    }
};

}

#endif	/* MPIDATA_H */

