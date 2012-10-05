/*
 * File:   IntersectionFlagsSet.hpp
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 */

#ifndef INTERSECTIONFLAGSSET_HPP
#define	INTERSECTIONFLAGSSET_HPP

#include <string>
#include <set>
#include <algorithm>
#include <cassert>

////////////////////////////////////////////////////////////////////////////////

template <typename T>
T tmpinc(T a)
{
    return ++a;
}

template <typename T>
T tmpdec(T a)
{
    return --a;
}

////////////////////////////////////////////////////////////////////////////////

template <typename T>
class IntersectionFlagsSet
{

    bool negated;
    std::set<T> flagsSet;

public:

    IntersectionFlagsSet()
    {
        negated = false;
    }

    IntersectionFlagsSet(const IntersectionFlagsSet<T> &set)
    {
        (*this) = set;
    }

    IntersectionFlagsSet(int a)
    {
        (*this) = a;
    }

    IntersectionFlagsSet &operator=(const IntersectionFlagsSet<T> &set)
    {
        negated = set.negated;
        flagsSet = set.flagsSet;
        return (*this);
    }

    IntersectionFlagsSet &operator=(int a)
    {
        negated = false;
        flagsSet.clear();
        SetFromFlags(flagsSet, a);
        return (*this);
    }

    IntersectionFlagsSet &operator|=(const IntersectionFlagsSet &set)
    {
        std::set<T> result;
        if (!negated && !set.negated)
        {
            std::set_union(flagsSet.begin(), flagsSet.end(), set.flagsSet.begin(), set.flagsSet.end(), std::inserter(result, result.begin()));
        }
        else if (negated && set.negated)
        {
            std::set_intersection(flagsSet.begin(), flagsSet.end(), set.flagsSet.begin(), set.flagsSet.end(), std::inserter(result, result.begin()));
        }
        else if (!negated && set.negated)
        {
            negated = true;
            std::set_difference(set.flagsSet.begin(), set.flagsSet.end(), flagsSet.begin(), flagsSet.end(), std::inserter(result, result.begin()));
        }
        else if (negated && !set.negated)
        {
            std::set_difference(flagsSet.begin(), flagsSet.end(), set.flagsSet.begin(), set.flagsSet.end(), std::inserter(result, result.begin()));
        }
        else
        {
            assert(false);
        }
        flagsSet = result;
        return (*this);
    }

    IntersectionFlagsSet &operator&=(const IntersectionFlagsSet &set)
    {
        std::set<T> result;
        if (!negated && !set.negated)
        {
            std::set_intersection(flagsSet.begin(), flagsSet.end(), set.flagsSet.begin(), set.flagsSet.end(), std::inserter(result, result.begin()));
        }
        else if (negated && set.negated)
        {
            std::set_union(flagsSet.begin(), flagsSet.end(), set.flagsSet.begin(), set.flagsSet.end(), std::inserter(result, result.begin()));
        }
        else if (!negated && set.negated)
        {
            std::set_difference(flagsSet.begin(), flagsSet.end(), set.flagsSet.begin(), set.flagsSet.end(), std::inserter(result, result.begin()));
        }
        else if (negated && !set.negated)
        {
            negated = false;
            std::set_difference(set.flagsSet.begin(), set.flagsSet.end(), flagsSet.begin(), flagsSet.end(), std::inserter(result, result.begin()));
        }
        else
        {
            assert(false);
        }
        flagsSet = result;
        return (*this);
    }

    IntersectionFlagsSet operator~()
    {
        IntersectionFlagsSet set;
        set.flagsSet = flagsSet;
        set.negated = ~negated;
        return set;
    }

public:

    IntersectionFlagsSet &operator<<(int a)
    {
        std::set<T> result;
        std::transform(flagsSet.begin(), flagsSet.end(), std::inserter(result, result.begin()), std::bind1st(std::plus<T>(), T(a)));
        flagsSet = result;
        return (*this);
    }

    IntersectionFlagsSet &operator>>(int a)
    {
        std::set<T> result;
        std::transform(flagsSet.begin(), flagsSet.end(), std::inserter(result, result.begin()), std::bind1st(std::minus<T>(), T(a)));
        while (result.size() > 0 && *result.begin() < 0)
        {
            result.erase(result.begin());
        }
        flagsSet = result;
        return (*this);
    }

private:

