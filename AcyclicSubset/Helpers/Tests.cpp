/*
 * File:   Tests.cpp
 * Author: Piotr Brendel
 */

#include "Tests.h"
#include "RedHomHelpers.hpp"
#include "Utils.hpp"
#include "../AcyclicSubset/SimplexUtils.hpp"
#include "../AcyclicSubset/IncidenceGraphHelpers.hpp"

#include <cassert>

////////////////////////////////////////////////////////////////////////////////

int Tests::testType = 0;
int Tests::accTestNumber = 0;
std::string Tests::inputFilename = "tests.txt";
int Tests::simplicesCount = 1000;
int Tests::simplicesDim = 3;
int Tests::vertsCount = 100;
int Tests::sortSimplices = 0;
int Tests::sortVerts = 0;
int Tests::useAlgebraic = 0;
int Tests::useCoreduction = 0;
int Tests::useAccSub = 0;
int Tests::useAccSubIG = 1;
int Tests::useAccSubST = 0;
int Tests::useAccSubParallel = 0;
int Tests::packsCount = 6;
int Tests::parallelAccSubAlgorithm = 0;
int Tests::prepareData = 1;
int Tests::processRank = 0;

////////////////////////////////////////////////////////////////////////////////

void Tests::PrintHelp()
{
    std::cout<<"-input [[-r ilosc_sympleksow] | [-i nazwa_pliku] | [-l nazwa_pliku]] [-opcje]"<<std::endl;
    std::cout<<"    -r ilosc_sympleksow wymiar_sympleksow - losowe zbiory okresonego rozmiaru"<<std::endl;
    std::cout<<"    -i nazwa_pliku - pojedynczy zbior wczytany z okreslonego pliku"<<std::endl;
    std::cout<<"    -l nazwa_pliku - lista zbiorow okreslonych w podanym pliku"<<std::endl;
    std::cout<<"  opcje:"<<std::endl;
    std::cout<<"    -test numer - numer testu acyklicznosci:"<<std::endl;
    std::cout<<"                    - 0 - tablice"<<std::endl;
    std::cout<<"                    - 1 - test CoDim 1"<<std::endl;
    std::cout<<"                  (def. "<<accTestNumber<<")"<<std::endl;
    std::cout<<"    -verts vc - ilosc wierzcholkow z ktorych losujemy sympleksy"<<std::endl;
    std::cout<<"                (def. "<<vertsCount<<")"<<std::endl;
    std::cout<<"    -ss [0|1] - sortuj sympleksy przed rozpoczeciem obliczen"<<std::endl;
    std::cout<<"                (def. "<<sortSimplices<<")"<<std::endl;
    std::cout<<"    -sv [0|1] - sortuj wierzcholki w kazdym sympleksie"<<std::endl;
    std::cout<<"                (def. "<<sortVerts<<")"<<std::endl;
    std::cout<<"    -use_alg [0|1] - wykonaj obliczenia bez zadnych redukcji"<<std::endl;
    std::cout<<"                     (def. "<<useAlgebraic<<")"<<std::endl;
    std::cout<<"    -use_cored [0|1] - wykonaj obliczenia dla koredukcji"<<std::endl;
    std::cout<<"                       (def. "<<useCoreduction<<")"<<std::endl;
    std::cout<<"    -use_acc [0|1] - wykonaj obliczenia dla podzbioru acyklicznego uzywajac algorytmu Acc"<<std::endl;
    std::cout<<"                       (def. "<<useAccSub<<")"<<std::endl;
    std::cout<<"    -use_accig [0|1] - wykonaj obliczenia dla podzbioru acyklicznegoo uzywajac algorytmu AccIG"<<std::endl;
    std::cout<<"                         (def. "<<useAccSubIG<<")"<<std::endl;
    std::cout<<"    -use_accst [0|1] - wykonaj obliczenia dla podzbioru acyklicznegoo uzywajac algorytmu AccST"<<std::endl;
    std::cout<<"                         (def. "<<useAccSubST<<")"<<std::endl;
    std::cout<<"    -parallel [0|1] pc alg pd - wykonaj rownolegle obliczenia dla podzbioru acyklicznego"<<std::endl;
    std::cout<<"                                   - pc - ilosc paczek dla obliczen rownoleglych"<<std::endl;
    std::cout<<"                                     (def. "<<packsCount<<")"<<std::endl;
    std::cout<<"                                   - alg - numer algorytmu obliczajacego podzbior acykliczny"<<std::endl;
    std::cout<<"                                       - 0 - Acc"<<std::endl;
    std::cout<<"                                       - 1 - AccIG"<<std::endl;
    std::cout<<"                                       - 2 - AccST"<<std::endl;
    std::cout<<"                                     (def. "<<parallelAccSubAlgorithm<<")"<<std::endl;
    std::cout<<"                                   - pd [0|1] - przetworz dane przed podzialem na paczki"<<std::endl;
    std::cout<<"                                     (def. "<<prepareData<<")"<<std::endl;
}

////////////////////////////////////////////////////////////////////////////////

