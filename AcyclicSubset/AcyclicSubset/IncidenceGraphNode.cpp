/*
 * File:   IncidenceGraphNode.cpp
 * Author: Piotr Brendel
 */

#include "IncidenceGraph.h"
#include "OutputGraph.h"

#include <map>
#include <algorithm>

#ifdef ACCSUB_TRACE
#include "../Helpers/Utils.h"
#endif

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

void IncidenceGraph::Node::AddEdge(IncidenceGraph::Edge *edge)
{
    // mozemy w ramach debugowania sprawdzic, czy krawedz nie zostala juz dodana
    // w rzeczywistosci taka sytuacja nie moze miec miejsca (sprawdzamy to
    // przed wywolaniem AddNeighbour)
    edges.push_back(edge);
}

bool IncidenceGraph::Node::HasNeighbour(IncidenceGraph::Node *neighbour)
{
    for (Edges::iterator i = edges.begin(); i != edges.end(); i++)
    {
        if ((*i)->GetNeighbour(this) == neighbour) return true;
    }
    return false;
}

void IncidenceGraph::Node::RemoveNeighbour(IncidenceGraph::Node *neighbour)
{
    for (Edges::iterator i = edges.begin(); i != edges.end(); i++)
    {
        if ((*i)->GetNeighbour(this) == neighbour)
        {
            edges.erase(i);
            return;
        }
    }
}

IncidenceGraph::IntersectionFlags IncidenceGraph::Node::GetNormalizedIntersectionFlags(const Simplex &intersection)
{
    return graph->subconfigurationsFlags[Normalize(intersection)];
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

void IncidenceGraph::Node::UpdateAcyclicIntersectionWithSimplex(const Simplex &simplex)
{
    for (Edges::iterator edge = edges.begin(); edge != edges.end(); edge++)
    {
        if ((*edge)->GetNeighbour(this)->IsAcyclic())
        {
            continue;
        }
        if (!(*edge)->IntersectionCalculated())
        {
            (*edge)->CalculateIntersection();
        }
        // obliczamy czesc wspolna (przeciecie) krawedzi i przeciecia z sasiadem
        Simplex s;
        Simplex::GetIntersection((*edge)->intersection, simplex, s);
        // jezeli przeciecie to jest niepuste (maja wspolne punkty)
        // to aktualizujemy jego acyclic flags
        if (s.size() > 0)
        {
            Node *neighbour = (*edge)->GetNeighbour(this);
            s = neighbour->Normalize(s);
            neighbour->UpdateAcyclicIntersectionFlags(graph->subconfigurationsFlags[s], graph->configurationsFlags[s]);
        }
    }
    acyclicIntersectionFlags |= graph->subconfigurationsFlags[Normalize(simplex)];
}

void IncidenceGraph::Node::UpdateAcyclicIntersectionWithVertex(Vertex v)
{
    Simplex s = Simplex::FromVertex(v);
    for (Edges::iterator edge = edges.begin(); edge != edges.end(); edge++)
    {
        if ((*edge)->GetNeighbour(this)->IsAcyclic())
        {
            continue;
        }
        if (!(*edge)->IntersectionCalculated())
        {
            (*edge)->CalculateIntersection();
        }
        if ((*edge)->intersection.ContainsVertex(v))
        {
            Node *neighbour = (*edge)->GetNeighbour(this);
            Simplex s1 = neighbour->Normalize(s);
            neighbour->UpdateAcyclicIntersectionFlags(graph->subconfigurationsFlags[s1], graph->configurationsFlags[s1]);
        }
    }
    acyclicIntersectionFlags |= graph->subconfigurationsFlags[Normalize(s)];
}

void IncidenceGraph::Node::UpdateAcyclicIntersectionWithEdge(Vertex v1, Vertex v2)
{
    assert (v1 != v2);
    UpdateAcyclicIntersectionWithSimplex(Simplex::FromVertices(v1, v2));
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
        Node *neighbour = (*edge)->GetNeighbour(this);
        if (neighbour->IsAcyclic())
        {
            continue;
        }
        Simplex::GetIntersection(this->simplex, neighbour->simplex, intersection);
        intersection = neighbour->Normalize(intersection);
        neighbour->UpdateAcyclicIntersectionFlags(graph->subconfigurationsFlags[intersection], graph->configurationsFlags[intersection]);
    }
}

////////////////////////////////////////////////////////////////////////////////

Simplex IncidenceGraph::Node::Normalize(const Simplex &simplex)
{
    Simplex s = Simplex::WithSize(simplex.size());
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
