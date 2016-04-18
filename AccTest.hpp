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
#include "RedHomHelpers.hpp"

////////////////////////////////////////////////////////////////////////////////

class AccTestStats
{
    int allTests;
    int fullTests;
    int acyclic;
    int notAcyclic;

public:

    AccTestStats()
    {
        allTests = 0;
        fullTests = 0;
        acyclic = 0;
        notAcyclic = 0;
    }

    void TestPerformed()
    {
        allTests++;
    }

    void FullTestPerformed()
    {
        fullTests++;
    }

    void IsAcyclic()
    {
        acyclic++;
    }

    void IsNotAcyclic()
    {
        notAcyclic++;
    }

    bool RecordTest(bool isAcyclic)
    {
        if (isAcyclic)
        {
            acyclic++;
        }
        else
        {
            notAcyclic++;
        }
        return isAcyclic;
    }

    void Print()
    {
        std::cout<<"all tests: "<<allTests<<std::endl;
        if (fullTests > 0)
        {
            std::cout<<"full tests: "<<fullTests<<std::endl;
        }
        std::cout<<"acyclic: "<<acyclic<<std::endl;
        std::cout<<"not acyclic: "<<notAcyclic<<std::endl;
    }
};

////////////////////////////////////////////////////////////////////////////////

template <typename Traits>
class AccTestT
{
    typedef typename Traits::Simplex Simplex;
    typedef typename Traits::SimplexList SimplexList;
    typedef typename Traits::IntersectionFlags IntersectionFlags;

protected:

    int *flagsDimensions;
    int dim;

    int NewtonCoefficient(int n, int k)
    {
        if (k > n / 2)
        {
            k = n - k;
        }
        int numerator = n;
        int denominator = 1;
        for (int i = 2; i <= k; i++)
        {
            numerator *= (n - i + 1);
            denominator *= i;
        }
        return numerator / denominator;
    }

    void CreateFlagsDimensions()
    {
        flagsDimensions = new int[dim + 1];
        int index = 0;
        for (int i = 0; i < dim; i++)
        {
            index += this->NewtonCoefficient(dim + 1, i + 1);
            flagsDimensions[i] = index;
        }
    }

public:

    AccTestT(int d) : dim(d), flagsDimensions(0) { }
    virtual ~AccTestT() { delete [] flagsDimensions; };
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

