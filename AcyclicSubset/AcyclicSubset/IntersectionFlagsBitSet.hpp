#ifndef _INTERSECTION_FLAGS_BIT_SET_HPP_
#define	_INTERSECTION_FLAGS_BIT_SET_HPP_

#include <assert.h>

////////////////////////////////////////////////////////////////////////////////

template <int N>
class IntersectionFlagsBitSet
{
    int data[N];

public:

    IntersectionFlagsBitSet()
    {
        for (int i = 0; i < N; i++) data[i] = 0;
    }

    IntersectionFlagsBitSet(const IntersectionFlagsBitSet<N> &set)
    {
        (*this) = set;
    }

    IntersectionFlagsBitSet(int a)
    {
        (*this) = a;
    }

    IntersectionFlagsBitSet<N> &operator=(const IntersectionFlagsBitSet<N> &set)
    {
        for (int i = 0; i < N; i++) data[i] = set.data[i];
        return (*this);
    }

    IntersectionFlagsBitSet<N> &operator=(int a)
    {
        data[0] = a;
        for (int i = 1; i < N; i++) data[i] = 0;
        return (*this);
    }

    IntersectionFlagsBitSet<N> &operator|=(const IntersectionFlagsBitSet<N> &set)
    {
        for (int i = 0; i < N; i++) data[i] |= set.data[i];
        return (*this);
    }

    IntersectionFlagsBitSet<N> &operator&=(const IntersectionFlagsBitSet<N> &set)
    {
        for (int i = 0; i < N; i++) data[i] &= set.data[i];
        return (*this);
    }

    IntersectionFlagsBitSet<N> &operator~()
    {
        for (int i = 0; i < N; i++) data[i] = ~data[i];
        return (*this);
    }

    IntersectionFlagsBitSet<N> &operator<<(int a)
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

    IntersectionFlagsBitSet<N> &operator>>(int a)
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

    template <int M> friend bool operator==(const IntersectionFlagsBitSet<M> &a, const IntersectionFlagsBitSet<M> &b);
    template <int M> friend bool operator!=(const IntersectionFlagsBitSet<M> &a, const IntersectionFlagsBitSet<M> &b);
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

template <int N>
bool operator==(const IntersectionFlagsBitSet<N> &a, const IntersectionFlagsBitSet<N> &b)
{
    for (int i = 0; i < N; i++)
        if (a.data[i] != b.data[i]) return false;
    return true;
}

template <int N>
bool operator!=(const IntersectionFlagsBitSet<N> &a, const IntersectionFlagsBitSet<N> &b)
{
    for (int i = 0; i < N; i++)
        if (a.data[i] != b.data[i]) return true;
    return false;
}

template <int N>
inline bool operator==(const IntersectionFlagsBitSet<N> &a, int b)
{
    if (a.data[0] != b) return false;
    for (int i = 1; i < N; i++)
        if (a.data[i] != 0) return false;
    return true;
}

template <int N>
inline bool operator!=(const IntersectionFlagsBitSet<N> &a, int b)
{
    if (a.data[0] != b) return true;
    for (int i = 1; i < N; i++)
        if (a.data[i] != 0) return true;
    return false;
}

template <int N>
IntersectionFlagsBitSet<N> operator|(const IntersectionFlagsBitSet<N> &a, const IntersectionFlagsBitSet<N> &b)
{
    IntersectionFlagsBitSet<N> c;
    for (int i = 0; i < N; i++) c.data[i] = a.data[i] | b.data[i];
    return c;
}

template <int N>
IntersectionFlagsBitSet<N> operator&(const IntersectionFlagsBitSet<N> &a, const IntersectionFlagsBitSet<N> &b)
{
    IntersectionFlagsBitSet<N> c;
    for (int i = 0; i < N; i++) c.data[i] = a.data[i] & b.data[i];
    return c;
}

template <int N>
IntersectionFlagsBitSet<N> operator|(const IntersectionFlagsBitSet<N> &a, int b)
{
    IntersectionFlagsBitSet<N> c;
    c.data[0] = a.data[0] | b;
    return c;
}

template <int N>
IntersectionFlagsBitSet<N> operator&(const IntersectionFlagsBitSet<N> &a, int b)
{
    IntersectionFlagsBitSet<N> c;
    c.data[0] = a.data[0] & b;
    return c;
}

template <int N>
std::ostream &operator<<(std::ostream &str, const IntersectionFlagsBitSet<N> &a)
{
    for (int i = N - 1; i >= 0; i--) str<<a.data[i];
    return str;
}

template <int N>
std::istream &operator>>(std::istream &str, IntersectionFlagsBitSet<N> &a)
{
    for (int i = N - 1; i >= 0; i--) str>>a.data[i];
    return str;
}

////////////////////////////////////////////////////////////////////////////////

#endif // _INTERSECTION_FLAGS_BIT_SET_HPP_
