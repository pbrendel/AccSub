#include "IncidenceGraph.h"
#include "IncidenceGraphUtils.h"
#include "ParallelGraph.h"

#define USE_HELPERS

#ifdef USE_HELPERS
#include "../Helpers/Utils.h"
#endif

#include <map>
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <cstring>

////////////////////////////////////////////////////////////////////////////////

IncidenceGraph::IncidenceGraph(const Params &p) : params(p)
{
    CreateConfigurationsFlags(params.dim, configurationsFlags, subconfigurationsFlags);
}

IncidenceGraph::IncidenceGraph(SimplexList &simplexList, const Params &p) : params(p)
{
    // najpierw tworzymy wszystkie node'y
    int index = 0;
    for (SimplexList::iterator i = simplexList.begin(); i != simplexList.end(); i++)
    {
        nodes.push_back(new Node(this, &(*i), index++));
    }
    
    // jezeli trzeba, sortujemy (zeby zaczac przegladanie od najwiekszych sympleksow)
    if (params.sortNodes)
    {
        std::sort(nodes.begin(), nodes.end(), Node::Sorter);
    }

    CreateConfigurationsFlags(params.dim, configurationsFlags, subconfigurationsFlags);
}

IncidenceGraph::IncidenceGraph(SimplexPtrList &simplexPtrList, const Params &p) : params(p)
{
    // najpierw tworzymy wszystkie node'y
    int index = 0;
    for (SimplexPtrList::iterator i = simplexPtrList.begin(); i != simplexPtrList.end(); i++)
    {
        nodes.push_back(new Node(this, (*i), index++));
    }

    // jezeli trzeba, sortujemy (zeby zaczac przegladanie od najwiekszych sympleksow)
    if (params.sortNodes)
    {
        std::sort(nodes.begin(), nodes.end(), Node::Sorter);
    }

    CreateConfigurationsFlags(params.dim, configurationsFlags, subconfigurationsFlags);
}

IncidenceGraph::~IncidenceGraph()
{
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        delete (*i);       
    }
    nodes.clear();
}

////////////////////////////////////////////////////////////////////////////////

IncidenceGraph *IncidenceGraph::Create(SimplexList& simplexList, const Params& params)
{
#ifdef USE_HELPERS
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexList, params);
    ig->CreateGraph(false);
    Timer::Update("incidence graph created");
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexList, params);
    ig->CreateGraph(false);
#endif
    return ig;
}

IncidenceGraph *IncidenceGraph::CreateWithBorder(SimplexList& simplexList, const VertsSet& borderVerts, const Params& params)
{
#ifdef USE_HELPERS
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexList, params);
    ig->borderVerts = borderVerts;
    ig->CreateGraphWithBorder(false);
    Timer::Update("incidence graph created");
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexList, params);
    ig->CreateGraph(false);
#endif
    return ig;
}

IncidenceGraph *IncidenceGraph::CreateWithBorder(SimplexPtrList& simplexPtrList, const VertsSet& borderVerts, const Params& params)
{
#ifdef USE_HELPERS
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexPtrList, params);
    ig->borderVerts = borderVerts;
    ig->CreateGraphWithBorder(false);
    Timer::Update("incidence graph created");
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexList, params);
    ig->CreateGraph(false);
#endif
    return ig;
}

IncidenceGraph *IncidenceGraph::CreateAndCalculateAcyclicSubset(SimplexList& simplexList, const Params& params, AcyclicTest<IntersectionFlags>* test)
{
#ifdef USE_HELPERS
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexList, params);
    ig->CreateGraph(params.minimizeSimplices);
    Timer::Update("incidence graph created");
    ig->CalculateAcyclicSubset(test);
    Timer::Update("acyclic subset calculated");
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexList, params);
    ig->CreateGraph(params.minimizeSimplices);
    ig->CalculateAcyclicSubset(test);
#endif
    return ig;
}

IncidenceGraph *IncidenceGraph::CreateAndCalculateAcyclicSubsetWithBorder(SimplexList& simplexList, const VertsSet &borderVerts, const Params& params, AcyclicTest<IntersectionFlags>* test)
{
#ifdef USE_HELPERS
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexList, params);
    ig->borderVerts = borderVerts;
    ig->CreateGraphWithBorder(params.minimizeSimplices);
    Timer::Update("incidence graph created");
    ig->CalculateAcyclicSubsetWithBorder(test);
    Timer::Update("acyclic subset calculated");
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexList, params);
    ig->CreateGraph(params.minimizeSimplices);
    ig->CalculateAcyclicSubset(test);
#endif
    return ig;
}

