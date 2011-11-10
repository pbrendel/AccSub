/*
 * File:   AccSpanningTree.cpp
 * Author: Piotr Brendel
 */

#include "AccSpanningTree.h"
#include "IncidenceGraphAlgorithms.h"

#ifdef USE_HELPERS
#include "../Helpers/Utils.h"
#endif

////////////////////////////////////////////////////////////////////////////////

void AccSpanningTree::Node::FindAcyclicSubsetToBorderConnection(Vertex borderVertex, IncidenceGraph::Path &path)
{
    if (acyclicSubsetSize > 0)
    {
        // szukanie sciezki od wierzcholka w brzegu do najbliszego acyklicznego sempleksu        
        path = FindPath(FindNode(parent->ig->nodes, FindNodeWithVertex(borderVertex)), FindPathToNodeWithAcyclicIntersection());
        assert(path.size() > 0);
    }
    else
    {
        singleBorderVerts.push_back(borderVertex);
    }
}

////////////////////////////////////////////////////////////////////////////////

void AccSpanningTree::Node::UpdateAcyclicSubsetToBorderConnection(Vertex borderVertex, IncidenceGraph::Path &path)
{
    if (acyclicSubsetSize > 0)
    {
        if (isConnectedToAcyclicSubset)
        {
            // std::cout<<"is connected to acyclic subset"<<std::endl;
            UpdatePathFromBorderToAcyclicSubset(borderVertex, path);
        }
        else
        {
            // std::cout<<"connecting to acyclic subset"<<std::endl;
            UpdatePathFromAcyclicSubsetToBorder(borderVertex, path);
            isConnectedToAcyclicSubset = true;
        }
    }
    else
    {
        // std::cout<<"updating border verts: "<<singleBorderVerts.size()<<std::endl;
        UpdateBorderVerts();
        singleBorderVerts.clear(); // zeby nie wykonalo sie drugi raz
    }
}

////////////////////////////////////////////////////////////////////////////////
// w ten funkcji dodajemy sciezke do podzbioru acyklicznego zaczynajac od
// wierzcholka na brzegu. zadaniem tej funkcji jest zagwarantowanie, zeby
// podany wierzcholek z brzegu mial polaczenie ze zbiorem acyklicznym,
// nie musimy zatem dojsc do koncowego node'a na sciezce, wystarczy ze gdzies
// wczesniej natrafimy na inny kawalek podzbioru acyklicznego
// zakladamy, ze jestesmy w "kawalku" kompleksu, ktory ma juz polaczenie
// ze zbiorem acyklicznym
void AccSpanningTree::Node::UpdatePathFromBorderToAcyclicSubset(Vertex borderVertex, IncidenceGraph::Path &path)
{
    IncidenceGraph::Node *prevNode = path.front();
    IncidenceGraph::Path::iterator i = path.begin();
    i++;
    // jezeli zaczynamy od acyklicznego wierzcholka to jestesmy w domu
    // (musimy zagwarantowac, ze wierzcholek na brzegu bedzie mial "dojscie"
    // do zbioru acyklicznego, wiec jezeli juz w nim jest, to znaczy, ze wczesniej
    // zostal do niego dolaczony)
    if (prevNode->GetAcyclicIntersectionFlags() & (1 << prevNode->NormalizeVertex(borderVertex)))
    {
        // std::cout<<"vertex "<<borderVertex<<" is in acyclic subset -> finishing"<<std::endl;
        return;
    }
    Vertex lastVertex = borderVertex;
    VertsSet vertsOnPath;
    vertsOnPath.insert(lastVertex);
    for (; i != path.end(); i++)
    {
        Vertex vertex = GetVertexFromIntersection(prevNode->simplex, (*i)->simplex);
        if (vertex == lastVertex)
        {
            prevNode = *i;
        }
        else if ((*i)->GetAcyclicIntersectionFlags() & (1 << (*i)->NormalizeVertex(vertex)))
        {
            // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<" -> finishing 1b"<<std::endl;
            prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, vertex);
            prevNode = 0;
            break;
        }
        else
        {
            vertsOnPath.insert(vertex);
            // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<std::endl;
            prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, vertex);
            prevNode = (*i);
            lastVertex = vertex;
            vertex = prevNode->FindAcyclicVertexNotIn(vertsOnPath);
            if (vertex != -1)
            {
                // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<" -> finishing 2b"<<std::endl;
                prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, vertex);
                prevNode = 0;
                break;
            }
        }
    }
    // jezeli na liscie byl tylko jeden node
    if (prevNode != 0)
    {
        Vertex vertex = prevNode->FindAcyclicVertexNotEqual(lastVertex);
        assert(vertex != -1);
        // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<" -> finishing 3"<<std::endl;
        prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, vertex);
           
    }
}