    void IncFlags()
    {
        T first = *flagsSet.begin();
        flagsSet.erase(flagsSet.begin());
        while (!flagsSet.empty() && (*flagsSet.begin() - first) == 1)
        {
            first = *flagsSet.begin();
            flagsSet.erase(flagsSet.begin());
        }
        flagsSet.insert(first + 1);
    }

    void DecFlags()
    {
        T first = *flagsSet.begin();
        flagsSet.erase(flagsSet.begin());
        for (T i = 0; i < first; i++)
        {
            flagsSet.insert(i);
        }
    }

public:

    IntersectionFlagsSet &operator++()
    {
        if (flagsSet.empty())
        {
            if (negated)
            {
                // its plus infinity plus one
                // so do nothing
            }
            else
            {
                // zero plus one is one -> zeroth flag set
                flagsSet.insert(0);
            }
        }
        else
        {
            if (negated)
            {
                DecFlags();
            }
            else
            {
                IncFlags();
            }
        }
        return *this;
    }

    IntersectionFlagsSet &operator--()
    {
        if (flagsSet.empty())
        {
            if (negated)
            {
                // its plus infinity minus one
                // so do nothing
            }
            else
            {
                // its zero minus one and since we
                // cannot represent -1 we do nothing
            }
        }
        else
        {
            if (negated)
            {
                IncFlags();
            }
            else
            {
                DecFlags();
            }
        }
        return *this;
    }

    operator int() const
    {
        int max = sizeof(int) * 8;
        int flags = 0;
        for (typename std::set<T>::iterator i = flagsSet.begin(); i != flagsSet.end(); i++)
        {
            if (*i > max)
            {
                break;
            }
            flags |= (1 << (*i));
        }
        return flags;
    }

    operator bool() const
    {
        return (negated || flagsSet.size() > 0);
    }

    static void SetFromFlags(std::set<T> &set, int flags)
    {
        T index = 0;
        while (flags != 0)
        {
            if (flags & 1)
            {
                set.insert(index);
            }
            flags = flags >> 1;
            index++;
        }
    }

    int GetBufferSize() const
    {
        // negated + size + list of flags as ints
        return 2 + flagsSet.size();
    }

    void ReadFromBuffer(int *buffer, int &index)
    {
        negated = (buffer[index++] != 0);
        int size = buffer[index++];
        flagsSet.clear();
        for (int i = 0; i < size; i++)
        {
            flagsSet.insert(T(buffer[index++]));
        }
    }

    void WriteToBuffer(int *buffer, int &index) const
    {
        buffer[index++] = negated ? 1 : 0;
        buffer[index++] = flagsSet.size();
        for (typename std::set<T>::const_iterator i = flagsSet.begin(); i != flagsSet.end(); i++)
        {
            buffer[index++] = *i;
        }
    }

    template <typename U> friend bool operator==(const IntersectionFlagsSet<U> &a, const IntersectionFlagsSet<U> &b);
    template <typename U> friend bool operator!=(const IntersectionFlagsSet<U> &a, const IntersectionFlagsSet<U> &b);
    template <typename U> friend bool operator<(const IntersectionFlagsSet<U> &a, const IntersectionFlagsSet<U> &b);
    template <typename U> friend bool operator>(const IntersectionFlagsSet<U> &a, const IntersectionFlagsSet<U> &b);
    template <typename U> friend bool operator==(const IntersectionFlagsSet<U> &a, int b);
    template <typename U> friend bool operator!=(const IntersectionFlagsSet<U> &a, int b);
    template <typename U> friend IntersectionFlagsSet<U> operator|(const IntersectionFlagsSet<U> &a, const IntersectionFlagsSet<U> &b);
    template <typename U> friend IntersectionFlagsSet<U> operator&(const IntersectionFlagsSet<U> &a, const IntersectionFlagsSet<U> &b);
    template <typename U> friend IntersectionFlagsSet<U> operator|(const IntersectionFlagsSet<U> &a, int b);
    template <typename U> friend IntersectionFlagsSet<U> operator&(const IntersectionFlagsSet<U> &a, int b);
    template <typename U> friend std::ostream &operator<<(std::ostream &str, const IntersectionFlagsSet<U> &a);
    template <typename U> friend std::istream &operator>>(std::istream &str, IntersectionFlagsSet<U> &a);
};