IncidenceGraph *IncidenceGraph::CreateAndCalculateAcyclicSubsetOnline(SimplexList& simplexList, const Params& params, AcyclicTest<IntersectionFlags>* test)
{
#ifdef USE_HELPERS
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexList, params);
    ig->CreateGraphAndCalculateAcyclicSubset(test);
    Timer::Update("incidence graph created and acyclic subset calculated");
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexList, params);
    ig->CreateGraphAndCalculateAcyclicSubset(test);
#endif
    return ig;
}

IncidenceGraph *IncidenceGraph::CreateAndCalculateAcyclicSubsetOnlineWithBorder(SimplexList& simplexList, const VertsSet &borderVerts, const Params& params, AcyclicTest<IntersectionFlags>* test)
{
#ifdef USE_HELPERS
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexList, params);
    ig->borderVerts = borderVerts;
    ig->CreateGraphAndCalculateAcyclicSubsetWithBorder(test);
    Timer::Update("incidence graph created and acyclic subset calculated");
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexList, params);
    ig->CreateGraphAndCalculateAcyclicSubset(test);
#endif
    return ig;
}

IncidenceGraph *IncidenceGraph::CreateAndCalculateAcyclicSubsetOnlineWithBorder(SimplexPtrList& simplexPtrList, const VertsSet &borderVerts, const Params& params, AcyclicTest<IntersectionFlags>* test)
{
#ifdef USE_HELPERS
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexPtrList, params);
    ig->borderVerts = borderVerts;
    ig->CreateGraphAndCalculateAcyclicSubsetWithBorder(test);
    Timer::Update("incidence graph created and acyclic subset calculated");
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexList, params);
    ig->CreateGraphAndCalculateAcyclicSubset(test);
#endif
    return ig;
}

IncidenceGraph *IncidenceGraph::CreateAndCalculateAcyclicSubsetSpanningTree(SimplexList& simplexList, const Params& params, AcyclicTest<IntersectionFlags>* test)
{
#ifdef USE_HELPERS
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexList, params);
    ig->CreateGraph(params.minimizeSimplices);
    Timer::Update("incidence graph created");
    ig->CalculateAcyclicSubsetSpanningTree(test);
    Timer::Update("acyclic subset calculated");
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexList, params);
    ig->CreateGraph(params.minimizeSimplices);
    ig->CalculateAcyclicSubsetSpanningTree(test);
#endif
    return ig;
}

IncidenceGraph *IncidenceGraph::CreateAndCalculateAcyclicSubsetSpanningTree(SimplexPtrList& simplexPtrList, const Params& params, AcyclicTest<IntersectionFlags>* test)
{
#ifdef USE_HELPERS
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexPtrList, params);
    ig->CreateGraph(params.minimizeSimplices);
    Timer::Update("incidence graph created");
    ig->CalculateAcyclicSubsetSpanningTree(test);
    Timer::Update("acyclic subset calculated");
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexList, params);
    ig->CreateGraph(params.minimizeSimplices);
    ig->CalculateAcyclicSubsetSpanningTree(test);
#endif
    return ig;
}

IncidenceGraph *IncidenceGraph::CreateAndCalculateAcyclicSubsetSpanningTreeWithBorder(SimplexList& simplexList, const VertsSet &borderVerts, const Params& params, AcyclicTest<IntersectionFlags>* test)
{
#ifdef USE_HELPERS
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexList, params);
    ig->borderVerts = borderVerts;
    ig->CreateGraphWithBorder(params.minimizeSimplices);
    Timer::Update("incidence graph created");
    ig->CalculateAcyclicSubsetSpanningTreeWithBorder(test);
    Timer::Update("acyclic subset calculated");
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexList, params);
    ig->CreateGraph(params.minimizeSimplices);
    ig->CalculateAcyclicSubsetSpanningTree(test);
#endif
    return ig;
}

IncidenceGraph *IncidenceGraph::CreateAndCalculateAcyclicSubsetSpanningTreeWithBorder(SimplexPtrList& simplexPtrList, const VertsSet &borderVerts, const Params& params, AcyclicTest<IntersectionFlags>* test)
{
#ifdef USE_HELPERS
    Timer::Update();
    IncidenceGraph *ig = new IncidenceGraph(simplexPtrList, params);
    ig->borderVerts = borderVerts;
    ig->CreateGraphWithBorder(params.minimizeSimplices);
    Timer::Update("incidence graph created");
    ig->CalculateAcyclicSubsetSpanningTreeWithBorder(test);
    Timer::Update("acyclic subset calculated");
#else
    IncidenceGraph *ig = new IncidenceGraph(simplexList, params);
    ig->CreateGraph(params.minimizeSimplices);
    ig->CalculateAcyclicSubsetSpanningTree(test);
#endif
    return ig;
}

