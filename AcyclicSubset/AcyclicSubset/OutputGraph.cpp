/*
 * File:   OutputGraph.cpp
 * Author: Piotr Brendel
 */

#include "OutputGraph.h"
#include "IncidenceGraph.h"

#ifdef ACCSUB_TRACE
#include "../Helpers/Utils.h"
#endif

#include <queue>

////////////////////////////////////////////////////////////////////////////////
// OutputGraph::Node
////////////////////////////////////////////////////////////////////////////////

OutputGraph::Node::Node(int index, const Simplex &s)
{
    this->index = index;
    simplex = s;
}

//////////////////////////////7//////////////////////////////////////////////////

void OutputGraph::Node::AddSubnode(OutputGraph::Node *subnode, int kappa)
{
    subnodes.push_back(subnode);
    this->kappa.push_back(kappa);
}


void OutputGraph::Node::GetSubnodes(std::vector<OutputGraph::Node *> &nodes)
{
    nodes.push_back(this);
    for (Nodes::iterator i = subnodes.begin(); i != subnodes.end(); i++)
    {
        (*i)->GetSubnodes(nodes);
    }
}

////////////////////////////////////////////////////////////////////////////////

OutputGraph::Node *OutputGraph::Node::FindNodeWithSimplex(const Simplex &s)
{
    if (simplex == s)
    {
        return this;
    }
    for (Nodes::iterator i = subnodes.begin(); i != subnodes.end(); i++)
    {
        Node *n = (*i)->FindNodeWithSimplex(s);
        if (n != 0)
        {
            return n;
        } 
    }       
    return 0;
}


OutputGraph::Node *OutputGraph::Node::FindNodeWithSimplex(std::vector<OutputGraph::Node *> nodes, const Simplex &s)
{
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        if ((*i)->simplex == s)
        {
            return (*i);
        }        
    }   
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

bool OutputGraph::Node::Sorter(const OutputGraph::Node *a, const OutputGraph::Node *b)
{
    return (a->simplex.size() > b->simplex.size());
}

////////////////////////////////////////////////////////////////////////////////
// OutputGraph
////////////////////////////////////////////////////////////////////////////////

OutputGraph::OutputGraph(IncidenceGraph* ig)
{
    nodeIndex = 0;
    incidenceGraph = ig;
    std::queue<IncidenceGraph::Node *> Q;
    for (IncidenceGraph::Nodes::iterator i = ig->nodes.begin(); i != ig->nodes.end(); i++)
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
            IncidenceGraph::Node *currentNode = Q.front();
            Q.pop();
            // flagi wygenerowanych podsympleksow
            IncidenceGraph::IntersectionFlags subnodesFlags = 0;
            // wygenerowane podsympleksy
            Nodes subnodes;
            for (IncidenceGraph::Edges::iterator edge = currentNode->edges.begin(); edge != currentNode->edges.end(); edge++)
            {
                IncidenceGraph::Node *neighbour = (*edge)->GetNeighbour(currentNode);
                // jezeli jest acykliczny to znaczy, ze przeciecie napewno bylo
                // wyliczone wczesniej, wiec aktualizujemy flagi przeciecia acyklicznego
                if (!neighbour->IsInAccSub())
                {
                    if (!(*edge)->IntersectionCalculated())
                    {
                        (*edge)->CalculateIntersection();
                    }
                    // jezeli byl juz dodany do wyjscia
                    if (neighbour->IsAddedToOutput())
                    {
                        // jezeli przeciecie nie jest w zbiorze acyklicznym
                        IncidenceGraph::IntersectionFlags intersectionFlags = (*edge)->GetIntersectionFlags(currentNode);
                        if (!currentNode->GetAccInfo().IsInsideAccIntersection(intersectionFlags))
                        //if ((intersectionFlags & currentNode->GetAccInfo().GetAccIntersectionFlags()) != intersectionFlags)
                        {
                            Node *outputNode = ((Node *)neighbour->outputData)->FindNodeWithSimplex((*edge)->intersection);
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
            currentNode->outputData = GenerateNode(currentNode, *currentNode->simplex, subnodes, subnodesFlags);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

OutputGraph::OutputGraph(const SimplexList &simplexList)
{
    nodeIndex = 0;
    for (SimplexList::const_iterator i = simplexList.begin(); i != simplexList.end(); i++)
    {
        Node *node = AddNode(*i);
        if ((*i).size() > 1)
        {
            Simplex s = *i;
            int kappa = 1;
            for (Simplex::iterator j = s.begin(); j != s.end(); j++)
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

////////////////////////////////////////////////////////////////////////////////

OutputGraph::~OutputGraph()
{
    for (Nodes::iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        delete (*i);
    }
    nodes.clear();
}

////////////////////////////////////////////////////////////////////////////////
// GenerateNode
// za pomoca flag sprawdzamy, czy generowany sympleks nie jest w zbiorze acyklicznym
// jezeli sympleks byl juz wczesniej dodany do wyjsciowego grafu, to szukamy go
// baseNode - node z sympleksem maksymalnym, ktorego sciana jest aktualny sympleks
// baseSimplex - sympleks dla ktorego aktualnie generujemy node
// generatedSubnodes - aktualnie wygenerowane podsympleksy
// subnodesFlags - flagi sympleksow juz wygenerowanych

OutputGraph::Node *OutputGraph::GenerateNode(IncidenceGraph::Node *baseNode, Simplex &baseSimplex, Nodes &generatedSubnodes, IncidenceGraph::IntersectionFlags &subnodesFlags)
{
    IncidenceGraph::IntersectionFlags flags = incidenceGraph->configurationsFlags[baseNode->Normalize(baseSimplex)];
    if (flags == 0)
    {
        baseSimplex = baseNode->Normalize(baseSimplex);
    }
    assert(flags != 0);

    OutputGraph::Node *newNode = 0;

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
        for (Simplex::iterator i = baseSimplex.begin(); i != baseSimplex.end(); i++)
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

////////////////////////////////////////////////////////////////////////////////

OutputGraph::Node *OutputGraph::AddNode(const Simplex &s)
{
    Node *n = new Node(nodeIndex++, s);
    nodes.push_back(n);        
    return n;
}

OutputGraph::Node *OutputGraph::FindOrAddNode(const Simplex &simplex)
{
    Node *o = Node::FindNodeWithSimplex(nodes, simplex);
    if (o != 0)
    {
        return o;
    }

    Node *node = AddNode(simplex);
    if (simplex.size() > 1)
    {
        Simplex s = simplex;
        int kappa = 1;
        for (Simplex::iterator j = s.begin(); j != s.end(); j++)
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

////////////////////////////////////////////////////////////////////////////////
// eof