////////////////////////////////////////////////////////////////////////////////

template <typename T>
bool operator==(const IntersectionFlagsSet<T> &a, const IntersectionFlagsSet<T> &b)
{
    if (a.flagsSet.size() != b.flagsSet.size())
    {
        return false;
    }
    return std::equal(a.flagsSet.begin(), a.flagsSet.end(), b.flagsSet.begin());
}

template <typename T>
bool operator!=(const IntersectionFlagsSet<T> &a, const IntersectionFlagsSet<T> &b)
{
    if (a.flagsSet.size() != b.flagsSet.size())
    {
        return true;
    }
    return !std::equal(a.flagsSet.begin(), a.flagsSet.end(), b.flagsSet.begin());
}

template <typename T>
bool operator<(const IntersectionFlagsSet<T> &a, const IntersectionFlagsSet<T> &b)
{
    typename std::set<T>::reverse_iterator ia = a.flagsSet.rbegin();
    typename std::set<T>::reverse_iterator ib = b.flagsSet.rbegin();

    while (ia != a.flagsSet.rend() && ib != b.flagsSet.rend())
    {
        if (*ia == *ib)
        {
            ia++;
            ib++;
        }
        else
        {
            return (*ia < *ib);
        }
    }
    return (ib != b.flagsSet.rend());
}

template <typename T>
bool operator>(const IntersectionFlagsSet<T> &a, const IntersectionFlagsSet<T> &b)
{
    typename std::set<T>::reverse_iterator ia = a.flagsSet.rbegin();
    typename std::set<T>::reverse_iterator ib = b.flagsSet.rbegin();

    while (ia != a.flagsSet.rend() && ib != b.flagsSet.rend())
    {
        if (*ia == *ib)
        {
            ia++;
            ib++;
        }
        else
        {
            return (*ia > *ib);
        }
    }
    return (ia != a.flagsSet.rend());
}

template <typename T>
bool operator==(const IntersectionFlagsSet<T> &a, int b)
{
    std::set<T> flagsSet;
    IntersectionFlagsSet<T>::SetFromFlags(flagsSet, b);
    if (a.flagsSet.size() != flagsSet.size())
    {
        return false;
    }
    return std::equal(a.flagsSet.begin(), a.flagsSet.end(), flagsSet.begin());
}

template <typename T>
bool operator!=(const IntersectionFlagsSet<T> &a, int b)
{
    std::set<T> flagsSet;
    IntersectionFlagsSet<T>::SetFromFlags(flagsSet, b);
    if (a.flagsSet.size() != flagsSet.size())
    {
        return true;
    }
    return !std::equal(a.flagsSet.begin(), a.flagsSet.end(), flagsSet.begin());
}

template <typename T>
IntersectionFlagsSet<T> operator|(const IntersectionFlagsSet<T> &a, const IntersectionFlagsSet<T> &b)
{
    IntersectionFlagsSet<T> c;
    if (!a.negated && !b.negated)
    {
        std::set_union(a.flagsSet.begin(), a.flagsSet.end(), b.flagsSet.begin(), b.flagsSet.end(), std::inserter(c.flagsSet, c.flagsSet.begin()));
    }
    else if (a.negated && b.negated)
    {
        std::set_intersection(a.flagsSet.begin(), a.flagsSet.end(), b.flagsSet.begin(), b.flagsSet.end(), std::inserter(c.flagsSet, c.flagsSet.begin()));
    }
    else if (!a.negated && b.negated)
    {
        c.negated = true;
        std::set_difference(b.flagsSet.begin(), b.flagsSet.end(), a.flagsSet.begin(), a.flagsSet.end(), std::inserter(c.flagsSet, c.flagsSet.begin()));
    }
    else if (a.negated && !b.negated)
    {
        std::set_difference(a.flagsSet.begin(), a.flagsSet.end(), b.flagsSet.begin(), b.flagsSet.end(), std::inserter(c.flagsSet, c.flagsSet.begin()));
    }
    else
    {
        assert(false);
    }
    return c;
}

