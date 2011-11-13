/*
 * File:   IncidenceGraphAlgorithms.cpp
 * Author: Piotr Brendel
 */

#include "IncidenceGraphAlgorithms.h"

////////////////////////////////////////////////////////////////////////////////

void CreateConfigurationsFlags(int maxDim, std::map<Simplex, IncidenceGraph::IntersectionFlags> &configurationsFlags, std::map<Simplex, IncidenceGraph::IntersectionFlags> &subconfigurationsFlags)
{
    // generowanie hasha konfiguracja->flaga
    // najpierw tworzymy "bazowy" sympleks maksymalne wymiarowy
    Simplex s;
    for (int i = 0; i <= maxDim; i++)
    {
        s.push_back(i);
    }

    // potem generujemy wszystkie podsympleksy posortowane rosnaco wymiarami
    // w kolejnosci leksykograficznej
    SimplexList faces;
    GenerateProperFaces(s, faces);
    // dodajemy caly sympleks
    faces.push_back(s);

    // i tworzymy hasha z flagami konfiguracji
    IncidenceGraph::IntersectionFlags flags = 1;
    for (SimplexList::iterator i = faces.begin(); i != faces.end(); i++)
    {
        // ustawiamy flage konfiguracji
        configurationsFlags[(*i)] = flags;

        IncidenceGraph::IntersectionFlags subFlags = flags;
        // teraz generujemy wszystkie podkonfiguracje i aktualizujemy flagi
        // mozemy to zrobic w tym miejscu, bo flagi nizej wymiarowych konfiguracji
        // sa juz ustawione
        SimplexList subconfigurations;
        GenerateProperFaces((*i), subconfigurations);
        for (SimplexList::iterator j = subconfigurations.begin(); j != subconfigurations.end(); j++)
        {
            subFlags |= configurationsFlags[(*j)];
        }
        subconfigurationsFlags[(*i)] = subFlags;

        flags = flags << 1;
    }

//    Log::stream<<"configurations flags:"<<std::endl;
//    Debug::Print(Log::stream, configurationsFlags);
//    Log::stream<<std::dec<<std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void GetSortedIntersectionOfUnsortedSets(std::vector<Vertex> &intersection, const std::set<Vertex> &setA, const std::set<Vertex> &setB)
{
    std::vector<Vertex> va;
    va.assign(setA.begin(), setA.end());
    std::sort(va.begin(), va.end());
    std::vector<Vertex> vb;
    vb.assign(setB.begin(), setB.end());
    std::sort(vb.begin(), vb.end());

    if (intersection.size() > 0)
    {
        intersection.clear();
    }

    if (va.size() == 0 || vb.size() == 0) return;
    if (va.front() > vb.back()) return;
    if (va.back() < vb.front()) return;

    std::vector<Vertex>::iterator ia = va.begin();
    std::vector<Vertex>::iterator ib = vb.begin();
    while (ia != va.end() && ib != vb.end())
    {
        if (*ia < *ib) ia++;
        else if (*ib < *ia) ib++;
        else
        {
            intersection.push_back(*ia);
            ia++;
            ib++;
        }
    }
}

void GetIntersectionOfUnsortedSets(std::set<Vertex> &intersection, const std::set<Vertex> &setA, const std::set<Vertex> &setB)
{
    std::vector<Vertex> va;
    va.assign(setA.begin(), setA.end());
    std::sort(va.begin(), va.end());
    std::vector<Vertex> vb;
    vb.assign(setB.begin(), setB.end());
    std::sort(vb.begin(), vb.end());

    if (intersection.size() > 0)
    {
        intersection.clear();
    }

    if (va.size() == 0 || vb.size() == 0) return;
    if (va.front() > vb.back()) return;
    if (va.back() < vb.front()) return;

    std::vector<Vertex>::iterator ia = va.begin();
    std::vector<Vertex>::iterator ib = vb.begin();
    while (ia != va.end() && ib != vb.end())
    {
        if (*ia < *ib) ia++;
        else if (*ib < *ia) ib++;
        else
        {
            intersection.insert(*ia);
            ia++;
            ib++;
        }
    }
}

void GetIntersectionOfUnsortedSetAndSortedVector(std::set<Vertex> &intersection, const std::set<Vertex> &setA, const std::vector<Vertex> &vb)
{
    std::vector<Vertex> va;
    va.assign(setA.begin(), setA.end());
    std::sort(va.begin(), va.end());

    if (intersection.size() > 0)
    {
        intersection.clear();
    }

    if (va.size() == 0 || vb.size() == 0) return;
    if (va.front() > vb.back()) return;
    if (va.back() < vb.front()) return;

    std::vector<Vertex>::iterator ia = va.begin();
    std::vector<Vertex>::const_iterator ib = vb.begin();
    while (ia != va.end() && ib != vb.end())
    {
        if (*ia < *ib) ia++;
        else if (*ib < *ia) ib++;
        else
        {
            intersection.insert(*ia);
            ia++;
            ib++;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// eof