IncidenceGraph *IncidenceGraph::CreateAndCalculateAcyclicSubsetParallel(SimplexList& simplexList, const Params& params, const ParallelParams& parallelParams, AcyclicTest<IntersectionFlags>* test, bool local)
{
    IncidenceGraph *ig = new IncidenceGraph(params);
    ParallelGraph *pg = new ParallelGraph(ig, simplexList, params, parallelParams, test, local);
    delete pg;
    return ig;
}

////////////////////////////////////////////////////////////////////////////////

void IncidenceGraph::CreateGraph(bool minimizeSimplices)
{
    VertexNodesMap H; // hash<numer_wierzcholka, lista_wskaznikow_do_nodow_zawierajacych wierzcholek>
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        Simplex *s = (*i)->simplex;
        for (Simplex::iterator j = s->begin(); j != s->end(); j++)
        {
            H[*j].push_back(*i);            
        }        
    }

    Simplex minimalSimplex;
    std::queue<Node *> L;    
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        // w tym miejscu zaczynamy budowanie nowej spojnek skladowej
        // zatem jesli wierzcholek byl odwiedzony, znaczy, ze nalezy
        // do zbudowanej juz (innej) spojnej skladowej
        if ((*i)->IsAddedToGraph())
        {
            continue;
        }
        // spojne skladowe identyfikujemy przez wskaznik do pierwszego
        // wierzcholka (tyle nam wystarczy)
        connectedComponents.push_back(*i);
        // zaznaczamy, ze wierzcholek jest w grafie + dodajemy do listy
        (*i)->IsAddedToGraph(true);
        L.push(*i);
        while (!L.empty())
        {
            Node *currentNode = L.front();
            L.pop();
            if (minimizeSimplices && minimalSimplex.size() > 0)
            {
                minimalSimplex.clear();
            }
            // sprawdzamy wszytkie punkty nalezace do sympleksu w tym wierzcholku
            for (Simplex::iterator vertex = currentNode->simplex->begin(); vertex != currentNode->simplex->end(); vertex++)
            {
                // dla kazdego punktu pobieramy liste wierzcholkow do ktorej nalezy
                Nodes nodes = H[*vertex];
                if (minimizeSimplices && nodes.size() > 1)
                {
                    minimalSimplex.push_back(*vertex);
                }
                for (Nodes::iterator neighbour = nodes.begin(); neighbour != nodes.end(); neighbour++)
                {
                    // jezeli to wierzcholek w ktorym aktualnie jestesmy -> dalej
                    if ((*neighbour) == currentNode)
                    {
                        continue;
                    }
                    // jezeli wierzcholek jest juz w grafie to aktualizujemy krawedzie
                    if ((*neighbour)->IsAddedToGraph())
                    {
                        if (!(*neighbour)->HasNeighbour(currentNode))
                        {
                            (*neighbour)->AddNeighbour(currentNode);
                            currentNode->AddNeighbour(*neighbour);
                        }
                    }
                    // wpp dodajemy wierzcholek i krawedzie
                    else
                    {
                        (*neighbour)->IsAddedToGraph(true);
                        L.push(*neighbour);
                        (*neighbour)->AddNeighbour(currentNode);
                        currentNode->AddNeighbour(*neighbour);
                    }
                }
            }
            if (minimizeSimplices)
            {
            //    currentNode->simplex->assign(minimalSimplex.begin(), minimalSimplex.end());
                (*currentNode->simplex) = minimalSimplex;
            }
        }
    }

    #ifdef USE_LOG
    Log::stream<<"number of connected components: "<<connectedComponents.size()<<std::endl;
    #endif   
//    std::cout<<"number of connected components: "<<connectedComponents.size()<<std::endl;
}

