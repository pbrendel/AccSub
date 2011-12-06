/*
 * File:   IncidenceGraph.cpp
 * Author: Piotr Brendel
 */

#include "IncidenceGraph.h"
#include "IncidenceGraphAlgorithms.h"
#include "PartitionGraph.h"

#ifdef ACCSUB_TRACE
#include "../Helpers/Utils.h"
#endif

#include <map>
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <cstring>

// test!!!
int IncidenceGraph::counter = 0;
#include "../Helpers/Utils.h"
#include <fstream>

////////////////////////////////////////////////////////////////////////////////

IncidenceGraph::IncidenceGraph(int dim)
{
    this->dim = dim;
    CreateConfigurationsFlags(dim, configurationsFlags, subconfigurationsFlags);
}

IncidenceGraph::IncidenceGraph(SimplexList &simplexList)
{
    // najpierw tworzymy wszystkie node'y
    int index = 0;
    for (SimplexList::iterator i = simplexList.begin(); i != simplexList.end(); i++)
    {
        nodes.push_back(new Node(this, &(*i), index++));
    }

    dim = Simplex::GetSimplexListDimension(simplexList);
    CreateConfigurationsFlags(dim, configurationsFlags, subconfigurationsFlags);
}

IncidenceGraph::IncidenceGraph(SimplexPtrList &simplexPtrList)
{
    // najpierw tworzymy wszystkie node'y
    int index = 0;
    for (SimplexPtrList::iterator i = simplexPtrList.begin(); i != simplexPtrList.end(); i++)
    {
        nodes.push_back(new Node(this, (*i), index++));
    }

    dim = Simplex::GetSimplexListDimension(simplexPtrList);
    CreateConfigurationsFlags(dim, configurationsFlags, subconfigurationsFlags);
}

IncidenceGraph::~IncidenceGraph()
{
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        delete (*i);       
    }
    for (Edges::iterator i = edges.begin(); i != edges.end(); i++)
    {
        delete (*i);
    }
    nodes.clear();
}

////////////////////////////////////////////////////////////////////////////////

void IncidenceGraph::CreateVertexHash(VertexHash &H)
{
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        Simplex *s = (*i)->simplex;
        for (Simplex::iterator j = s->begin(); j != s->end(); j++)
        {
            H[*j].push_back(*i);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void IncidenceGraph::CreateGraph()
{
    VertexHash H;
    CreateVertexHash(H);
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
        L.push(*i);
        while (!L.empty())
        {
            Node *currentNode = L.front();
            L.pop();
            currentNode->IsAddedToList(false);
            currentNode->IsAddedToGraph(true);
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
                    if (!(*neighbour)->IsAddedToGraph())
                    {
                        if (!currentNode->HasNeighbour(*neighbour))
                        {
                            Edge *edge = new Edge(currentNode, *neighbour);
                            edges.push_back(edge);
                            currentNode->AddEdge(edge);
                            (*neighbour)->AddEdge(edge);
                        }
                        if (!(*neighbour)->IsAddedToList())
                        {
                            L.push(*neighbour);
                            (*neighbour)->IsAddedToList(true);
                        }
                    }
                }
            }
        }
    }
}

void IncidenceGraph::CreateGraphWithBorder()
{
    VertexHash H;
    CreateVertexHash(H);
    
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
        // tutaj bedziemy wrzucac wierzcholki w brzegu dla tej spojnej skladowej
        VertsSet connectedComponentBorderVerts;
        // spojne skladowe identyfikujemy przez wskaznik do pierwszego
        // wierzcholka (tyle nam wystarczy)
        connectedComponents.push_back(*i);
        L.push(*i);
        while (!L.empty())
        {
            Node *currentNode = L.front();
            L.pop();
            currentNode->IsAddedToGraph(true);
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
                    if (!(*neighbour)->IsAddedToGraph())
                    {
                        if (!currentNode->HasNeighbour(*neighbour))
                        {
                            Edge *edge = new Edge(currentNode, *neighbour);
                            edges.push_back(edge);
                            currentNode->AddEdge(edge);
                            (*neighbour)->AddEdge(edge);
                        }
                        if (!(*neighbour)->IsAddedToList())
                        {
                            L.push(*neighbour);
                            (*neighbour)->IsAddedToList(true);
                        }
                    }
                }
            }
        }
        VertsSet tempSet;
        GetIntersectionOfUnsortedSetAndSortedVector(tempSet, connectedComponentBorderVerts, vectorBorderVerts);
        connectedComponentsBorders.push_back(tempSet);
    }
}

