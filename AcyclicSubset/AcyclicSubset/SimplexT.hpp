/* 
 * File:   SimplexT.hpp
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 */

#ifndef SIMPLEXT_HPP
#define	SIMPLEXT_HPP

#include <vector>
#include <algorithm>

template <typename VertexType>
class SimplexT : public std::vector<VertexType>
{

    SimplexT(int size) : std::vector<VertexType>(size) { }

public:

    SimplexT() { }

    static SimplexT WithDimension(int dim)
    {
        SimplexT s(dim + 1);
        return s;
    }

    static SimplexT WithSize(int size)
    {
        SimplexT s(size);
        return s;
    }

    static SimplexT FromVertex(VertexType v)
    {
        SimplexT s(1);
        s[0] = v;
        return s;
    }

    static SimplexT FromVertices(VertexType v1, VertexType v2)
    {
        if (v1 == v2)
        {
            SimplexT s(1);
            s[0] = v1;
            return s;
        }
        else if (v1 < v2)
        {
            SimplexT s(2);
            s[0] = v1;
            s[1] = v2;
            return s;
        }
        else
        {
            SimplexT s(2);
            s[0] = v2;
            s[1] = v1;
            return s;
        }
    }

    bool ContainsVertex(VertexType vertex)
    {
        return (std::find(this->begin(), this->end(), vertex) != this->end());
    }

    static bool GetIntersection(const SimplexT &a, const SimplexT &b, SimplexT &intersection)
    {
        if (intersection.size() > 0)
        {
            intersection.clear();
        }
        typename SimplexT::const_iterator i = a.begin();
        typename SimplexT::const_iterator j = b.begin();
        while (i != a.end() && j != b.end())
        {
            if ((*i) < (*j)) i++;
            else if ((*i) > (*j)) j++;
            else
            {
                intersection.push_back(*i);
                i++;
                j++;
            }
        }
        return (intersection.size() > 0);
    }

    static bool GetIntersection(const SimplexT *a, const SimplexT *b, SimplexT &intersection)
    {
        return GetIntersection(*a, *b, intersection);
    }

    static VertexType GetVertexFromIntersection(const SimplexT &a, const SimplexT &b)
    {
        typename SimplexT::const_iterator i = a.begin();
        typename SimplexT::const_iterator j = b.begin();
        while (i != a.end() && j != b.end())
        {
            if ((*i) < (*j)) i++;
            else if ((*i) > (*j)) j++;
            else
            {
                return (*i);
            }
        }
        return VertexType(-1);
    }

    static VertexType GetVertexFromIntersection(const SimplexT *a, const SimplexT *b)
    {
        return GetVertexFromIntersection(*a, *b);
    }

    static VertexType GetVertexFromIntersectionNotEqual(const SimplexT &a, const SimplexT &b, VertexType vertex)
    {
        typename SimplexT::const_iterator i = a.begin();
        typename SimplexT::const_iterator j = b.begin();
        while (i != a.end() && j != b.end())
        {
            if ((*i) < (*j)) i++;
            else if ((*i) > (*j)) j++;
            else if ((*i) == vertex)
            {
                i++;
                j++;
            }
            else
            {
                return (*i);
            }
        }
        return VertexType(-1);
    }

    static VertexType GetVertexFromIntersectionNotEqual(const SimplexT *a, const SimplexT *b, VertexType vertex)
    {
        return GetVertexFromIntersectionNotEqual(*a, *b, vertex);
    }

    template <template <typename V, typename A = std::allocator<V> > class VerticesSetT>
    static VertexType GetVertexFromIntersectionNotIn(const SimplexT &a, const SimplexT &b, const VerticesSetT<VertexType> &verticesSet)
    {
        typename SimplexT::const_iterator i = a.begin();
        typename SimplexT::const_iterator j = b.begin();
        while (i != a.end() && j != b.end())
        {
            if ((*i) < (*j)) i++;
            else if ((*i) > (*j)) j++;
            else if (std::find(verticesSet.begin(), verticesSet.end(), *i) != verticesSet.end())
            {
                i++;
                j++;
            }
            else
            {
                return (*i);
            }
        }
        return VertexType(-1);
    }