void IncidenceGraph::CreateGraphWithBorder(bool minimizeSimplices)
{
    VertexNodesMap H; // hash<numer_wierzcholka, lista_wskaznikow_do_nodow_zawierajacych wierzcholek>
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        Simplex *s = (*i)->simplex;
        for (Simplex::iterator j = s->begin(); j != s->end(); j++)
        {
            H[*j].push_back(*i);
        }
    }

    for (VertsSet::iterator v = borderVerts.begin(); v != borderVerts.end(); v++)
    {
        Nodes nodes = H[*v];
        for (Nodes::iterator node = nodes.begin(); node != nodes.end(); node++)
        {
            (*node)->IsOnBorder(true);
        }
    }

    std::vector<Vertex> vectorBorderVerts;
    vectorBorderVerts.assign(borderVerts.begin(), borderVerts.end());
    std::sort(vectorBorderVerts.begin(), vectorBorderVerts.end());

    std::queue<Node *> L;
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        // w tym miejscu zaczynamy budowanie nowej spojnek skladowej
        // zatem jesli wierzcholek byl odwiedzony, znaczy, ze nalezy
        // do zbudowanej juz (innej) spojnej skladowej
        if ((*i)->IsAddedToGraph())
        {
            continue;
        }
        // spojne skladowe identyfikujemy przez wskaznik do pierwszego
        // wierzcholka (tyle nam wystarczy)
        connectedComponents.push_back(*i);
        // tutaj bedziemy wrzucac wierzcholki w brzegu dla tej spojnej skladowej
        VertsSet connectedComponentBorderVerts;
        // zaznaczamy, ze wierzcholek jest w grafie + dodajemy do listy
        (*i)->IsAddedToGraph(true);
        L.push(*i);
        while (!L.empty())
        {
            Node *currentNode = L.front();
            L.pop();
            if (currentNode->IsOnBorder())
            {
                connectedComponentBorderVerts.insert(currentNode->simplex->begin(), currentNode->simplex->end());
            }
            // sprawdzamy wszytkie punkty nalezace do sympleksu w tym wierzcholku
            for (Simplex::iterator vertex = currentNode->simplex->begin(); vertex != currentNode->simplex->end(); vertex++)
            {
                // dla kazdego punktu pobieramy liste wierzcholkow do ktorej nalezy
                Nodes nodes = H[*vertex];
                for (Nodes::iterator neighbour = nodes.begin(); neighbour != nodes.end(); neighbour++)
                {
                    // jezeli to wierzcholek w ktorym aktualnie jestesmy -> dalej
                    if ((*neighbour) == currentNode)
                    {
                        continue;
                    }
                    // jezeli wierzcholek jest juz w grafie to aktualizujemy krawedzie
                    if ((*neighbour)->IsAddedToGraph())
                    {
                        if (!(*neighbour)->HasNeighbour(currentNode))
                        {
                            (*neighbour)->AddNeighbour(currentNode);
                            currentNode->AddNeighbour(*neighbour);
                        }
                    }
                    // wpp dodajemy wierzcholek i krawedzie
                    else
                    {
                        (*neighbour)->IsAddedToGraph(true);
                        L.push(*neighbour);
                        (*neighbour)->AddNeighbour(currentNode);
                        currentNode->AddNeighbour(*neighbour);
                    }
                }
            }
        }
        VertsSet tempSet;
        GetIntersectionOfUnsortedSetAndSortedVector(tempSet, connectedComponentBorderVerts, vectorBorderVerts);
        connectedComponentsBorders.push_back(tempSet);
    }

    #ifdef USE_LOG
    Log::stream<<"number of connected components: "<<connectedComponents.size()<<std::endl;
    #endif
//    std::cout<<"number of connected components: "<<connectedComponents.size()<<std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void IncidenceGraph::CreateGraphAndCalculateAcyclicSubset(AcyclicTest<IntersectionFlags> *test)
{
    VertexNodesMap H; // hash<numer_wierzcholka, lista_wskaznikow_do_nodow_zawierajacych wierzcholek>
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        Simplex *s = (*i)->simplex;
        for (Simplex::iterator j = s->begin(); j != s->end(); j++)
        {
            H[*j].push_back(*i);
        }
    }

    std::queue<Node *> L;
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        if ((*i)->IsAddedToGraph() || (*i)->IsAcyclic())
        {
            continue;
        }
        AddNeighboursToListAndUpdateAcyclicIntersection(*i, H, L);
        (*i)->IsAcyclic(true);
        while (!L.empty())
        {
            Node *currentNode = L.front();
            L.pop();
            currentNode->IsAddedToList(false);

            if (currentNode->HasAcyclicIntersection(test))
            {
                currentNode->IsAcyclic(true);
                AddNeighboursToListAndUpdateAcyclicIntersection(currentNode, H, L);
                if (currentNode->IsAddedToGraph())
                {
                    RemoveNodeFromGraph(currentNode);
                }
            }
            else
            {
                if (currentNode->IsAddedToGraph())
                {
                    // nie robimy nic, po prostu usuwamy z listy
                }
                else
                {
                    AddNodeToGraphAndNeighboursToList(currentNode, H, L);
                }
            }
        }
    }
}

