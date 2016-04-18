/*
 * File:   IntersectionFlagsBitSet.hpp
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 */

#ifndef INTERSECTIONFLAGSBITSET_HPP
#define	INTERSECTIONFLAGSBITSET_HPP

#include <cassert>

////////////////////////////////////////////////////////////////////////////////

template <int N>
class Min1
{
public:
    enum
    {
        value = N,
    };
};

template <>
class Min1<0>
{
public:
    enum
    {
        value = 1,
    };
};

////////////////////////////////////////////////////////////////////////////////

template <int D>
class IntersectionFlagsBitSet
{
public:

    enum
    {
        dim = D,
        size = Min1<(1 << (D + 1)) / (8 * sizeof(unsigned int))>::value,
    };

private:

    unsigned int data[size];

public:

    IntersectionFlagsBitSet()
    {
        for (int i = 0; i < size; i++) data[i] = 0;
    }

    IntersectionFlagsBitSet(const IntersectionFlagsBitSet<D> &set)
    {
        (*this) = set;
    }

    IntersectionFlagsBitSet(int a)
    {
        (*this) = a;
    }

    IntersectionFlagsBitSet &operator=(const IntersectionFlagsBitSet &set)
    {
        for (int i = 0; i < size; i++) data[i] = set.data[i];
        return (*this);
    }

    IntersectionFlagsBitSet &operator=(int a)
    {
        data[0] = (unsigned int)a;
        for (int i = 1; i < size; i++) data[i] = 0;
        return (*this);
    }

    IntersectionFlagsBitSet &operator|=(const IntersectionFlagsBitSet &set)
    {
        for (int i = 0; i < size; i++) data[i] |= set.data[i];
        return (*this);
    }

    IntersectionFlagsBitSet &operator&=(const IntersectionFlagsBitSet &set)
    {
        for (int i = 0; i < size; i++) data[i] &= set.data[i];
        return (*this);
    }

    IntersectionFlagsBitSet operator~()
    {
        IntersectionFlagsBitSet set;
        for (int i = 0; i < size; i++) set.data[i] = ~data[i];
        return set;
    }

    IntersectionFlagsBitSet &operator<<(int a)
    {
        assert(a >= 0);
        while (a-- > 0)
        {
            for (int i = size - 1; i >= 0; i--)
            {
                if (i > 0) data[i] = (data[i] << 1) | ((data[i - 1] & (1 << 31)) ? 1 : 0);
                else data[i] = data[i] << 1;
            }
        }
        return (*this);
    }

    IntersectionFlagsBitSet &operator>>(int a)
    {
        assert(a >= 0);
        while (a-- > 0)
        {
            for (int i = 0; i < size; i++)
            {
                if (i < size - 1) data[i] = (data[i] >> 1) | ((data[i + 1] & 1) ? (1 << 31) : 0);
                else data[i] = data[i] >> 1;
            }
        }
        return (*this);
    }
    
    IntersectionFlagsBitSet &operator++()
    {
        unsigned int minus1 = (unsigned int)-1;
        int index = 0;
        while (index < size && data[index] == minus1)
        {
            data[index] = 0;
            index++;
        }
        if (index < size)
        {
            data[index]++;
        }
        return *this;
    }

    IntersectionFlagsBitSet &operator--()
    {
        unsigned int minus1 = (unsigned int)-1;
        int index = 0;
        while (index < size - 1 && data[index] == 0)
        {
            data[index] = minus1;
            index++;
        }
        if (index == size - 1 && data[index] == 0)
        {
            for (int i = 0; i < size; i++)
            {
                data[i] = minus1;
            }
        }
        else
        {
            if (index < size)
            {
                data[index]--;
            }
        }
        return *this;        
    }
    
    operator int() const
    {
        return data[0];
    }

    operator bool() const
    {
        for (int i = 0; i < size; i++)
        {
            if (data[i]) return true;
        }
        return false;
    }
    
    int GetBufferSize() const
    {
        return size;
    }

    void ReadFromBuffer(int *buffer, int &index)
    {
        for (int i = 0; i < size; i++)
        {
            data[i] = buffer[index++];
        }
    }

    void WriteToBuffer(int *buffer, int &index) const
    {
        for (int i = 0; i < size; i++)
        {
            buffer[index++] = data[i];
        }
    }

