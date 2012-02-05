/*
 * File:   AccTest.hpp
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 */

#ifndef ACCTEST_HPP
#define ACCTEST_HPP

#include <map>
#include <queue>
#include <set>
#include <string>

#include <cassert>

#include "ConfigurationsFlags.hpp"
#include "../Helpers/RedHomHelpers.hpp"

////////////////////////////////////////////////////////////////////////////////

template <typename Traits>
class AccTestT
{
    typedef typename Traits::Simplex Simplex;
    typedef typename Traits::SimplexList SimplexList;
    typedef typename Traits::IntersectionFlags IntersectionFlags;

public:

    virtual ~AccTestT() { };
    virtual bool IsAcyclic(const Simplex &simplex, SimplexList &intersectionMF) = 0;
    virtual bool IsAcyclic(const Simplex &simplex, const IntersectionFlags &intersectionFlags, const IntersectionFlags &intersectionFlagsMF) = 0;
    virtual int GetID() = 0;

    int TrivialTest(const Simplex &simplex, SimplexList &intersectionMF)
    {
        if (intersectionMF.size() == 0)
        {
            return -1;
        }
        if (simplex.size() < 2)
        {
            return 1;
        }
        return 0;
    }

    int TrivialTest(const Simplex &simplex, const IntersectionFlags &intersectionFlags, const IntersectionFlags &intersectionFlagsMF)
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

    static AccTestT *Create(int accTestNumber, int dim);

};

#define TRIVIAL_TEST_I(s, i) int tt = this->TrivialTest(s, i); if (tt < 0) return false; else if (tt > 0) return true;
#define TRIVIAL_TEST_F(s, i, im) int tt = this->TrivialTest(s, i, im); if (tt < 0) return false; else if (tt > 0) return true;

////////////////////////////////////////////////////////////////////////////////

template <typename Traits>
class AccTestFalse : public AccTestT<Traits>
{
    typedef typename Traits::Simplex Simplex;
    typedef typename Traits::SimplexList SimplexList;
    typedef typename Traits::IntersectionFlags IntersectionFlags;
    
public:

    bool IsAcyclic(const Simplex &simplex, SimplexList &intersectionMF)
    {
        return false;
    }
    bool IsAcyclic(const Simplex &simplex, const IntersectionFlags &intersectionFlags, const IntersectionFlags &intersectionFlagsMF)
    {
        return false;
    }

    int GetID() { return -1; }
};

////////////////////////////////////////////////////////////////////////////////

#define TAB_2D  "tablica2bBin.txt"
#define TAB_3D  "tablica3bBin.txt"
#define TAB_4D  "tab4d.txt"

template <typename Traits>
class AccTestTabs : public AccTestT<Traits>
{
    typedef typename Traits::Simplex Simplex;
    typedef typename Traits::SimplexList SimplexList;
    typedef typename Traits::IntersectionFlags IntersectionFlags;
    
public:

    AccTestTabs(int dim)
    {
        if (dim < 2 || dim > 4)
        {
            throw std::string("AccTestTabs: dim < 2 || dim > 4");
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

    ~AccTestTabs()
    {
        delete [] data;
    }

    bool IsAcyclic(const Simplex &simplex, SimplexList &intersectionMF)
    {
        // if there is only one face in the intersection -> it is acyclic
        if (intersectionMF.size() == 1)
        {
            return true;
        }
        // we assume that simplices are normalized
        int index = 0;
        for (typename SimplexList::iterator i = intersectionMF.begin(); i != intersectionMF.end(); i++)
        {
            index |= configurationsFlags[*i];
        }
        return GetValue(index);
    }

    bool IsAcyclic(const Simplex &simplex, const IntersectionFlags &intersectionFlags, const IntersectionFlags &intersectionFlagsMF)
    {
        return GetValue((int)intersectionFlags);
    }

    int GetID() { return 0; }

private:

    ConfigurationsFlags<Simplex, IntersectionFlags> configurationsFlags;
    long int dataSize;
    unsigned char *data;

    bool GetValue(int index)
    {
        // packed on 8 bytes
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

template <typename Traits>
class AccTestCodim1 : public AccTestT<Traits>
{
    typedef typename Traits::Simplex Simplex;
    typedef typename Traits::SimplexList SimplexList;
    typedef typename Traits::IntersectionFlags IntersectionFlags;
    
    int maxSimplexSize;
    std::map<int, IntersectionFlags> codim1flags;

public:

    AccTestCodim1(int dim)
    {
        maxSimplexSize = dim + 1;

        ConfigurationsFlags<Simplex, IntersectionFlags> configurationsFlags(dim, false, false);

        Simplex s = Simplex::FromVertices(0, 1);
        for (int d = 2; d <= maxSimplexSize; d++)
        {
            SimplexList faces;
            s.GenerateProperFaces(faces);
            IntersectionFlags flags = 0;
            for (typename SimplexList::iterator i = faces.begin(); i != faces.end(); i++)
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

    bool IsAcyclic(const Simplex &simplex, SimplexList &intersectionMF)
    {
        TRIVIAL_TEST_I(simplex, intersectionMF);
        int d = simplex.size() - 1;
        for (typename SimplexList::iterator i = intersectionMF.begin(); i != intersectionMF.end(); i++)
        {
            if (i->size() != d)
            {
                  return false;
            }
        }
        return intersectionMF.size() < simplex.size();
    }

    bool IsAcyclic(const Simplex &simplex, const IntersectionFlags &intersectionFlags, const IntersectionFlags &intersectionFlagsMF)
    {
        TRIVIAL_TEST_F(simplex, intersectionFlags, intersectionFlagsMF);
        int d = simplex.size();
        if (d > maxSimplexSize) return false;
        IntersectionFlags flags = codim1flags[d];
        // intersection contains all faces with CoDim == 1
        if ((intersectionFlagsMF & flags) == flags) return false;
        // intersection contains faces with CoDim != 1
        if ((intersectionFlagsMF & (~flags)) != 0) return false;
        return true;
    }

    int GetID() { return 1; }
};

////////////////////////////////////////////////////////////////////////////////

template <typename Traits>
class AccTestStar : public AccTestT<Traits>
{
    typedef typename Traits::Vertex Vertex;
    typedef typename Traits::Simplex Simplex;
    typedef typename Traits::SimplexList SimplexList;
    typedef typename Traits::IntersectionFlags IntersectionFlags;
    
    int firstMaximalFacePower;
    int lastMaximalFacePower;
    Vertex firstVertex;
    Vertex lastVertex;
    std::map<IntersectionFlags, IntersectionFlags> confToSubconf;

public:

    AccTestStar(int dim)
    {
        firstMaximalFacePower = dim + 1;
        lastMaximalFacePower = (1 << (dim + 1)) - 2;
        firstVertex = Vertex(0);
        lastVertex = Vertex(dim);

        ConfigurationsFlags<Simplex, IntersectionFlags> configurationsFlags(dim, false, false);
        ConfigurationsFlags<Simplex, IntersectionFlags> subconfigurationsFlags(dim, true, false);

        IntersectionFlags flag = 1 << firstMaximalFacePower;
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

    bool IsAcyclic(const Simplex &simplex, SimplexList &intersectionMF)
    {
        TRIVIAL_TEST_I(simplex, intersectionMF);
        int totalMaximalFaces = 0;
        int vertsCount = 0;
        for (typename SimplexList::iterator face = intersectionMF.begin(); face != intersectionMF.end(); face++)
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
        for (Vertex vertex = firstVertex; vertex <= lastVertex; vertex++)
        {
            int count = 0;
            for (typename SimplexList::iterator face = intersectionMF.begin(); face != intersectionMF.end(); face++)
            {
                // skipping vertices (0-dimensional simplices)
                if (face->size() > 1 && std::find(face->begin(), face->end(), vertex) != face->end())
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

    bool IsAcyclic(const Simplex &simplex, const IntersectionFlags &intersectionFlags, const IntersectionFlags &intersectionFlagsMF)
    {
        TRIVIAL_TEST_F(simplex, intersectionFlags, intersectionFlagsMF);
        // if we have more than one vertex that is maximal face
        // the intersection is not acyclic
        IntersectionFlags flag = 1;
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
        // computing flags for faces of dimension greater than 0
        std::vector<IntersectionFlags> maximalFacesFlags;
        for (int i = firstMaximalFacePower; i < lastMaximalFacePower; i++)
        {
            if ((intersectionFlagsMF & flag) == flag)
            {
                maximalFacesFlags.push_back(confToSubconf[flag]);
            }
            flag = flag << 1;
        }
        // if there are no faces of dimension greater than 0
        // then the intersection is acyclic
        if (maximalFacesFlags.size() == 0)
        {
            return true;
        }
        // if there are faces of dimension greater than 0 and there is
        // at least one vertex being maximal face then the intersection
        // is not acyclic
        else if (vertsCount > 0)
        {
            return false;
        }
        // finally we try to find a vertex that is contained in all
        // maximal faces. if we find such then we have a start topology
        // and the intersection is acyclic.
        flag = 1;
        for (int i = 0; i < firstMaximalFacePower; i++)
        {
            int count = 0;
            for (typename std::vector<IntersectionFlags>::iterator flags = maximalFacesFlags.begin(); flags != maximalFacesFlags.end(); flags++)
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

template <typename Traits>
class AccTestRecursive : public AccTestT<Traits>
{
    typedef typename Traits::Simplex Simplex;
    typedef typename Traits::SimplexList SimplexList;
    typedef typename Traits::IntersectionFlags IntersectionFlags;
    
    struct MaximalFace;
    
    typedef MaximalFace *MaximalFacePtr;
    
    struct MaximalFace
    {
        IntersectionFlags flag;
        IntersectionFlags subconfFlags;
        IntersectionFlags accIntersectionFlags;
        IntersectionFlags accIntersectionFlagsMF;
        bool isInAccSub;
        bool isAddedToQueue;
        
        MaximalFace(IntersectionFlags f, IntersectionFlags sf)
        {
            flag = f;
            subconfFlags = sf;
            accIntersectionFlags = 0;
            accIntersectionFlagsMF = 0;
            isInAccSub = false;
            isAddedToQueue = false;
        }
        
        void GetNeighboursNotInAccSub(const std::vector<MaximalFacePtr> &faces, std::vector<MaximalFacePtr> &neighbours)
        {
            neighbours.clear();
            for (typename std::vector<MaximalFacePtr>::const_iterator f = faces.begin(); f != faces.end(); f++)
            {
                if ((*f) == this || (*f)->isInAccSub)
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
    std::map<IntersectionFlags, IntersectionFlags> confToSubconf;
    
public:

    AccTestRecursive(int dim)
    {
        lastMaximalFacePower = (1 << (dim + 1)) - 2;

        ConfigurationsFlags<Simplex, IntersectionFlags> configurationsFlags(dim, false, false);
        ConfigurationsFlags<Simplex, IntersectionFlags> subconfigurationsFlags(dim, true, false);

        IntersectionFlags flag = 1;
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
    
    bool IsAcyclic(const Simplex &simplex, SimplexList &intersectionMF)
    {
        TRIVIAL_TEST_I(simplex, intersectionMF);

        std::queue<Simplex> Q;
        std::set<Simplex> simplicesAddedToQueue;
        SimplexList simplicesInAccSub;
        Simplex s = intersectionMF[0];
        simplicesInAccSub.push_back(s);
        Q.push(s);
        simplicesAddedToQueue.insert(s);
        while (!Q.empty())
        {
            s = Q.front();
            Q.pop();
            simplicesAddedToQueue.erase(s);
            SimplexList neighbours;
            GetNeighboursNotInAccSub(s, intersectionMF, simplicesInAccSub, neighbours);
            for (typename SimplexList::iterator n = neighbours.begin(); n != neighbours.end(); n++)
            {
                SimplexList intersection;
                GetIntersectionWithAccSub(*n, simplicesInAccSub, intersection);
                if (IsAcyclic(simplex, intersection))
                {
                    simplicesInAccSub.push_back(*n);
                    if (simplicesAddedToQueue.find(*n) == simplicesAddedToQueue.end())
                    {                      
                        Q.push(*n);
                        simplicesAddedToQueue.insert(s);
                    }
                }
            }
        }

        return (simplicesInAccSub.size() == intersectionMF.size());
    }
    
    bool IsAcyclic(const Simplex &simplex, const IntersectionFlags &intersectionFlags, const IntersectionFlags &intersectionFlagsMF)
    {
        TRIVIAL_TEST_F(simplex, intersectionFlags, intersectionFlagsMF);
        
        IntersectionFlags flag = 1;
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

        std::queue<MaximalFacePtr> Q;
        MaximalFacePtr mf = maximalFaces[0];
        mf->isAddedToQueue = true;
        Q.push(mf);
        int facesInAccSub = 1;
        mf->isInAccSub = true;
        UpdateNeighboursAccIntersection(mf, maximalFaces);
        while (!Q.empty())
        {
            mf = Q.front();
            Q.pop();
            mf->isAddedToQueue = false;
            std::vector<MaximalFacePtr> neighbours;
            mf->GetNeighboursNotInAccSub(maximalFaces, neighbours);
            for (typename std::vector<MaximalFacePtr>::iterator n = neighbours.begin(); n != neighbours.end(); n++)
            {
                if (IsAcyclic(simplex, (*n)->accIntersectionFlags, (*n)->accIntersectionFlagsMF))
                {
                    facesInAccSub++;
                    (*n)->isInAccSub = true;
                    UpdateNeighboursAccIntersection((*n), maximalFaces);
                    if (!(*n)->isAddedToQueue)
                    {
                        (*n)->isAddedToQueue = true;
                        Q.push(*n);
                    }
                }
            }
        }
        
        Clear(maximalFaces);
        return (facesInAccSub == maximalFaces.size());
    }
    
    int GetID() { return 3; }
    
private:
        
    void UpdateNeighboursAccIntersection(const MaximalFacePtr &face, const std::vector<MaximalFacePtr> &maximalFaces)
    {
        std::vector<MaximalFacePtr> neighbours;
        face->GetNeighboursNotInAccSub(maximalFaces, neighbours);
        for (typename std::vector<MaximalFacePtr>::const_iterator f = neighbours.begin(); f != neighbours.end(); f++)
        {
            if ((*f) == face || (*f)->isInAccSub)
            {
                continue;
            }
            IntersectionFlags intersection = (*f)->subconfFlags & face->subconfFlags;
            // if flags have been already set we do nothing
            if (((*f)->accIntersectionFlags & intersection) == intersection)
            {
                continue;
            }
            IntersectionFlags flags = 1 << lastMaximalFacePower;
            IntersectionFlags intersectionMF = intersection;
            for (int i = 0; i < lastMaximalFacePower; i++)
            {
                if ((intersectionMF & flags) != 0)
                {
                    IntersectionFlags subconf = confToSubconf[flags];
                    intersectionMF &= (~(subconf & (~flags)));
                }
                flags = flags >> 1;
            }
            (*f)->accIntersectionFlags |= intersection;
            (*f)->accIntersectionFlagsMF |= intersectionMF;
            IntersectionFlags flagsSubfaces = intersection & (~intersectionMF);
            (*f)->accIntersectionFlagsMF &= (~flagsSubfaces);
        }
    }        
    
    void Clear(const std::vector<MaximalFacePtr> &maximalFaces)
    {
        for (typename std::vector<MaximalFacePtr>::const_iterator f = maximalFaces.begin(); f != maximalFaces.end(); f++)
        {
            delete (*f);
        }
    }

    void GetNeighboursNotInAccSub(const Simplex &s, const SimplexList &simplexList, const SimplexList &simplicesInAccSub, SimplexList &neighbours)
    {
        if (neighbours.size() > 0)
        {
            neighbours.clear();
        }
        for (typename SimplexList::const_iterator i = simplexList.begin(); i != simplexList.end(); i++)
        {
            if (s == (*i) || std::find(simplicesInAccSub.begin(), simplicesInAccSub.end(), *i) != simplicesInAccSub.end())
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

    void GetIntersectionWithAccSub(const Simplex &s, const SimplexList &simplicesInAccSub, SimplexList &accIntersection)
    {
        if (accIntersection.size() > 0)
        {
            accIntersection.clear();
        }
        for (typename SimplexList::const_iterator i = simplicesInAccSub.begin(); i != simplicesInAccSub.end(); i++)
        {
            if (s == (*i))
            {
                continue;
            }
            Simplex intersection;
            if (Simplex::GetIntersection(s, *i, intersection))
            {
                accIntersection.push_back(intersection);
            }
        }
    }
};

////////////////////////////////////////////////////////////////////////////////

template <typename Traits, typename HomologyHelper>
class AccTestHomology : public AccTestT<Traits>
{
    typedef typename Traits::Simplex Simplex;
    typedef typename Traits::SimplexList SimplexList;
    typedef typename Traits::IntersectionFlags IntersectionFlags;

    std::map<IntersectionFlags, Simplex> simplexMap;
    int lastMaximalFacePower;

public:

    AccTestHomology(int dim)
    {
        lastMaximalFacePower = (1 << (dim + 1)) - 2;
        ConfigurationsFlags<Simplex, IntersectionFlags> configurationsFlags(dim, false, true);
        configurationsFlags.GetReverseMap(simplexMap);
    }

    bool IsAcyclic(const Simplex &simplex, SimplexList &intersectionMF)
    {
        TRIVIAL_TEST_I(simplex, intersectionMF);
        return HomologyHelper::IsTrivialHomology(intersectionMF);
    }

    bool IsAcyclic(const Simplex &simplex, const IntersectionFlags &intersectionFlags, const IntersectionFlags &intersectionFlagsMF)
    {
        TRIVIAL_TEST_F(simplex, intersectionFlags, intersectionFlagsMF);
        IntersectionFlags flag = 1;
        SimplexList simplexList;
        for (int i = 0; i < lastMaximalFacePower; i++)
        {
            if ((intersectionFlagsMF & flag) == flag)
            {
                simplexList.push_back(simplexMap[flag]);
            }
            flag = flag << 1;
        }
        return HomologyHelper::IsTrivialHomology(simplexList);
    }

    int GetID() { return 4; }

};

////////////////////////////////////////////////////////////////////////////////

template <typename Traits>
AccTestT<Traits> *AccTestT<Traits>::Create(int accTestNumber, int dim)
{
    if (dim < 2) dim = 2;
    if (accTestNumber == 0) // if we have chosen tabs then maximal dim is 4
    {
        if (dim > 4) dim = 4;
    }
    if (accTestNumber == 1) return new AccTestCodim1<Traits>(dim);
    if (accTestNumber == 2) return new AccTestStar<Traits>(dim);
    if (accTestNumber == 3) return new AccTestRecursive<Traits>(dim);
    if (accTestNumber == 4) return new AccTestHomology<Traits, RedHomHelpers>(dim);
    return new AccTestTabs<Traits>(dim); // default
}

////////////////////////////////////////////////////////////////////////////////

#endif /* ACCTEST_HPP */
