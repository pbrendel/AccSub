/*
 * File:   IncidenceGraph.cpp
 * Author: Piotr Brendel
 */

#include "IncidenceGraph.h"
#include "IncidenceGraphAlgorithms.hpp"
#include "PartitionGraph.h"

#ifdef ACCSUB_TRACE
#include "../Helpers/Utils.h"
#endif

#include <map>
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <cstring>

////////////////////////////////////////////////////////////////////////////////

IncidenceGraph::IncidenceGraph(int dim)
{
    this->dim = dim;
    configurationsFlags.Create(dim, false, true);
    subconfigurationsFlags.Create(dim, true, true);
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
    configurationsFlags.Create(dim, false, true);
    subconfigurationsFlags.Create(dim, true, true);
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
    configurationsFlags.Create(dim, false, true);
    subconfigurationsFlags.Create(dim, true, true);
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
    std::queue<Node *> Q;

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
        Q.push(*i);
        while (!Q.empty())
        {
            Node *currentNode = Q.front();
            Q.pop();
            currentNode->IsAddedToQueue(false);
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
                        if (!(*neighbour)->IsAddedToQueue())
                        {
                            Q.push(*neighbour);
                            (*neighbour)->IsAddedToQueue(true);
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

    std::queue<Node *> Q;
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
        Q.push(*i);
        while (!Q.empty())
        {
            Node *currentNode = Q.front();
            Q.pop();
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
                        if (!(*neighbour)->IsAddedToQueue())
                        {
                            Q.push(*neighbour);
                            (*neighbour)->IsAddedToQueue(true);
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

void IncidenceGraph::CreateGraphAndCalculateAccSub(AccTest<IntersectionFlags> *test)
{
    VertexHash H;
    CreateVertexHash(H);
    std::queue<Node *> Q;
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        if ((*i)->IsAddedToGraph() || (*i)->IsInAccSub())
        {
            continue;
        }
        (*i)->IsInAccSub(true);
        EnqNeighboursAndUpdateAccIntersection(*i, H, Q);
        while (!Q.empty())
        {
            Node *currentNode = Q.front();
            Q.pop();
            currentNode->IsAddedToQueue(false);

            if (currentNode->GetAccInfo().IsAccIntersectionAcyclic(test))
            {
                currentNode->IsInAccSub(true);
                EnqNeighboursAndUpdateAccIntersection(currentNode, H, Q);
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
                    AddToGraphAndEnqNeighbours(currentNode, H, Q);
                }
            }
        }
    }
    RemoveEdgesWithAccSub();
}

void IncidenceGraph::CreateGraphAndCalculateAccSubWithBorder(AccTest<IntersectionFlags> *test)
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

    std::queue<Node *> Q;
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        if ((*i)->IsAddedToGraph() || (*i)->IsInAccSub())
        {
            continue;
        }
        
        Node *connectedComponent = 0;
        VertsSet connectedComponentBorderVerts;
        int accSubSize = 0;

        Q.push(*i);
        while (!Q.empty())
        {
            Node *currentNode = Q.front();
            Q.pop();
            currentNode->IsAddedToQueue(false);

            if (accSubSize == 0 && !currentNode->IsOnBorder())
            {
                currentNode->IsInAccSub(true);
                EnqNeighboursAndUpdateAccIntersection(currentNode, H, Q);
                accSubSize = 1;
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
                    AddToGraphAndEnqNeighbours(currentNode, H, Q);
                    connectedComponent = currentNode;
                }
            }
            else if (currentNode->GetAccInfo().IsAccIntersectionAcyclic(test))
            {
                currentNode->IsInAccSub(true);
                EnqNeighboursAndUpdateAccIntersection(currentNode, H, Q);
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
                    AddToGraphAndEnqNeighbours(currentNode, H, Q);
                }
            }
        }
        if (connectedComponent != 0)
        {
        //    assert(accSubSize > 0);
            connectedComponents.push_back(connectedComponent);
            VertsSet tempSet;
            GetIntersectionOfUnsortedSetAndSortedVector(tempSet, connectedComponentBorderVerts, vectorBorderVerts);
            connectedComponentsBorders.push_back(tempSet);
            connectedComponentsAccSubSize.push_back(accSubSize);
        }
        else
        {
            assert(false);
        }
    }
    RemoveEdgesWithAccSub();
}

