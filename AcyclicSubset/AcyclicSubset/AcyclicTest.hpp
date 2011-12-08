/*
 * File:   AcyclicTest.hpp
 * Author: Piotr Brendel
 */

#ifndef ACYCLICTEST_HPP
#define ACYCLICTEST_HPP

#include "Simplex.h"
#include "ConfigurationsFlags.hpp"
#include <map>
#include <string>

#include <cstdio>
#include <iostream>

////////////////////////////////////////////////////////////////////////////////

template <typename FlagsType>
class AcyclicTest
{

public:

    virtual ~AcyclicTest() { };
    virtual bool IsAcyclic(Simplex &simplex, SimplexList &intersection) = 0;
    virtual bool IsAcyclic(Simplex &simplex, FlagsType intersectionFlags, FlagsType intersectionFlagsMaximalFaces) = 0;
    virtual int GetID() = 0;

    int TrivialTest(Simplex &simplex, SimplexList &intersection)
    {
        if (intersection.size() == 0)
        {
            return -1;
        }
        if (intersection.size() == 1)
        {
            return 1;
        }
        if (simplex.size() < 2)
        {
            return 1;
        }
        return 0;
    }

    int TrivialTest(Simplex &simplex, FlagsType intersectionFlags, FlagsType intersectionFlagsMaximalFaces)
    {
        if (intersectionFlagsMaximalFaces == 0)
        {
            return -1;
        }
        if (simplex.size() < 2)
        {
            return 1;
        }
        return 0;
    }

    static AcyclicTest *Create(int acyclicTestNumber, int dim);

};

#define TRIVIAL_TEST_I(s, i) int tt = this->TrivialTest(s, i); if (tt < 0) return false; else if (tt > 0) return true;
#define TRIVIAL_TEST_F(s, i, im) int tt = this->TrivialTest(s, i, im); if (tt < 0) return false; else if (tt > 0) return true;

////////////////////////////////////////////////////////////////////////////////

template <typename FlagsType>
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

#define TAB_2D  "tablica2bBin.txt"
#define TAB_3D  "tablica3bBin.txt"
#define TAB_4D  "tab4d.txt"

template <typename FlagsType>
class AcyclicTestTabs : public AcyclicTest<FlagsType>
{

public:

    AcyclicTestTabs(int dim)
    {
        if (dim < 2 || dim > 4)
        {
            throw std::string("dim < 2 || dim > 4");
        }

        configurationsFlags.Create(dim, true, false);

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

    ConfigurationsFlags<Simplex, FlagsType> configurationsFlags;
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

template <typename FlagsType>
class AcyclicTestCodim1 : public AcyclicTest<FlagsType>
{

    int maxSimplexSize;
    std::map<int, FlagsType> codim1flags;

public:

    AcyclicTestCodim1(int dim)
    {
        maxSimplexSize = dim + 1;

        ConfigurationsFlags<Simplex, FlagsType> configurationsFlags(dim, false, false);

        Simplex s = Simplex::FromVertices(0, 1);
        for (int d = 2; d <= maxSimplexSize; d++)
        {
            SimplexList faces;
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
        TRIVIAL_TEST_I(simplex, intersection);
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
        TRIVIAL_TEST_F(simplex, intersectionFlags, intersectionFlagsMaximalFaces);
        int d= simplex.size();
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

template <typename FlagsType>
class AcyclicTestStar : public AcyclicTest<FlagsType>
{
    int firstMaximalFacePower;
    int lastMaximalFacePower;
    std::map<FlagsType, FlagsType> confToSubconf;

public:

    AcyclicTestStar(int dim)
    {
        firstMaximalFacePower = dim + 1;
        lastMaximalFacePower = (1 << (dim + 1)) - 2;

        ConfigurationsFlags<Simplex, FlagsType> configurationsFlags(dim, false, false);
        ConfigurationsFlags<Simplex, FlagsType> subconfigurationsFlags(dim, true, false);

        FlagsType flag = 1 << firstMaximalFacePower;
        for (int i = firstMaximalFacePower; i < lastMaximalFacePower; i++)
        {
            Simplex s;
            if (configurationsFlags.GetSimplex(flag, s))
            {
                confToSubconf[flag] = subconfigurationsFlags[s];
            }
            else
            {
                assert(false);
            }
            flag = flag << 1;
        }
    }

    bool IsAcyclic(Simplex &simplex, SimplexList &intersection)
    {
        TRIVIAL_TEST_I(simplex, intersection);
        int totalMaximalFaces = 0;
        int vertsCount = 0;
        for (SimplexList::iterator face = intersection.begin(); face != intersection.end(); face++)
        {
            if (face->size() > 1)
            {
                totalMaximalFaces++;
            }
            else
            {
                vertsCount++;
                if (vertsCount > 1)
                {
                    return false;
                }
            }
        }
        if (totalMaximalFaces == 0)
        {
            return true;
        }
        else if (vertsCount > 0)
        {
            return false;
        }
        for (Simplex::iterator vertex = simplex.begin(); vertex != simplex.end(); vertex++)
        {
            int count = 0;
            for (SimplexList::iterator face = intersection.begin(); face != intersection.end(); face++)
            {
                // omijamy wierzcholki
                if (face->size() > 1 && std::find(face->begin(), face->end(), *vertex) != face->end())
                {
                    count++;
                }
            }
            if (count == totalMaximalFaces)
            {
                return true;
            }
        }
        return false;
    }

    bool IsAcyclic(Simplex &simplex, FlagsType intersectionFlags, FlagsType intersectionFlagsMaximalFaces)
    {
        TRIVIAL_TEST_F(simplex, intersectionFlags, intersectionFlagsMaximalFaces);
        // sprawdzamy liczbe wierzcholkow, ktore sa maksymalnymi podscianami
        // jezeli wiecej niz 1 -> przeciecie nie jest acykliczne
        FlagsType flag = 1;
        int vertsCount = 0;
        for (int i = 0; i < firstMaximalFacePower; i++)
        {
            if ((intersectionFlagsMaximalFaces & flag) == flag)
            {
                vertsCount++;
                if (vertsCount > 1)
                {
                    return false;
                }
            }
            flag = flag << 1;
        }
        // teraz obliczamy flagi przeciec dla wszystkich maksymalnych
        // scian o wymiarze wiekszym niz 1
        std::vector<FlagsType> maximalFacesFlags;
        for (int i = firstMaximalFacePower; i < lastMaximalFacePower; i++)
        {
            if ((intersectionFlagsMaximalFaces & flag) == flag)
            {
                maximalFacesFlags.push_back(confToSubconf[flag]);
            }
            flag = flag << 1;
        }
        // jezeli nie ma scian o wymiarze wiekszym niz 1 -> przeciecie acykliczne
        if (maximalFacesFlags.size() == 0)
        {
            return true;
        }
        // jezeli sa takie sciany, a jest co najmniej jeden wolnu wierzcholek
        // to przeciecie nie jest acykliczne
        else if (vertsCount > 0)
        {
            return false;
        }
        // teraz dla kazdego wierzcholka sprawdzamy do ilu scian nalezy
        // jezeli nalezy do wszystkich z przeciecia -> przeciecie jest acykliczne
        flag = 1;
        for (int i = 0; i < firstMaximalFacePower; i++)
        {
            int count = 0;
            for (typename std::vector<FlagsType>::iterator flags = maximalFacesFlags.begin(); flags != maximalFacesFlags.end(); flags++)
            {
                if (((*flags) & flag) == flag)
                {
                    count++;
                }
            }
            if (count == maximalFacesFlags.size())
            {
                return true;
            }
            flag = flag << 1;
        }
        return false;
    }

    int GetID() { return 2; }

};

////////////////////////////////////////////////////////////////////////////////

template <typename FlagsType>
AcyclicTest<FlagsType> *AcyclicTest<FlagsType>::Create(int acyclicTestNumber, int dim)
{
    if (dim < 2) dim = 2;
    if (acyclicTestNumber == 0) // jezeli tablice, to gorne ograniczenie na wymiar == 4
    {
        if (dim > 4) dim = 4;
    }
    if (acyclicTestNumber == 1) return new AcyclicTestCodim1<FlagsType>(dim);
    if (acyclicTestNumber == 2) return new AcyclicTestStar<FlagsType>(dim);
    return new AcyclicTestTabs<FlagsType>(dim); // default
}

////////////////////////////////////////////////////////////////////////////////

#endif /* ACYCLICTEST_HPP */
