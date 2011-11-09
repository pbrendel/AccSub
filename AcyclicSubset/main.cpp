#include "Helpers/Utils.h"
#include "Helpers/Tests.h"
#include "Helpers/MPITest.h"

////////////////////////////////////////////////////////////////////////////////
// co mozna zrobic szybciej?
// - hash vertex -> index
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{               
    try
    {
#ifdef USE_LOG
        Log::Init("log.txt");
#endif

#ifdef USE_MPI
    MPITest::Test(argc, argv);
#else
    Tests::TestFromCommandLine(argc, argv);
#endif

#ifdef USE_LOG
        Log::Close();
#endif
    }
    catch (std::string s)
    {
        std::cerr<<"Something went wrong: "<<s<<std::endl;
        return 1;
    }
            
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// eof
