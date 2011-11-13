/*
 * File:   Tests.cpp
 * Author: Piotr Brendel
 */

#include "Tests.h"
#include "Utils.h"
#include "../AcyclicSubset/Simplex.h"
#include "../AcyclicSubset/SimplexUtils.h"
#include "../AcyclicSubset/IncidenceGraph.h"
#include "../AcyclicSubset/IncidenceGraphHelpers.h"
#include "../AcyclicSubset/HomologyHelpers.h"

////////////////////////////////////////////////////////////////////////////////
// static variables

int Tests::testType = 0;
int Tests::acyclicTestNumber = 0;
std::string Tests::inputFilename = "tests.txt";
int Tests::simplicesCount = 1000;
int Tests::simplicesDim = 3;
int Tests::vertsCount = 100;
int Tests::sortSimplices = 0;
int Tests::sortVerts = 0;
int Tests::useAlgebraic = 0;
int Tests::useCoreduction = 0;
int Tests::useAcc = 0;
int Tests::useAccIG = 1;
int Tests::useAccST = 0;
int Tests::useParallel = 0;
int Tests::packsCount = 6;
int Tests::parallelAccSubAlgorithm = 0;
int Tests::prepareData = 1;

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
    std::cout<<"                  (def. "<<acyclicTestNumber<<")"<<std::endl;
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
    std::cout<<"                       (def. "<<useAcc<<")"<<std::endl;
    std::cout<<"    -use_accig [0|1] - wykonaj obliczenia dla podzbioru acyklicznegoo uzywajac algorytmu AccIG"<<std::endl;
    std::cout<<"                         (def. "<<useAccIG<<")"<<std::endl;
    std::cout<<"    -use_accst [0|1] - wykonaj obliczenia dla podzbioru acyklicznegoo uzywajac algorytmu AccST"<<std::endl;
    std::cout<<"                         (def. "<<useAccST<<")"<<std::endl;
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
        else if (CHECK_ARG_NEXT("-test")) { acyclicTestNumber = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-verts")) { vertsCount = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-ss")) { sortSimplices = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-sv")) { sortVerts = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-use_alg")) { useAlgebraic = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-use_cored")) { useCoreduction = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-use_acc")) { useAcc = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-use_accig")) { useAccIG = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-use_accst")) { useAccST = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-use_parallel"))
        {
            useParallel = atoi(argv[index + 1]);
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
            GenerateSimplexList(simplexList, simplicesCount, vertsCount, simplicesDim);
            break;
        case 1:
            ReadSimplexList(simplexList, inputFilename.c_str(), sortVerts);
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

bool Tests::IsAcyclicSubsetReduction(ReductionType rt)
{
    return (rt == RT_Acc || rt == RT_AccIG || rt == RT_AccST || rt == RT_AccParallel);
}

////////////////////////////////////////////////////////////////////////////////

void Tests::Test(SimplexList &simplexList, ReductionType reductionType)
{
    float total = 0;

    Timer::Update();
    Timer::Time timeStart = Timer::Now();

    AcyclicTest<IncidenceGraph::IntersectionFlags> *test = IsAcyclicSubsetReduction(reductionType) ? AcyclicTest<IncidenceGraph::IntersectionFlags>::Create(acyclicTestNumber, Simplex::GetSimplexListDimension(simplexList)) : 0;
    IncidenceGraph *ig = 0;
    if (reductionType == RT_Acc)
    {
        ig = IncidenceGraphHelpers::CreateAndCalculateAcyclicSubset(simplexList, test);
    }
    else if (reductionType == RT_AccIG)
    {
        ig = IncidenceGraphHelpers::CreateAndCalculateAcyclicSubsetOnline(simplexList, test);
    }
    else if (reductionType == RT_AccST)
    {
        ig = IncidenceGraphHelpers::CreateAndCalculateAcyclicSubsetSpanningTree(simplexList, test);
    }
    else if (reductionType == RT_AccParallel)
    {
        ig = IncidenceGraphHelpers::CreateAndCalculateAcyclicSubsetParallel(simplexList, packsCount, (AccSubAlgorithm)parallelAccSubAlgorithm, test);
    }
    else // (reductionType == RT_Coreduction || reductionType == RT_None)
    {
        ig = IncidenceGraphHelpers::Create(simplexList);
    }
    if (test)
    {
        delete test;
    }
    total = Timer::TimeFrom(timeStart, "total graph processing");
    MemoryInfo::Print();
    
    if (IsAcyclicSubsetReduction(reductionType))
    {
        std::cout<<"acyclic subset size: "<<ig->GetAcyclicSubsetSize()<<std::endl;
    }

    Timer::Update();
    OutputGraph *og = new OutputGraph(ig);
    total += Timer::Update("creating output");
    MemoryInfo::Print();

    timeStart = Timer::Now();
    HomologyHelpers::ComputeHomology(og, reductionType == RT_Coreduction);
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

    if (useAcc)
    {
        std::cout<<"Acc:"<<std::endl;
        Test(simplexList, RT_Acc);
    }

    if (useAccIG)
    {
        std::cout<<std::endl<<"AccIG:"<<std::endl;
        Test(simplexList, RT_AccIG);
        cout<<std::endl<<std::endl;
    }

    if (useAccST)
    {
        std::cout<<std::endl<<"AccST:"<<std::endl;
        Test(simplexList, RT_AccST);
    }

    if (useParallel)
    {
        std::cout<<std::endl<<"parallel:"<<std::endl;
        Test(simplexList, RT_AccParallel);
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
// eof
