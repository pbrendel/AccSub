/*
 * File:   main.cpp
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 *
 * todo:
 * - AccInfo and IntersectionInfo as maximal simplex list:
 *   - AccInfoSimplexMF
 *   - IntersectionInfoMF
 * - AccInfo and IntersectionInfo storing no data (computing it each time):
 *   - AccInfoNone
 *   - IntersectionInfoNone
 * - IntersectionFlags as set - IntersectionFlagsSet
 * - full tests of AccTest classes in maximal simplex list case
 *
 */

#include "Helpers/Tests.h"
#include <iostream>

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{               
    try
    {
#ifdef USE_MPI
    Tests::MPITestFromCommandLine(argc, argv);
#else
    Tests::TestFromCommandLine(argc, argv);
#endif
    }
    catch (std::string s)
    {
        std::cerr<<"something went wrong: "<<s<<std::endl;
        return 1;
    }
            
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// eof
