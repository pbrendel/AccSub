/*
 * File:   IncidenceGraph.hpp
 * Author: Piotr Brendel
 */

#ifndef INCIDENCEGRAPH_HPP
#define INCIDENCEGRAPH_HPP

#include "IncidenceGraphAlgorithms.hpp"
#include "ConfigurationsFlags.hpp"
#include "AccTest.hpp"

#include <list>
#include <queue>
#include <set>

////////////////////////////////////////////////////////////////////////////////

template <typename Traits, template<typename> class IntersectionInfoT, template<typename> class AccInfoT>
class IncidenceGraphT
{
public:

    typedef typename Traits::Vertex  Vertex;
    typedef typename Traits::VertsSet VertsSet;
    typedef typename Traits::Simplex Simplex;
    typedef typename Traits::SimplexList SimplexList;
    typedef typename Traits::SimplexPtr SimplexPtr;
    typedef typename Traits::SimplexPtrList SimplexPtrList;
    typedef typename Traits::IntersectionFlags IntersectionFlags;
    typedef IntersectionInfoT<IncidenceGraphT> IntersectionInfo;
    typedef AccInfoT<IncidenceGraphT> AccInfo;
    typedef AccTestT<Traits> AccTest;
    typedef typename Traits::AccSubAlgorithm AccSubAlgorithm;

    struct Node;
    struct Edge;

    typedef std::vector<Node *> Nodes;
    typedef std::vector<Edge *> Edges;
    //typedef std::list<Edge *> Edges;
    typedef std::list<Node *> Path;
    typedef Node *ConnectedComponent;
    typedef std::vector<ConnectedComponent> ConnectedComponents;
    typedef std::map<Vertex, Nodes> VertexHash;

    struct Node
    {
        typedef unsigned short int PropertiesFlags;

        enum  // properties flags
        {
            IGNPF_IN_ACC_SUB                = 0x0001,
            IGNPF_ADDED_TO_QUEUE            = 0x0002,
            IGNPF_ADDED_TO_GRAPH            = 0x0004,
            IGNPF_ADDED_TO_OUTPUT           = 0x0008,
            IGNPF_ON_BORDER                 = 0x0010,
            IGNPF_HELPER_FLAG_1             = 0x1000,
            IGNPF_HELPER_FLAG_2             = 0x2000,
            IGNPF_HELPER_FLAG_3             = 0x4000,
            IGNPF_HELPER_FLAG_4             = 0x8000,
        };
        
#define GET_SET(FUNC, FLAG) inline bool Is##FUNC() const { return (propertiesFlags & IGNPF_##FLAG) == IGNPF_##FLAG; } \
                            inline void Is##FUNC(bool f) { if (f) propertiesFlags |= IGNPF_##FLAG; else propertiesFlags &= ~(IGNPF_##FLAG); }

        GET_SET(InAccSub, IN_ACC_SUB)
        GET_SET(AddedToQueue, ADDED_TO_QUEUE)
        GET_SET(AddedToGraph, ADDED_TO_GRAPH)
        GET_SET(AddedToOutput, ADDED_TO_OUTPUT)
        GET_SET(OnBorder, ON_BORDER)
        GET_SET(HelperFlag1, HELPER_FLAG_1)
        GET_SET(HelperFlag2, HELPER_FLAG_2)
        GET_SET(HelperFlag3, HELPER_FLAG_3)
        GET_SET(HelperFlag4, HELPER_FLAG_4)

#undef GET_SET

        Node(IncidenceGraphT *graph, Simplex *simplex, int index) : accInfo(this)
        {
            this->graph = graph;
            this->simplex = simplex;
            this->index = index;
            this->propertiesFlags = 0;

            for (int i = 0; i < simplex->size(); i++)
            {
                v2i[(*simplex)[i]] = i;
            }
        }

        void AddEdge(Edge *edge)
        {
            // mozemy w ramach debugowania sprawdzic, czy krawedz nie zostala juz dodana
            // w rzeczywistosci taka sytuacja nie moze miec miejsca (sprawdzamy to
            // przed wywolaniem AddNeighbour)
            edges.push_back(edge);
        }

