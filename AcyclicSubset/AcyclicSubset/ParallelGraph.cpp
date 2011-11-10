/*
 * File:   ParallelGraph.cpp
 * Author: Piotr Brendel
 */

#include "ParallelGraph.h"
#include "IncidenceGraph.h"
#include "IncidenceGraphUtils.h"
#include "../Helpers/Utils.h"
//#include "redHom/complex/scomplex/SComplexBuilderFromSimplices.hpp"

#include <iostream>
#include <algorithm>
#include <set>
#include <map>
#include <list>
//#include <cstdlib>
#include <cmath> // ceil()

#include "ComputationsLocal.h"
#include "ComputationsLocalMPITest.h"

// test!!!
#include "../Helpers/Tests.h"
#include "PrepareDataBFS.h"

////////////////////////////////////////////////////////////////////////////////

int ParallelGraph::DataNode::GetConstantSimplexSize()
{
   int size = simplexPtrList[0]->size();
   for (SimplexPtrList::iterator i = simplexPtrList.begin(); i != simplexPtrList.end(); i++)
   {
       if ((*i)->size() != size)
       {
           return 0;
       }
   }
   return size;
}

void ParallelGraph::DataNode::CreateIntNodesMapWithBorderNodes()
{
    if (H.size() > 0)
    {
        // juz zbudowany
        return;
    }
    for (IncidenceGraph::Nodes::iterator node = ig->nodes.begin(); node != ig->nodes.end(); node++)
    {
        if ((*node)->IsOnBorder())
        {
            for (Simplex::iterator v = (*node)->simplex->begin(); v != (*node)->simplex->end(); v++)
            {
                H[*v].push_back(*node);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void ParallelGraph::DataNode::RemoveChildAndCopySimplexPtrList(SpanningTreeNode* node, SimplexPtrList& simplexPtrList)
{
    // usuwamy node z listy dzieci
    std::vector<SpanningTreeNode *>::iterator it = std::find(spanningTreeNodes.begin(), spanningTreeNodes.end(), node);
    assert(it != spanningTreeNodes.end());
    delete *it;
    spanningTreeNodes.erase(it);

    // przelatujemy cala spojna skladowa i zaznaczamy elementy do niej nalezace
    // dodatkowo dodajemy do listy sympleksy z tej spojnej skladowej
    std::queue<IncidenceGraph::Node *> L;
    L.push(node->connectedComponent);
    node->connectedComponent->IsHelperFlag2(true);
    simplexPtrList.push_back(node->connectedComponent->simplex);
    while (!L.empty())
    {
        IncidenceGraph::Node *n = L.front();
        L.pop();
        for (IncidenceGraph::Edges::iterator edge = n->edges.begin(); edge != n->edges.end(); edge++)
        {
            if (edge->node->IsHelperFlag2())
            {
                continue;
            }
            edge->node->IsHelperFlag2(true);
            L.push(edge->node);
            simplexPtrList.push_back(edge->node->simplex);
        }
    }

    // na koncu wszystkie zaznaczone node'y usuwamy z grafu
    IncidenceGraph::Nodes::iterator i = ig->nodes.begin();
    while (i != ig->nodes.end())
    {
        if ((*i)->IsHelperFlag2())
        {
            delete *i;
            i = ig->nodes.erase(i);
        }
        else
        {
            i++;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void ParallelGraph::SpanningTreeNode::FindAcyclicSubsetToBorderConnection(Vertex borderVertex, IncidenceGraph::Path &path)
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

void ParallelGraph::SpanningTreeNode::UpdateAcyclicSubsetToBorderConnection(Vertex borderVertex, IncidenceGraph::Path &path)
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
void ParallelGraph::SpanningTreeNode::UpdatePathFromBorderToAcyclicSubset(Vertex borderVertex, IncidenceGraph::Path &path)
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
void ParallelGraph::SpanningTreeNode::UpdatePathFromAcyclicSubsetToBorder(Vertex borderVertex, IncidenceGraph::Path &path)
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
void ParallelGraph::SpanningTreeNode::UpdateBorderVerts()
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

void ParallelGraph::SpanningTreeEdge::FindAcyclicConnections()
{
    nodeA->FindAcyclicSubsetToBorderConnection(intersectionVertex, pathToA);
    nodeB->FindAcyclicSubsetToBorderConnection(intersectionVertex, pathToB);
}

void ParallelGraph::SpanningTreeEdge::UpdateAcyclicConnections()
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

ParallelGraph::ParallelGraph(SimplexList &simplexList, int packsCount, AccSubAlgorithm accSubAlgorithm, AcyclicTest<IncidenceGraph::IntersectionFlags> *acyclicTest)
{
    this->incidenceGraph = new IncidenceGraph(GetDimension(simplexList));
    this->initialSize = simplexList.size();
    this->accSubAlgorithm = accSubAlgorithm;
    this->acyclicTest = acyclicTest;
    int packSize = (int)ceil(float(simplexList.size()) / packsCount);
    std::cout<<"pack size: "<<packSize<<std::endl;
    PrepareDataBFS::Prepare(simplexList, packSize);
    Timer::Update("preparing data");
    DivideData(simplexList, packSize);
    Timer::Update("dividing data");
    CreateDataEdges();
    Timer::Update("creating data connections");
    CalculateIncidenceGraphs(dataNodes);
    Timer::Update("creating incidence graphs");
    CreateSpanningTree();
    CalculateIncidenceGraphs(secondPhaseDataNodes);
    Timer::Update("creating second phase incidence graphs");
    CombineGraphs();
}

////////////////////////////////////////////////////////////////////////////////

ParallelGraph::~ParallelGraph()
{
    for (DataNodes::iterator i = dataNodes.begin(); i != dataNodes.end(); i++)
    {
        delete (*i);
    }
    for (DataEdges::iterator i = dataEdges.begin(); i != dataEdges.end(); i++)
    {
        delete (*i);
    }
    for (DataNodes::iterator i = secondPhaseDataNodes.begin(); i != secondPhaseDataNodes.end(); i++)
    {
        delete (*i);
    }
    for (SpanningTreeNodes::iterator i = spanningTreeNodes.begin(); i != spanningTreeNodes.end(); i++)
    {
        delete (*i);
    }
    for (SpanningTreeEdges::iterator i = spanningTreeEdges.begin(); i != spanningTreeEdges.end(); i++)
    {
        delete (*i);
    }
}

////////////////////////////////////////////////////////////////////////////////

void ParallelGraph::DivideData(SimplexList& simplexList, int packSize)
{
    DataNode *currentNode =  new DataNode();
    int simplicesLeft = packSize;
    SimplexList::iterator it = simplexList.begin();

    while (it != simplexList.end())
    {
        // tempSimplexList.push_back(*it);

        currentNode->simplexPtrList.push_back(&(*it));
        for (Simplex::const_iterator i = it->begin(); i != it->end(); i++)
        {
            currentNode->verts.insert(*i);
        }
        simplicesLeft--;
        if (simplicesLeft == 0)
        {
            dataNodes.push_back(currentNode);
            currentNode = new DataNode();
            simplicesLeft = packSize;
        }
        it++;

        // test!!!
    //    if (tempSimplexList.size() > 27500) break;
    }
    if (currentNode->simplexPtrList.size() > 0)
    {
        dataNodes.push_back(currentNode);
    }

     // std::cout<<"temp simplex list ("<<tempSimplexList.size()<<") homology:"<<std::endl;
     // Tests::Test(tempSimplexList, RT_Coreduction);
}

////////////////////////////////////////////////////////////////////////////////

void ParallelGraph::CreateDataEdges()
{
    for (DataNodes::iterator i = dataNodes.begin(); i != dataNodes.end(); i++)
    {
        for (DataNodes::iterator j = i + 1; j != dataNodes.end(); j++)
        {
            Simplex intersection;
            GetSortedIntersectionOfUnsortedSets(intersection, (*i)->verts, (*j)->verts);
            if (intersection.size() > 0)
            {
                DataEdge *edge = new DataEdge(*i, *j);
                dataEdges.push_back(edge);
                (*i)->AddEdge(edge, intersection);
                (*j)->AddEdge(edge, intersection);
            }
        }
    }

//    for (DataNodes::iterator i = dataNodes.begin(); i != dataNodes.end(); i++)
//    {
//        DataNode *node = *i;
//        std::cout<<"border verts: "<<node->borderVerts.size()<<" verts size: "<<node->verts.size()<<std::endl;
//    }
}

////////////////////////////////////////////////////////////////////////////////

void ParallelGraph::CalculateIncidenceGraphs(DataNodes &sourceNodes)
{
    ComputationsLocalMPITest::Compute(sourceNodes, accSubAlgorithm, acyclicTest);
}

////////////////////////////////////////////////////////////////////////////////

void ParallelGraph::CreateSpanningTree()
{
    // budujemy graf, w ktorym wierzcholkami beda podzbiory acykliczne
    // (czyli de facto skladowe spojne poszczegolnych paczek), ktore potem
    // polaczymy drzewem rozpinajacym
    int currentID = 1;
    for (DataNodes::iterator i = dataNodes.begin(); i != dataNodes.end(); i++)
    {
        IncidenceGraph *ig = (*i)->ig;
        std::vector<std::set<Vertex> >::iterator ccb = ig->connectedComponentsBorders.begin();
        std::vector<int>::iterator ccass = ig->connectedComponentsAcyclicSubsetSize.begin();
        for (IncidenceGraph::ConnectedComponents::iterator cc = ig->connectedComponents.begin(); cc != ig->connectedComponents.end(); cc++)
        {
            SpanningTreeNode *newNode = new SpanningTreeNode(*i, currentID++, *cc, *ccb, *ccass);
            (*i)->spanningTreeNodes.push_back(newNode);
            spanningTreeNodes.push_back(newNode);
            ccb++;
            ccass++;
        }
    }

    Timer::Update("creating acyclic tree nodes");

    // tworzymy krawedzie w grafie
    for (SpanningTreeNodes::iterator node = spanningTreeNodes.begin(); node != spanningTreeNodes.end(); node++)
    {
        DataNode *parent = (*node)->parent;
        std::vector<SpanningTreeNode *> potentialNeighbours;
        for (DataEdges::iterator edge = parent->edges.begin(); edge != parent->edges.end(); edge++)
        {
            DataNode *neighbour = ((*edge)->nodeA == parent) ? (*edge)->nodeB : (*edge)->nodeA;
            potentialNeighbours.insert(potentialNeighbours.end(), neighbour->spanningTreeNodes.begin(), neighbour->spanningTreeNodes.end());
        }
        for (SpanningTreeNodes::iterator neighbour = potentialNeighbours.begin(); neighbour != potentialNeighbours.end(); neighbour++)
        {
            if ((*neighbour)->subtreeID <= (*node)->subtreeID)
            {
                continue;
            }
            Simplex intersection;
            GetSortedIntersectionOfUnsortedSets(intersection, (*node)->borderVerts, (*neighbour)->borderVerts);
            if (intersection.size() > 0)
            {
                SpanningTreeEdge *edge = new SpanningTreeEdge(*node, *neighbour, intersection.front());
                spanningTreeEdges.push_back(edge);
                (*node)->AddEdge(edge);
                (*neighbour)->AddEdge(edge);
            }
        }
    }

    Timer::Update("creating acyclic tree edges");

    // tworzymy hasha, w ktorym dla kazdego ID poddrzewa bedziemy przechowywali
    // rozmiar podzbioru acyklicznego w nim zawartego (w poddrzewie)
    std::map<int, int> spanningTreeAcyclicSubsetSize;
    for (SpanningTreeNodes::iterator node = spanningTreeNodes.begin(); node != spanningTreeNodes.end(); node++)
    {
        spanningTreeAcyclicSubsetSize[(*node)->subtreeID] = (*node)->acyclicSubsetSize;
    }

    // i na koncu drzewo rozpinajace
    for (SpanningTreeEdges::iterator edge = spanningTreeEdges.begin(); edge != spanningTreeEdges.end(); edge++)
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
        for (SpanningTreeNodes::iterator node = spanningTreeNodes.begin(); node != spanningTreeNodes.end(); node++)
        {
            if ((*node)->subtreeID == oldID)
            {
                (*node)->subtreeID = newID;
            }
        }
    }

    Timer::Update("creating spanning tree");

    // jezeli zostalo jakies poddrzewo, ktore nie zawiera podzbioru acyklicznego
    // to usuwamy wszystkie sympleksy z utworzonego grafu, dodajemy do wspolnej
    // listy, a nastepnie traktujemy jako jedna spojna skladowa i jeszcze raz
    // wysylamy do obliczen, ale tym razem nie laczymy juz brzegow, tylko po
    // prostu "przenosimy" nowo utworzony graf jako osobna spojna skladowa
    // duzego grafu
    std::map<int, SimplexPtrList> simplexPtrLists;
    SpanningTreeNodes::iterator node = spanningTreeNodes.begin();
    while (node != spanningTreeNodes.end())
    {
        if (spanningTreeAcyclicSubsetSize[(*node)->subtreeID] == 0)
        {
            (*node)->parent->RemoveChildAndCopySimplexPtrList(*node, simplexPtrLists[(*node)->subtreeID]);
            // usuwamy krawedzie drzewa rozpinajacego laczace tego node'a
            SpanningTreeEdges::iterator edge = spanningTreeEdges.begin();
            while (edge != spanningTreeEdges.end())
            {
                if ((*edge)->nodeA == *node || (*edge)->nodeB == *node)
                {
                    edge = spanningTreeEdges.erase(edge);
                }
                else
                {
                    edge++;
                }
            }
            node = spanningTreeNodes.erase(node);
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
        std::cout<<"creating second phase data node with "<<i->second.size()<<" simplices"<<std::endl;
        DataNode *newNode = new DataNode();
        newNode->simplexPtrList = i->second;
        secondPhaseDataNodes.push_back(newNode);
    }

    Timer::Update("finding second phase data nodes");
}

////////////////////////////////////////////////////////////////////////////////

void ParallelGraph::CombineGraphs()
{
    // na drzewie rozpinajacym
    // szukamny polaczen acyklicznych zbiorow, ale na razie ich nie laczymy
    // bedziemy robic to po polaczeniu wszystkich sympleksow, zeby dobrze
    // zaktualizowac podzbior acykliczny
    for (SpanningTreeEdges::iterator i = spanningTreeEdges.begin(); i != spanningTreeEdges.end(); i++)
    {
        if ((*i)->isInSpanningTree) (*i)->FindAcyclicConnections();
    }

    Timer::Update("searching paths from acyclic subsets to border");
    Timer::Time timeStart = Timer::Now();

    float creatingMapsTime = 0;
    float connectingSimplicesTime = 0;
    int neighboursCount = 0;
    int nodesCount = 0;

    // laczymy sympleksy
    for (DataEdges::iterator edge = dataEdges.begin(); edge != dataEdges.end(); edge++)
    {
        IncidenceGraph::Nodes nodesA = (*edge)->nodeA->ig->nodes;
        (*edge)->nodeB->CreateIntNodesMapWithBorderNodes();
        IncidenceGraph::VertexNodesMap HB = (*edge)->nodeB->H;

        creatingMapsTime += Timer::Update();

        for (IncidenceGraph::Nodes::iterator node = nodesA.begin(); node != nodesA.end(); node++)
        {
            if (!(*node)->IsOnBorder())
            {
                continue;
            }
            for (Simplex::iterator v = (*node)->simplex->begin(); v != (*node)->simplex->end(); v++)
            {
                IncidenceGraph::Nodes neighbours = HB[*v];
                if (neighbours.size() == 0)
                {
                    continue;
                }
                neighboursCount += neighbours.size();
                nodesCount++;
                for (IncidenceGraph::Nodes::iterator neighbour = neighbours.begin(); neighbour != neighbours.end(); neighbour++)
                {                    
                    if (!(*node)->HasNeighbour(*neighbour))
                    {
                        (*node)->AddNeighbour(*neighbour);
                        (*neighbour)->AddNeighbour(*node);
                    }
                }
            }
        }
        connectingSimplicesTime += Timer::Update();
    }

    Timer::TimeFrom(timeStart, "connecting simplices on border");
    std::cout<<"creating maps: "<<creatingMapsTime<<std::endl;
    std::cout<<"connecting simplices: "<<connectingSimplicesTime<<std::endl;
    if (nodesCount > 0)
    {
        std::cout<<"avg neighbours count: "<<neighboursCount / nodesCount<<std::endl;
    }
    Timer::Update();

    // przenosimy wszystkie sympleksy do jednego grafu
    for (DataNodes::iterator i = dataNodes.begin(); i != dataNodes.end(); i++)
    {
         incidenceGraph->nodes.insert(incidenceGraph->nodes.end(), (*i)->ig->nodes.begin(), (*i)->ig->nodes.end());
        (*i)->ig->nodes.clear();
    }

    // to samo ze spojnymi skladowymi z drugiej fazy obliczen
    for (DataNodes::iterator i = secondPhaseDataNodes.begin(); i != secondPhaseDataNodes.end(); i++)
    {
         incidenceGraph->nodes.insert(incidenceGraph->nodes.end(), (*i)->ig->nodes.begin(), (*i)->ig->nodes.end());
        (*i)->ig->nodes.clear();
    }

    // przypisywanie nowego grafu do nodow
    for (IncidenceGraph::Nodes::iterator i = incidenceGraph->nodes.begin(); i != incidenceGraph->nodes.end(); i++)
    {
        (*i)->SetParentGraph(incidenceGraph);
    }

    std::cout<<"total simplices after connecting graphs: "<<incidenceGraph->nodes.size()<<std::endl;
    std::cout<<"reduced acyclic subset size: "<<(initialSize - incidenceGraph->nodes.size())<<" ("<<((initialSize - incidenceGraph->nodes.size()) * 100 / initialSize)<<"%)"<<std::endl;

    Timer::Update("moving simplices to single incidence graph");

    // aktualizujemy zbior acykliczny o polaczenia pomiedzy podgrafami
    for (SpanningTreeEdges::iterator i = spanningTreeEdges.begin(); i != spanningTreeEdges.end(); i++)
    {
        if ((*i)->isInSpanningTree) (*i)->UpdateAcyclicConnections();
    }

    Timer::Update("adding paths to acyclic subset");
}

////////////////////////////////////////////////////////////////////////////////
// eof
