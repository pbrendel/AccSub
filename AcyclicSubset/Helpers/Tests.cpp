/*
 * File:   Tests.cpp
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 */

#include "Tests.h"
#include "RedHomHelpers.hpp"
#include "Utils.hpp"
#include "SimplexUtils.hpp"
#include "../AcyclicSubset/IncidenceGraphHelpers.hpp"

#include <cassert>

////////////////////////////////////////////////////////////////////////////////

int Tests::inputType = 0;
int Tests::accTestNumber = 0;
std::string Tests::inputFilename = "tests.txt";
int Tests::sortVerts = 0;
int Tests::useAlgebraic = 0;
int Tests::useCoreductions = 0;
int Tests::useAccSub = 0;
int Tests::useAccSubIG = 1;
int Tests::useAccSubST = 0;
int Tests::useAccSubDist = 0;
int Tests::packsCount = 6;
int Tests::distAccSubAlgorithm = 0;
int Tests::processRank = 0;
int Tests::randomPointsCount = 100;
float Tests::randomPointsDiam = 1;
int Tests::randomPointsDim = 4;
float Tests::ripsComplexEpsilon = 0.05f;
int Tests::ripsComplexDim = 4;

////////////////////////////////////////////////////////////////////////////////

void Tests::PrintHelp()
{
    std::cout<<std::endl;
    std::cout<<"usage: AccSub input options"<<std::endl;
    std::cout<<std::endl;
    std::cout<<"input:"<<std::endl;
    std::cout<<"  --i filename - use filename as input ["<<inputFilename<<"]"<<std::endl;
    std::cout<<"  --l filename - use filename as list of inputs ["<<inputFilename<<"]"<<std::endl;
    std::cout<<"  --r count diam s_dim epsilon c_dim - random Rips complex where"<<std::endl;
    std::cout<<"                                       - count - number of points ["<<randomPointsCount<<"]"<<std::endl;
    std::cout<<"                                       - diam - diam of set of random points ["<<randomPointsDiam<<"]"<<std::endl;
    std::cout<<"                                       - s_dim - dimension of space ["<<randomPointsDim<<"]"<<std::endl;
    std::cout<<"                                       - epsilon - epsilon of complex ["<<ripsComplexEpsilon<<"]"<<std::endl;
    std::cout<<"                                       - c_dim - max dimension of complex ["<<ripsComplexDim<<"]"<<std::endl;
    std::cout<<std::endl;
    std::cout<<"options:"<<std::endl;
    std::cout<<"  --use_alg [0|1]      - compute homology without reductions ["<<useAlgebraic<<"]"<<std::endl;
    std::cout<<"  --use_cored [0|1]    - use coreductions ["<<useCoreductions<<"]"<<std::endl;
    std::cout<<"  --use_accsub [0|1]   - use AccSub algorithm ["<<useAccSub<<"]"<<std::endl;
    std::cout<<"  --use_accsubig [0|1] - use AccSubIG algorithm ["<<useAccSubIG<<"]"<<std::endl;
    std::cout<<"  --use_accsubst [0|1] - use AccSubST algorithm ["<<useAccSubST<<"]"<<std::endl;
    std::cout<<"  --use_dist [0|1] pc alg  - use distributed computations with specified"<<std::endl;
    std::cout<<"                             packs count and selected algorithm ["<<useAccSubDist<<"]["<<packsCount<<"]["<<distAccSubAlgorithm<<"]"<<std::endl;
    std::cout<<"                              - 0 - AccSub"<<std::endl;
    std::cout<<"                              - 1 - AccSubIG"<<std::endl;
    std::cout<<"                              - 2 - AccSubST"<<std::endl;
    std::cout<<"  --test number        - select acyclicity test number ["<<accTestNumber<<"]"<<std::endl;
    std::cout<<"                          - 0 - tabulated configurations (full)"<<std::endl;
    std::cout<<"                          - 1 - CoDim 1 (partial)"<<std::endl;
    std::cout<<"                          - 2 - star (partial)"<<std::endl;
    std::cout<<"                          - 3 - recursive (partial)"<<std::endl;
    std::cout<<"                          - 4 - homology (full)"<<std::endl;
    std::cout<<"  --sv [0|1] - sort vertices before performing computations ["<<sortVerts<<"] "<<std::endl;
    std::cout<<std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void Tests::ProcessArgument(std::vector<std::string> &args)
{
#define CC(a, n) if (args.size() != n + 1) { std::cout<<"Error: "<<a<<" expects "<<n<<" params"<<std::endl; return; }

    std::string arg = args[0];
    if (arg == "help")
    {
        CC("help", 0)
        PrintHelp();
    }
    else if (arg == "i")
    {
        CC("i", 1)
        inputType = 0;
        inputFilename = args[1];
    }
    else if (arg == "l")
    {
        CC("l", 1)
        inputType = 1;
        inputFilename = args[1];
    }
    else if (arg == "r")
    {
        CC("r", 5)
        inputType = 2;
        randomPointsCount = atoi(args[1].c_str());
        randomPointsDiam = atof(args[2].c_str());
        randomPointsDim = atoi(args[3].c_str());
        ripsComplexEpsilon = atof(args[4].c_str());
        ripsComplexDim = atoi(args[5].c_str());
    }
    else if (arg == "use_alg")
    {
        CC("use_alg", 1)
        useAlgebraic = atoi(args[1].c_str());
    }
    else if (arg == "use_cored")
    {
        CC("use_cored", 1)
        useCoreductions = atoi(args[1].c_str());
    }
    else if (arg == "use_accsub")
    {
        CC("use_accsub", 1)
        useAccSub = atoi(args[1].c_str());
    }
    else if (arg == "use_accsubig")
    {
        CC("use_accsubig", 1)
        useAccSubIG = atoi(args[1].c_str());
    }
    else if (arg == "use_accsubst")
    {
        CC("use_accsubst", 1)
        useAccSubST = atoi(args[1].c_str());
    }
    else if (arg == "use_dist")
    {
        CC("use_dist", 3)
        useAccSubDist = atoi(args[1].c_str());
        packsCount = atoi(args[2].c_str());
        distAccSubAlgorithm = atoi(args[3].c_str());
    }
    else if (arg == "test")
    {
        CC("test", 1)
        accTestNumber = atoi(args[1].c_str());
    }
    else if (arg == "sv")
    {
        CC("sv", 1)
        sortVerts = atoi(args[1].c_str());
    }
    else
    {
        std::cout<<"Unknown argument: "<<arg<<std::endl;
    }

#undef CC
}

void Tests::ProcessArguments(int argc, char **argv)
{
    std::vector<std::string> args;
    for (int i = 1; i < argc; i++)
    {
        std::string s = std::string(argv[i]);
        if (s.size() > 2 && s[0] == '-' && s[1] == '-')
        {
            if (args.size() > 0)
            {
                ProcessArgument(args);
            }
            args.clear();
            args.push_back(s.substr(2));
        }
        else
        {
            args.push_back(s);
        }
    }
    if (args.size() > 0)
    {
        ProcessArgument(args);
    }
}

////////////////////////////////////////////////////////////////////////////////

bool Tests::IsAccSubReduction(ReductionType rt)
{
    return (rt == RT_AccSub || rt == RT_AccSubIG || rt == RT_AccSubST || rt == RT_AccSubDist);
}

////////////////////////////////////////////////////////////////////////////////

void Tests::Test(SimplexList &simplexList, ReductionType reductionType)
{
    float total = 0;

    Timer::Update();
    Timer::Time timeStart = Timer::Now();

    AccTest *accTest = IsAccSubReduction(reductionType) ? AccTest::Create(accTestNumber, Simplex::GetSimplexListDimension(simplexList)) : 0;
    if (accTest)
    {
        std::cout<<"acyclic test number: "<<accTest->GetID()<<std::endl;
    }
    IncidenceGraph *ig = 0;
    if (reductionType == RT_AccSub)
    {
        ig = IncidenceGraphHelpers<IncidenceGraph>::CreateAndCalculateAccSub(simplexList, accTest);
    }
    else if (reductionType == RT_AccSubIG)
    {
        ig = IncidenceGraphHelpers<IncidenceGraph>::CreateAndCalculateAccSubIG(simplexList, accTest);
    }
    else if (reductionType == RT_AccSubST)
    {
        ig = IncidenceGraphHelpers<IncidenceGraph>::CreateAndCalculateAccSubST(simplexList, accTest);
    }
    else if (reductionType == RT_AccSubDist)
    {
        ig = IncidenceGraphHelpers<IncidenceGraph>::CreateAndCalculateAccSubDist<PartitionGraph>(simplexList, packsCount, (AccSubAlgorithm)distAccSubAlgorithm, accTest);
    }
    else // (reductionType == RT_Coreduction || reductionType == RT_None)
    {
        ig = 0;
    }
    if (accTest)
    {
        delete accTest;
    }
    total = Timer::TimeFrom(timeStart, "total graph processing");
    MemoryInfo::Print();
    
    if (IsAccSubReduction(reductionType))
    {
        std::cout<<"acyclic subset size: "<<ig->GetAccSubSize()<<std::endl;
    }

    Timer::Update();

    if (ig != 0)
    {
        OutputGraph *og = new OutputGraph(ig);
        total += Timer::Update("creating output");
        MemoryInfo::Print();

        timeStart = Timer::Now();
        RedHomHelpers::ComputeHomology(og, reductionType == RT_Coreduction);
        total += Timer::TimeFrom(timeStart);

        delete og;
    }
    else
    {
        timeStart = Timer::Now();
        RedHomHelpers::ComputeHomology(simplexList, reductionType == RT_Coreduction);
        total += Timer::TimeFrom(timeStart);
    }

    std::cout<<"total: "<<total<<std::endl;
    MemoryInfo::Print();
    
    delete ig;
}

void Tests::TestAndCompare(SimplexList &simplexList)
{
    std::cout<<"simplices count: "<<simplexList.size()<<std::endl;
    std::cout<<"dim: "<<Simplex::GetSimplexListDimension(simplexList)<<std::endl;

    if (useAlgebraic)
    {
        std::cout<<std::endl<<"algebraic:"<<std::endl;
        Test(simplexList, RT_None);
        cout<<std::endl;
    }

    if (useCoreductions)
    {
        std::cout<<std::endl<<"coreductions:"<<std::endl;
        Test(simplexList, RT_Coreduction);
        cout<<std::endl;
    }

    if (useAccSub)
    {
        std::cout<<"AccSub:"<<std::endl;
        Test(simplexList, RT_AccSub);
    }

    if (useAccSubIG)
    {
        std::cout<<std::endl<<"AccSubIG:"<<std::endl;
        Test(simplexList, RT_AccSubIG);
        cout<<std::endl<<std::endl;
    }

    if (useAccSubST)
    {
        std::cout<<std::endl<<"AccSubST:"<<std::endl;
        Test(simplexList, RT_AccSubST);
    }

    if (useAccSubDist)
    {
        std::cout<<std::endl<<"AccSubDist:"<<std::endl;
        Test(simplexList, RT_AccSubDist);
    }
}

////////////////////////////////////////////////////////////////////////////////

void Tests::TestSingleFile()
{
    std::cout<<"testing "<<inputFilename<<std::endl;
    SimplexList simplexList;
    Timer::Init();
    SimplexUtils<Simplex>::ReadSimplexList(simplexList, inputFilename.c_str(), sortVerts);
    std::cout<<"input size: "<<simplexList.size()<<std::endl;
    Timer::Update("data read");
    MemoryInfo::Print();
    TestAndCompare(simplexList);
}

void Tests::TestFromList()
{
    std::fstream input(inputFilename.c_str(), std::ios::in);
    if (!input.is_open())
    {
        throw std::string("Can't open file ") + inputFilename;
    }

    while (!input.eof())
    {
        input>>inputFilename;
        if (inputFilename != "")
        {
            TestSingleFile();
        }
        else
        {
            break;
        }
    }
    input.close();
}

void Tests::TestRandomRipsComplex()
{
    std::cout<<"generating Rips complex with:"<<std::endl;
    std::cout<<"count = "<<randomPointsCount<<std::endl;
    std::cout<<"diam = "<<randomPointsDiam<<std::endl;
    std::cout<<"s_dim = "<<randomPointsDim<<std::endl;
    std::cout<<"epsilon = "<<ripsComplexEpsilon<<std::endl;
    std::cout<<"c_dim = "<<ripsComplexDim<<std::endl;
    SimplexList simplexList;
    Timer::Init();
    SimplexUtils<Simplex>::GenerateRandomRipsComplex(simplexList, randomPointsCount, randomPointsDiam, randomPointsDim, ripsComplexEpsilon, ripsComplexDim);
    std::cout<<"input size: "<<simplexList.size()<<std::endl;
    Timer::Update("data generated");
    MemoryInfo::Print();
    TestAndCompare(simplexList);
}

////////////////////////////////////////////////////////////////////////////////

void Tests::TestFromCommandLine(int argc, char **argv)
{
    std::cout<<"Use AccSub --help for more info"<<std::endl;

    ProcessArguments(argc, argv);

    switch (inputType)
    {
        case 0:
            TestSingleFile();
            break;
        case 1:
            TestFromList();
            break;
        case 2:
            TestRandomRipsComplex();
            break;
        default:
            break;
    }

    MemoryInfo::Print();
    
}

////////////////////////////////////////////////////////////////////////////////

void Tests::MPIMaster(int argc, char** argv)
{
#ifdef USE_MPI
    TestFromCommandLine(argc, argv);
    ComputationsParallelMPI<PartitionGraph>::CollectDebugMemoryInfo();
    ComputationsParallelMPI<PartitionGraph>::KillSlaves();
#endif
}

void Tests::MPISlave(int processRank)
{
#ifdef USE_MPI
    ComputationsParallelMPI<PartitionGraph>::Slave(processRank);
#endif
}

void Tests::MPITestFromCommandLine(int argc, char **argv)
{
#ifdef USE_MPI
    MPI_Init(&argc, &argv);
    Timer::Init();

    MPI_Comm_rank(MPI_COMM_WORLD, &processRank);
    std::cout<<"starting process: "<<processRank<<std::endl;
    Timer::Time now = Timer::Now();

    if (processRank == 0)
    {
        MPIMaster(argc, argv);
    }
    else
    {
        MPISlave(processRank);
    }

    std::cout<<"terminating process: "<<processRank<<std::endl;
    Timer::TimeFrom(now, "total");
    std::cout<<"process "<<processRank<<" ";
    MemoryInfo::Print();
    MPI_Finalize();
#endif
}

////////////////////////////////////////////////////////////////////////////////
// eof