void Tests::ProcessArguments(int argc, char **argv)
{
#define CHECK_ARG(s) !strcmp(argv[index], s)
#define CHECK_ARG_NEXT(s) !strcmp(argv[index], s) && index + 1 < argc

    int index = 1;
    while (index < argc)
    {
        if (CHECK_ARG("-help")) { PrintHelp(); index++; }
        else if (CHECK_ARG_NEXT("-r"))
        {
            testType = 0;
            simplicesCount = atoi(argv[index + 1]);
            simplicesDim = atoi(argv[index + 2]);
            index += 3;
        }
        else if (CHECK_ARG_NEXT("-i"))
        {
            testType = 1;
            inputFilename = argv[index + 1];
            index += 2;
        }
        else if (CHECK_ARG_NEXT("-l"))
        {
            testType = 2;
            inputFilename = argv[index + 1];
            index += 2;
        }
        else if (CHECK_ARG_NEXT("-test")) { accTestNumber = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-verts")) { vertsCount = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-ss")) { sortSimplices = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-sv")) { sortVerts = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-use_alg")) { useAlgebraic = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-use_cored")) { useCoreduction = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-use_acc")) { useAccSub = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-use_accig")) { useAccSubIG = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-use_accst")) { useAccSubST = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-use_parallel"))
        {
            useAccSubParallel = atoi(argv[index + 1]);
            packsCount = atoi(argv[index + 2]);
            parallelAccSubAlgorithm = atoi(argv[index + 3]);
            prepareData = atoi(argv[index + 4]);
            index += 5;
        }
        else
        {
            std::cout<<"nieznany parametr: "<<argv[index]<<std::endl;
            index++;
        }
    }
#undef CHECK_ARG
#undef CHECH_ARG_NEXT
}

////////////////////////////////////////////////////////////////////////////////

void Tests::GenerateData(SimplexList &simplexList)
{
    switch (testType)
    {
        case 0:
            SimplexUtils<Simplex>::GenerateSimplexList(simplexList, simplicesCount, vertsCount, simplicesDim);
            break;
        case 1:
            SimplexUtils<Simplex>::ReadSimplexList(simplexList, inputFilename.c_str(), sortVerts);
            break;
        default:
            break;
    }
    if (sortSimplices)
    {
        std::sort(simplexList.begin(), simplexList.end(), Simplex::SortBySize);
    }
    Timer::Update("data generated");
    MemoryInfo::Print();
}

////////////////////////////////////////////////////////////////////////////////

bool Tests::IsAccSubReduction(ReductionType rt)
{
    return (rt == RT_AccSub || rt == RT_AccSubIG || rt == RT_AccSubST || rt == RT_AccSubParallel);
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
    else if (reductionType == RT_AccSubParallel)
    {
        ig = IncidenceGraphHelpers<IncidenceGraph>::CreateAndCalculateAccSubParallel<PartitionGraph>(simplexList, packsCount, (AccSubAlgorithm)parallelAccSubAlgorithm, accTest);
    }
    else // (reductionType == RT_Coreduction || reductionType == RT_None)
    {
        ig = IncidenceGraphHelpers<IncidenceGraph>::Create(simplexList);
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
    OutputGraph *og = new OutputGraph(ig);
    total += Timer::Update("creating output");
    MemoryInfo::Print();

    timeStart = Timer::Now();
    RedHomHelpers<OutputGraph>::ComputeHomology(og, reductionType == RT_Coreduction);
    total += Timer::TimeFrom(timeStart);
    std::cout<<"total: "<<total<<std::endl;
    MemoryInfo::Print();
    
    delete og;
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

    if (useCoreduction)
    {
        std::cout<<std::endl<<"coreduction:"<<std::endl;
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

    if (useAccSubParallel)
    {
        std::cout<<std::endl<<"parallel:"<<std::endl;
        Test(simplexList, RT_AccSubParallel);
    }
}

////////////////////////////////////////////////////////////////////////////////

void Tests::StandardTest()
{
    SimplexList simplexList;
    Timer::Init();
    GenerateData(simplexList);

    char buff[100] = { 0 };
    switch (testType)
    {
        case 0:
            sprintf(buff, "random set of size: %d", (int)simplexList.size());
            break;
        case 1:
            sprintf(buff, "testing %s", inputFilename.c_str());
            break;
        default:
            break;
    }

    std::cout<<buff<<std::endl;

    TestAndCompare(simplexList);
}

void Tests::TestFromList()
{
    std::fstream input(inputFilename.c_str(), std::ios::in);
    if (!input.is_open())
    {
        throw std::string("Can't open file ") + inputFilename;
    }

    testType = 1; // dane z pliku
    while (!input.eof())
    {
        input>>inputFilename;
        if (inputFilename != "")
        {
            StandardTest();
        }
        else
        {
            break;
        }
    }
    input.close();
}

////////////////////////////////////////////////////////////////////////////////

void Tests::TestFromCommandLine(int argc, char **argv)
{
    ProcessArguments(argc, argv);

    std::cout<<"Aby uzyskac wiecej informacji uruchom z parametrem -help"<<std::endl;

    switch (testType)
    {
        case 0:
        case 1:
            StandardTest();
            break;
        case 2:
            TestFromList();
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
    MemoryInfo::Print();
    MPI_Finalize();
#endif
}

////////////////////////////////////////////////////////////////////////////////
// eof
