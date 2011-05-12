#include "Helpers/Utils.h"
#include "Helpers/Tests.h"
#include "Helpers/MPITest.h"

////////////////////////////////////////////////////////////////////////////////
// co mozna zrobic szybciej?
// - hash vertex -> index
////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG_MEMORY_OPERATORS

void UpdateAllocatedMemory(int sz)
{
    const int infoInterval = 10 * 1024 * 1024;
    static int nextInfo = infoInterval;
    static int currentAllocation = 0;

    currentAllocation += sz;
    if (currentAllocation > nextInfo)
    {
        nextInfo += infoInterval;
        std::cout<<"allocated: "<<(currentAllocation / infoInterval)<<"0 megabytes"<<std::endl;
    }
}

void* operator new(size_t sz)
{
    UpdateAllocatedMemory(sz);
    void* m = malloc(sz);
    return m;
}

void operator delete(void* m)
{
    free(m);
}

#endif

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
