/*
 * File:   IntersectionFlagsSet.h
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 */

#ifndef INTERSECTIONFLAGSSET_H
#define	INTERSECTIONFLAGSSET_H

#include <cassert>

////////////////////////////////////////////////////////////////////////////////
// todo!!!

class IntersectionFlagsSet
{

    bool negated;
    

public:

    IntersectionFlagsSet()
    {
    }

    IntersectionFlagsSet(const IntersectionFlagsSet &set)
    {
    }

    IntersectionFlagsSet(unsigned int a)
    {
        (*this) = a;
    }

    IntersectionFlagsSet &operator=(const IntersectionFlagsSet &set)
    {
    }

    IntersectionFlagsSet &operator=(unsigned int a)
    {
        return (*this);
    }

    IntersectionFlagsSet &operator|=(const IntersectionFlagsSet &set)
    {
        return (*this);
    }

    IntersectionFlagsSet &operator&=(const IntersectionFlagsSet &set)
    {
        return (*this);
    }

    IntersectionFlagsSet &operator~()
    {
        return (*this);
    }

    IntersectionFlagsSet &operator<<(int a)
    {
        return (*this);
    }

    IntersectionFlagsSet &operator>>(int a)
    {
        return (*this);
    }
    
    operator unsigned int()
    {
        return 0;
    }

    friend bool operator==(const IntersectionFlagsSet &a, const IntersectionFlagsSet &b);
    friend bool operator!=(const IntersectionFlagsSet &a, const IntersectionFlagsSet &b);
    friend bool operator==(const IntersectionFlagsSet &a, unsigned int b);
    friend bool operator!=(const IntersectionFlagsSet &a, unsigned int b);
    friend IntersectionFlagsSet operator|(const IntersectionFlagsSet &a, const IntersectionFlagsSet &b);
    friend IntersectionFlagsSet operator&(const IntersectionFlagsSet &a, const IntersectionFlagsSet &b);
    friend IntersectionFlagsSet operator|(const IntersectionFlagsSet &a, unsigned int b);
    friend IntersectionFlagsSet operator&(const IntersectionFlagsSet &a, unsigned int b);
    friend std::ostream &operator<<(std::ostream &str, const IntersectionFlagsSet &a);
    friend std::istream &operator>>(std::istream &str, IntersectionFlagsSet &a);
};

////////////////////////////////////////////////////////////////////////////////

inline bool operator==(const IntersectionFlagsSet &a, const IntersectionFlagsSet &b)
{
    return true;
}

inline bool operator!=(const IntersectionFlagsSet &a, const IntersectionFlagsSet &b)
{
    return false;
}

inline bool operator==(const IntersectionFlagsSet &a, unsigned int b)
{
    return true;
}

inline bool operator!=(const IntersectionFlagsSet &a, unsigned int b)
{
    return false;
}

inline IntersectionFlagsSet operator|(const IntersectionFlagsSet &a, const IntersectionFlagsSet &b)
{
    return a;
}

inline IntersectionFlagsSet operator&(const IntersectionFlagsSet &a, const IntersectionFlagsSet &b)
{
    return a;
}

inline IntersectionFlagsSet operator|(const IntersectionFlagsSet &a, unsigned int b)
{
    return a;
}

inline IntersectionFlagsSet operator&(const IntersectionFlagsSet &a, unsigned int b)
{
    return a;
}

inline std::ostream &operator<<(std::ostream &str, const IntersectionFlagsSet &a)
{
    return str;
}

inline std::istream &operator>>(std::istream &str, IntersectionFlagsSet &a)
{
    return str;
}

////////////////////////////////////////////////////////////////////////////////

#endif /* INTERSECTIONFLAGSSET_H */