    template <template <typename V, typename A = std::allocator<V> > class VerticesSetT>
    static VertexType GetVertexFromIntersectionNotIn(const SimplexT *a, const SimplexT *b, const VerticesSetT<VertexType> &verticesSet)
    {
        return GetVertexFromIntersectionNotIn(*a, *b, verticesSet);
    }

    template <template <typename S, typename A = std::allocator<S> > class SimplexListT>
    static int GetSimplexListDimension(const SimplexListT<SimplexT> &simplexList)
    {
        return (simplexList.size() > 0) ? (int)simplexList[0].size() - 1 : 0;
    }

    template <template <typename S, typename A = std::allocator<S> > class SimplexListT>
    static int GetSimplexListDimension(const SimplexListT<SimplexT *> &simplexPtrList)
    {
        return (simplexPtrList.size() > 0) ? (int)simplexPtrList[0]->size() - 1 : 0;
    }

    template <template <typename S, typename A = std::allocator<S> > class SimplexListT>
    static int GetSimplexListConstantSize(const SimplexListT<SimplexT> &simplexList)
    {
        int size = simplexList[0].size();
        for (typename SimplexListT<SimplexT>::const_iterator i = simplexList.begin(); i != simplexList.end(); i++)
        {
            if ((*i).size() != size)
            {
                return 0;
            }
        }
        return size;
    }

    template <template <typename S, typename A = std::allocator<S> > class SimplexListT>
    static int GetSimplexListConstantSize(const SimplexListT<SimplexT *> &simplexPtrList)
    {
        int size = simplexPtrList[0]->size();
        for (typename SimplexListT<SimplexT *>::const_iterator i = simplexPtrList.begin(); i != simplexPtrList.end(); i++)
        {
            if ((*i)->size() != size)
            {
                return 0;
            }
        }
        return size;
    }

private:
    template <template <typename S, typename A = std::allocator<S> > class SimplexListT>
    void GenerateProperFaces(int currentDim, int targetDim, int firstIndex, SimplexT face, SimplexListT<SimplexT> &faces)
    {
        face.push_back((*this)[firstIndex]);
        if (currentDim == targetDim)
        {
            faces.push_back(face);
        }
        else
        {
            while (++firstIndex < this->size())
            {
                GenerateProperFaces(currentDim + 1, targetDim, firstIndex, face, faces);
            }
        }
    }

public:
    template <template <typename S, typename A = std::allocator<S> > class SimplexListT>
    void GenerateProperFaces(SimplexListT<SimplexT> &faces)
    {
        SimplexT face;
        for (int targetDim = 0; targetDim < this->size() - 1; targetDim++)
        {
            for (int firstIndex = 0; firstIndex < this->size(); firstIndex++)
            {
                GenerateProperFaces(0, targetDim, firstIndex, face, faces);
            }
        }
    }

    template <template <typename S, typename A = std::allocator<S> > class SimplexListT>
    void AddMissingProperFaces(SimplexListT<SimplexT> &faces)
    {
        // generating all faces
        SimplexListT<SimplexT> newFaces;
        GenerateProperFaces(newFaces);

        // adding only those tha have not been yet added
        for (typename SimplexListT<SimplexT>::iterator i = newFaces.begin(); i != newFaces.end(); i++)
        {
            if (std::find(faces.begin(), faces.end(), (*i)) == faces.end())
            {
                faces.push_back(*i);
            }
        }
    }

    static bool SortBySize(const SimplexT &a, const SimplexT &b)
    {
        return (a.size() < b.size());
    }
};

////////////////////////////////////////////////////////////////////////////////

template <typename VertexType>
bool operator==(const SimplexT<VertexType> &a, const SimplexT<VertexType> &b)
{
    if (a.size() != b.size())
    {
        return false;
    }
    typename SimplexT<VertexType>::const_iterator i = a.begin();
    typename SimplexT<VertexType>::const_iterator j = b.begin();
    while (i != a.end())
    {
        if ((*i) != (*j)) return false;
        i++;
        j++;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

#endif	/* SIMPLEXT_HPP */

