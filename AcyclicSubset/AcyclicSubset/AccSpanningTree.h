/*
 * File:   AccSpanningTree.hpp
 * Author: Piotr Brendel
 */

#ifndef ACCSPANNINGTREE_HPP
#define ACCSPANNINGTREE_HPP

#include "IncidenceGraphAlgorithms.hpp"
#include <vector>
#include <set>
#include <map>

#ifdef ACCSUB_TRACE
#include "../Helpers/Utils.h"
#endif

template <typename PartitionGraph>
class AccSpanningTreeT
{
    typedef typename PartitionGraph::IncidenceGraph IncidenceGraph;
    typedef typename IncidenceGraph::Vertex Vertex;
    typedef typename IncidenceGraph::VertsSet VertsSet;
    typedef typename IncidenceGraph::Simplex Simplex;
    typedef typename IncidenceGraph::SimplexPtrList SimplexPtrList;
    typedef typename IncidenceGraph::ConnectedComponent ConnectedComponent;
    typedef typename IncidenceGraph::ConnectedComponents ConnectedComponents;
    typedef typename IncidenceGraph::Path Path;

    typedef typename IncidenceGraph::Node IncidenceGraphNode;
    typedef typename PartitionGraph::Node PartitionGraphNode;

public:
    
    struct Edge;
    
    struct Node
    {
        PartitionGraphNode      *parent;
        int                     subtreeID;
        std::set<Vertex>        borderVerts;
        int                     accSubSize;
        std::vector<Edge *>     edges;
        ConnectedComponent      connectedComponent;
        std::vector<Vertex>     boundaryVertsToConnect;
        std::vector<Path>       boundaryVertsConnectingPaths;
        bool                    isConnectedToAccSub;

        Node(PartitionGraphNode *parent, int id, ConnectedComponent connectedComponent, std::set<Vertex> &borderVerts, int accSubSize)
        {
            this->parent = parent;
            this->subtreeID = id;
            this->connectedComponent = connectedComponent;
            this->borderVerts = borderVerts;
            this->accSubSize = accSubSize;
            this->isConnectedToAccSub = false;
        }

        void AddEdge(Edge *edge)
        {
            edges.push_back(edge);
        }
        
        void FindAccSubToBorderConnection(Vertex borderVertex, Path &path)
        {
            if (accSubSize > 0)
            {
                // szukanie sciezki od wierzcholka w brzegu do najbliszego acyklicznego sempleksu
                path = FindPath(FindNode(parent->ig->nodes, FindNodeWithVertex<IncidenceGraph>(borderVertex)), FindPathToNodeWithAccIntersection<IncidenceGraph>());
                assert(path.size() > 0);
            }
            else
            {
                boundaryVertsToConnect.push_back(borderVertex);
            }
        }