    template <int M> friend bool operator==(const IntersectionFlagsBitSet<M> &a, const IntersectionFlagsBitSet<M> &b);
    template <int M> friend bool operator!=(const IntersectionFlagsBitSet<M> &a, const IntersectionFlagsBitSet<M> &b);
    template <int M> friend bool operator<(const IntersectionFlagsBitSet<M> &a, const IntersectionFlagsBitSet<M> &b);
    template <int M> friend bool operator>(const IntersectionFlagsBitSet<M> &a, const IntersectionFlagsBitSet<M> &b);
    template <int M> friend bool operator==(const IntersectionFlagsBitSet<M> &a, int b);
    template <int M> friend bool operator!=(const IntersectionFlagsBitSet<M> &a, int b);
    template <int M> friend IntersectionFlagsBitSet<M> operator|(const IntersectionFlagsBitSet<M> &a, const IntersectionFlagsBitSet<M> &b);
    template <int M> friend IntersectionFlagsBitSet<M> operator&(const IntersectionFlagsBitSet<M> &a, const IntersectionFlagsBitSet<M> &b);
    template <int M> friend IntersectionFlagsBitSet<M> operator|(const IntersectionFlagsBitSet<M> &a, int b);
    template <int M> friend IntersectionFlagsBitSet<M> operator&(const IntersectionFlagsBitSet<M> &a, int b);
    template <int M> friend std::ostream &operator<<(std::ostream &str, const IntersectionFlagsBitSet<M> &a);
    template <int M> friend std::istream &operator>>(std::istream &str, IntersectionFlagsBitSet<M> &a);
};

////////////////////////////////////////////////////////////////////////////////

template <int D>
bool operator==(const IntersectionFlagsBitSet<D> &a, const IntersectionFlagsBitSet<D> &b)
{
    for (int i = 0; i < IntersectionFlagsBitSet<D>::size; i++)
        if (a.data[i] != b.data[i]) return false;
    return true;
}

template <int D>
bool operator!=(const IntersectionFlagsBitSet<D> &a, const IntersectionFlagsBitSet<D> &b)
{
    for (int i = 0; i < IntersectionFlagsBitSet<D>::size; i++)
        if (a.data[i] != b.data[i]) return true;
    return false;
}

template <int D>
bool operator<(const IntersectionFlagsBitSet<D> &a, const IntersectionFlagsBitSet<D> &b)
{
    for (int i = 0; i < IntersectionFlagsBitSet<D>::size; i++)
        if (a.data[i] < b.data[i]) return true;
        else if (a.data[i] > b.data[i]) return false;
    return false;
}

template <int D>
bool operator>(const IntersectionFlagsBitSet<D> &a, const IntersectionFlagsBitSet<D> &b)
{
    for (int i = 0; i < IntersectionFlagsBitSet<D>::size; i++)
        if (a.data[i] > b.data[i]) return true;
        else if (a.data[i] < b.data[i]) return false;
    return false;
}

template <int D>
inline bool operator==(const IntersectionFlagsBitSet<D> &a, int b)
{
    if (a.data[0] != (unsigned int)b) return false;
    for (int i = 1; i < IntersectionFlagsBitSet<D>::size; i++)
        if (a.data[i] != 0) return false;
    return true;
}

template <int D>
inline bool operator!=(const IntersectionFlagsBitSet<D> &a, int b)
{
    if (a.data[0] != (unsigned int)b) return true;
    for (int i = 1; i < IntersectionFlagsBitSet<D>::size; i++)
        if (a.data[i] != 0) return true;
    return false;
}

template <int D>
IntersectionFlagsBitSet<D> operator|(const IntersectionFlagsBitSet<D> &a, const IntersectionFlagsBitSet<D> &b)
{
    IntersectionFlagsBitSet<D> c;
    for (int i = 0; i < IntersectionFlagsBitSet<D>::size; i++) c.data[i] = a.data[i] | b.data[i];
    return c;
}

template <int D>
IntersectionFlagsBitSet<D> operator&(const IntersectionFlagsBitSet<D> &a, const IntersectionFlagsBitSet<D> &b)
{
    IntersectionFlagsBitSet<D> c;
    for (int i = 0; i < IntersectionFlagsBitSet<D>::size; i++) c.data[i] = a.data[i] & b.data[i];
    return c;
}

template <int D>
IntersectionFlagsBitSet<D> operator|(const IntersectionFlagsBitSet<D> &a, int b)
{
    IntersectionFlagsBitSet<D> c;
    c.data[0] = a.data[0] | (unsigned int)b;
    return c;
}

template <int D>
IntersectionFlagsBitSet<D> operator&(const IntersectionFlagsBitSet<D> &a, int b)
{
    IntersectionFlagsBitSet<D> c;
    c.data[0] = a.data[0] & (unsigned int)b;
    return c;
}

template <int D>
std::ostream &operator<<(std::ostream &str, const IntersectionFlagsBitSet<D> &a)
{
    for (int i = IntersectionFlagsBitSet<D>::size - 1; i >= 0; i--) str<<a.data[i];
    return str;
}

template <int D>
std::istream &operator>>(std::istream &str, IntersectionFlagsBitSet<D> &a)
{
    for (int i = IntersectionFlagsBitSet<D>::size - 1; i >= 0; i--) str>>a.data[i];
    return str;
}

////////////////////////////////////////////////////////////////////////////////

#endif /* INTERSECTIONFLAGSBITSET_HPP */
