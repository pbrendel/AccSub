#ifndef _INTERSECTION_FLAGS_SET_H_
#define	_INTERSECTION_FLAGS_SET_H_

#include <assert.h>

////////////////////////////////////////////////////////////////////////////////

const int N = 4;

class IntersectionFlagsSet
{
    int data[N];

public:

    IntersectionFlagsSet()
    {
        for (int i = 0; i < N; i++) data[i] = 0;
    }

    IntersectionFlagsSet(const IntersectionFlagsSet &set)
    {
        (*this) = set;
    }

    IntersectionFlagsSet(int a)
    {
        (*this) = a;
    }

    IntersectionFlagsSet &operator=(const IntersectionFlagsSet &set)
    {
        for (int i = 0; i < N; i++) data[i] = set.data[i];
        return (*this);
    }

    IntersectionFlagsSet &operator=(int a)
    {
        data[0] = a;
        for (int i = 1; i < N; i++) data[i] = 0;
        return (*this);
    }

    IntersectionFlagsSet &operator|=(const IntersectionFlagsSet &set)
    {
        for (int i = 0; i < N; i++) data[i] |= set.data[i];
        return (*this);
    }

    IntersectionFlagsSet &operator&=(const IntersectionFlagsSet &set)
    {
        for (int i = 0; i < N; i++) data[i] &= set.data[i];
        return (*this);
    }

    IntersectionFlagsSet &operator~()
    {
        for (int i = 0; i < N; i++) data[i] = ~data[i];
        return (*this);
    }

    IntersectionFlagsSet &operator<<(int a)
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

    IntersectionFlagsSet &operator>>(int a)
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

    friend bool operator==(const IntersectionFlagsSet &a, const IntersectionFlagsSet &b);
    friend bool operator!=(const IntersectionFlagsSet &a, const IntersectionFlagsSet &b);
    friend bool operator==(const IntersectionFlagsSet &a, int b);
    friend bool operator!=(const IntersectionFlagsSet &a, int b);
    friend IntersectionFlagsSet operator|(const IntersectionFlagsSet &a, const IntersectionFlagsSet &b);
    friend IntersectionFlagsSet operator&(const IntersectionFlagsSet &a, const IntersectionFlagsSet &b);
    friend IntersectionFlagsSet operator|(const IntersectionFlagsSet &a, int b);
    friend IntersectionFlagsSet operator&(const IntersectionFlagsSet &a, int b);
    friend std::ostream &operator<<(std::ostream &str, const IntersectionFlagsSet &a);
    friend std::istream &operator>>(std::istream &str, IntersectionFlagsSet &a);
};

////////////////////////////////////////////////////////////////////////////////

inline bool operator==(const IntersectionFlagsSet &a, const IntersectionFlagsSet &b)
{
    for (int i = 0; i < N; i++)
        if (a.data[i] != b.data[i]) return false;
    return true;
}


inline bool operator!=(const IntersectionFlagsSet &a, const IntersectionFlagsSet &b)
{
    for (int i = 0; i < N; i++)
        if (a.data[i] != b.data[i]) return true;
    return false;
}


inline bool operator==(const IntersectionFlagsSet &a, int b)
{
    if (a.data[0] != b) return false;
    for (int i = 1; i < N; i++)
        if (a.data[i] != 0) return false;
    return true;
}


inline bool operator!=(const IntersectionFlagsSet &a, int b)
{
    if (a.data[0] != b) return true;
    for (int i = 1; i < N; i++)
        if (a.data[i] != 0) return true;
    return false;
}


inline IntersectionFlagsSet operator|(const IntersectionFlagsSet &a, const IntersectionFlagsSet &b)
{
    IntersectionFlagsSet c;
    for (int i = 0; i < N; i++) c.data[i] = a.data[i] | b.data[i];
    return c;
}


inline IntersectionFlagsSet operator&(const IntersectionFlagsSet &a, const IntersectionFlagsSet &b)
{
    IntersectionFlagsSet c;
    for (int i = 0; i < N; i++) c.data[i] = a.data[i] & b.data[i];
    return c;
}


inline IntersectionFlagsSet operator|(const IntersectionFlagsSet &a, int b)
{
    IntersectionFlagsSet c;
    c.data[0] = a.data[0] | b;
    return c;
}


inline IntersectionFlagsSet operator&(const IntersectionFlagsSet &a, int b)
{
    IntersectionFlagsSet c;
    c.data[0] = a.data[0] & b;
    return c;
}

inline std::ostream &operator<<(std::ostream &str, const IntersectionFlagsSet &a)
{
    for (int i = N - 1; i >= 0; i--) str<<a.data[i];
    return str;
}

inline std::istream &operator>>(std::istream &str, IntersectionFlagsSet &a)
{
    for (int i = N - 1; i >= 0; i--) str>>a.data[i];
    return str;
}

////////////////////////////////////////////////////////////////////////////////

#endif // _INTERSECTION_FLAGS_SET_H_