        void UpdateAccSubToBorderConnection(Vertex borderVertex, Path &path)
        {
            if (accSubSize > 0)
            {
                if (isConnectedToAccSub)
                {
                    // std::cout<<"is connected to acyclic subset"<<std::endl;
                    UpdatePathFromBorderToAccSub(borderVertex, path);
                }
                else
                {
                    // std::cout<<"connecting to acyclic subset"<<std::endl;
                    UpdatePathFromAccSubToBorder(borderVertex, path);
                    isConnectedToAccSub = true;
                }
            }
            else
            {
                // std::cout<<"updating border verts: "<<singleBorderVerts.size()<<std::endl;
                UpdateBoundaryVertsConnectingPaths();
                boundaryVertsConnectingPaths.clear();
                isConnectedToAccSub = true;
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
        void UpdatePathFromBorderToAccSub(Vertex borderVertex, Path &path)
        {
            IncidenceGraphNode *prevNode = path.front();
            typename Path::iterator i = path.begin();
            i++;
            // jezeli zaczynamy od acyklicznego wierzcholka to jestesmy w domu
            // (musimy zagwarantowac, ze wierzcholek na brzegu bedzie mial "dojscie"
            // do zbioru acyklicznego, wiec jezeli juz w nim jest, to znaczy, ze wczesniej
            // zostal do niego dolaczony)
            if (prevNode->GetAccInfo().IsVertexInAccIntersection(borderVertex))
            {
                // std::cout<<"vertex "<<borderVertex<<" is in acyclic subset -> finishing"<<std::endl;
                return;
            }
            Vertex lastVertex = borderVertex;
            VertsSet vertsOnPath;
            vertsOnPath.insert(lastVertex);
            for (; i != path.end(); i++)
            {
                Vertex vertex = Simplex::GetVertexFromIntersection(prevNode->simplex, (*i)->simplex);
                if (vertex == lastVertex)
                {
                    prevNode = *i;
                }
                else if ((*i)->GetAccInfo().IsVertexInAccIntersection(vertex))
                {
                    // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<" -> finishing 1b"<<std::endl;
                    prevNode->GetAccInfo().UpdateAccIntersectionWithEdge(lastVertex, vertex);
                    prevNode = 0;
                    break;
                }
                else
                {
                    vertsOnPath.insert(vertex);
                    // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<std::endl;
                    prevNode->GetAccInfo().UpdateAccIntersectionWithEdge(lastVertex, vertex);
                    prevNode = (*i);
                    lastVertex = vertex;
                    vertex = prevNode->GetAccInfo().FindAccVertexNotIn(vertsOnPath);
                    if (vertex != Vertex(-1))
                    {
                        // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<" -> finishing 2b"<<std::endl;
                        prevNode->GetAccInfo().UpdateAccIntersectionWithEdge(lastVertex, vertex);
                        prevNode = 0;
                        break;
                    }
                }
            }
            // jezeli na liscie byl tylko jeden node
            if (prevNode != 0)
            {
                Vertex vertex = prevNode->GetAccInfo().FindAccVertexNotEqual(lastVertex);
                assert(vertex != Vertex(-1));
                // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<" -> finishing 3"<<std::endl;
                prevNode->GetAccInfo().UpdateAccIntersectionWithEdge(lastVertex, vertex);

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
        void UpdatePathFromAccSubToBorder(Vertex borderVertex, Path &path)
        {
            path.reverse();
            typename Path::iterator i = path.begin();
            // pierwszy node sasiaduje ze zbiorem acyklicznym
            IncidenceGraphNode *prevNode = *i;
            i++;
            // pierwszy node musi zawierac wierzcholek znajdujacy sie w zbiorze
            // acyklicznym, bo takie byly warunki szukania sciezki
            Vertex lastVertex = prevNode->GetAccInfo().FindAccVertexNotIn(borderVerts);
            // std::cout<<"vertex "<<lastVertex<<" is acyclic"<<std::endl;
            assert(lastVertex != Vertex(-1));
            for (; i != path.end(); i++)
            {
                Vertex vertex = Simplex::GetVertexFromIntersection(prevNode->simplex, (*i)->simplex);
                if (vertex == lastVertex)
                {
                    prevNode = *i;
                }
                else if ((*i)->GetAccInfo().IsVertexInAccIntersection(vertex))
                {
                    // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<" -> finishing 1a"<<std::endl;
                    prevNode->GetAccInfo().UpdateAccIntersectionWithEdge(lastVertex, vertex);
                    prevNode = 0;
                    break;
                }
                else
                {
                    // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<std::endl;
                    prevNode->GetAccInfo().UpdateAccIntersectionWithEdge(lastVertex, vertex);
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
                    prevNode->GetAccInfo().UpdateAccIntersectionWithEdge(lastVertex, borderVertex);
                }
            }
        }
        
// szukanie sciezek laczacych wszystkie punkty lezace na brzegu
// docelowo powstaje drzewo, ktorego "korzeniem" jest pierwszy wierzcholek
// na liscie
        void FindBoundaryVertsConnectingPaths()
        {
            // jezeli sa mniej niz dwa wierzcholki, to znaczy, ze ta czesc kompleksu
            // nie laczy zadnych zbiorow acyklicznych wiec mozna ja zignorowac
            if (boundaryVertsToConnect.size() < 2)
            {
                return;
            }

            typename std::vector<Vertex>::iterator vertex = boundaryVertsToConnect.begin();
            Vertex firstVertex = *vertex;
            IncidenceGraphNode *firstNode = FindNode(connectedComponent, FindNodeWithVertex<IncidenceGraph>(firstVertex));
            vertex++;
            for (; vertex != boundaryVertsToConnect.end(); vertex++)
            {
                // szukanie sciezki pomiedzy wierzcholkami
                Path path = FindPath(firstNode, FindPathToVertex<IncidenceGraph>(*vertex));
                assert(path.size() > 0);
                boundaryVertsConnectingPaths.push_back(path);
            }
        }

////////////////////////////////////////////////////////////////////////////////
// uprzednio wyliczone sciezki laczace poszczegolne punkty na brzegu
// dodajemy do zbioru acyklicznego
// jezeli w trakcie dodawania sciezki trafimy ma
// zbior acykliczny to konczymy dodawanie sciezki => "podlaczylismy" nowy
// pozdbior acykliczny do juz utworzonego
        void UpdateBoundaryVertsConnectingPaths()
        {
            // jezeli sa mniej niz dwa wierzcholki, to znaczy, ze ta czesc kompleksu
            // nie laczy zadnych zbiorow acyklicznych wiec mozna ja zignorowac
            if (boundaryVertsConnectingPaths.size() < 1)
            {
                // std::cout<<"less than 1 -> finishing"<<std::endl;
                return;
            }

            typename std::vector<Vertex>::iterator vertex = boundaryVertsToConnect.begin();
            Vertex firstVertex = *vertex;
            IncidenceGraphNode *firstNode = FindNode(connectedComponent, FindNodeWithVertex<IncidenceGraph>(firstVertex));
            firstNode->GetAccInfo().UpdateAccIntersectionWithVertex(firstVertex);
            vertex++;
            for (typename std::vector<Path>::iterator path = boundaryVertsConnectingPaths.begin(); path != boundaryVertsConnectingPaths.end(); path++, vertex++)
            {
                assert(vertex != boundaryVertsToConnect.end());
                IncidenceGraphNode *prevNode = path->back();
                // jezeli wierzcholek jest juz w zbiorze acyklicznym, to kontynuujemy
                // znaczy to, ze jakas wczesniejsza sciezka przeszla przez niego
                // i dolaczyla go do zbioru acyklicznego
                if (prevNode->GetAccInfo().IsVertexInAccIntersection(*vertex))
                {
                    continue;
                }
                Vertex lastVertex = *vertex;
                // std::cout<<"vertex: "<<lastVertex<<std::endl;
                typename Path::reverse_iterator i = path->rbegin();
                i++;
                VertsSet vertsOnPath;
                vertsOnPath.insert(lastVertex);
                for (; i != path->rend(); i++)
                {
                    Vertex vertex = Simplex::GetVertexFromIntersection(prevNode->simplex, (*i)->simplex);
                    if (vertex == lastVertex)
                    {
                        prevNode = *i;
                    }
                    else if ((*i)->GetAccInfo().IsVertexInAccIntersection(vertex))
                    {
                        // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<" -> finishing 2c"<<std::endl;
                        prevNode->GetAccInfo().UpdateAccIntersectionWithEdge(lastVertex, vertex);
                        prevNode = 0;
                        break;
                    }
                    else
                    {
                        vertsOnPath.insert(vertex);
                        // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<std::endl;
                        prevNode->GetAccInfo().UpdateAccIntersectionWithEdge(lastVertex, vertex);
                        prevNode = (*i);
                        lastVertex = vertex;
                        vertex = prevNode->GetAccInfo().FindAccVertexNotIn(vertsOnPath);
                        if (vertex != Vertex(-1))
                        {
                            // std::cout<<"adding acyclic edge "<<lastVertex<<" " <<vertex<<" -> finishing 2b"<<std::endl;
                            prevNode->GetAccInfo().UpdateAccIntersectionWithEdge(lastVertex, vertex);
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
                        prevNode->GetAccInfo().UpdateAccIntersectionWithEdge(lastVertex, firstVertex);
                    }
                }
            }
        }
    };
    
    struct Edge
    {
        Node    *nodeA;
        Node    *nodeB;
        Path    pathToA;
        Path    pathToB;
        Vertex  intersectionVertex;
        bool    isInSpanningTree;

        Edge(Node *na, Node *nb, Vertex iv)
        {
            nodeA = na;
            nodeB = nb;
            intersectionVertex = iv;
            isInSpanningTree = false;
        }

        void FindAccConnections()
        {
            nodeA->FindAccSubToBorderConnection(intersectionVertex, pathToA);
            nodeB->FindAccSubToBorderConnection(intersectionVertex, pathToB);
        }

        void UpdateAccConnections()
        {
            // najpierw uaktualnbiamy sciezke zbioru, ktory juz jest
            // dolaczony do drzewa
            // mamy wtedy pewnosc, ze jezeli pierwszy wierzcholek jest zaznaczony
            // jako acykliczny, to znaczy, ze zostal dodany do zbioru acyklicznego
            // juz wczesniej i mozna pod niego podpiac nowo dodawana czesc zbioru ac.

            // std::cout<<"intersection vertex: "<<intersectionVertex<<std::endl;
            if (nodeA->isConnectedToAccSub)
            {
                // Debug::Print(std::cout, pathToA);
                nodeA->UpdateAccSubToBorderConnection(intersectionVertex, pathToA);
                // Debug::Print(std::cout, pathToB);
                nodeB->UpdateAccSubToBorderConnection(intersectionVertex, pathToB);
            }
            else
            {
                // Debug::Print(std::cout, pathToB);
                nodeB->UpdateAccSubToBorderConnection(intersectionVertex, pathToB);
                // Debug::Print(std::cout, pathToA);
                nodeA->UpdateAccSubToBorderConnection(intersectionVertex, pathToA);
            }
        }
    };

    typedef std::vector<Node *> Nodes;
    typedef std::vector<Edge *> Edges;
    
    Nodes nodes;
    Edges edges;    

    AccSpanningTreeT(PartitionGraph *pg)
    {
        // budujemy graf, w ktorym wierzcholkami beda podzbiory acykliczne
        // (czyli de facto skladowe spojne poszczegolnych paczek), ktore potem
        // polaczymy drzewem rozpinajacym
        typename std::map<PartitionGraphNode *, std::vector<Node *> > dataNodeChildren;
        int currentID = 1;
        for (typename PartitionGraph::Nodes::iterator i = pg->nodes.begin(); i != pg->nodes.end(); i++)
        {
            IncidenceGraph *ig = (*i)->ig;
            typename std::vector<std::set<Vertex> >::iterator ccb = ig->connectedComponentsBorders.begin();
            std::vector<int>::iterator ccass = ig->connectedComponentsAccSubSize.begin();
            for (typename ConnectedComponents::iterator cc = ig->connectedComponents.begin(); cc != ig->connectedComponents.end(); cc++)
            {
                Node *newNode = new Node(*i, currentID++, *cc, *ccb, *ccass);
                dataNodeChildren[*i].push_back(newNode);
                nodes.push_back(newNode);
                ccb++;
                ccass++;
            }
        }

#ifdef ACCSUB_TRACE
        Timer::Update("creating acyclic tree nodes");
#endif

        // tworzymy krawedzie w grafie
        for (typename Nodes::iterator node = nodes.begin(); node != nodes.end(); node++)
        {
            PartitionGraphNode *parent = (*node)->parent;
            std::vector<Node *> potentialNeighbours;
            for (typename PartitionGraph::Edges::iterator edge = parent->edges.begin(); edge != parent->edges.end(); edge++)
            {
                PartitionGraphNode *neighbour = ((*edge)->nodeA == parent) ? (*edge)->nodeB : (*edge)->nodeA;
                potentialNeighbours.insert(potentialNeighbours.end(), dataNodeChildren[neighbour].begin(), dataNodeChildren[neighbour].end());
            }
            for (typename Nodes::iterator neighbour = potentialNeighbours.begin(); neighbour != potentialNeighbours.end(); neighbour++)
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

#ifdef ACCSUB_TRACE
        Timer::Update("creating acyclic tree edges");
#endif

        // tworzymy hasha, w ktorym dla kazdego ID poddrzewa bedziemy przechowywali
        // rozmiar podzbioru acyklicznego w nim zawartego (w poddrzewie)
        std::map<int, int> spanningTreeAccSubSize;
        for (typename Nodes::iterator node = nodes.begin(); node != nodes.end(); node++)
        {
            spanningTreeAccSubSize[(*node)->subtreeID] = (*node)->accSubSize;
        }

        // i na koncu drzewo rozpinajace
        for (typename Edges::iterator edge = edges.begin(); edge != edges.end(); edge++)
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
            spanningTreeAccSubSize[newID] = spanningTreeAccSubSize[newID] + spanningTreeAccSubSize[oldID];
            for (typename Nodes::iterator node = nodes.begin(); node != nodes.end(); node++)
            {
                if ((*node)->subtreeID == oldID)
                {
                    (*node)->subtreeID = newID;
                }
            }
        }

#ifdef ACCSUB_TRACE
        Timer::Update("creating spanning tree");
#endif

        // jezeli zostalo jakies poddrzewo, ktore nie zawiera podzbioru acyklicznego
        // to usuwamy wszystkie sympleksy z utworzonego grafu, dodajemy do wspolnej
        // listy, a nastepnie traktujemy jako jedna spojna skladowa i jeszcze raz
        // wysylamy do obliczen, ale tym razem nie laczymy juz brzegow, tylko po
        // prostu "przenosimy" nowo utworzony graf jako osobna spojna skladowa
        // duzego grafu
        typename std::map<int, SimplexPtrList> simplexPtrLists;
        typename Nodes::iterator node = nodes.begin();
        while (node != nodes.end())
        {
            if (spanningTreeAccSubSize[(*node)->subtreeID] == 0)
            {
                // usuwamy spojna skladowa i kopiujemy jej liste sympleksow
                (*node)->parent->ig->RemoveConnectedComponentAndCopySimplexList((*node)->connectedComponent, simplexPtrLists[(*node)->subtreeID]);
                // usuwamy krawedzie drzewa rozpinajacego laczace tego node'a
                typename Edges::iterator edge = edges.begin();
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
        for (typename std::map<int, SimplexPtrList>::iterator i = simplexPtrLists.begin(); i != simplexPtrLists.end(); i++)
        {
            pg->isolatedNodes.push_back(new PartitionGraphNode(i->second));
        }

#ifdef ACCSUB_TRACE
        Timer::Update("finding isolated data nodes");
#endif

        // na drzewie rozpinajacym
        // szukamny polaczen acyklicznych zbiorow, ale na razie ich nie laczymy
        // bedziemy robic to po polaczeniu wszystkich sympleksow, zeby dobrze
        // zaktualizowac podzbior acykliczny
        for (typename Edges::iterator i = edges.begin(); i != edges.end(); i++)
        {
            if ((*i)->isInSpanningTree) (*i)->FindAccConnections();
        }
        for (typename Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            (*i)->FindBoundaryVertsConnectingPaths();
        }

#ifdef ACCSUB_TRACE
        Timer::Update("searching paths from acyclic subsets to border");
#endif
    }

    ~AccSpanningTreeT()
    {
        for (typename Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            delete (*i);
        }
        for (typename Edges::iterator i = edges.begin(); i != edges.end(); i++)
        {
            delete (*i);
        }
    }
    
    void JoinAccSubsets()
    {
        // aktualizujemy zbior acykliczny o polaczenia pomiedzy podgrafami
        for (typename Edges::iterator i = edges.begin(); i != edges.end(); i++)
        {
            if ((*i)->isInSpanningTree) (*i)->UpdateAccConnections();
        }
#ifdef ACCSUB_TRACE
        Timer::Update("adding paths to acyclic subset");
#endif
    }
 
};

#endif /* ACCSPANNINGTREE_HPP */
