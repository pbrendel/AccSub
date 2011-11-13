/*
 * File:   AcyclicTest.hpp
 * Author: Piotr Brendel
 */

#ifndef ACYCLICTEST_HPP
#define ACYCLICTEST_HPP

#include "Simplex.h"
#include <map>
#include <string>

#include <cstdio>
#include <iostream>

////////////////////////////////////////////////////////////////////////////////

template <class FlagsType>
class AcyclicTest
{

public:

    virtual ~AcyclicTest() { };
    virtual bool IsAcyclic(Simplex &simplex, SimplexList &intersection) = 0;
    virtual bool IsAcyclic(Simplex &simplex, FlagsType intersectionFlags, FlagsType intersectionFlagsMaximalFaces) = 0;
    virtual int GetID() = 0;

    static AcyclicTest *Create(int acyclicTestNumber, int dim);

};

////////////////////////////////////////////////////////////////////////////////

template <class FlagsType>
class AcyclicTestCodim1 : public AcyclicTest<FlagsType>
{

    int maxSimplexSize;
    std::map<int, FlagsType> codim1flags;

public:

    AcyclicTestCodim1(int dim)
    {
        maxSimplexSize = dim + 1;

        // uwaga: ponizszy kod jest identyczny z tym w klasie IncidenceGraph
        Simplex s;
        for (int i = 0; i < maxSimplexSize; i++)
        {
            s.push_back(i);
        }

        // generujemy wszystkie sciany posortowane rosnaco wymiarami
        // w kolejnosci leksykograficznej i ustawiamy flagi
        SimplexList faces;
        s.GenerateProperFaces(faces);
        std::map<Simplex, FlagsType> configurationsFlags;
        FlagsType flags = 1;
        for (SimplexList::iterator i = faces.begin(); i != faces.end(); i++)
        {
            configurationsFlags[(*i)] = flags;
            flags = flags << 1;
        }

        s.clear();
        s.push_back(0);
        s.push_back(1);
        for (int d = 2; d <= maxSimplexSize; d++)
        {
            faces.clear();
            s.GenerateProperFaces(faces);
            FlagsType flags = 0;
            for (SimplexList::iterator i = faces.begin(); i != faces.end(); i++)
            {
                if ((*i).size() == d - 1)
                {
                    flags |= configurationsFlags[*i];
                }
            }
            codim1flags[d] = flags;
            s.push_back(d);
        }
    }
    
    bool IsAcyclic(Simplex &simplex, SimplexList &intersection)
    {
        // jezeli przeciecie jest puste
        if (intersection.size() == 0)
        {
            return false;
        }
        // jezeli w przecieciu jest tylko jedna sciana -> jest acykliczne
        if (intersection.size() == 1)
        {
            return true;
        }
        int d = simplex.size() - 1;
        for (SimplexList::iterator i = intersection.begin(); i != intersection.end(); i++)
        {
            if (i->size() != d)
            {
                return false;
            }
        }
        return intersection.size() < simplex.size();
    }
    
    bool IsAcyclic(Simplex &simplex, FlagsType intersectionFlags, FlagsType intersectionFlagsMaximalFaces)
    {
        // jezeli przeciecie jest puste
        if (intersectionFlagsMaximalFaces == 0)
        {
            return false;
        }
        int d = simplex.size();
        // dla sympleksow o liczbie wierzcholkow < 2 przeciecie
        // zawsze jest acykliczne
        if (d < 2) return true;
        // dla rozmiarow wiekszych niz maxDim nie mamy danych
        if (d > maxSimplexSize) return false;
        FlagsType flags = codim1flags[d];
        // w przecieciu sa wszystkie podsympleksy z codim == 1
        if ((intersectionFlagsMaximalFaces & flags) == flags) return false;
        // w przecieciu jest cos poza podsympleksami z codim == 1
        if ((intersectionFlagsMaximalFaces & (~flags)) != 0) return false;
        return true;
    }

    int GetID() { return 1; }
};

////////////////////////////////////////////////////////////////////////////////

#define TAB_2D  "tablica2bBin.txt"
#define TAB_3D  "tablica3bBin.txt"
#define TAB_4D  "tab4d.txt"

template <class FlagsType>
class AcyclicTestTabs : public AcyclicTest<FlagsType>
{

public:

    AcyclicTestTabs(int dim)
    {
        this->dim = dim;
        if (dim < 2 || dim > 4)
        {
            throw std::string("dim < 2 || dim > 4");
        }

        // uwaga: ponizszy kod jest identyczny z tym w klasie IncidenceGraph
        // do tworzenia flag wszystkich podkonfiguracji
        // generowanie hasha konfiguracja->flaga
        // najpierw tworzymy "bazowy" sympleks maksymalne wymiarowy
        Simplex s;
        for (int i = 0; i <= dim; i++)
        {
            s.push_back(i);
        }
        // potem generujemy wszystkie sciany posortowane rosnaco wymiarami
        // w kolejnosci leksykograficznej
        SimplexList faces;
        s.GenerateProperFaces(faces);
        // i tworzymy hasha z flagami konfiguracji
        FlagsType flag = 1;
        for (SimplexList::iterator i = faces.begin(); i != faces.end(); i++)
        {
            FlagsType subFlags = flag;
            // teraz generujemy wszystkie podkonfiguracje i aktualizujemy flagi
            // mozemy to zrobic w tym miejscu, bo flagi nizej wymiarowych konfiguracji
            // sa juz ustawione
            SimplexList subconfigurations;
            i->GenerateProperFaces(subconfigurations);
            for (SimplexList::iterator j = subconfigurations.begin(); j != subconfigurations.end(); j++)
            {
                subFlags |= configurationsFlags[(*j)];
            }
            configurationsFlags[(*i)] = subFlags;
            
            flag = flag << 1;
        }

        // wczytywanie tablicy
        std::string filename;
        if (dim == 2) filename = TAB_2D;
        else if (dim == 3) filename = TAB_3D;
        else if (dim == 4) filename = TAB_4D;
        FILE *fp = fopen(filename.c_str(), "rb");
        if (fp == 0)
        {
            throw (std::string("Can't open data file: ") + filename);
        }
        fseek(fp, 0, SEEK_END);
        dataSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        data = new unsigned char[dataSize];
        fread(data, sizeof(unsigned char), dataSize, fp);
        fclose(fp);
    }

    ~AcyclicTestTabs()
    {
        delete [] data;
    }

    bool IsAcyclic(Simplex &simplex, SimplexList &intersection)
    {
        // jezeli w przecieciu jest tylko jedna sciana -> jest acykliczne
        if (intersection.size() == 1)
        {
            return true;
        }
        // zakladamy, ze wejsciowe simpleksy sa w wersji "indeksowej"
        int index = 0;
        for (SimplexList::iterator i = intersection.begin(); i != intersection.end(); i++)
        {
            index |= configurationsFlags[*i];
        }
        return GetValue(index);
    }

    bool IsAcyclic(Simplex &simplex, FlagsType intersectionFlags, FlagsType intersectionFlagsMaximalFaces)
    {
        return GetValue((int)intersectionFlags);
    }

    int GetID() { return 0; }

private:

    int dim;
    std::map<Simplex, FlagsType> configurationsFlags;
    long int dataSize;
    unsigned char *data;

    bool GetValue(int index)
    {
        // 8 bo pakowane na bajtach
        int i = index / 8;
        if (i < 0 || i >= dataSize)
        {
            throw std::string("i < 0 || i >= dataSize");
        }
        int flag = 1 << (index % 8);
        return ((data[i] & flag) != 0);
    }

};

////////////////////////////////////////////////////////////////////////////////

template <class FlagsType>
class AcyclicTestFalse : public AcyclicTest<FlagsType>
{

public:

    bool IsAcyclic(Simplex &simplex, SimplexList &intersection)
    {
        return false;
    }
    bool IsAcyclic(Simplex &simplex, FlagsType intersectionFlags, FlagsType intersectionFlagsMaximalFaces)
    {
        return false;
    }

    int GetID() { return -1; }
};


////////////////////////////////////////////////////////////////////////////////

template <class FlagsType>
AcyclicTest<FlagsType> *AcyclicTest<FlagsType>::Create(int acyclicTestNumber, int dim)
{
    if (dim < 2) dim = 2;
    if (acyclicTestNumber == 0) // jezeli tablice, to gorne ograniczenie na wymiar == 4
    {
        if (dim > 4) dim = 4;
    }
    if (acyclicTestNumber == 1) return new AcyclicTestCodim1<FlagsType>(dim);
    return new AcyclicTestTabs<FlagsType>(dim); // default
}

////////////////////////////////////////////////////////////////////////////////

#endif /* ACYCLICTEST_HPP */
