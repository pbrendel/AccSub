/*
 * File:   AcyclicTest.hpp
 * Author: Piotr Brendel
 */

#ifndef ACYCLICTEST_HPP
#define ACYCLICTEST_HPP

#include "Simplex.h"
#include "ConfigurationsFlags.hpp"
#include <map>
#include <queue>
#include <string>

#include <cstdio>
#include <iostream>
#include <cassert>

////////////////////////////////////////////////////////////////////////////////

template <typename FlagsType>
class AcyclicTest
{

public:

    virtual ~AcyclicTest() { };
    virtual bool IsAcyclic(Simplex &simplex, SimplexList &intersectionMF) = 0;
    virtual bool IsAcyclic(Simplex &simplex, FlagsType intersectionFlags, FlagsType intersectionFlagsMF) = 0;
    virtual int GetID() = 0;

    int TrivialTest(Simplex &simplex, SimplexList &intersectionMF)
    {
        if (intersectionMF.size() == 0)
        {
            return -1;
        }
        if (intersectionMF.size() == 1)
        {
            return 1;
        }
        if (simplex.size() < 2)
        {
            return 1;
        }
        return 0;
    }

    int TrivialTest(Simplex &simplex, FlagsType intersectionFlags, FlagsType intersectionFlagsMF)
    {
        if (intersectionFlags == 0 || intersectionFlagsMF == 0)
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

    bool IsAcyclic(Simplex &simplex, SimplexList &intersectionMF)
    {
        return false;
    }
    bool IsAcyclic(Simplex &simplex, FlagsType intersectionFlags, FlagsType intersectionFlagsMF)
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

    bool IsAcyclic(Simplex &simplex, SimplexList &intersectionMF)
    {
        // jezeli w przecieciu jest tylko jedna sciana -> jest acykliczne
        if (intersectionMF.size() == 1)
        {
            return true;
        }
        // zakladamy, ze wejsciowe simpleksy sa w wersji "indeksowej"
        int index = 0;
        for (SimplexList::iterator i = intersectionMF.begin(); i != intersectionMF.end(); i++)
        {
            index |= configurationsFlags[*i];
        }
        return GetValue(index);
    }

    bool IsAcyclic(Simplex &simplex, FlagsType intersectionFlags, FlagsType intersectionFlagsMF)
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

    bool IsAcyclic(Simplex &simplex, SimplexList &intersectionMF)
    {
        TRIVIAL_TEST_I(simplex, intersectionMF);
        int d = simplex.size() - 1;
        for (SimplexList::iterator i = intersectionMF.begin(); i != intersectionMF.end(); i++)
        {
            if (i->size() != d)
            {
                return false;
            }
        }
        return intersectionMF.size() < simplex.size();
    }

    bool IsAcyclic(Simplex &simplex, FlagsType intersectionFlags, FlagsType intersectionFlagsMF)
    {
        TRIVIAL_TEST_F(simplex, intersectionFlags, intersectionFlagsMF);
        int d= simplex.size();
        if (d > maxSimplexSize) return false;
        FlagsType flags = codim1flags[d];
        // w przecieciu sa wszystkie podsympleksy z codim == 1
        if ((intersectionFlagsMF & flags) == flags) return false;
        // w przecieciu jest cos poza podsympleksami z codim == 1
        if ((intersectionFlagsMF & (~flags)) != 0) return false;
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

    bool IsAcyclic(Simplex &simplex, SimplexList &intersectionMF)
    {
        TRIVIAL_TEST_I(simplex, intersectionMF);
        int totalMaximalFaces = 0;
        int vertsCount = 0;
        for (SimplexList::iterator face = intersectionMF.begin(); face != intersectionMF.end(); face++)
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
            for (SimplexList::iterator face = intersectionMF.begin(); face != intersectionMF.end(); face++)
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

    bool IsAcyclic(Simplex &simplex, FlagsType intersectionFlags, FlagsType intersectionFlagsMF)
    {
        TRIVIAL_TEST_F(simplex, intersectionFlags, intersectionFlagsMF);
        // sprawdzamy liczbe wierzcholkow, ktore sa maksymalnymi podscianami
        // jezeli wiecej niz 1 -> przeciecie nie jest acykliczne
        FlagsType flag = 1;
        int vertsCount = 0;
        for (int i = 0; i < firstMaximalFacePower; i++)
        {
            if ((intersectionFlagsMF & flag) == flag)
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
            if ((intersectionFlagsMF & flag) == flag)
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
class AcyclicTestRecursive : public AcyclicTest<FlagsType>
{
    struct MaximalFace;
    
    typedef MaximalFace *MaximalFacePtr;
    
    struct MaximalFace
    {
        FlagsType flag;
        FlagsType subconfFlags;
        FlagsType acyclicIntersectionFlags;
        FlagsType acyclicIntersectionFlagsMF;
        bool isAcyclic;
        bool isAddedToQueue;
        
        MaximalFace(FlagsType f, FlagsType sf)
        {
            flag = f;
            subconfFlags = sf;
            acyclicIntersectionFlags = 0;
            acyclicIntersectionFlagsMF = 0;
            isAcyclic = false;
            isAddedToQueue = false;
        }
        
        void GetNotAcyclicNeighbours(const std::vector<MaximalFacePtr> &faces, std::vector<MaximalFacePtr> &neighbours)
        {
            neighbours.clear();
            for (typename std::vector<MaximalFacePtr>::const_iterator f = faces.begin(); f != faces.end(); f++)
            {
                if ((*f) == this || (*f)->isAcyclic)
                {
                    continue;
                }
                if ((subconfFlags & (*f)->subconfFlags) != 0)
                {
                    neighbours.push_back((*f));
                }
            }
        }        
    };
        
    int lastMaximalFacePower;
    std::map<FlagsType, FlagsType> confToSubconf;
    
public:

    AcyclicTestRecursive(int dim)
    {
        lastMaximalFacePower = (1 << (dim + 1)) - 2;

        ConfigurationsFlags<Simplex, FlagsType> configurationsFlags(dim, false, false);
        ConfigurationsFlags<Simplex, FlagsType> subconfigurationsFlags(dim, true, false);

        FlagsType flag = 1;
        for (int i = 0; i < lastMaximalFacePower; i++)
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
    
    bool IsAcyclic(Simplex &simplex, SimplexList &intersectionMF)
    {
        TRIVIAL_TEST_I(simplex, intersectionMF);

        std::queue<Simplex> L;
        std::set<Simplex> enququedSimplices;
        SimplexList acyclicSimplices;
        Simplex s = intersectionMF[0];
        acyclicSimplices.push_back(s);
        L.push(s);
        enququedSimplices.insert(s);
        while (!L.empty())
        {
            s = L.front();
            L.pop();
            enququedSimplices.erase(s);
            SimplexList neighbours;
            GetNotAcyclicNeighbours(s, intersectionMF, acyclicSimplices, neighbours);
            for (typename SimplexList::iterator n = neighbours.begin(); n != neighbours.end(); n++)
            {
                SimplexList intersection;
                GetAcyclicIntersection(*n, acyclicSimplices, intersection);
                if (IsAcyclic(simplex, intersection))
                {
                    acyclicSimplices.push_back(*n);
                    if (enququedSimplices.find(*n) == enququedSimplices.end())
                    {                      
                        L.push(*n);
                        enququedSimplices.insert(s);
                    }
                }
            }
        }

        return (acyclicSimplices.size() == intersectionMF.size());
    }
    
    bool IsAcyclic(Simplex &simplex, FlagsType intersectionFlags, FlagsType intersectionFlagsMF)
    {
        TRIVIAL_TEST_F(simplex, intersectionFlags, intersectionFlagsMF);
        
        FlagsType flag = 1;
        std::vector<MaximalFacePtr> maximalFaces;
        for (int i = 0; i < lastMaximalFacePower; i++)
        {
            if ((intersectionFlagsMF & flag) != 0)
            {
                maximalFaces.push_back(new MaximalFace(flag, confToSubconf[flag]));                
            }
            flag = flag << 1;
        }
        if (maximalFaces.size() == 1)
        {
            Clear(maximalFaces);
            return true;
        }

        std::queue<MaximalFacePtr> L;
        MaximalFacePtr mf = maximalFaces[0];
        mf->isAddedToQueue = true;
        L.push(mf);
        int acyclicFaces = 1;
        mf->isAcyclic = true;
        UpdateNeighboursAccInt(mf, maximalFaces);
        while (!L.empty())      
        {
            mf = L.front();
            L.pop();
            mf->isAddedToQueue = false;
            std::vector<MaximalFacePtr> neighbours;
            mf->GetNotAcyclicNeighbours(maximalFaces, neighbours);
            for (typename std::vector<MaximalFacePtr>::iterator n = neighbours.begin(); n != neighbours.end(); n++)
            {
                if (IsAcyclic(simplex, (*n)->acyclicIntersectionFlags, (*n)->acyclicIntersectionFlagsMF))
                {
                    acyclicFaces++;
                    (*n)->isAcyclic = true;
                    UpdateNeighboursAccInt((*n), maximalFaces);
                    if (!(*n)->isAddedToQueue)
                    {
                        (*n)->isAddedToQueue = true;
                        L.push(*n);
                    }
                }
            }
        }
        
        Clear(maximalFaces);
        return (acyclicFaces == maximalFaces.size());
    }
    
    int GetID() { return 3; }
    
private:
        
    void UpdateNeighboursAccInt(const MaximalFacePtr &face, const std::vector<MaximalFacePtr> &maximalFaces)
    {
        std::vector<MaximalFacePtr> neighbours;
        face->GetNotAcyclicNeighbours(maximalFaces, neighbours);
        for (typename std::vector<MaximalFacePtr>::const_iterator f = neighbours.begin(); f != neighbours.end(); f++)
        {
            if ((*f) == face || (*f)->isAcyclic)
            {
                continue;
            }
            FlagsType intersection = (*f)->subconfFlags & face->subconfFlags;                        
            // jezeli flagi juz sa ustawione to nic nie robimy
            if (((*f)->acyclicIntersectionFlags & intersection) == intersection)
            {
                continue;
            }
            FlagsType flags = 1 << lastMaximalFacePower;
            FlagsType intersectionMF = intersection;
            for (int i = 0; i < lastMaximalFacePower; i++)
            {
                if ((intersectionMF & flags) != 0)
                {
                    FlagsType subconf = confToSubconf[flags];
                    intersectionMF &= (~(subconf & (~flags)));
                }
                flags = flags >> 1;
            }
            (*f)->acyclicIntersectionFlags |= intersection;
            (*f)->acyclicIntersectionFlagsMF |= intersectionMF;
            FlagsType flagsSubfaces = intersection & (~intersectionMF);
            (*f)->acyclicIntersectionFlagsMF &= (~flagsSubfaces);            
        }
    }        
    
    void Clear(const std::vector<MaximalFacePtr> &maximalFaces)
    {
        for (typename std::vector<MaximalFacePtr>::const_iterator f = maximalFaces.begin(); f != maximalFaces.end(); f++)
        {
            delete (*f);
        }
    }

    void GetNotAcyclicNeighbours(const Simplex &s, const SimplexList &simplexList, const SimplexList &acyclicSimplices, SimplexList &neighbours)
    {
        if (neighbours.size() > 0)
        {
            neighbours.clear();
        }
        for (SimplexList::const_iterator i = simplexList.begin(); i != simplexList.end(); i++)
        {
            if (s == (*i) || std::find(acyclicSimplices.begin(), acyclicSimplices.end(), *i) != acyclicSimplices.end())
            {
                continue;
            }
            Simplex intersection;
            if (Simplex::GetIntersection(s, *i, intersection))
            {
                neighbours.push_back(*i);
            }
        }
    }

    void GetAcyclicIntersection(const Simplex &s, const SimplexList &acyclicSimplices, SimplexList &acyclicIntersection)
    {
        if (acyclicIntersection.size() > 0)
        {
            acyclicIntersection.clear();
        }
        for (SimplexList::const_iterator i = acyclicSimplices.begin(); i != acyclicSimplices.end(); i++)
        {
            if (s == (*i))
            {
                continue;
            }
            Simplex intersection;
            if (Simplex::GetIntersection(s, *i, intersection))
            {
                acyclicIntersection.push_back(intersection);
            }
        }
    }
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
    if (acyclicTestNumber == 3) return new AcyclicTestRecursive<FlagsType>(dim);
    return new AcyclicTestTabs<FlagsType>(dim); // default
}

////////////////////////////////////////////////////////////////////////////////

#endif /* ACYCLICTEST_HPP */
