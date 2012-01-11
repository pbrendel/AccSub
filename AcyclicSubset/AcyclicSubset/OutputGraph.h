/*
 * File:   OutputGraph.h
 * Author: Piotr Brendel
 */

#ifndef OUTPUTGRAPH_H
#define OUTPUTGRAPH_H

template <typename IncidenceGraphType>
class OutputGraphT
{
public:

    typedef IncidenceGraphType IncidenceGraph;
    typedef typename IncidenceGraph::Vertex Vertex;
    typedef typename IncidenceGraph::Simplex Simplex;
    typedef typename IncidenceGraph::SimplexList SimplexList;
    typedef typename IncidenceGraph::IntersectionFlags IntersectionFlags;

    struct Node
    {        
        int                 index;
        Simplex             simplex;
        std::vector<Node *> subnodes;    
        std::vector<int>    kappa;
    
        Node(int index, const Simplex &s)
        {
            this->index = index;
            simplex = s;
        }
 
        void AddSubnode(Node *subnode, int kappa)
        {
            subnodes.push_back(subnode);
            this->kappa.push_back(kappa);
        }

        void GetSubnodes(std::vector<Node *> &nodes)
        {
            nodes.push_back(this);
            for (typename Nodes::iterator i = subnodes.begin(); i != subnodes.end(); i++)
            {
                (*i)->GetSubnodes(nodes);
            }
        }
        
        Node *FindNodeWithSimplex(const Simplex &s)
        {
            if (simplex == s)
            {
                return this;
            }
            for (typename Nodes::iterator i = subnodes.begin(); i != subnodes.end(); i++)
            {
                Node *n = (*i)->FindNodeWithSimplex(s);
                if (n != 0)
                {
                    return n;
                }
            }
            return 0;
        }

        static Node *FindNodeWithSimplex(std::vector<Node *> nodes, const Simplex &s)
        {
            for (typename Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
            {
                if ((*i)->simplex == s)
                {
                    return (*i);
                }
            }
            return 0;
        }

        static bool Sorter(const Node *a, const Node *b)
        {
            return (a->simplex.size() > b->simplex.size());
        }
    };

    typedef std::vector<Node *> Nodes;
    Nodes nodes;

    OutputGraphT(IncidenceGraph *ig)
    {
        nodeIndex = 0;
        incidenceGraph = ig;
        std::queue<typename IncidenceGraph::Node *> Q;
        for (typename IncidenceGraph::Nodes::iterator i = ig->nodes.begin(); i != ig->nodes.end(); i++)
        {
            // omijamy sympleksy nalezace do zbioru acyklicznego
            // albo te juz dodane do grafu
            if ((*i)->IsInAccSub() || (*i)->IsAddedToOutput())
            {
                continue;
            }
            Q.push(*i);
            (*i)->IsAddedToQueue(true);
            while (!Q.empty())
            {
                // bierzemy pierwszy na liscie wierzcholek
                typename IncidenceGraph::Node *currentNode = Q.front();
                Q.pop();
                // flagi wygenerowanych podsympleksow
                IntersectionFlags subnodesFlags = 0;
                // wygenerowane podsympleksy
                Nodes subnodes;
                for (typename IncidenceGraph::Edges::iterator edge = currentNode->edges.begin(); edge != currentNode->edges.end(); edge++)
                {
                    typename IncidenceGraph::Node *neighbour = (*edge)->GetNeighbour(currentNode);
                    // jezeli jest acykliczny to znaczy, ze przeciecie napewno bylo
                    // wyliczone wczesniej, wiec aktualizujemy flagi przeciecia acyklicznego
                    if (!neighbour->IsInAccSub())
                    {
                        // jezeli byl juz dodany do wyjscia
                        if (neighbour->IsAddedToOutput())
                        {
                            // jezeli przeciecie nie jest w zbiorze acyklicznym
                            IntersectionFlags intersectionFlags = (*edge)->intersection.GetFlags(currentNode);
                            if (!currentNode->GetAccInfo().IsInsideAccIntersection(intersectionFlags))
                            //if ((intersectionFlags & currentNode->GetAccInfo().GetAccIntersectionFlags()) != intersectionFlags)
                            {
                                Node *outputNode = ((Node *)neighbour->helpers.ptr)->FindNodeWithSimplex((*edge)->intersection.Get());
                                assert(outputNode != 0);
                                outputNode->GetSubnodes(subnodes);
                                subnodesFlags |= intersectionFlags;
                            }
                        }
                        // wpp. dodajemy sasiada do listy
                        else
                        {
                            if (!neighbour->IsAddedToQueue())
                            {
                                neighbour->IsAddedToQueue(true);
                                Q.push(neighbour);
                            }
                        }
                    }
                }
                currentNode->IsAddedToOutput(true);
                currentNode->helpers.ptr = GenerateNode(currentNode, *currentNode->simplex, subnodes, subnodesFlags);
            }
        }
    }

