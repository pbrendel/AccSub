/*
 * File:   IntersectionFlagsBitSet.h
 * Author: Piotr Brendel
 */

#ifndef INTERSECTIONFLAGSBITSET_H
#define	INTERSECTIONFLAGSBITSET_H

#include <assert.h>

////////////////////////////////////////////////////////////////////////////////

const int N = 4;

class IntersectionFlagsBitSet
{
    int data[N];

public:

    IntersectionFlagsBitSet()
    {
        for (int i = 0; i < N; i++) data[i] = 0;
    }

    IntersectionFlagsBitSet(const IntersectionFlagsBitSet &set)
    {
        (*this) = set;
    }

    IntersectionFlagsBitSet(int a)
    {
        (*this) = a;
    }

    IntersectionFlagsBitSet &operator=(const IntersectionFlagsBitSet &set)
    {
        for (int i = 0; i < N; i++) data[i] = set.data[i];
        return (*this);
    }

    IntersectionFlagsBitSet &operator=(int a)
    {
        data[0] = a;
        for (int i = 1; i < N; i++) data[i] = 0;
        return (*this);
    }

    IntersectionFlagsBitSet &operator|=(const IntersectionFlagsBitSet &set)
    {
        for (int i = 0; i < N; i++) data[i] |= set.data[i];
        return (*this);
    }

    IntersectionFlagsBitSet &operator&=(const IntersectionFlagsBitSet &set)
    {
        for (int i = 0; i < N; i++) data[i] &= set.data[i];
        return (*this);
    }

    IntersectionFlagsBitSet &operator~()
    {
        for (int i = 0; i < N; i++) data[i] = ~data[i];
        return (*this);
    }

    IntersectionFlagsBitSet &operator<<(int a)
    {
        assert(a >= 0);
        while (a-- > 0)
        {
            for (int i = N - 1; i >= 0; i--)
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
            for (int i = 0; i < N; i++)
            {
                if (i < N - 1) data[i] = (data[i] >> 1) | ((data[i + 1] & 1) ? (1 << 31) : 0);
                else data[i] = data[i] >> 1;
            }
        }
        return (*this);
    }
    
    operator int()
    {
        return data[0];
    }

    friend bool operator==(const IntersectionFlagsBitSet &a, const IntersectionFlagsBitSet &b);
    friend bool operator!=(const IntersectionFlagsBitSet &a, const IntersectionFlagsBitSet &b);
    friend bool operator==(const IntersectionFlagsBitSet &a, int b);
    friend bool operator!=(const IntersectionFlagsBitSet &a, int b);
    friend IntersectionFlagsBitSet operator|(const IntersectionFlagsBitSet &a, const IntersectionFlagsBitSet &b);
    friend IntersectionFlagsBitSet operator&(const IntersectionFlagsBitSet &a, const IntersectionFlagsBitSet &b);
    friend IntersectionFlagsBitSet operator|(const IntersectionFlagsBitSet &a, int b);
    friend IntersectionFlagsBitSet operator&(const IntersectionFlagsBitSet &a, int b);
    friend std::ostream &operator<<(std::ostream &str, const IntersectionFlagsBitSet &a);
    friend std::istream &operator>>(std::istream &str, IntersectionFlagsBitSet &a);
};

////////////////////////////////////////////////////////////////////////////////

inline bool operator==(const IntersectionFlagsBitSet &a, const IntersectionFlagsBitSet &b)
{
    for (int i = 0; i < N; i++)
        if (a.data[i] != b.data[i]) return false;
    return true;
}


inline bool operator!=(const IntersectionFlagsBitSet &a, const IntersectionFlagsBitSet &b)
{
    for (int i = 0; i < N; i++)
        if (a.data[i] != b.data[i]) return true;
    return false;
}


inline bool operator==(const IntersectionFlagsBitSet &a, int b)
{
    if (a.data[0] != b) return false;
    for (int i = 1; i < N; i++)
        if (a.data[i] != 0) return false;
    return true;
}


inline bool operator!=(const IntersectionFlagsBitSet &a, int b)
{
    if (a.data[0] != b) return true;
    for (int i = 1; i < N; i++)
        if (a.data[i] != 0) return true;
    return false;
}


inline IntersectionFlagsBitSet operator|(const IntersectionFlagsBitSet &a, const IntersectionFlagsBitSet &b)
{
    IntersectionFlagsBitSet c;
    for (int i = 0; i < N; i++) c.data[i] = a.data[i] | b.data[i];
    return c;
}


inline IntersectionFlagsBitSet operator&(const IntersectionFlagsBitSet &a, const IntersectionFlagsBitSet &b)
{
    IntersectionFlagsBitSet c;
    for (int i = 0; i < N; i++) c.data[i] = a.data[i] & b.data[i];
    return c;
}


inline IntersectionFlagsBitSet operator|(const IntersectionFlagsBitSet &a, int b)
{
    IntersectionFlagsBitSet c;
    c.data[0] = a.data[0] | b;
    return c;
}


inline IntersectionFlagsBitSet operator&(const IntersectionFlagsBitSet &a, int b)
{
    IntersectionFlagsBitSet c;
    c.data[0] = a.data[0] & b;
    return c;
}

inline std::ostream &operator<<(std::ostream &str, const IntersectionFlagsBitSet &a)
{
    for (int i = N - 1; i >= 0; i--) str<<a.data[i];
    return str;
}

inline std::istream &operator>>(std::istream &str, IntersectionFlagsBitSet &a)
{
    for (int i = N - 1; i >= 0; i--) str>>a.data[i];
    return str;
}

////////////////////////////////////////////////////////////////////////////////

#endif /* INTERSECTIONFLAGSBITSET_H */
