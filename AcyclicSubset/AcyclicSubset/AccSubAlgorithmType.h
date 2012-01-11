/* 
 * File:   AccSubAlgorithmType.h
 * Author: Piotr Brendel
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

