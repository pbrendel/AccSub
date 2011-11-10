#include "Tests.h"
#include "../AcyclicSubset/SimplexUtils.h"

////////////////////////////////////////////////////////////////////////////////
// RedHom stuff

#ifndef LINUX
#define LINUX
long double powl(long double, int);
#endif

#include <cstdlib>
#include <iostream>
#include <string>

#include <redHom/complex/scomplex/SComplex.hpp>
#include <redHom/complex/scomplex/SComplexReader.hpp>
#include <redHom/complex/scomplex/SComplexDefaultTraits.hpp>
#include <redHom/complex/scomplex/SComplexBuilderFromSimplices.hpp>
#include <redHom/algorithm/Algorithms.hpp>

#include <redHom/complex/simplicial/SimplexSubdivision.hpp>

#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/assign/list_inserter.hpp>

using namespace boost;
using namespace boost::assign;

long double powl(long double x, int y)
{
    return pow(x, y);
}

ofstreamcout fcout;

////////////////////////////////////////////////////////////////////////////////
// RedHom setup

typedef ElementaryCell ElementaryCellType;
typedef int ScalarType;
typedef FreeModule<int,capd::vectalg::Matrix<int,0,0> > FreeModuleType;
typedef FreeChainComplex<FreeModuleType> FreeChainComplexType;
typedef ReducibleFreeChainComplex<FreeModuleType,int> ReducibleFreeChainComplexType;
typedef SComplex<SComplexDefaultTraits> Complex;
typedef int Id;

////////////////////////////////////////////////////////////////////////////////

#include "Utils.h"
#include "../AcyclicSubset/Simplex.h"
#include "../AcyclicSubset/IncidenceGraph.h"

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
std::string Tests::logFilename = "tests_log.xml";
std::ofstream Tests::log;
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
    std::cout<<"    -log nazwa_pliku - zmien nazwe pliku z logami"<<std::endl;
    std::cout<<"                (def. "<<logFilename<<")"<<std::endl;
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
        else if (CHECK_ARG_NEXT("-log")) { logFilename = argv[index + 1]; index += 2; }
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

bool Tests::IsAcyclicSubsetReduction(ReductionType rt)
{
    return (rt == RT_Acc || rt == RT_AccIG || rt == RT_AccST || rt == RT_AccParallel);
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
        std::sort(simplexList.begin(), simplexList.end(), SimplexSorterSize);
    }
    Timer::Update("data generated");
    MemoryInfo::Print();
}

////////////////////////////////////////////////////////////////////////////////

void Tests::Test(SimplexList &simplexList, ReductionType reductionType)
{
    float total = 0;
    float t = 0;

    Timer::Update();
    Timer::Time timeStart = Timer::Now();

    AcyclicTest<IncidenceGraph::IntersectionFlags> *test = IsAcyclicSubsetReduction(reductionType) ? AcyclicTest<IncidenceGraph::IntersectionFlags>::Create(acyclicTestNumber, GetDimension(simplexList)) : 0;
    IncidenceGraph *ig = 0;
    if (reductionType == RT_Acc)
    {
        ig = IncidenceGraph::CreateAndCalculateAcyclicSubset(simplexList, test);
    }
    else if (reductionType == RT_AccIG)
    {
        ig = IncidenceGraph::CreateAndCalculateAcyclicSubsetOnline(simplexList, test);
    }
    else if (reductionType == RT_AccST)
    {
        ig = IncidenceGraph::CreateAndCalculateAcyclicSubsetSpanningTree(simplexList, test);
    }
    else if (reductionType == RT_AccParallel)
    {
        ig = IncidenceGraph::CreateAndCalculateAcyclicSubsetParallel(simplexList, packsCount, (AccSubAlgorithm)parallelAccSubAlgorithm, test);
    }
    else // (reductionType == RT_Coreduction || reductionType == RT_None)
    {
        ig = IncidenceGraph::Create(simplexList);
    }
    if (test)
    {
        delete test;
    }
    Timer::Update();
    total = Timer::TimeFrom(timeStart, "total graph processing");
    MemoryInfo::Print();
    Tests::Test(ig, reductionType, total);
    delete ig;
}