////////////////////////////////////////////////////////////////////////////////

void IncidenceGraph::CreateGraphAndCalculateAcyclicSubset(AcyclicTest<IntersectionFlags> *test)
{
    VertexHash H;
    CreateVertexHash(H);
    std::queue<Node *> L;
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        if ((*i)->IsAddedToGraph() || (*i)->IsAcyclic())
        {
            continue;
        }
        (*i)->IsAcyclic(true);
        EnqNeighboursAndUpdateAcyclicIntersection(*i, H, L);
        while (!L.empty())
        {
            Node *currentNode = L.front();
            L.pop();
            currentNode->IsAddedToList(false);

            if (currentNode->HasAcyclicIntersection(test))
            {
                currentNode->IsAcyclic(true);
                EnqNeighboursAndUpdateAcyclicIntersection(currentNode, H, L);
                if (currentNode->IsAddedToGraph())
                {
                    currentNode->IsAddedToGraph(false);
                    currentNode->edges.clear();
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
                    AddToGraphAndEnqNeighbours(currentNode, H, L);
                }
            }
        }
    }
    RemoveAcyclicEdges();
}

void IncidenceGraph::CreateGraphAndCalculateAcyclicSubsetWithBorder(AcyclicTest<IntersectionFlags> *test)
{
    VertexHash H;
    CreateVertexHash(H);
    
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

            if (acyclicSubsetSize == 0 && !currentNode->IsOnBorder())
            {
                currentNode->IsAcyclic(true);
                EnqNeighboursAndUpdateAcyclicIntersection(currentNode, H, L);
                acyclicSubsetSize = 1;
                if (currentNode->IsAddedToGraph())
                {
                    currentNode->IsAddedToGraph(false);
                    currentNode->edges.clear();
                }
                continue;
            }

            if (currentNode->IsOnBorder())
            {
                if (!currentNode->IsAddedToGraph())
                {
                    connectedComponentBorderVerts.insert(currentNode->simplex->begin(), currentNode->simplex->end());
                    AddToGraphAndEnqNeighbours(currentNode, H, L);
                    connectedComponent = currentNode;
                }
            }
            else if (currentNode->HasAcyclicIntersection(test))
            {
                currentNode->IsAcyclic(true);
                EnqNeighboursAndUpdateAcyclicIntersection(currentNode, H, L);
                if (currentNode->IsAddedToGraph())
                {
                    currentNode->IsAddedToGraph(false);
                    currentNode->edges.clear();
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
                    connectedComponent = currentNode;
                    AddToGraphAndEnqNeighbours(currentNode, H, L);
                }
            }
        }
        if (connectedComponent != 0)
        {
        //    assert(acyclicSubsetSize > 0);
            connectedComponents.push_back(connectedComponent);
            VertsSet tempSet;
            GetIntersectionOfUnsortedSetAndSortedVector(tempSet, connectedComponentBorderVerts, vectorBorderVerts);
            connectedComponentsBorders.push_back(tempSet);
            connectedComponentsAcyclicSubsetSize.push_back(acyclicSubsetSize);
        }
        else
        {
            assert(false);
        }
    }
    RemoveAcyclicEdges();
}

void IncidenceGraph::EnqNeighboursAndUpdateAcyclicIntersection(Node* node, VertexHash &H, std::queue<Node*> &L)
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
            Simplex::GetIntersection(node->simplex, (*neighbour)->simplex, intersection);
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

