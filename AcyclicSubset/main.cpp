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

    MPITest::Test(argc, argv);
//    Tests::TestFromCommandLine(argc, argv);

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