void IncidenceGraph::EnqNeighboursAndUpdateAccIntersection(Node* node, VertexHash &H, std::queue<Node*> &Q)
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
            if ((*neighbour)->IsInAccSub() || (*neighbour) == node)
            {
                continue;
            }
            Simplex::GetIntersection(node->simplex, (*neighbour)->simplex, intersection);
            intersection = (*neighbour)->Normalize(intersection);
            (*neighbour)->GetAccInfo().UpdateAccIntersection(intersection);
            if (!(*neighbour)->IsAddedToQueue())
            {
                Q.push(*neighbour);
                (*neighbour)->IsAddedToQueue(true);
            }
        }
    }
}

void IncidenceGraph::AddToGraphAndEnqNeighbours(Node* node, VertexHash &H, std::queue<Node*> &Q)
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
            if ((*neighbour)->IsInAccSub() || (*neighbour) == node)
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
            if (!(*neighbour)->IsAddedToQueue())
            {
                Q.push(*neighbour);
                (*neighbour)->IsAddedToQueue(true);
            }
        }
    }
}

void IncidenceGraph::RemoveEdgesWithAccSub()
{
    Edges newEdges;
    for (Edges::iterator i = edges.begin(); i != edges.end(); i++)
    {
        bool acyclic = false;
        if ((*i)->nodeA->IsInAccSub() && (*i)->nodeB->IsInAccSub())
        {
            delete (*i);
        }
        else if ((*i)->nodeA->IsInAccSub())
        {
            (*i)->nodeB->RemoveNeighbour((*i)->nodeA);
            delete (*i);
        }
        else if ((*i)->nodeB->IsInAccSub())
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

void IncidenceGraph::CalculateAccSub(AccTest<IntersectionFlags> *test)
{
    std::queue<Node *> Q;
    for (std::vector<ConnectedComponent>::iterator i = connectedComponents.begin(); i != connectedComponents.end(); i++)
    {
        // pierwszy wierzcholek w skladowej zaznaczamy jako acykliczny
        (*i)->IsInAccSub(true);
        (*i)->GetAccInfo().UpdateNeighboursAccIntersection();
        Q.push(*i);
        while (!Q.empty())
        {
            Node *currentNode = Q.front();
            Q.pop();

            for (Edges::iterator j = currentNode->edges.begin(); j != currentNode->edges.end(); j++)
            {
                Node *neighbour = (*j)->GetNeighbour(currentNode);
                if (neighbour->IsInAccSub())
                {
                    continue;
                }
                if (neighbour->GetAccInfo().IsAccIntersectionAcyclic(test))
                {
                    neighbour->IsInAccSub(true);
                    neighbour->GetAccInfo().UpdateNeighboursAccIntersection();
                    Q.push(neighbour);
                }
            }            
        }        
    }
}

void IncidenceGraph::CalculateAccSubWithBorder(AccTest<IntersectionFlags> *test)
{
    std::queue<Node *> Q;
    for (std::vector<ConnectedComponent>::iterator i = connectedComponents.begin(); i != connectedComponents.end(); i++)
    {
        Node *first = FindNode(*i, FindNodeNotOnBorder());
        // wszystkie sympleksy w tej spojnej sa w brzegu
        if (first == 0)
        {
            continue;
        }
        // pierwszy wierzcholek w skladowej zaznaczamy jako acykliczny
        first->IsInAccSub(true);
        first->GetAccInfo().UpdateNeighboursAccIntersection();
        Q.push(first);
        while (!Q.empty())
        {
            Node *currentNode = Q.front();
            Q.pop();

            for (Edges::iterator j = currentNode->edges.begin(); j != currentNode->edges.end(); j++)
            {
                Node *neighbour = (*j)->GetNeighbour(currentNode);
                if (neighbour->IsInAccSub() || neighbour->IsOnBorder())
                {
                    continue;
                }
                if (neighbour->GetAccInfo().IsAccIntersectionAcyclic(test))
                {
                    neighbour->IsInAccSub(true);
                    neighbour->GetAccInfo().UpdateNeighboursAccIntersection();
                    Q.push(neighbour);
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void IncidenceGraph::CalculateAccSubSpanningTree(AccTest<IntersectionFlags> *test)
{
    int index = 0;
    std::queue<Node *> Q;
    for (std::vector<ConnectedComponent>::iterator i = connectedComponents.begin(); i != connectedComponents.end(); i++)
    {
        int accSubID = 0;
        Nodes firstNodes;
        std::vector<Path> paths;
        Node *first = *i;
        int size = 0;
        while (first != 0)
        {
            accSubID++;
            // pierwszy wierzcholek w skladowej zaznaczamy jako acykliczny
            firstNodes.push_back(first);
            first->IsInAccSub(true);
            first->GetAccInfo().SetAccSubID(accSubID);
            first->GetAccInfo().UpdateNeighboursAccIntersection();
            Q.push(first);
            size++;
            while (!Q.empty())
            {
                Node *currentNode = Q.front();
                Q.pop();
                for (Edges::iterator j = currentNode->edges.begin(); j != currentNode->edges.end(); j++)
                {
                    Node *neighbour = (*j)->GetNeighbour(currentNode);
                    if (neighbour->IsInAccSub())
                    {
                        continue;
                    }
                    if (neighbour->GetAccInfo().IsAccIntersectionAcyclic(test))
                    {
                        size++;
                        neighbour->IsInAccSub(true);
                        neighbour->GetAccInfo().SetAccSubID(accSubID);
                        neighbour->GetAccInfo().UpdateNeighboursAccIntersection();
                        Q.push(neighbour);
                    }
                }
            }
            Path path = FindPath(first, FindPathToNodeNotInAccSub());
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

        connectedComponentsAccSubSize.push_back(size);

        // jezeli wiecej niz jeden podzbior acykliczny, to tworzymy graf
        if (firstNodes.size() > 1)
        {
            CreateAccSpanningTree(paths, ++accSubID);
        }
    }
}

void IncidenceGraph::CalculateAccSubSpanningTreeWithBorder(AccTest<IntersectionFlags> *test)
{
    std::queue<Node *> Q;
    for (std::vector<ConnectedComponent>::iterator i = connectedComponents.begin(); i != connectedComponents.end(); i++)
    {
        int accSubID = 0;
        Nodes firstNodes;
        std::vector<Path> paths;
        Node *first = FindNode(*i, FindNodeNotOnBorder());
        int size = 0;
        while (first != 0)
        {
            accSubID++;
            // pierwszy wierzcholek w skladowej zaznaczamy jako acykliczny
            firstNodes.push_back(first);
            first->IsInAccSub(true);
            first->GetAccInfo().SetAccSubID(accSubID);
            first->GetAccInfo().UpdateNeighboursAccIntersection();
            Q.push(first);
            size++;
            while (!Q.empty())
            {
                Node *currentNode = Q.front();
                Q.pop();
                for (Edges::iterator j = currentNode->edges.begin(); j != currentNode->edges.end(); j++)
                {
                    Node *neighbour = (*j)->GetNeighbour(currentNode);
                    if (neighbour->IsInAccSub() || neighbour->IsOnBorder())
                    {
                        continue;
                    }
                    if (neighbour->GetAccInfo().IsAccIntersectionAcyclic(test))
                    {
                        size++;
                        neighbour->IsInAccSub(true);
                        neighbour->GetAccInfo().SetAccSubID(accSubID);
                        neighbour->GetAccInfo().UpdateNeighboursAccIntersection();
                        Q.push(neighbour);
                    }
                }
            }
            Path path = FindPath(first, FindPathToNodeNotInAccSubNorOnBorder());
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

        connectedComponentsAccSubSize.push_back(size);

        // jezeli wiecej niz jeden podzbior acykliczny, to tworzymy graf
        if (firstNodes.size() > 1)
        {
            CreateAccSpanningTree(paths, ++accSubID);
        }
    }
}

void IncidenceGraph::CreateAccSpanningTree(std::vector<IncidenceGraph::Path> &paths, int maxAccSubID)
{
    char *addedToAccTree = new char[maxAccSubID];
    memset(addedToAccTree, 0, sizeof(char) * maxAccSubID);

    // dodajemy pierwszy wierzcholek pierwszej sciezki do drzewa
    // acyklicznego (tak naprawde jest to jakis losowy wierzcholek)
    assert(paths.size() > 0);
    addedToAccTree[paths[0].front()->GetAccInfo().GetAccSubID()] = true;

    while (!paths.empty())
    {
        Path path;
        for (std::vector<Path>::iterator p = paths.begin(); p != paths.end(); p++)
        {
            if (addedToAccTree[(*p).front()->GetAccInfo().GetAccSubID()] || addedToAccTree[(*p).back()->GetAccInfo().GetAccSubID()])
            {
                path = *p;
                paths.erase(p);
                break;
            }
        }
        assert(!addedToAccTree[path.front()->GetAccInfo().GetAccSubID()] || !addedToAccTree[path.back()->GetAccInfo().GetAccSubID()]);
        // ze wzgledu na to jak budujemy podzbiory acykliczne zazwyczaj wierzcholki
        // juz dodane do drzewa rozpinajacego beda na poczatku sciezek
        // jezeli natomiast zdarzy sie, ze sa na koncu, to musimy odwrocic liste
        if (addedToAccTree[path.back()->GetAccInfo().GetAccSubID()])
        {
            path.reverse();
        }
        addedToAccTree[path.back()->GetAccInfo().GetAccSubID()] = 1;

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
            if (n->GetAccInfo().IsVertexInAccIntersection(vertex))
            {
                // aktualizujemy w aktualnym nodzie flagi acyklicznego przeciecia
                (*current)->GetAccInfo().UpdateAccIntersectionWithEdge(vertex, lastVertex);
                break;
            }
            // wpp oznacza, ze mozemy isc dalej
            else
            {
                // aktualizujemy w aktualnym nodzie flagi acyklicznego przeciecia
                (*current)->GetAccInfo().UpdateAccIntersectionWithEdge(vertex, lastVertex);
                lastVertex = vertex;
            }
            current = next;
            next++;
        }
    }
    delete [] addedToAccTree;
}

////////////////////////////////////////////////////////////////////////////////

void IncidenceGraph::UpdateConnectedComponents()
{
    ConnectedComponents::iterator cc = connectedComponents.begin();
    std::vector<std::set<Vertex> >::iterator ccb = connectedComponentsBorders.begin();
    std::vector<int>::iterator ccass = connectedComponentsAccSubSize.begin();
    while (cc != connectedComponents.end())
    {
        if ((*cc)->IsInAccSub())
        {
            Node *node = FindNode(*cc, FindNodeNotInAccSub());
            // jezeli nie ma zadnego sympleksu nieacyklicznego, to usuwamy
            // informacje o spojnej skladowej
            if (node == 0)
            {
                cc = connectedComponents.erase(cc);
                ccb = connectedComponentsBorders.erase(ccb);
                ccass = connectedComponentsAccSubSize.erase(ccass);
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

void IncidenceGraph::RemoveAccSub()
{
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        if ((*i)->IsInAccSub())
        {
            continue;
        }
        Edges newEdges;
        for (Edges::iterator edge = (*i)->edges.begin(); edge != (*i)->edges.end(); edge++)
        {
            if ((*edge)->GetNeighbour(*i)->IsInAccSub())
            {
                continue;
            }
            newEdges.push_back(*edge);
        }
        (*i)->edges = newEdges;
    }
    RemoveNodesWithPredicate(this, RemoveNodesWithFlags(Node::IGNPF_IN_ACC_SUB));
}

////////////////////////////////////////////////////////////////////////////////

void IncidenceGraph::AssignNewIndices(bool checkAcyclicity)
{
    int index = 0;
    if (checkAcyclicity)
    {
        for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            if ((*i)->IsInAccSub())
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
    std::queue<Node *> Q;
    Q.push(cc);
    cc->IsHelperFlag2(true);
    simplexPtrList.push_back(cc->simplex);
    while (!Q.empty())
    {
        IncidenceGraph::Node *node = Q.front();
        Q.pop();
        for (Edges::iterator edge = node->edges.begin(); edge != node->edges.end(); edge++)
        {
            Node *neighbour = (*edge)->GetNeighbour(node);
            if (neighbour->IsHelperFlag2())
            {
                continue;
            }
            neighbour->IsHelperFlag2(true);
            Q.push(neighbour);
            simplexPtrList.push_back(neighbour->simplex);
        }
    }
    RemoveNodesWithPredicate(this, RemoveNodesWithFlags(Node::IGNPF_HELPER_FLAG_2));
}

////////////////////////////////////////////////////////////////////////////////

void IncidenceGraph::GetAccSub(SimplexList &simplexList)
{
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        if ((*i)->IsInAccSub())
        {
            simplexList.push_back(*(*i)->simplex);
        }
    }
}

int IncidenceGraph::GetAccSubSize()
{
    int size = 0;
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        if ((*i)->IsInAccSub())
        {
            size++;
        }
    }
    return size;
}

////////////////////////////////////////////////////////////////////////////////
// eof