void IncidenceGraph::CreateGraphAndCalculateAcyclicSubsetWithBorder(AcyclicTest<IntersectionFlags> *test)
{
    VertexNodesMap H; // hash<numer_wierzcholka, lista_wskaznikow_do_nodow_zawierajacych wierzcholek>
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        Simplex *s = (*i)->simplex;
        for (Simplex::iterator j = s->begin(); j != s->end(); j++)
        {
            H[*j].push_back(*i);
        }
    }

    for (VertsSet::iterator v = borderVerts.begin(); v != borderVerts.end(); v++)
    {
        Nodes nodes = H[*v];
        for (Nodes::iterator node = nodes.begin(); node != nodes.end(); node++)
        {
            (*node)->IsOnBorder(true);
        }
    }

    std::vector<Vertex> vectorBorderVerts;
    vectorBorderVerts.assign(borderVerts.begin(), borderVerts.end());
    std::sort(vectorBorderVerts.begin(), vectorBorderVerts.end());

    std::queue<Node *> L;
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        if ((*i)->IsAddedToGraph() || (*i)->IsAcyclic())
        {
            continue;
        }
        
        Node *connectedComponent = 0;
        VertsSet connectedComponentBorderVerts;
        int acyclicSubsetSize = 0;

        L.push(*i);
        while (!L.empty())
        {
            Node *currentNode = L.front();
            L.pop();
            currentNode->IsAddedToList(false);

            if (acyclicSubsetSize == 0 && ! currentNode->IsOnBorder())
            {
                AddNeighboursToListAndUpdateAcyclicIntersection(currentNode, H, L);
                currentNode->IsAcyclic(true);
                acyclicSubsetSize = 1;
                continue;
            }

            if (currentNode->IsOnBorder())
            {
                if (!currentNode->IsAddedToGraph())
                {
                    connectedComponentBorderVerts.insert(currentNode->simplex->begin(), currentNode->simplex->end());
                    connectedComponent = currentNode;
                    AddNodeToGraphAndNeighboursToList(currentNode, H, L);
                }
            }
            else if (currentNode->HasAcyclicIntersection(test))
            {
                currentNode->IsAcyclic(true);
                AddNeighboursToListAndUpdateAcyclicIntersection(currentNode, H, L);
                if (currentNode->IsAddedToGraph())
                {
                    RemoveNodeFromGraph(currentNode);
                }
            }
            else
            {
                if (currentNode->IsAddedToGraph())
                {
                    // nie robimy nic, po prostu usuwamy z listy
                }
                else
                {
                    AddNodeToGraphAndNeighboursToList(currentNode, H, L);
                }
            }
        }
        if (connectedComponent != 0)
        {
            connectedComponents.push_back(connectedComponent);
            VertsSet tempSet;
            GetIntersectionOfUnsortedSetAndSortedVector(tempSet, connectedComponentBorderVerts, vectorBorderVerts);
            connectedComponentsBorders.push_back(tempSet);
            connectedComponentsAcyclicSubsetSize.push_back(acyclicSubsetSize);
        }
    }

    // std::cout<<"connected components: "<<connectedComponents.size()<<std::endl;
}

void IncidenceGraph::AddNeighboursToListAndUpdateAcyclicIntersection(Node* node, VertexNodesMap &H, std::queue<Node*> &L)
{
    Simplex intersection;
    // sprawdzamy wszytkie punkty nalezace do sympleksu w tym wierzcholku
    for (Simplex::iterator vertex = node->simplex->begin(); vertex != node->simplex->end(); vertex++)
    {
        // dla kazdego punktu pobieramy liste wierzcholkow do ktorej nalezy
        Nodes nodes = H[*vertex];
        for (Nodes::iterator neighbour = nodes.begin(); neighbour != nodes.end(); neighbour++)
        {
            // jezeli to wierzcholek w ktorym aktualnie jestesmy -> dalej
            if ((*neighbour)->IsAcyclic() || (*neighbour) == node)
            {
                continue;
            }
            GetIntersection(node->simplex, (*neighbour)->simplex, intersection);
            intersection = (*neighbour)->Normalize(intersection);
            (*neighbour)->UpdateAcyclicIntersectionFlags(subconfigurationsFlags[intersection], configurationsFlags[intersection]);
            if (!(*neighbour)->IsAddedToList())
            {
                L.push(*neighbour);
                (*neighbour)->IsAddedToList(true);
            }
        }
    }
}