void IncidenceGraph::AddToGraphAndEnqNeighbours(Node* node, VertexHash &H, std::queue<Node*> &L)
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
            if ((*neighbour)->IsAddedToGraph() && !node->HasNeighbour(*neighbour))
            {
                Edge *edge = new Edge(node, *neighbour);
                edges.push_back(edge);
                node->AddEdge(edge);
                (*neighbour)->AddEdge(edge);
            }
            if (!(*neighbour)->IsAddedToList())
            {
                L.push(*neighbour);
                (*neighbour)->IsAddedToList(true);
            }
        }
    }
}

void IncidenceGraph::RemoveAcyclicEdges()
{
    Edges newEdges;
    for (Edges::iterator i = edges.begin(); i != edges.end(); i++)
    {
        bool acyclic = false;
        if ((*i)->nodeA->IsAcyclic() && (*i)->nodeB->IsAcyclic())
        {
            delete (*i);
        }
        else if ((*i)->nodeA->IsAcyclic())
        {
            (*i)->nodeB->RemoveNeighbour((*i)->nodeA);
            delete (*i);
        }
        else if ((*i)->nodeB->IsAcyclic())
        {
            (*i)->nodeA->RemoveNeighbour((*i)->nodeB);
            delete (*i);
        }
        else
        {
            newEdges.push_back(*i);
        }
    }
    edges = newEdges;
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
                Node *neighbour = (*j)->GetNeighbour(currentNode);
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
                Node *neighbour = (*j)->GetNeighbour(currentNode);
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
                    Node *neighbour = (*j)->GetNeighbour(currentNode);
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
                    Node *neighbour = (*j)->GetNeighbour(currentNode);
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

        // jezeli wiecej niz jeden podzbior acykliczny, to tworzymy graf
        if (firstNodes.size() > 1)
        {
            CreateAcyclicSpanningTree(paths, ++acyclicSubsetID);
        }
    }
}

void IncidenceGraph::CreateAcyclicSpanningTree(std::vector<IncidenceGraph::Path> &paths, int maxAcyclicSubsetID)
{
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
        Vertex lastVertex = Simplex::GetVertexFromIntersection((*current)->simplex, (*next)->simplex);
        current = next;
        next++;
        while (next != path.rend())
        {
            Node *n = *next;

            // sprawdzamy przeciecie z nastepnym sympleksem
            Vertex vertex = Simplex::GetVertexFromIntersection((*current)->simplex, n->simplex);

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
    delete [] addedToAcyclicTree;
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

////////////////////////////////////////////////////////////////////////////////

void IncidenceGraph::RemoveAcyclicSubset()
{
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        if ((*i)->IsAcyclic())
        {
            continue;
        }
        Edges newEdges;
        for (Edges::iterator edge = (*i)->edges.begin(); edge != (*i)->edges.end(); edge++)
        {
            if ((*edge)->GetNeighbour(*i)->IsAcyclic())
            {
                continue;
            }
            newEdges.push_back(*edge);
        }
        (*i)->edges = newEdges;
    }
    RemoveNodesWithPredicate(this, RemoveNodesWithFlags(Node::IGNPF_ACYCLIC));
}

////////////////////////////////////////////////////////////////////////////////

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

void IncidenceGraph::RemoveConnectedComponentAndCopySimplexList(ConnectedComponent cc, SimplexPtrList& simplexPtrList)
{
    // przelatujemy cala spojna skladowa i zaznaczamy elementy do niej nalezace
    // dodatkowo dodajemy do listy sympleksy z tej spojnej skladowej
    std::queue<Node *> L;
    L.push(cc);
    cc->IsHelperFlag2(true);
    simplexPtrList.push_back(cc->simplex);
    while (!L.empty())
    {
        IncidenceGraph::Node *node = L.front();
        L.pop();
        for (Edges::iterator edge = node->edges.begin(); edge != node->edges.end(); edge++)
        {
            Node *neighbour = (*edge)->GetNeighbour(node);
            if (neighbour->IsHelperFlag2())
            {
                continue;
            }
            neighbour->IsHelperFlag2(true);
            L.push(neighbour);
            simplexPtrList.push_back(neighbour->simplex);
        }
    }
    RemoveNodesWithPredicate(this, RemoveNodesWithFlags(Node::IGNPF_HELPER_FLAG_2));
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
// eof
