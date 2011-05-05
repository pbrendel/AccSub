#include <set>

#include "MPIData.h"
#include "IncidenceGraph.h"

using namespace MPIData;

////////////////////////////////////////////////////////////////////////////////

SimplexData::SimplexData(int* buffer, int size)
{
    this->buffer = buffer;
    this->size = size;
}

SimplexData::SimplexData(const SimplexPtrList& simplexPtrList, const std::set<Vertex>& borderVerts, int dim, int acyclicTestNumber, int simplexSize)
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
    // wymiar
    buffer[index++] = dim;
    // numer testu acyklicznosc
    buffer[index++] = acyclicTestNumber;
    // dane sympleksow
    if (simplexSize == 0)
    {
        for (SimplexPtrList::const_iterator i = simplexPtrList.begin(); i != simplexPtrList.end(); i++)
        {
            buffer[index++] = (*i)->size();
            for (Simplex::const_iterator v = (*i)->begin(); v != (*i)->end(); v++)
            {
                buffer[index++] = (*v);
            }
        }
    }
    else
    {
        for (SimplexPtrList::const_iterator i = simplexPtrList.begin(); i != simplexPtrList.end(); i++)
        {
            for (Simplex::const_iterator v = (*i)->begin(); v != (*i)->end(); v++)
            {
                buffer[index++] = (*v);
            }
        }
    }
    buffer[index++] = borderVerts.size();
    for (std::set<Vertex>::const_iterator v = borderVerts.begin(); v != borderVerts.end(); v++)
    {
        buffer[index++] = (*v);
    }
}

SimplexData::~SimplexData()
{
    delete [] buffer;
}

