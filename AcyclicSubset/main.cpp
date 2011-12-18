/*
 * File:   main.cpp
 * Author: Piotr Brendel
 */

#include "Helpers/Tests.h"

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