        bool HasNeighbour(Node *neighbour)
        {
            for (typename Edges::iterator i = edges.begin(); i != edges.end(); i++)
            {
                if ((*i)->GetNeighbour(this) == neighbour) return true;
            }
            return false;
        }

        void RemoveNeighbour(Node *neighbour)
        {
            for (typename Edges::iterator i = edges.begin(); i != edges.end(); i++)
            {
                if ((*i)->GetNeighbour(this) == neighbour)
                {
                    edges.erase(i);
                    return;
                }
            }
        }

        Simplex Normalize(const Simplex &simplex)
        {
            Simplex s = Simplex::WithSize(simplex.size());
            int index = 0;
            for (typename Simplex::const_iterator i = simplex.begin(); i != simplex.end(); i++)
            {
                s[index++] = v2i[(*i)];
            }
            return s;
        }

        int NormalizeVertex(Vertex v)
        {
            return v2i[v];    
        }

        IntersectionFlags GetNormalizedIntersectionFlags(const Simplex &intersection)
        {
            return graph->subconfigurationsFlags[Normalize(intersection)];
        }

        void SetParentGraph(IncidenceGraphT *g) { graph = g; }
        PropertiesFlags GetPropertiesFlags() { return propertiesFlags; }
        AccInfo &GetAccInfo() { return accInfo; }

        IntersectionFlags GetConfigurationsFlags(const Simplex &s) { return graph->configurationsFlags[s]; }
        IntersectionFlags GetSubconfigurationsFlags(const Simplex &s) { return graph->subconfigurationsFlags[s]; }

        bool operator==(const Node &node)
        {
            return (this->simplex == node.simplex);
        }

        Simplex             *simplex;
        Edges               edges;
        int                 index;

        union
        {
            void *ptr;
            int  i;
        } helpers;

    private:

        IncidenceGraphT         *graph;
        PropertiesFlags         propertiesFlags;
        AccInfo                 accInfo;
        std::map<Vertex, int>   v2i;    // vertex to index map

    };

    struct Edge
    {
        Node                *nodeA;
        Node                *nodeB;
        IntersectionInfo    intersection;

        Edge(Node *na, Node *nb) : intersection(this)
        {
            nodeA = na;
            nodeB = nb;
        }

        Node *GetNeighbour(Node *node)
        {
            return (node == nodeA) ? nodeB : nodeA;
        }

        bool Contains(Node *node)
        {
            return (node == nodeA || node == nodeB);
        }
    };

    ConfigurationsFlags<Simplex, IntersectionFlags> configurationsFlags;
    ConfigurationsFlags<Simplex, IntersectionFlags> subconfigurationsFlags;

    int                 dim;
    Nodes               nodes;
    Edges               edges;
    ConnectedComponents connectedComponents;
    VertsSet            borderVerts;
    std::vector<VertsSet> connectedComponentsBorders;
    std::vector<int>    connectedComponentsAccSubSize;

    IncidenceGraphT(int dim)
    {
        this->dim = dim;
        configurationsFlags.Create(dim, false, true);
        subconfigurationsFlags.Create(dim, true, true);
    }

    IncidenceGraphT(SimplexList &simplexList)
    {
        // najpierw tworzymy wszystkie node'y
        int index = 0;
        for (typename SimplexList::iterator i = simplexList.begin(); i != simplexList.end(); i++)
        {
            nodes.push_back(new Node(this, &(*i), index++));
        }
        dim = Simplex::GetSimplexListDimension(simplexList);
        configurationsFlags.Create(dim, false, true);
        subconfigurationsFlags.Create(dim, true, true);
     }

    IncidenceGraphT(SimplexPtrList &simplexPtrList)
    {
        // najpierw tworzymy wszystkie node'y
        int index = 0;
        for (typename SimplexPtrList::iterator i = simplexPtrList.begin(); i != simplexPtrList.end(); i++)
        {
            nodes.push_back(new Node(this, (*i), index++));
        }
        dim = Simplex::GetSimplexListDimension(simplexPtrList);
        configurationsFlags.Create(dim, false, true);
        subconfigurationsFlags.Create(dim, true, true);
    }