void IncidenceGraph::AddNodeToGraphAndNeighboursToList(Node* node, VertexNodesMap &H, std::queue<Node*> &L)
{
    node->IsAddedToGraph(true);
    // sprawdzamy wszytkie punkty nalezace do sympleksu w tym wierzcholku
    for (Simplex::iterator vertex = node->simplex->begin(); vertex != node->simplex->end(); vertex++)
    {
        // dla kazdego punktu pobieramy liste wierzcholkow do ktorej nalezy
        Nodes nodes = H[*vertex];
        for (Nodes::iterator neighbour = nodes.begin(); neighbour != nodes.end(); neighbour++)
        {
            // jezeli to wierzcholek w ktorym aktualnie jestesmy -> dalej
            if ((*neighbour)->IsAcyclic() || (*neighbour) == node)
            {
                continue;
            }
            if (!(*neighbour)->HasNeighbour(node))
            {
                (*neighbour)->AddNeighbour(node);
                node->AddNeighbour(*neighbour);
            }
            if (!(*neighbour)->IsAddedToList())
            {
                L.push(*neighbour);
                (*neighbour)->IsAddedToList(true);
            }
        }
    }
}

void IncidenceGraph::RemoveNodeFromGraph(Node* node)
{
    node->IsAddedToGraph(false);
    for (Edges::iterator edge = node->edges.begin(); edge != node->edges.end(); edge++)
    {
        edge->node->RemoveNeighbour(node);
    }
}

////////////////////////////////////////////////////////////////////////////////

void IncidenceGraph::CalculateAcyclicSubset(AcyclicTest<IntersectionFlags> *test)
{
    std::queue<Node *> L;
    for (std::vector<ConnectedComponent>::iterator i = connectedComponents.begin(); i != connectedComponents.end(); i++)
    {
        // pierwszy wierzcholek w skladowej zaznaczamy jako acykliczny
        (*i)->IsAcyclic(true);
        (*i)->UpdateNeighboursAcyclicIntersection();
        L.push(*i);
        while (!L.empty())
        {
            Node *currentNode = L.front();
            L.pop();

            for (Edges::iterator j = currentNode->edges.begin(); j != currentNode->edges.end(); j++)
            {
                Node *neighbour = j->node;
                if (neighbour->IsAcyclic())
                {
                    continue;
                }
                if (neighbour->HasAcyclicIntersection(test))
                {
                    neighbour->IsAcyclic(true);
                    neighbour->UpdateNeighboursAcyclicIntersection();
                    L.push(neighbour);
                }
            }            
        }        
    }
}

