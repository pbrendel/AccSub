/* 
 * File:   AccSubAlgorithmType.h
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 */

#ifndef ACCSUBALGORITHMTYPE_H
#define	ACCSUBALGORITHMTYPE_H

class AccSubAlgorithmType
{
    int type;
    
public:

    static const int AccSub = 0;
    static const int AccSubIG = 1;
    static const int AccSubST = 2;

    AccSubAlgorithmType()
    {
        type = 0;
    }

    AccSubAlgorithmType(const AccSubAlgorithmType &a)
    {
        type = a.type;
    }

    AccSubAlgorithmType(int t)
    {
        type = t;
    }

    operator int()
    {
        return type;
    }

    bool operator==(const AccSubAlgorithmType &a)
    {
        return (type == a.type);
    }

    bool operator==(int t)
    {
        return (type == t);
    }
};

#endif	/* ACCSUBALGORITHMTYPE_H */