    ~IncidenceGraphT()
    {
        for (typename Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            delete (*i);
        }
        for (typename Edges::iterator i = edges.begin(); i != edges.end(); i++)
        {
            delete (*i);
        }
        nodes.clear();
    }

    void CreateGraph()
    {
        VertexHash H;
        CreateVertexHash(H);
        std::queue<Node *> Q;

        for (typename Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
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
                for (typename Simplex::iterator vertex = currentNode->simplex->begin(); vertex != currentNode->simplex->end(); vertex++)
                {
                    // dla kazdego punktu pobieramy liste wierzcholkow do ktorej nalezy
                    Nodes nodes = H[*vertex];
                    for (typename Nodes::iterator neighbour = nodes.begin(); neighbour != nodes.end(); neighbour++)
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

    void CreateGraphWithBorder()
    {
        VertexHash H;
        CreateVertexHash(H);

        for (typename VertsSet::iterator v = borderVerts.begin(); v != borderVerts.end(); v++)
        {
            Nodes nodes = H[*v];
            for (typename Nodes::iterator node = nodes.begin(); node != nodes.end(); node++)
            {
                (*node)->IsOnBorder(true);
            }
        }

        std::vector<Vertex> vectorBorderVerts;
        vectorBorderVerts.assign(borderVerts.begin(), borderVerts.end());
        std::sort(vectorBorderVerts.begin(), vectorBorderVerts.end());

        std::queue<Node *> Q;
        for (typename Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
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
                for (typename Simplex::iterator vertex = currentNode->simplex->begin(); vertex != currentNode->simplex->end(); vertex++)
                {
                    // dla kazdego punktu pobieramy liste wierzcholkow do ktorej nalezy
                    Nodes nodes = H[*vertex];
                    for (typename Nodes::iterator neighbour = nodes.begin(); neighbour != nodes.end(); neighbour++)
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

    void CalculateAccSub(AccTest *accTest)
    {
        std::queue<Node *> Q;
        for (typename std::vector<ConnectedComponent>::iterator i = connectedComponents.begin(); i != connectedComponents.end(); i++)
        {
            // pierwszy wierzcholek w skladowej zaznaczamy jako acykliczny
            (*i)->IsInAccSub(true);
            (*i)->GetAccInfo().UpdateNeighboursAccIntersection();
            Q.push(*i);
            while (!Q.empty())
            {
                Node *currentNode = Q.front();
                Q.pop();

                for (typename Edges::iterator j = currentNode->edges.begin(); j != currentNode->edges.end(); j++)
                {
                    Node *neighbour = (*j)->GetNeighbour(currentNode);
                    if (neighbour->IsInAccSub())
                    {
                        continue;
                    }
                    if (neighbour->GetAccInfo().IsAccIntersectionAcyclic(accTest))
                    {
                        neighbour->IsInAccSub(true);
                        neighbour->GetAccInfo().UpdateNeighboursAccIntersection();
                        Q.push(neighbour);
                    }
                }
            }
        }
    }

    void CalculateAccSubWithBorder(AccTest *accTest)
    {
        std::queue<Node *> Q;
        for (typename std::vector<ConnectedComponent>::iterator i = connectedComponents.begin(); i != connectedComponents.end(); i++)
        {
            Node *first = FindNode(*i, FindNodeNotOnBorder<IncidenceGraphT>());
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

                for (typename Edges::iterator j = currentNode->edges.begin(); j != currentNode->edges.end(); j++)
                {
                    Node *neighbour = (*j)->GetNeighbour(currentNode);
                    if (neighbour->IsInAccSub() || neighbour->IsOnBorder())
                    {
                        continue;
                    }
                    if (neighbour->GetAccInfo().IsAccIntersectionAcyclic(accTest))
                    {
                        neighbour->IsInAccSub(true);
                        neighbour->GetAccInfo().UpdateNeighboursAccIntersection();
                        Q.push(neighbour);
                    }
                }
            }
        }
    }

    void CalculateAccSubSpanningTree(AccTest *accTest)
    {
        int index = 0;
        std::queue<Node *> Q;
        for (typename std::vector<ConnectedComponent>::iterator i = connectedComponents.begin(); i != connectedComponents.end(); i++)
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
                    for (typename Edges::iterator j = currentNode->edges.begin(); j != currentNode->edges.end(); j++)
                    {
                        Node *neighbour = (*j)->GetNeighbour(currentNode);
                        if (neighbour->IsInAccSub())
                        {
                            continue;
                        }
                        if (neighbour->GetAccInfo().IsAccIntersectionAcyclic(accTest))
                        {
                            size++;
                            neighbour->IsInAccSub(true);
                            neighbour->GetAccInfo().SetAccSubID(accSubID);
                            neighbour->GetAccInfo().UpdateNeighboursAccIntersection();
                            Q.push(neighbour);
                        }
                    }
                }
                Path path = FindPath(first, FindPathToNodeNotInAccSub<IncidenceGraphT>());
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

    void CalculateAccSubSpanningTreeWithBorder(AccTest *accTest)
    {
        std::queue<Node *> Q;
        for (typename std::vector<ConnectedComponent>::iterator i = connectedComponents.begin(); i != connectedComponents.end(); i++)
        {
            int accSubID = 0;
            Nodes firstNodes;
            std::vector<Path> paths;
            Node *first = FindNode(*i, FindNodeNotOnBorder<IncidenceGraphT>());
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
                    for (typename Edges::iterator j = currentNode->edges.begin(); j != currentNode->edges.end(); j++)
                    {
                        Node *neighbour = (*j)->GetNeighbour(currentNode);
                        if (neighbour->IsInAccSub() || neighbour->IsOnBorder())
                        {
                            continue;
                        }
                        if (neighbour->GetAccInfo().IsAccIntersectionAcyclic(accTest))
                        {
                            size++;
                            neighbour->IsInAccSub(true);
                            neighbour->GetAccInfo().SetAccSubID(accSubID);
                            neighbour->GetAccInfo().UpdateNeighboursAccIntersection();
                            Q.push(neighbour);
                        }
                    }
                }
                Path path = FindPath(first, FindPathToNodeNotInAccSubNorOnBorder<IncidenceGraphT>());
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

private:

    void CreateAccSpanningTree(std::vector<Path> &paths, int maxAccSubID)
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
            for (typename std::vector<Path>::iterator p = paths.begin(); p != paths.end(); p++)
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
            typename Path::reverse_iterator current = path.rbegin();
            typename Path::reverse_iterator next = current;
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
    
public:

    void CreateGraphAndCalculateAccSub(AccTest *accTest)
    {
        VertexHash H;
        CreateVertexHash(H);
        std::queue<Node *> Q;
        for (typename Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
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

                if (currentNode->GetAccInfo().IsAccIntersectionAcyclic(accTest))
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

    void CreateGraphAndCalculateAccSubWithBorder(AccTest *accTest)
    {
        VertexHash H;
        CreateVertexHash(H);

        for (typename VertsSet::iterator v = borderVerts.begin(); v != borderVerts.end(); v++)
        {
            Nodes nodes = H[*v];
            for (typename Nodes::iterator node = nodes.begin(); node != nodes.end(); node++)
            {
                (*node)->IsOnBorder(true);
            }
        }

        std::vector<Vertex> vectorBorderVerts;
        vectorBorderVerts.assign(borderVerts.begin(), borderVerts.end());
        std::sort(vectorBorderVerts.begin(), vectorBorderVerts.end());

        std::queue<Node *> Q;
        for (typename Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
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
                else if (currentNode->GetAccInfo().IsAccIntersectionAcyclic(accTest))
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

private:

    void EnqNeighboursAndUpdateAccIntersection(Node *node, VertexHash &H, std::queue<Node *> &Q)
    {
        Simplex intersection;
        // sprawdzamy wszytkie punkty nalezace do sympleksu w tym wierzcholku
        for (typename Simplex::iterator vertex = node->simplex->begin(); vertex != node->simplex->end(); vertex++)
        {
            // dla kazdego punktu pobieramy liste wierzcholkow do ktorej nalezy
            Nodes nodes = H[*vertex];
            for (typename Nodes::iterator neighbour = nodes.begin(); neighbour != nodes.end(); neighbour++)
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

    void AddToGraphAndEnqNeighbours(Node *node, VertexHash &H, std::queue<Node *> &Q)
    {
        node->IsAddedToGraph(true);
        // sprawdzamy wszytkie punkty nalezace do sympleksu w tym wierzcholku
        for (typename Simplex::iterator vertex = node->simplex->begin(); vertex != node->simplex->end(); vertex++)
        {
            // dla kazdego punktu pobieramy liste wierzcholkow do ktorej nalezy
            Nodes nodes = H[*vertex];
            for (typename Nodes::iterator neighbour = nodes.begin(); neighbour != nodes.end(); neighbour++)
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

    void RemoveEdgesWithAccSub()
    {
        Edges newEdges;
        for (typename Edges::iterator i = edges.begin(); i != edges.end(); i++)
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
    
public:

    void UpdateConnectedComponents()
    {
        typename ConnectedComponents::iterator cc = connectedComponents.begin();
        typename std::vector<std::set<Vertex> >::iterator ccb = connectedComponentsBorders.begin();
        typename std::vector<int>::iterator ccass = connectedComponentsAccSubSize.begin();
        while (cc != connectedComponents.end())
        {
            if ((*cc)->IsInAccSub())
            {
                Node *node = FindNode(*cc, FindNodeNotInAccSub<IncidenceGraphT>());
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

    void RemoveAccSub()
    {
        for (typename Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            if ((*i)->IsInAccSub())
            {
                continue;
            }
            Edges newEdges;
            for (typename Edges::iterator edge = (*i)->edges.begin(); edge != (*i)->edges.end(); edge++)
            {
                if ((*edge)->GetNeighbour(*i)->IsInAccSub())
                {
                    continue;
                }
                newEdges.push_back(*edge);
            }
            (*i)->edges = newEdges;
        }
        RemoveNodesWithPredicate(this, RemoveNodesWithFlags<IncidenceGraphT>(Node::IGNPF_IN_ACC_SUB));
    }

    void AssignNewIndices(bool checkAcyclicity)
    {
        int index = 0;
        if (checkAcyclicity)
        {
            for (typename Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
            {
                if ((*i)->IsInAccSub())
                {
                    continue;
                }
                (*i)->helpers.i = (*i)->index;
                (*i)->index = index++;
            }
        }
        else
        {
            for (typename Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
            {
                (*i)->helpers.i = (*i)->index;
                (*i)->index = index++;
            }
        }
    }

    void RemoveConnectedComponentAndCopySimplexList(ConnectedComponent cc, SimplexPtrList &simplexPtrList)
    {
        // przelatujemy cala spojna skladowa i zaznaczamy elementy do niej nalezace
        // dodatkowo dodajemy do listy sympleksy z tej spojnej skladowej
        std::queue<Node *> Q;
        Q.push(cc);
        cc->IsHelperFlag2(true);
        simplexPtrList.push_back(cc->simplex);
        while (!Q.empty())
        {
            Node *node = Q.front();
            Q.pop();
            for (typename Edges::iterator edge = node->edges.begin(); edge != node->edges.end(); edge++)
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
        RemoveNodesWithPredicate(this, RemoveNodesWithFlags<IncidenceGraphT>(Node::IGNPF_HELPER_FLAG_2));
    }
    
    void GetAccSub(SimplexList &simplexList)
    {
        for (typename Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            if ((*i)->IsInAccSub())
            {
                simplexList.push_back(*(*i)->simplex);
            }
        }
    }

    int GetAccSubSize()
    {
        int size = 0;
        for (typename Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            if ((*i)->IsInAccSub())
            {
                size++;
            }
        }
        return size;
    }

private:

    void CreateVertexHash(VertexHash &H)
    {
        for (typename Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            Simplex *s = (*i)->simplex;
            for (typename Simplex::iterator j = s->begin(); j != s->end(); j++)
            {
                H[*j].push_back(*i);
            }
        }
    }
    
};

////////////////////////////////////////////////////////////////////////////////

#endif /* INCIDENCEGRAPH_HPP */