void IncidenceGraph::CalculateAcyclicSubsetWithBorder(AcyclicTest<IntersectionFlags> *test)
{
    std::queue<Node *> L;
    for (std::vector<ConnectedComponent>::iterator i = connectedComponents.begin(); i != connectedComponents.end(); i++)
    {
        Node *first = FindNode(*i, FindNodeNotOnBorder());
        // wszystkie sympleksy w tej spojnej sa w brzegu
        if (first == 0)
        {
            continue;
        }
        // pierwszy wierzcholek w skladowej zaznaczamy jako acykliczny
        first->IsAcyclic(true);
        first->UpdateNeighboursAcyclicIntersection();
        L.push(first);
        while (!L.empty())
        {
            Node *currentNode = L.front();
            L.pop();

            for (Edges::iterator j = currentNode->edges.begin(); j != currentNode->edges.end(); j++)
            {
                Node *neighbour = j->node;
                if (neighbour->IsAcyclic() || neighbour->IsOnBorder())
                {
                    continue;
                }
                if (neighbour->HasAcyclicIntersection(test))
                {
                    neighbour->IsAcyclic(true);
                    neighbour->UpdateNeighboursAcyclicIntersection();
                    L.push(neighbour);
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void IncidenceGraph::CalculateAcyclicSubsetSpanningTree(AcyclicTest<IntersectionFlags> *test)
{
    int index = 0;
    std::queue<Node *> L;
    for (std::vector<ConnectedComponent>::iterator i = connectedComponents.begin(); i != connectedComponents.end(); i++)
    {
        int acyclicSubsetID = 0;
        Nodes firstNodes;
        std::vector<Path> paths;
        Node *first = *i;
        int size = 0;
        while (first != 0)
        {
            acyclicSubsetID++;
            // pierwszy wierzcholek w skladowej zaznaczamy jako acykliczny
            firstNodes.push_back(first);
            first->IsAcyclic(true);
            first->SetAcyclicSubsetID(acyclicSubsetID);
            first->UpdateNeighboursAcyclicIntersection();
            L.push(first);
            size++;
            while (!L.empty())
            {
                Node *currentNode = L.front();
                L.pop();
                for (Edges::iterator j = currentNode->edges.begin(); j != currentNode->edges.end(); j++)
                {
                    Node *neighbour = j->node;
                    if (neighbour->IsAcyclic())
                    {
                        continue;
                    }
                    if (neighbour->HasAcyclicIntersection(test))
                    {
                        size++;
                        neighbour->IsAcyclic(true);
                        neighbour->SetAcyclicSubsetID(acyclicSubsetID);
                        neighbour->UpdateNeighboursAcyclicIntersection();
                        L.push(neighbour);
                    }
                }
            }
            Path path = FindPath(first, FindPathToNodeNotInAcyclicSubset());
            if (path.size() > 0)
            {
                first = path.back();
                paths.push_back(path);
            }
            else
            {
                first = 0;
            }
       }

        connectedComponentsAcyclicSubsetSize.push_back(size);
        // std::cout<<"number of acyclic subsets in "<<index++<<"-th connected component: "<<firstNodes.size()<<std::endl;

        // jezeli wiecej niz jeden podzbior acykliczny, to tworzymy graf
        if (firstNodes.size() > 1)
        {
            CreateAcyclicSpanningTree(paths, ++acyclicSubsetID);
        }
    }
}

void IncidenceGraph::CalculateAcyclicSubsetSpanningTreeWithBorder(AcyclicTest<IntersectionFlags> *test)
{
    std::queue<Node *> L;
    for (std::vector<ConnectedComponent>::iterator i = connectedComponents.begin(); i != connectedComponents.end(); i++)
    {
        int acyclicSubsetID = 0;
        Nodes firstNodes;
        std::vector<Path> paths;
        Node *first = FindNode(*i, FindNodeNotOnBorder());
        int size = 0;
        while (first != 0)
        {
            acyclicSubsetID++;
            // pierwszy wierzcholek w skladowej zaznaczamy jako acykliczny
            firstNodes.push_back(first);
            first->IsAcyclic(true);
            first->SetAcyclicSubsetID(acyclicSubsetID);
            first->UpdateNeighboursAcyclicIntersection();
            L.push(first);
            size++;
            while (!L.empty())
            {
                Node *currentNode = L.front();
                L.pop();
                for (Edges::iterator j = currentNode->edges.begin(); j != currentNode->edges.end(); j++)
                {
                    Node *neighbour = j->node;
                    if (neighbour->IsAcyclic() || neighbour->IsOnBorder())
                    {
                        continue;
                    }
                    if (neighbour->HasAcyclicIntersection(test))
                    {
                        size++;
                        neighbour->IsAcyclic(true);
                        neighbour->SetAcyclicSubsetID(acyclicSubsetID);
                        neighbour->UpdateNeighboursAcyclicIntersection();
                        L.push(neighbour);
                    }
                }
            }
            Path path = FindPath(first, FindPathToNodeNotInAcyclicSubsetNorOnBorder());
            if (path.size() > 0)
            {
                first = path.back();
                paths.push_back(path);
            }
            else
            {
                first = 0;
            }
       }

        connectedComponentsAcyclicSubsetSize.push_back(size);
        // std::cout<<"number of acyclic subsets in "<<index++<<"-th connected component: "<<firstNodes.size()<<std::endl;

        // jezeli wiecej niz jeden podzbior acykliczny, to tworzymy graf
        if (firstNodes.size() > 1)
        {
            CreateAcyclicSpanningTree(paths, ++acyclicSubsetID);
        }
    }
}

void IncidenceGraph::CreateAcyclicSpanningTree(std::vector<IncidenceGraph::Path> &paths, int maxAcyclicSubsetID)
{
    // todo!!! memory leak
    char *addedToAcyclicTree = new char[maxAcyclicSubsetID];
    memset(addedToAcyclicTree, 0, sizeof(char) * maxAcyclicSubsetID);

    // dodajemy pierwszy wierzcholek pierwszej sciezki do drzewa
    // acyklicznego (tak naprawde jest to jakis losowy wierzcholek)
    assert(paths.size() > 0);
    addedToAcyclicTree[paths[0].front()->GetAcyclicSubsetID()] = true;

    while (!paths.empty())
    {
        Path path;
        for (std::vector<Path>::iterator p = paths.begin(); p != paths.end(); p++)
        {
            if (addedToAcyclicTree[(*p).front()->GetAcyclicSubsetID()] || addedToAcyclicTree[(*p).back()->GetAcyclicSubsetID()])
            {
                path = *p;
                paths.erase(p);
                break;
            }
        }
        assert(!addedToAcyclicTree[path.front()->GetAcyclicSubsetID()] || !addedToAcyclicTree[path.back()->GetAcyclicSubsetID()]);
        // ze wzgledu na to jak budujemy podzbiory acykliczne zazwyczaj wierzcholki
        // juz dodane do drzewa rozpinajacego beda na poczatku sciezek
        // jezeli natomiast zdarzy sie, ze sa na koncu, to musimy odwrocic liste
        if (addedToAcyclicTree[path.back()->GetAcyclicSubsetID()])
        {
            path.reverse();
        }
        addedToAcyclicTree[path.back()->GetAcyclicSubsetID()] = 1;

        // idziemy od konca listy i dodajemy dowolny wierzcholek z przeciecia
        // dwoch sasiednich sympleksow
        Path::reverse_iterator current = path.rbegin();
        Path::reverse_iterator next = current;
        next++;
        Vertex lastVertex = GetVertexFromIntersection((*current)->simplex, (*next)->simplex);
        current = next;
        next++;
        while (next != path.rend())
        {
            Node *n = *next;

            // sprawdzamy przeciecie z nastepnym sympleksem
            Vertex vertex = GetVertexFromIntersection((*current)->simplex, n->simplex);

            // jezeli wierzcholek jest w zbiorze acyklicznym -> konczymy
            // przy sprawdzaniu warunku korzystamy z faktu, ze flaga podsciany
            // bedacej pojedynczym wierzcholkiem jest rowna 1 << i gdzie
            // i to indeks wierzcholka
            if (n->GetAcyclicIntersectionFlags() & (1 << n->NormalizeVertex(vertex)))
            {
                // aktualizujemy w aktualnym nodzie flagi acyklicznego przeciecia
                (*current)->UpdateAcyclicIntersectionWithEdge(vertex, lastVertex);
                break;
            }
            // wpp oznacza, ze mozemy isc dalej
            else
            {
                // aktualizujemy w aktualnym nodzie flagi acyklicznego przeciecia
                (*current)->UpdateAcyclicIntersectionWithEdge(vertex, lastVertex);
                lastVertex = vertex;
            }
            current = next;
            next++;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void IncidenceGraph::UpdateConnectedComponents()
{
    ConnectedComponents::iterator cc = connectedComponents.begin();
    std::vector<std::set<Vertex> >::iterator ccb = connectedComponentsBorders.begin();
    std::vector<int>::iterator ccass = connectedComponentsAcyclicSubsetSize.begin();
    while (cc != connectedComponents.end())
    {
        if ((*cc)->IsAcyclic())
        {
            Node *node = FindNode(*cc, FindNotAcyclicNode());
            // jezeli nie ma zadnego sympleksu nieacyklicznego, to usuwamy
            // informacje o spojnej skladowej
            if (node == 0)
            {
                cc = connectedComponents.erase(cc);
                ccb = connectedComponentsBorders.erase(ccb);
                ccass = connectedComponentsAcyclicSubsetSize.erase(ccass);
                continue;
            }
            else
            {
                *cc = node;
            }
        }
        cc++;
        ccb++;
        ccass++;
    }
}

void IncidenceGraph::RemoveAcyclicSubset()
{
    Nodes newNodes;
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        if ((*i)->IsAcyclic())
        {
            continue;
        }
        Node *node = *i;
        newNodes.push_back(node);
        Edges newEdges;
        for (Edges::iterator edge = node->edges.begin(); edge != node->edges.end(); edge++)
        {
            if (edge->node->IsAcyclic())
            {
                continue;
            }
            newEdges.push_back(*edge);
        }
        node->edges = newEdges;
    }
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        if ((*i)->IsAcyclic())
        {
            delete *i;
        }
    }
    nodes = newNodes;
}

void IncidenceGraph::AssignNewIndices(bool checkAcyclicity)
{
    int index = 0;
    if (checkAcyclicity)
    {
        for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            if ((*i)->IsAcyclic())
            {
                continue;
            }
            (*i)->newIndex = index++;
        }
    }
    else
    {
        for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            (*i)->newIndex = index++;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void IncidenceGraph::GetAcyclicSubset(SimplexList &simplexList)
{
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        if ((*i)->IsAcyclic())
        {
            simplexList.push_back(*(*i)->simplex);
        }
    }
}

int IncidenceGraph::GetAcyclicSubsetSize()
{
    int size = 0;
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        if ((*i)->IsAcyclic())
        {
            size++;
        }
    }
    return size;
}

////////////////////////////////////////////////////////////////////////////////

void IncidenceGraph::CalculateNodesIntersection(Node *a, Node *b, Edge &edgeAtoB)
{
    if (GetIntersection(a->simplex, b->simplex, edgeAtoB.intersection))
    {
        edgeAtoB.intersectionFlags = subconfigurationsFlags[a->Normalize(edgeAtoB.intersection)];
        b->SetIntersection(a, edgeAtoB.intersection);
    }
    else
    {
        Debug::Print(std::cout, a->simplex);
        Debug::Print(std::cout, b->simplex);
        assert(false);
    }
}

////////////////////////////////////////////////////////////////////////////////
// eof