    OutputGraphT(const SimplexList &simplexList)
    {
        nodeIndex = 0;
        for (typename SimplexList::const_iterator i = simplexList.begin(); i != simplexList.end(); i++)
        {
            Node *node = AddNode(*i);
            if ((*i).size() > 1)
            {
                Simplex s = *i;
                int kappa = 1;
                for (typename Simplex::iterator j = s.begin(); j != s.end(); j++)
                {
                    Vertex vertex = (*j);
                    j = s.erase(j);
                    node->AddSubnode(FindOrAddNode(*i), kappa);
                    j = s.insert(j, vertex);
                    kappa = -kappa;
                }
            }
        }
    }

    ~OutputGraphT()
    {
        for (typename Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
        {
            delete (*i);
        }
        nodes.clear();
    }

private:

    int             nodeIndex;
    IncidenceGraph  *incidenceGraph;

    // za pomoca flag sprawdzamy, czy generowany sympleks nie jest w zbiorze acyklicznym
    // jezeli sympleks byl juz wczesniej dodany do wyjsciowego grafu, to szukamy go
    // baseNode - node z sympleksem maksymalnym, ktorego sciana jest aktualny sympleks
    // baseSimplex - sympleks dla ktorego aktualnie generujemy node
    // generatedSubnodes - aktualnie wygenerowane podsympleksy
    // subnodesFlags - flagi sympleksow juz wygenerowanych
    Node *GenerateNode(typename IncidenceGraph::Node *baseNode, Simplex &baseSimplex, Nodes &generatedSubnodes, IntersectionFlags &subnodesFlags)
    {
        IntersectionFlags flags = incidenceGraph->configurationsFlags[baseNode->Normalize(baseSimplex)];
        if (flags == 0)
        {
            baseSimplex = baseNode->Normalize(baseSimplex);
        }
        assert(flags != 0);

        Node *newNode = 0;

        // jezeli sciana nie jest w przecieciu ze zbiorem acyklicznym
        // ale juz ja wygenerowalismy, to szukamy jej w liscie wygenerowanych sympleksow
        // jezeli subnodes flags == 0 to zadna z podscian nie zostala jeszcze wygenerowana
        if (baseNode->GetAccInfo().IsDisjointWithAccIntersection(flags) && (subnodesFlags & flags) == flags)
        {
            return Node::FindNodeWithSimplex(generatedSubnodes, baseSimplex);
        }
        // jezeli sciana nie jest w przecieciu ze zbiorem acyklicznym
        // ani nie byla wczesniej dodana -> dodajemy
        else if (baseNode->GetAccInfo().IsDisjointWithAccIntersection(flags)/* && (subnodesFlags & flags) == 0 */)
        {
            assert(baseSimplex.size() != 0);
            newNode = AddNode(baseSimplex);
            generatedSubnodes.push_back(newNode);
            // zaznaczamy, ze sympleks jest juz wygenerowany
            // ustawiamy tylko flagi dla aktualnego sympleksu (bez podscian)
            subnodesFlags |= flags;
        }
        // calkowicie pokrywa sie ze zbiorem acyklicznym -> nie generujemy juz nic w dol
        else if (baseNode->GetAccInfo().IsInsideAccIntersection(flags))
        {
            return 0;
        }
        else /* if (!baseNode->GetAccInfo().IsDisjointWithAccIntersection(flags)) */
        {
            // nie mozemy tutaj wejsc, bo flags moze miec czesc wspolna ze zbiorem acyklicznym
            // tylko w jednym miejscu (czyli sama siebie)
            assert(false);
            return 0;
        }

        assert(newNode != 0);

        // jezeli mozemy zejsc w dol
        if (baseSimplex.size() > 1)
        {
            int kappa = 1;
            for (typename Simplex::iterator i = baseSimplex.begin(); i != baseSimplex.end(); i++)
            {
                Vertex vertex = (*i);
                i = baseSimplex.erase(i);
                Node *subnode = GenerateNode(baseNode, baseSimplex, generatedSubnodes, subnodesFlags);
                if (subnode != 0)
                {
                    newNode->AddSubnode(subnode, kappa);
                }
                i = baseSimplex.insert(i, vertex);
                kappa = -kappa;
            }
        }

        return newNode;
    }

    Node *AddNode(const Simplex &s)
    {
        Node *n = new Node(nodeIndex++, s);
        nodes.push_back(n);
        return n;
    }

    Node *FindOrAddNode(const Simplex &simplex)
    {
        Node *n = Node::FindNodeWithSimplex(nodes, simplex);
        if (n != 0)
        {
            return n;
        }
        Node *node = AddNode(simplex);
        if (simplex.size() > 1)
        {
            Simplex s = simplex;
            int kappa = 1;
            for (typename Simplex::iterator j = s.begin(); j != s.end(); j++)
            {
                Vertex vertex = (*j);
                j = s.erase(j);
                node->AddSubnode(FindOrAddNode(s), kappa);
                j = s.insert(j, vertex);
                kappa = -kappa;
            }
        }
        return node;
    }
};

#endif /* OUTPUTGRAPH_H */