void Tests::Test(IncidenceGraph *ig, ReductionType reductionType, float totalTime)
{ 
    if (IsAcyclicSubsetReduction(reductionType))
    {
        int size = ig->GetAcyclicSubsetSize();
        std::cout<<"acyclic subset size: "<<size<<std::endl;
        log<<"\t\t\t<acyclic_subset_size>"<<size<<"</acyclic_subset_size>"<<std::endl;
    }

    Timer::Update();
    OutputGraph *og = new OutputGraph(ig);
    float t = Timer::Update("creating output");
    MemoryInfo::Print();
    log<<"\t\t\t<output_graph>"<<t<<"</output_graph>"<<std::endl;
    totalTime += t;

    totalTime += ComputeHomology(og, reductionType == RT_Coreduction);
    std::cout<<"total: "<<totalTime<<std::endl;
    log<<"\t\t\t<total>"<<totalTime<<"</total>"<<std::endl;
    MemoryInfo::Print();
    
    delete og;
}

void Tests::TestAndCompare(SimplexList &simplexList)
{
    std::cout<<"simplices count: "<<simplexList.size()<<std::endl;
    std::cout<<"dim: "<<GetDimension(simplexList)<<std::endl;

    if (useAlgebraic)
    {
        std::cout<<std::endl<<"algebraic:"<<std::endl;
        log<<"\t\t<algebraic>"<<std::endl<<std::endl;
        Test(simplexList, RT_None);
        log<<"\t\t</algebraic>"<<std::endl<<std::endl;
        cout<<std::endl;
    }

    if (useCoreduction)
    {
        std::cout<<std::endl<<"coreduction:"<<std::endl;
        log<<"\t\t<coreduction>"<<std::endl<<std::endl;
        Test(simplexList, RT_Coreduction);
        log<<"\t\t</coreduction>"<<std::endl<<std::endl;
        cout<<std::endl;
    }

    if (useAcc)
    {
        std::cout<<"Acc:"<<std::endl;
        log<<"\t\t<acc>"<<std::endl<<std::endl;
        Test(simplexList, RT_Acc);
        log<<"\t\t</acc>"<<std::endl<<std::endl;
    }

    if (useAccIG)
    {
        std::cout<<std::endl<<"AccIG:"<<std::endl;
        log<<"\t\t<accig>"<<std::endl<<std::endl;
        Test(simplexList, RT_AccIG);
        log<<"\t\t</accig>"<<std::endl<<std::endl;
        cout<<std::endl<<std::endl;
    }

    if (useAccST)
    {
        std::cout<<std::endl<<"AccST:"<<std::endl;
        log<<"\t\t<accst>"<<std::endl<<std::endl;
        Test(simplexList, RT_AccST);
        log<<"\t\t</accst>"<<std::endl<<std::endl;
    }

    if (useParallel)
    {
        std::cout<<std::endl<<"parallel:"<<std::endl;
        log<<"\t\t<parallel>"<<std::endl<<std::endl;
        Test(simplexList, RT_AccParallel);
        log<<"\t\t</parallel>"<<std::endl<<std::endl;
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
    log<<"<input_size>"<<simplexList.size()<<"</input_size>"<<std::endl;
    log<<"<dim>"<<GetDimension(simplexList)<<"</dimension>"<<std::endl;
    log<<std::endl<<"\t<description>"<<buff<<"</description>"<<std::endl<<std::endl;

    TestAndCompare(simplexList);
}

void Tests::TestFromList()
{
    std::fstream input(inputFilename.c_str(), std::ios::in);
    if (!input.is_open())
    {
        throw std::string("Can't open file ") + inputFilename;
    }

    testType = 2; // dane z pliku
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

    OpenLog();

    switch (testType)
    {
        case 0:
        case 1:
        case 2:
        case 3:
            StandardTest();
            break;
        case 4:
            TestFromList();
            break;
        default:
            break;
    }

    CloseLog();

    MemoryInfo::Print();

}

////////////////////////////////////////////////////////////////////////////////

int Tests::GetBetti(SimplexList &simplexList, int n)
{
    IncidenceGraph *ig = IncidenceGraph::Create(simplexList);
    OutputGraph *og = new OutputGraph(ig);

    Complex::Dims dims;
    Complex::KappaMap kappaMap;

    for (OutputGraph::Nodes::iterator i = og->nodes.begin(); i != og->nodes.end(); i++)
    {
        dims.push_back((*i)->simplex.size() - 1);
        int index = (*i)->index;
        std::vector<int>::iterator k = (*i)->kappa.begin();
        for (OutputGraph::Nodes::iterator j = (*i)->subnodes.begin(); j != (*i)->subnodes.end(); j++)
        {
            kappaMap.push_back(boost::tuple<Id, Id, int>(index, (*j)->index, (*k)));
            k++;
        }
    }

    delete og;
    delete ig;

    Complex complex(3, dims, kappaMap, 1);
    (*CoreductionAlgorithmFactory<Complex>::createDefault(complex))();
    CRef<ReducibleFreeChainComplexType> RFCComplexCR=
  	(ReducibleFreeChainComplexOverZFromSComplexAlgorithm<SComplex<SComplexDefaultTraits>, ReducibleFreeChainComplexType>(complex))();
    CRef<HomologySignature<int> > homSignCR = HomAlgFunctors<FreeModuleType>::homSignViaAR_Random(RFCComplexCR);

    return homSignCR().bettiNumber(n);
}

////////////////////////////////////////////////////////////////////////////////

float Tests::ComputeHomology(OutputGraph *g, bool doCoreduction)
{
    Complex::Dims dims;
    Complex::KappaMap kappaMap;

    float total = 0;
    float t = 0;

    for (OutputGraph::Nodes::iterator i = g->nodes.begin(); i != g->nodes.end(); i++)
    {
//        Log::stream<<(*i)->index<<": ";
//        Debug::Print(Log::stream, (*i)->simplex);

        dims.push_back((*i)->simplex.size() - 1);
        int index = (*i)->index;
        std::vector<int>::iterator k = (*i)->kappa.begin();
        for (OutputGraph::Nodes::iterator j = (*i)->subnodes.begin(); j != (*i)->subnodes.end(); j++)
        {
            kappaMap.push_back(boost::tuple<Id, Id, int>(index, (*j)->index, (*k)));
            k++;
        }
    }

//    Log::stream<<std::endl<<"dims:"<<std::endl;
//    for (Complex::Dims::iterator i = dims.begin(); i != dims.end(); i++)
//    {
//        Log::stream<<(*i)<<std::endl;
//    }
//    Log::stream<<std::endl<<"kappa:"<<std::endl;
//    for (Complex::KappaMap::iterator i = kappaMap.begin(); i != kappaMap.end(); i++)
//    {
//        Log::stream<<boost::get<0>(*i)<<" "<<boost::get<1>(*i)<<" "<<boost::get<2>(*i)<<std::endl;
//    }

    Complex complex(3, dims, kappaMap, 1);
    t = Timer::Update("creating complex");
    log<<"\t\t\t<creating_complex>"<<t<<"</creating_complex>"<<std::endl;
    total += t;

    if (doCoreduction)
    {
        Timer::Update();
        (*CoreductionAlgorithmFactory<Complex>::createDefault(complex))();
        t = Timer::Update("coreduction");
        log<<"\t\t\t<coreduction>"<<t<<"</coreduction>"<<std::endl;
        total += t;
        MemoryInfo::Print();
    }

    Timer::Update();
    CRef<ReducibleFreeChainComplexType> RFCComplexCR = (ReducibleFreeChainComplexOverZFromSComplexAlgorithm<SComplex<SComplexDefaultTraits>, ReducibleFreeChainComplexType>(complex))();
    CRef<HomologySignature<int> > homSignCR = HomAlgFunctors<FreeModuleType>::homSignViaAR_Random(RFCComplexCR);
    t = Timer::Update("computing homology");
    log<<"\t\t\t<homology>"<<t<<"</homology>"<<std::endl;
    total += t;
    MemoryInfo::Print();
    
//    testsLog<<homSignCR();
    std::cout<<homSignCR();

    return total;
}

////////////////////////////////////////////////////////////////////////////////

void Tests::OpenLog()
{
    log.open(logFilename.c_str());
    log<<"<tests>"<<std::endl<<std::endl;
    log<<"<input>"<<inputFilename<<"</input>"<<std::endl;
    log<<"<acyclic_test_number>"<<acyclicTestNumber<<"</acyclic_test_number>"<<std::endl;
}

void Tests::CloseLog()
{
    log<<"</tests>"<<std::endl;
    log.close();
}

////////////////////////////////////////////////////////////////////////////////
// eof
