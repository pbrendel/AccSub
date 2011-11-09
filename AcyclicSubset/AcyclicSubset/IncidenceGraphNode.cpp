#include "IncidenceGraph.h"
#include "OutputGraph.h"
#include "../Helpers/Utils.h"

#include <map>
#include <algorithm>

////////////////////////////////////////////////////////////////////////////////

IncidenceGraph::Node::Node(IncidenceGraph *graph, Simplex *simplex, int index)
{
    this->graph = graph;
    this->simplex = simplex;
    this->index = newIndex = index;
    this->propertiesFlags = 0;
    this->acyclicIntersectionFlags = 0;
    this->acyclicIntersectionFlagsMaximalFaces = 0;
    this->acyclicSubsetID = 0;

    for (int i = 0; i < simplex->size(); i++)
    {
        v2i[(*simplex)[i]] = i;
    }
}

////////////////////////////////////////////////////////////////////////////////

void IncidenceGraph::Node::AddNeighbour(IncidenceGraph::Node *neighbour)
{
    // mozemy w ramach debugowania sprawdzic, czy krawedz nie zostala juz dodana
    // w rzeczywistosci taka sytuacja nie moze miec miejsca (sprawdzamy to
    // przed wywolaniem AddNeighbour)
    edges.push_back(Edge(neighbour));
}

bool IncidenceGraph::Node::HasNeighbour(IncidenceGraph::Node *neighbour)
{
    for (Edges::iterator i = edges.begin(); i != edges.end(); i++)
    {
        if (i->node == neighbour) return true;
    }
    return false;
}

void IncidenceGraph::Node::RemoveNeighbour(IncidenceGraph::Node *neighbour)
{
    for (Edges::iterator i = edges.begin(); i != edges.end(); i++)
    {
        if (i->node == neighbour)
        {
            edges.erase(i);
            return;
        }
    }
}