int SimplexData::CalcBufferSize(const SimplexPtrList& simplexPtrList, int borderVertsCount, int simplexSize)
{
    int size = 0;
    if (simplexSize == 0)
    {
        for (SimplexPtrList::const_iterator i = simplexPtrList.begin(); i != simplexPtrList.end(); i++)
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
    // - wymiar
    // - numer testu acyklicznosci
    // - rozmiar borderVerts
    return size + borderVertsCount + 5;
}

void SimplexData::GetSimplexData(SimplexList& simplexList, std::set<Vertex>& borderVerts, int &dim, int &acyclicTestNumber)
{
    int index = 0;
    int simplexSize = buffer[index++];
    int simplexCount = buffer[index++];
    dim = buffer[index++];
    acyclicTestNumber = buffer[index++];
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

////////////////////////////////////////////////////////////////////////////////

IncidenceGraphData::IncidenceGraphData(int* buffer, int size)
{
    this->buffer = buffer;
    this->size = size;
}

IncidenceGraphData::IncidenceGraphData(const IncidenceGraph* ig)
{
    size = CalcBufferSize(ig);
    std::vector<int> simplicesOnBorder;
    std::vector<int> simplicesInAcyclicSubset;
    buffer = new int[size];
    int index = 0;
    // wymiar
    buffer[index++] = ig->params.dim;
    // ilosc wierzcholkow grafu
    buffer[index++] = ig->nodes.size();
    for (IncidenceGraph::Nodes::const_iterator node = ig->nodes.begin(); node != ig->nodes.end(); node++)
    {
        if ((*node)->IsOnBorder())
        {
            simplicesOnBorder.push_back((*node)->newIndex);
        }
        if ((*node)->IsAcyclic())
        {
            simplicesInAcyclicSubset.push_back((*node)->newIndex);
        }
        buffer[index++] = (*node)->index;
        buffer[index++] = (*node)->newIndex;
        buffer[index++] = (*node)->GetAcyclicIntersectionFlags();
        buffer[index++] = (*node)->edges.size();
        for (IncidenceGraph::Edges::const_iterator edge = (*node)->edges.begin(); edge != (*node)->edges.end(); edge++)
        {
            buffer[index++] = edge->node->newIndex;
        }
    }
    buffer[index++] = simplicesOnBorder.size();
    for (std::vector<int>::iterator i = simplicesOnBorder.begin(); i != simplicesOnBorder.end(); i++)
    {
        buffer[index++] = (*i);
    }
    buffer[index++] = simplicesInAcyclicSubset.size();
    for (std::vector<int>::iterator i = simplicesInAcyclicSubset.begin(); i != simplicesInAcyclicSubset.end(); i++)
    {
        buffer[index++] = (*i);
    }
    buffer[index++] = ig->connectedComponents.size();
    for (IncidenceGraph::ConnectedComponents::const_iterator i = ig->connectedComponents.begin(); i != ig->connectedComponents.end(); i++)
    {
        buffer[index++] = (*i)->newIndex;
    }
    for (std::vector<VertsSet>::const_iterator i = ig->connectedComponentsBorders.begin(); i != ig->connectedComponentsBorders.end(); i++)
    {
        buffer[index++] = i->size();
        for (VertsSet::const_iterator v = i->begin(); v != i->end(); v++)
        {
            buffer[index++] = (*v);
        }
    }
    for (std::vector<int>::const_iterator i = ig->connectedComponentsAcyclicSubsetSize.begin(); i != ig->connectedComponentsAcyclicSubsetSize.end(); i++)
    {
        buffer[index++] = (*i);
    }
}

IncidenceGraphData::~IncidenceGraphData()
{
    delete [] buffer;
}

////////////////////////////////////////////////////////////////////////////////

int IncidenceGraphData::CalcBufferSize(const IncidenceGraph* ig)
{
    int size = 0;
    for (IncidenceGraph::Nodes::const_iterator node = ig->nodes.begin(); node != ig->nodes.end(); node++)
    {
        // jezeli jest na brzegu to bedziemy zapisywali jego indeks
        if ((*node)->IsOnBorder())
        {
            size++;
        }
        // podobnie ma sie sprawa z sympleksami w zbiorze acyklicznym
        if ((*node)->IsAcyclic())
        {
            size++;
        }
        // 4 inty to:
        // - index
        // - newIndex
        // - acyclicIntersectionFlags
        // - edges.size()
        size += ((*node)->edges.size() + 4);
    }
    int connectedComponentsCount = ig->connectedComponents.size();
    for (std::vector<VertsSet>::const_iterator i = ig->connectedComponentsBorders.begin(); i != ig->connectedComponentsBorders.end(); i++)
    {
        // ilosc wierzcholkow + same wierzcholki
        size += (i->size() + 1);
    }
    // dodatkowe inty to:
    // - wymiar
    // - ilosc wierzcholkow grafu
    // - ilosc sympleksow na brzegu
    // - ilosc sympleksow w zbiorze acyklicznym
    // - ilosc skladowych spojnych
    // connectedComponentsCount mnozymy razy 2 bo bedziemy zapisywali
    // dla kazdej skladowej indeks jej reprezentanta i rozmiar podzbioru
    // acyklicznego
    return size + connectedComponentsCount * 2 + 5;
}

////////////////////////////////////////////////////////////////////////////////

IncidenceGraph *IncidenceGraphData::GetIncidenceGraph(const SimplexPtrList &simplexPtrList)
{
    int index = 0;
    
    IncidenceGraph::Params params(buffer[index++], false, false);
    IncidenceGraph *ig = new IncidenceGraph(params);

    // tworzymy node'y
    std::vector<std::vector<int> > edges;
    int nodesCount = buffer[index++];
    for (int i = 0; i < nodesCount; i++)
    {
        int ind = buffer[index++];
        int newIndex = buffer[index++];
        int acyclicIntersectionFlags = buffer[index++];
        int edgesCount = buffer[index++];
        IncidenceGraph::Node *node = new IncidenceGraph::Node(ig, const_cast<Simplex *>(simplexPtrList.at(ind)), newIndex);
        node->UpdateAcyclicIntersectionFlags(acyclicIntersectionFlags, 0);
        ig->nodes.push_back(node);
        std::vector<int> e;
        for (int j = 0; j < edgesCount; j++)
        {
            e.push_back(buffer[index++]);
        }
        edges.push_back(e);
    }

    // tworzymy krawedzie
    std::vector<std::vector<int> >::iterator e = edges.begin();
    for (IncidenceGraph::Nodes::iterator node = ig->nodes.begin(); node != ig->nodes.end(); node++)
    {
        for (std::vector<int>::iterator i = e->begin(); i != e->end(); i++)
        {
            (*node)->AddNeighbour(ig->nodes.at(*i));
        }
        e++;
    }

    // zaznaczamy sympleksy brzegu
    int simplicesOnBorderCount = buffer[index++];
    for (int i = 0; i < simplicesOnBorderCount; i++)
    {
        ig->nodes.at(buffer[index++])->IsOnBorder(true);
    }
    // i sympleksy w zbiorze acyklicznym
    int simplicesInAcyclicSubsetCount = buffer[index++];
    for (int i = 0; i < simplicesInAcyclicSubsetCount; i++)
    {
        ig->nodes.at(buffer[index++])->IsAcyclic(true);
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
        ig->connectedComponentsAcyclicSubsetSize.push_back(buffer[index++]);
    }

    return ig;
}

////////////////////////////////////////////////////////////////////////////////
// eof