    int EulerCharacteristic(const IntersectionFlags &intersectionFlags)
    {
        int currentDim = 0;
        int index = 0;
        int eulerCharacteristics = 0;
        int delta = 1;
        int flags = 1;
        while (currentDim < dim)
        {
            if (intersectionFlags & flags)
            {
                eulerCharacteristics += delta;
            }
            flags = flags << 1;
            index++;
            while (index >= this->flagsDimensions[currentDim] && currentDim < dim)
            {
                currentDim++;
                delta = -delta;
            }
        }
        return eulerCharacteristics;
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

#define TAB_2D  "data/tablica2bBin.txt"
#define TAB_3D  "data/tablica3bBin.txt"
#define TAB_4D  "data/tab4d.txt"

template <typename Traits>
class AccTestTabs : public AccTestT<Traits>
{
    typedef typename Traits::Simplex Simplex;
    typedef typename Traits::SimplexList SimplexList;
    typedef typename Traits::IntersectionFlags IntersectionFlags;

public:

    AccTestTabs(int dim) : AccTestT<Traits>(dim)
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

    AccTestCodim1(int dim) : AccTestT<Traits>(dim)
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

    AccTestStar(int dim) : AccTestT<Traits>(dim)
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

    AccTestRecursive(int dim) : AccTestT<Traits>(dim)
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

    AccTestHomology(int dim) : AccTestT<Traits>(dim)
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

template <typename Traits, typename HomologyHelper>
class AccTestReductions : public AccTestT<Traits>
{
    typedef typename Traits::Simplex Simplex;
    typedef typename Traits::SimplexList SimplexList;
    typedef typename Traits::IntersectionFlags IntersectionFlags;

    std::map<IntersectionFlags, Simplex> simplexMap;
    int lastMaximalFacePower;

public:

    AccTestReductions(int dim) : AccTestT<Traits>(dim)
    {
        lastMaximalFacePower = (1 << (dim + 1)) - 2;
        ConfigurationsFlags<Simplex, IntersectionFlags> configurationsFlags(dim, false, true);
        configurationsFlags.GetReverseMap(simplexMap);
    }

    bool IsAcyclic(const Simplex &simplex, SimplexList &intersectionMF)
    {
        TRIVIAL_TEST_I(simplex, intersectionMF);
        return HomologyHelper::IsFullyReducible(intersectionMF);
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
        return HomologyHelper::IsFullyReducible(simplexList);
    }

    int GetID() { return 5; }

};

////////////////////////////////////////////////////////////////////////////////

#define ACCTREE_2D  "acctree2d.dat"
#define ACCTREE_3D  "acctree3d.dat"
#define ACCTREE_4D  "acctree4d.dat"

template <typename Traits>
class AccTestTree : public AccTestT<Traits>
{
    typedef typename Traits::Simplex Simplex;
    typedef typename Traits::SimplexList SimplexList;
    typedef typename Traits::IntersectionFlags IntersectionFlags;

    struct Node
    {
        std::map<int, Node*> nodes;
        unsigned char type;
        std::set<IntersectionFlags> acyclicConfigurations;

        static const unsigned char STANDARD_NODE = 0;
        static const unsigned char FINAL_NODE_ACC = 1;
        static const unsigned char FINAL_NODE_CONFLICT = 2;

        Node()
        {
            type = STANDARD_NODE;
        }

        ~Node()
        {
            for (typename std::map<int, Node*>::iterator i = nodes.begin(); i != nodes.end(); i++)
            {
                delete i->second;
            }
        }

        int IsAcyclic(int dim, std::map<int, int> &facesCount, int lastNonzeroDim, const IntersectionFlags &intersectionFlagsMF)
        {
            if (dim == lastNonzeroDim)
            {
                if (type == STANDARD_NODE) return -1;
                if (type == FINAL_NODE_ACC) return 1;
                if (acyclicConfigurations.size() > 0)
                {
                    return (std::find(acyclicConfigurations.begin(), acyclicConfigurations.end(), intersectionFlagsMF) != acyclicConfigurations.end()) ? 1 : -1;
                }
                return 0; // type == FINAL_NODE_CONFLICT
            }
            else
            {
                int fc = facesCount[dim + 1];
                if (nodes[fc] == 0)
                {
                    return -1;
                }
                else
                {
                    return nodes[fc]->IsAcyclic(dim + 1, facesCount, lastNonzeroDim, intersectionFlagsMF);
                }
            }
        }

        void Read(FILE *fp)
        {
            fread(&type, sizeof(type), 1, fp);
            if (type == FINAL_NODE_CONFLICT)
            {
                int configurationsCount = 0;
                fread(&configurationsCount, sizeof(configurationsCount), 1, fp);
                for (int i = 0; i < configurationsCount; i++)
                {
                    IntersectionFlags flags = 0;
                    unsigned char simplexCount = 0;
                    fread(&simplexCount, sizeof(simplexCount), 1, fp);
                    for (int j = 0; j < simplexCount; j++)
                    {
                        unsigned char index = 0;
                        fread(&index, sizeof(index), 1, fp);
                        IntersectionFlags f = 1;
                        f = f << index;
                        flags = flags | f;
                    }
                    acyclicConfigurations.insert(flags);
                }
            }
            unsigned char nodesCount = 0;
            fread(&nodesCount, sizeof(nodesCount), 1, fp);
            for (int i = 0; i < nodesCount; i++)
            {
                unsigned char count = 0;
                fread(&count, sizeof(count), 1, fp);
                nodes[count] = new Node();
                nodes[count]->Read(fp);
            }
        }

        void Write(int dim, std::ostream &str)
        {
            if (type != STANDARD_NODE)
            {
                for (int i = 0; i < dim * 2; i++)
                {
                    str<<"=";
                }
                str<<"type "<<(int)type<<std::endl;
            }
            for (typename std::map<int, Node*>::iterator it = nodes.begin(); it != nodes.end(); it++)
            {
                for (int i = 0; i < dim * 2; i++)
                {
                    str<<"=";
                }
                str<<"dim : "<<(dim + 1)<<" faces count: "<<it->first<<std::endl;
                it->second->Write(dim + 1, str);
            }
        }
    };

    Node rootNode;
    AccTestT<Traits> *fullTest;
    ConfigurationsFlags<Simplex, IntersectionFlags> maxFacesFlags;

public:

    AccTestTree(int dim) : AccTestT<Traits>(dim), maxFacesFlags(dim, false, false)
    {
        if (dim < 2 || dim > 4)
        {
            throw std::string("AccTestTree: dim < 2 || dim > 4");
        }

        fullTest = 0; //sAccTestT<Traits>::Create(5, dim);

        std::string filename;
        if (dim == 2) filename = ACCTREE_2D;
        else if (dim == 3) filename = ACCTREE_3D;
        else if (dim == 4) filename = ACCTREE_4D;
        FILE *fp = fopen(filename.c_str(), "rb");
        if (fp == 0)
        {
            throw (std::string("Can't open data file: ") + filename);
        }
        rootNode.Read(fp);
        fclose(fp);

        this->CreateFlagsDimensions();
    }

    ~AccTestTree()
    {
        if (fullTest != 0)
        {
            delete fullTest;
        }
    }

    bool IsAcyclic(const Simplex &simplex, SimplexList &intersectionMF)
    {
        IntersectionFlags intersectionFlagsMF = 0;
        std::map<int, int> facesCount;
        int lastNonzeroDim = 0;
        for (typename SimplexList::iterator i = intersectionMF.begin(); i != intersectionMF.end(); i++)
        {
            intersectionFlagsMF = intersectionFlagsMF | maxFacesFlags[*i];
            int d = i->size() - 1;
            facesCount[d] = facesCount[d] + 1;
            if (d > lastNonzeroDim)
            {
                lastNonzeroDim = d;
            }
        }
        int res = IsAcyclic(facesCount, lastNonzeroDim, intersectionFlagsMF);
        if (res == 1) return true;
        if (res = -1) return false;
        if (fullTest != 0)
        {
            return fullTest->IsAcyclic(simplex, intersectionMF);
        }
        return false;
    }

    bool IsAcyclic(const Simplex &simplex, const IntersectionFlags &intersectionFlags, const IntersectionFlags &intersectionFlagsMF)
    {
        std::map<int, int> facesCount;
        int currentDim = 0;
        int index = 0;
        int flags = 1;
        int count = 0;
        int lastNonzeroDim = 0;
        while (currentDim < this->dim)
        {
            if (intersectionFlagsMF & flags)
            {
                count++;
            }
            flags = flags << 1;
            index++;
            while (index >= this->flagsDimensions[currentDim] && currentDim < this->dim)
            {
                facesCount[currentDim] = count;
                if (count != 0)
                {
                    lastNonzeroDim = currentDim;
                }
                count = 0;
                currentDim++;
            }
        }

        int res = IsAcyclic(facesCount, lastNonzeroDim, intersectionFlagsMF);
        if (res == 1) return true;
        if (res == -1) return false;
        if (fullTest != 0)
        {
            return fullTest->IsAcyclic(simplex, intersectionFlags, intersectionFlagsMF);
        }
        return false;
    }

    int GetID() { return 6; }

private:

    int IsAcyclic(std::map<int, int> &facesCount, int lastNonzeroDim, const IntersectionFlags &intersectionFlagsMF)
    {
        if (facesCount[0] > 1 || (facesCount[0] == 1 && lastNonzeroDim > 0))
        {
            return -1;
        }
        if (lastNonzeroDim == 0)
        {
            return 1;
        }
        return rootNode.IsAcyclic(0, facesCount, lastNonzeroDim, intersectionFlagsMF);
    }
};

////////////////////////////////////////////////////////////////////////////////

template <typename Traits>
AccTestT<Traits> *AccTestT<Traits>::Create(int accTestNumber, int dim)
{
    if (dim < 2) dim = 2;
    if (accTestNumber == 1) return new AccTestCodim1<Traits>(dim);
    if (accTestNumber == 2) return new AccTestStar<Traits>(dim);
    if (accTestNumber == 3) return new AccTestRecursive<Traits>(dim);
    if (accTestNumber == 4) return new AccTestHomology<Traits, RedHomHelpers>(dim);
    if (accTestNumber == 5) return new AccTestReductions<Traits, RedHomHelpers>(dim);
    if (accTestNumber == 6) return new AccTestTree<Traits>(dim);
    return new AccTestTabs<Traits>(dim); // default
}

////////////////////////////////////////////////////////////////////////////////

#endif /* ACCTEST_HPP */