void IncidenceGraph::Node::SetIntersection(IncidenceGraph::Node *neighbour, const Simplex &intersection)
{
    for (Edges::iterator i = edges.begin(); i != edges.end(); i++)
    {
        if (i->node == neighbour)
        {
            i->intersection = intersection;
            i->intersectionFlags = graph->subconfigurationsFlags[Normalize(intersection)];
            return;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

bool IncidenceGraph::Node::HasAcyclicIntersection(AcyclicTest<IntersectionFlags>* test)
{
    test->IsAcyclic(*simplex, acyclicIntersectionFlags, acyclicIntersectionFlagsMaximalFaces);
}

Vertex IncidenceGraph::Node::FindAcyclicVertex()
{
    if (this->acyclicIntersectionFlags == 0)
    {
        return Vertex(-1);
    }
    for (Simplex::iterator v = simplex->begin(); v != simplex->end(); v++)
    {
        if (acyclicIntersectionFlags & (1 << NormalizeVertex(*v)))
        {
            return *v;
        }
    }
    return Vertex(-1);
}

Vertex IncidenceGraph::Node::FindAcyclicVertexNotEqual(Vertex vertex)
{
    if (this->acyclicIntersectionFlags == 0)
    {
        return Vertex(-1);
    }
    for (Simplex::iterator v = simplex->begin(); v != simplex->end(); v++)
    {
        if (*v == vertex)
        {
            continue;
        }
        if (acyclicIntersectionFlags & (1 << NormalizeVertex(*v)))
        {
            return *v;
        }
    }
    return Vertex(-1);
}

Vertex IncidenceGraph::Node::FindAcyclicVertexNotIn(const VertsSet &vertsSet)
{
    if (this->acyclicIntersectionFlags == 0)
    {
        return Vertex(-1);
    }
    for (Simplex::iterator v = simplex->begin(); v != simplex->end(); v++)
    {
        if (std::find(vertsSet.begin(), vertsSet.end(), *v) != vertsSet.end())
        {
            continue;
        }
        if (acyclicIntersectionFlags & (1 << NormalizeVertex(*v)))
        {
            return *v;
        }
    }
    return Vertex(-1);
}

void IncidenceGraph::Node::UpdateAcyclicIntersectionWithVertex(Vertex v)
{
    Simplex s(1);
    s[0] = v;
    for (Edges::iterator edge = edges.begin(); edge != edges.end(); edge++)
    {
        if (edge->node->IsAcyclic())
        {
            continue;
        }
        if (!edge->IntersectionCalculated())
        {
            graph->CalculateNodesIntersection(this, edge->node, *edge);
        }
        if (ContainsVertex(&edge->intersection, v))
        {
            Simplex s1 = edge->node->Normalize(s);
            edge->node->UpdateAcyclicIntersectionFlags(graph->subconfigurationsFlags[s1], graph->configurationsFlags[s1]);
        }
    }
    acyclicIntersectionFlags |= graph->subconfigurationsFlags[Normalize(s)];
}

void IncidenceGraph::Node::UpdateAcyclicIntersectionWithEdge(Vertex v1, Vertex v2)
{
    assert (v1 != v2);

    Simplex s(2);
    if (v1 < v2)
    {
        s[0] = v1;
        s[1] = v2;
    }
    else
    {
        s[0] = v2;
        s[1] = v1;
    }
    for (Edges::iterator edge = edges.begin(); edge != edges.end(); edge++)
    {
        if (edge->node->IsAcyclic())
        {
            continue;
        }
        if (!edge->IntersectionCalculated())
        {
            graph->CalculateNodesIntersection(this, edge->node, *edge);
        }
        // obliczamy czesc wspolna (przeciecie) krawedzi i przeciecia z sasiadem
        Simplex s1;
        GetIntersection(&edge->intersection, &s, s1);
        // jezeli przeciecie to jest niepuste (maja wspolne punkty)
        // to aktualizujemy jego acyclic flags
        if (s1.size() > 0)
        {
            s1 = edge->node->Normalize(s1);
            edge->node->UpdateAcyclicIntersectionFlags(graph->subconfigurationsFlags[s1], graph->configurationsFlags[s1]);
        }
    }
    acyclicIntersectionFlags |= graph->subconfigurationsFlags[Normalize(s)];
}

void IncidenceGraph::Node::UpdateAcyclicIntersectionFlags(IntersectionFlags flags, IntersectionFlags flagsMaximalFaces)
{
    // jezeli flagi juz sa ustawione to nic nie robimy
    if ((acyclicIntersectionFlags & flags) == flags)
    {
        return;
    }
    acyclicIntersectionFlags |= flags;
    acyclicIntersectionFlagsMaximalFaces |= flagsMaximalFaces;
    IntersectionFlags flagsSubfaces = flags & (~flagsMaximalFaces);
    acyclicIntersectionFlagsMaximalFaces &= (~flagsSubfaces);
}

////////////////////////////////////////////////////////////////////////////////

void IncidenceGraph::Node::UpdateNeighboursAcyclicIntersection()
{
    Simplex intersection;
    for (Edges::iterator edge = edges.begin(); edge != edges.end(); edge++)
    {
        Node *neighbour = edge->node;
        if (neighbour->IsAcyclic())
        {
            continue;
        }
        GetIntersection(this->simplex, neighbour->simplex, intersection);
        intersection = neighbour->Normalize(intersection);
        neighbour->UpdateAcyclicIntersectionFlags(graph->subconfigurationsFlags[intersection], graph->configurationsFlags[intersection]);
    }
}

////////////////////////////////////////////////////////////////////////////////

Simplex IncidenceGraph::Node::Normalize(const Simplex &simplex)
{
    Simplex s(simplex.size());
    int index = 0;
    for (Simplex::const_iterator i = simplex.begin(); i != simplex.end(); i++)
    {
        s[index++] = v2i[(*i)];
    }
    return s;
}

int IncidenceGraph::Node::NormalizeVertex(Vertex v)
{
    return v2i[v];
}

bool IncidenceGraph::Node::operator==(const IncidenceGraph::Node &node)
{
    return (this->simplex == node.simplex);
}

////////////////////////////////////////////////////////////////////////////////
// eof