////////////////////////////////////////////////////////////////////////////////
// ta funkcja rozni sie od poprzedniej tym, ze tutaj dodajemy wierzcholki
// do sciezki zaczynajac od podzbioru acyklicznego. zadaniem tej funkcji
// jest "podlaczenie" zbioru acyklicznego do dodanego juz innego kawalka
// podzbioru acyklicznego. jezeli zatem trafimy na podzbior acykliczny to
// albo trafilismy na wierzcholek w brzegu, ktorego podlaczenie gwarantuje
// poprzednia funkcja albo trafilismy na inny podzbior acykliczny znajdujacy
// sie w brzegu => tez ok
void AccSpanningTree::Node::UpdatePathFromAcyclicSubsetToBorder(Vertex borderVertex, IncidenceGraph::Path &path)
{
    path.reverse();
    IncidenceGraph::Path::iterator i = path.begin();
    // pierwszy node sasiaduje ze zbiorem acyklicznym
    IncidenceGraph::Node *prevNode = *i;
    i++;
    // pierwszy node musi zawierac wierzcholek znajdujacy sie w zbiorze
    // acyklicznym, bo takie byly warunki szukania sciezki
    Vertex lastVertex = prevNode->FindAcyclicVertexNotIn(borderVerts);
    // std::cout<<"vertex "<<lastVertex<<" is acyclic"<<std::endl;
    assert(lastVertex != -1);
    for (; i != path.end(); i++)
    {
        Vertex vertex = GetVertexFromIntersection(prevNode->simplex, (*i)->simplex);
        if (vertex == lastVertex)
        {
            prevNode = *i;
        }
        else if ((*i)->GetAcyclicIntersectionFlags() & (1 << (*i)->NormalizeVertex(vertex)))
        {
            // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<" -> finishing 1a"<<std::endl;
            prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, vertex);
            prevNode = 0;
            break;
        }
        else
        {
            // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<std::endl;
            prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, vertex);
            prevNode = (*i);
            lastVertex = vertex;
        }
    }
    // jezeli prevNode == 0 to znaczy ze trafilismy na podzbior acykliczny
    // i konczymy szukanie sciezki, wpp. musimy jeszcze wierzcholek z przeciecia
    // dwoch osatnich node'ow polaczyc z wierzcholkiem na brzegu
    if (prevNode != 0)
    {
        // moze sie zdazyc, ze w ostatnie dwa sympleksy beda sasiadowaly
        // wlasnie na wierzcholku w brzegu, dlatego sprawdzamy, czy nie
        // dodajemy zdegenerowanej krawedzi
        if (lastVertex != borderVertex)
        {
            // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<borderVertex<<" -> finishing 2a"<<std::endl;
            prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, borderVertex);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// ta funkcja bierze laczy wszystkie punkty na brzegu znajdujace sie w zbiorze
// acyklicznym w drzewo. korzeniem drzewa jest pierwszy na liscie wierzcholek
// jezeli w trakcie dodawania sciezki od nowego liscia do drzewa trafimy ma
// zbior acykliczny to konczymy dodawanie sciezki => "podlaczylismy" nowy
// pozdbior acykliczny do juz utworzonego
void AccSpanningTree::Node::UpdateBorderVerts()
{
    // jezeli sa mniej niz dwa wierzcholki, to znaczy, ze ta czesc kompleksu
    // nie laczy zadnych zbiorow acyklicznych wiec mozna ja zignorowac
    if (singleBorderVerts.size() < 2)
    {
        // std::cout<<"less than 2 -> finishing"<<std::endl;
        return;
    }

//    std::cout<<"*";

    std::vector<Vertex>::iterator vertex = singleBorderVerts.begin();
    Vertex firstVertex = *vertex;
    IncidenceGraph::Node *firstNode = FindNode(connectedComponent, FindNodeWithVertex(firstVertex));
    // std::cout<<"first vertex: "<<firstVertex<<std::endl;
    vertex++;
    for (; vertex != singleBorderVerts.end(); vertex++)
    {
        // szukanie sciezki pomiedzy wierzcholkami
        IncidenceGraph::Path path = FindPath(firstNode, FindPathToVertex(*vertex));
        assert(path.size() > 0);
        IncidenceGraph::Node *prevNode = path.back();
        // jezeli wierzcholek jest juz w zbiorze acyklicznym, to kontynuujemy
        // znaczy to, ze jakas wczesniejsza sciezka przeszla przez niego
        // i dolaczyla go do zbioru acyklicznego
        if (prevNode->GetAcyclicIntersectionFlags() & (1 << prevNode->NormalizeVertex(*vertex)))
        {
            continue;
        }
        Vertex lastVertex = *vertex;
        // std::cout<<"vertex: "<<lastVertex<<std::endl;
        IncidenceGraph::Path::reverse_iterator i = path.rbegin();
        i++;
        VertsSet vertsOnPath;
        vertsOnPath.insert(lastVertex);
        for (; i != path.rend(); i++)
        {
            Vertex vertex = GetVertexFromIntersection(prevNode->simplex, (*i)->simplex);
            if (vertex == lastVertex)
            {
                prevNode = *i;
            }
            else if ((*i)->GetAcyclicIntersectionFlags() & (1 << (*i)->NormalizeVertex(vertex)))
            {
                // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<" -> finishing 2c"<<std::endl;
                prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, vertex);
                prevNode = 0;
                break;
            }
            else
            {
                vertsOnPath.insert(vertex);
                // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<std::endl;
                prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, vertex);               
                prevNode = (*i);
                lastVertex = vertex;
                vertex = prevNode->FindAcyclicVertexNotIn(vertsOnPath);
                if (vertex != -1)
                {
                    // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<" -> finishing 2b"<<std::endl;
                    prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, vertex);
                    prevNode = 0;
                    break;
                }
            }
        }
        // jezeli doszlismy tutaj, to znaczy ze nie trafilismy wczesniej
        // na podzbior acykliczny i musimy polaczyc sciezke z pierwszym
        // wierzcholkiem
        if (prevNode != 0)
        {
            // moze sie zdazyc, ze w ostatnie dwa sympleksy beda sasiadowaly
            // wlasnie na wierzcholku w brzegu, dlatego sprawdzamy, czy nie
            // dodajemy zdegenerowanej krawedzi
            if (lastVertex != firstVertex)
            {
                // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<firstVertex<<" -> finishing 2c"<<std::endl;
                prevNode->UpdateAcyclicIntersectionWithEdge(lastVertex, firstVertex);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void AccSpanningTree::Edge::FindAcyclicConnections()
{
    nodeA->FindAcyclicSubsetToBorderConnection(intersectionVertex, pathToA);
    nodeB->FindAcyclicSubsetToBorderConnection(intersectionVertex, pathToB);
}

void AccSpanningTree::Edge::UpdateAcyclicConnections()
{
    // najpierw uaktualnbiamy sciezke zbioru, ktory juz jest
    // dolaczony do drzewa
    // mamy wtedy pewnosc, ze jezeli pierwszy wierzcholek jest zaznaczony
    // jako acykliczny, to znaczy, ze zostal dodany do zbioru acyklicznego
    // juz wczesniej i mozna pod niego podpiac nowo dodawana czesc zbioru ac.

    // std::cout<<"intersection vertex: "<<intersectionVertex<<std::endl;
    if (nodeA->isConnectedToAcyclicSubset)
    {
        // Debug::Print(std::cout, pathToA);
        nodeA->UpdateAcyclicSubsetToBorderConnection(intersectionVertex, pathToA);
        // Debug::Print(std::cout, pathToB);
        nodeB->UpdateAcyclicSubsetToBorderConnection(intersectionVertex, pathToB);
    }
    else
    {
        // Debug::Print(std::cout, pathToB);
        nodeB->UpdateAcyclicSubsetToBorderConnection(intersectionVertex, pathToB);
        // Debug::Print(std::cout, pathToA);
        nodeA->UpdateAcyclicSubsetToBorderConnection(intersectionVertex, pathToA);
    }
}

////////////////////////////////////////////////////////////////////////////////

AccSpanningTree::AccSpanningTree(PartitionGraph *pg)
{
    // budujemy graf, w ktorym wierzcholkami beda podzbiory acykliczne
    // (czyli de facto skladowe spojne poszczegolnych paczek), ktore potem
    // polaczymy drzewem rozpinajacym
    std::map<PartitionGraph::Node *, std::vector<Node *> > dataNodeChildren;
    int currentID = 1;
    for (PartitionGraph::Nodes::iterator i = pg->nodes.begin(); i != pg->nodes.end(); i++)
    {
        IncidenceGraph *ig = (*i)->ig;
        std::vector<std::set<Vertex> >::iterator ccb = ig->connectedComponentsBorders.begin();
        std::vector<int>::iterator ccass = ig->connectedComponentsAcyclicSubsetSize.begin();
        for (IncidenceGraph::ConnectedComponents::iterator cc = ig->connectedComponents.begin(); cc != ig->connectedComponents.end(); cc++)
        {
            Node *newNode = new Node(*i, currentID++, *cc, *ccb, *ccass);
            dataNodeChildren[*i].push_back(newNode);
            nodes.push_back(newNode);
            ccb++;
            ccass++;
        }
    }

#ifdef USE_HELPERS 
    Timer::Update("creating acyclic tree nodes");
#endif

    // tworzymy krawedzie w grafie
    for (Nodes::iterator node = nodes.begin(); node != nodes.end(); node++)
    {
        PartitionGraph::Node *parent = (*node)->parent;
        std::vector<Node *> potentialNeighbours;
        for (PartitionGraph::Edges::iterator edge = parent->edges.begin(); edge != parent->edges.end(); edge++)
        {
            PartitionGraph::Node *neighbour = ((*edge)->nodeA == parent) ? (*edge)->nodeB : (*edge)->nodeA;            
            potentialNeighbours.insert(potentialNeighbours.end(), dataNodeChildren[neighbour].begin(), dataNodeChildren[neighbour].end());
        }
        for (Nodes::iterator neighbour = potentialNeighbours.begin(); neighbour != potentialNeighbours.end(); neighbour++)
        {
            if ((*neighbour)->subtreeID <= (*node)->subtreeID)
            {
                continue;
            }
            Simplex intersection;
            GetSortedIntersectionOfUnsortedSets(intersection, (*node)->borderVerts, (*neighbour)->borderVerts);
            if (intersection.size() > 0)
            {
                Edge *edge = new Edge(*node, *neighbour, intersection.front());
                edges.push_back(edge);
                (*node)->AddEdge(edge);
                (*neighbour)->AddEdge(edge);
            }
        }
    }

#ifdef USE_HELPERS 
    Timer::Update("creating acyclic tree edges");
#endif

    // tworzymy hasha, w ktorym dla kazdego ID poddrzewa bedziemy przechowywali
    // rozmiar podzbioru acyklicznego w nim zawartego (w poddrzewie)
    std::map<int, int> spanningTreeAcyclicSubsetSize;
    for (Nodes::iterator node = nodes.begin(); node != nodes.end(); node++)
    {
        spanningTreeAcyclicSubsetSize[(*node)->subtreeID] = (*node)->acyclicSubsetSize;
    }

    // i na koncu drzewo rozpinajace
    for (Edges::iterator edge = edges.begin(); edge != edges.end(); edge++)
    {
        if ((*edge)->nodeA->subtreeID == (*edge)->nodeB->subtreeID)
        {
            continue;
        }
        (*edge)->isInSpanningTree = true;
        int newID = (*edge)->nodeA->subtreeID;
        int oldID = (*edge)->nodeB->subtreeID;
        // przy laczeniu poddrzew sumujemy rozmiary podzbiorow acyklicznych
        // w nich zawartych
        spanningTreeAcyclicSubsetSize[newID] = spanningTreeAcyclicSubsetSize[newID] + spanningTreeAcyclicSubsetSize[oldID];
        for (Nodes::iterator node = nodes.begin(); node != nodes.end(); node++)
        {
            if ((*node)->subtreeID == oldID)
            {
                (*node)->subtreeID = newID;
            }
        }
    }

#ifdef USE_HELPERS 
    Timer::Update("creating spanning tree");
#endif

    // jezeli zostalo jakies poddrzewo, ktore nie zawiera podzbioru acyklicznego
    // to usuwamy wszystkie sympleksy z utworzonego grafu, dodajemy do wspolnej
    // listy, a nastepnie traktujemy jako jedna spojna skladowa i jeszcze raz
    // wysylamy do obliczen, ale tym razem nie laczymy juz brzegow, tylko po
    // prostu "przenosimy" nowo utworzony graf jako osobna spojna skladowa
    // duzego grafu
    std::map<int, SimplexPtrList> simplexPtrLists;
    Nodes::iterator node = nodes.begin();
    while (node != nodes.end())
    {
        if (spanningTreeAcyclicSubsetSize[(*node)->subtreeID] == 0)
        {
            // usuwamy spojna skladowa i kopiujemy jej liste sympleksow
            (*node)->parent->ig->RemoveConnectedComponentAndCopySimplexList((*node)->connectedComponent, simplexPtrLists[(*node)->subtreeID]);
            // usuwamy krawedzie drzewa rozpinajacego laczace tego node'a
            Edges::iterator edge = edges.begin();
            while (edge != edges.end())
            {
                if ((*edge)->nodeA == *node || (*edge)->nodeB == *node)
                {
                    edge = edges.erase(edge);
                }
                else
                {
                    edge++;
                }
            }
            node = nodes.erase(node);
        }
        else
        {
            node++;
        }
    }

    // dla kazdej listy sympleksow, ktora otrzymalismy w poprzednim kroku
    // tworzymy osobny data node (nie interesuje nas zupelnie jego brzeg,
    // poniewaz wiemy, ze jest to oddzielna spojna skladowa i nie sasiaduje
    // z innymi sympleksami)
    for (std::map<int, SimplexPtrList>::iterator i = simplexPtrLists.begin(); i != simplexPtrLists.end(); i++)
    {
        pg->isolatedNodes.push_back(new PartitionGraph::Node(i->second));
    }

#ifdef USE_HELPERS 
    Timer::Update("finding isolated data nodes");
#endif
    
    // na drzewie rozpinajacym
    // szukamny polaczen acyklicznych zbiorow, ale na razie ich nie laczymy
    // bedziemy robic to po polaczeniu wszystkich sympleksow, zeby dobrze
    // zaktualizowac podzbior acykliczny
    for (Edges::iterator i = edges.begin(); i != edges.end(); i++)
    {
        if ((*i)->isInSpanningTree) (*i)->FindAcyclicConnections();
    }

#ifdef USE_HELPERS 
    Timer::Update("searching paths from acyclic subsets to border");    
#endif
}

////////////////////////////////////////////////////////////////////////////////

AccSpanningTree::~AccSpanningTree()
{
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        delete (*i);
    }
    for (Edges::iterator i = edges.begin(); i != edges.end(); i++)
    {
        delete (*i);
    }    
}

////////////////////////////////////////////////////////////////////////////////

void AccSpanningTree::JoinAcyclicSubsets()
{
    // aktualizujemy zbior acykliczny o polaczenia pomiedzy podgrafami
    for (Edges::iterator i = edges.begin(); i != edges.end(); i++)
    {
        if ((*i)->isInSpanningTree) (*i)->UpdateAcyclicConnections();
    }

#ifdef USE_HELPERS    
    Timer::Update("adding paths to acyclic subset");    
#endif
    
}

////////////////////////////////////////////////////////////////////////////////
// eof