template <typename T>
IntersectionFlagsSet<T> operator&(const IntersectionFlagsSet<T> &a, const IntersectionFlagsSet<T> &b)
{
    IntersectionFlagsSet<T> c;
    if (!a.negated && !b.negated)
    {
        std::set_intersection(a.flagsSet.begin(), a.flagsSet.end(), b.flagsSet.begin(), b.flagsSet.end(), std::inserter(c.flagsSet, c.flagsSet.begin()));
    }
    else if (a.negated && b.negated)
    {
        std::set_union(a.flagsSet.begin(), a.flagsSet.end(), b.flagsSet.begin(), b.flagsSet.end(), std::inserter(c.flagsSet, c.flagsSet.begin()));
    }
    else if (!a.negated && b.negated)
    {
        std::set_difference(a.flagsSet.begin(), a.flagsSet.end(), b.flagsSet.begin(), b.flagsSet.end(), std::inserter(c.flagsSet, c.flagsSet.begin()));
    }
    else if (a.negated && !b.negated)
    {
        c.negated = false;
        std::set_difference(b.flagsSet.begin(), b.flagsSet.end(), a.flagsSet.begin(), a.flagsSet.end(), std::inserter(c.flagsSet, c.flagsSet.begin()));
    }
    else
    {
        assert(false);
    }
    return c;
}

template <typename T>
IntersectionFlagsSet<T> operator|(const IntersectionFlagsSet<T> &a, int b)
{
    std::set<T> flagsSet;
    IntersectionFlagsSet<T>::SetFromFlags(flagsSet, b);
    IntersectionFlagsSet<T> c;
    if (!a.negated)
    {
        std::set_union(a.flagsSet.begin(), a.flagsSet.end(), flagsSet.begin(), flagsSet.end(), std::inserter(c.flagsSet, c.flagsSet.begin()));
    }
    else
    {
        std::set_difference(a.flagsSet.begin(), a.flagsSet.end(), flagsSet.begin(), flagsSet.end(), std::inserter(c.flagsSet, c.flagsSet.begin()));
    }
    return c;
}

template <typename T>
IntersectionFlagsSet<T> operator&(const IntersectionFlagsSet<T> &a, int b)
{
    std::set<T> flagsSet;
    IntersectionFlagsSet<T>::SetFromFlags(flagsSet, b);
    IntersectionFlagsSet<T> c;
    if (!a.negated)
    {
        std::set_intersection(a.flagsSet.begin(), a.flagsSet.end(), flagsSet.begin(), flagsSet.end(), std::inserter(c.flagsSet, c.flagsSet.begin()));
    }
    else
    {
        std::set_difference(flagsSet.begin(), flagsSet.end(), a.flagsSet.begin(), a.flagsSet.end(), std::inserter(c.flagsSet, c.flagsSet.begin()));
    }
    return c;
}

template <typename T>
std::ostream &operator<<(std::ostream &str, const IntersectionFlagsSet<T> &a)
{
    T last = -1;
    for (typename std::set<T>::reverse_iterator i = a.flagsSet.rbegin(); i != a.flagsSet.rend(); i++)
    {
        if (last != -1)
        {
            for (int j = *i; j < last - 1; j++)
            {
                str<<0;
            }
        }
        last = *i;
        str<<1;
    }
    for (int j = 0; j < last; j++)
    {
        str<<0;
    }
    return str;
}

template <typename T>
std::istream &operator>>(std::istream &str, IntersectionFlagsSet<T> &a)
{
    a.flagsSet.clear();
    std::string s;
    str>>s;
    for (int i = 0; i < s.size(); i++)
    {
        if (s[i] == '1')
        {
            a.flagsSet.insert((T)i);
        }
        else
        {
            assert(s[i] == '0');
        }
    }
    return str;
}

////////////////////////////////////////////////////////////////////////////////

#endif /* INTERSECTIONFLAGSSET_HPP */
