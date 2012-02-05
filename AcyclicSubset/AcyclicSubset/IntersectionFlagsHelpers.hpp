/* 
 * File:   IntersectionFlagsHelpers.hpp
 * Author: Piotr Brendel
 */

#ifndef INTERSECTIONFLAGSHELPERS_HPP
#define	INTERSECTIONFLAGSHELPERS_HPP

template <typename IntersectionFlags>
class IntersectionFlagsHelpers
{
public:

    static int GetBufferSize(const IntersectionFlags &intersectionFlags)
    {
        return intersectionFlags.GetBufferSize();
    }

    static void ReadFromBuffer(IntersectionFlags &intersectionFlags, int *buffer, int &index)
    {
        intersectionFlags.ReadFromBuffer(buffer, index);
    }

    static void WriteToBuffer(IntersectionFlags &intersectionFlags, int *buffer, int &index)
    {
        intersectionFlags.WriteToBuffer(buffer, index);
    }
};

template <>
class IntersectionFlagsHelpers<unsigned int>
{
public:

    static int GetBufferSize(const unsigned int &intersectionFlags)
    {
        return 1;
    }

    static void ReadFromBuffer(unsigned int &intersectionFlags, int *buffer, int &index)
    {
        intersectionFlags = (unsigned int)buffer[index++];
    }

    static void WriteToBuffer(const unsigned int &intersectionFlags, int *buffer, int &index)
    {
        buffer[index++] = (int)intersectionFlags;
    }
};

#endif	/* INTERSECTIONFLAGSHELPERS_HPP */

