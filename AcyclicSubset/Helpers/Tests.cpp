#include "Tests.h"

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
#include "DebugMemory.h"
#include "../AcyclicSubset/Simplex.h"
#include "../AcyclicSubset/IncidenceGraph.h"

////////////////////////////////////////////////////////////////////////////////
// static variables

int Tests::testType = 0;
std::string Tests::inputFilename = "tests.txt";
int Tests::simplicesCount = 1000;
int Tests::vertsCount = 100;
int Tests::nodeSimplicesCount = 32;
int Tests::nodesCount = 8;
int Tests::sortVerts = 1;
std::string Tests::logFilename = "tests_log.xml";
std::ofstream Tests::log;
int Tests::useAlgebraic = 0;
int Tests::useCoreduction = 0;
int Tests::useAcyclicSubset = 0;
int Tests::useAcyclicSubsetSpanningTree = 1;
int Tests::useAcyclicSubsetOnline = 0;
int Tests::useAcyclicSubsetParallel = 0;
IncidenceGraph::Params Tests::incidenceGraphParams(4, 0, true, false);
IncidenceGraph::ParallelParams Tests::parallelParams(1000);

////////////////////////////////////////////////////////////////////////////////

void Tests::PrintHelp()
{
    std::cout<<"-input [[-r ilosc_sympleksow] | [-i nazwa_pliku] | [-l nazwa_pliku]] [-opcje]"<<std::endl;
    std::cout<<"    -r ilosc_sympleksow - losowe zbiory okresonego rozmiaru"<<std::endl;
    std::cout<<"    -rr ilosc_sympleksow - maksymalna ilosc sympleksow dla danego wymiaru minus okreslona ilosc"<<std::endl;
    std::cout<<"    -i nazwa_pliku - pojedynczy zbior wczytany z okreslonego pliku"<<std::endl;
    std::cout<<"    -l nazwa_pliku - lista zbiorow okreslonych w podanym pliku"<<std::endl;
    std::cout<<"    -act rozmiar_node ilosc_nodow - testy przy wykorzystaniu acyklicznego drzewa"<<std::endl;
    std::cout<<"  opcje:"<<std::endl;
    std::cout<<"    -verts vc - ilosc wierzcholkow z ktorych losujemy sympleksy"<<std::endl;
    std::cout<<"                (def. "<<vertsCount<<")"<<std::endl;
    std::cout<<"    -test numer - numer testu acyklicznosci: 0 - tablice 1 - test CoDim 1"<<std::endl;
    std::cout<<"                  (def. "<<incidenceGraphParams.acyclicTestNumber<<")"<<std::endl;
    std::cout<<"    -dim d - wymiar danych wejsciowych"<<std::endl;
    std::cout<<"             (def. "<<incidenceGraphParams.dim<<")"<<std::endl;
    std::cout<<"    -ss [0|1] - sortuj sympleksy przed rozpoczeciem obliczen"<<std::endl;
    std::cout<<"                (def. "<<incidenceGraphParams.sortNodes<<")"<<std::endl;
    std::cout<<"    -sv [0|1] - sortuj wierzcholki w kazdym sympleksie"<<std::endl;
    std::cout<<"                (def. "<<sortVerts<<")"<<std::endl;
    std::cout<<"    -min [0|1] - minimalizuj sympleksy"<<std::endl;
    std::cout<<"                (def. "<<incidenceGraphParams.minimizeSimplices<<")"<<std::endl;
    std::cout<<"    -log nazwa_pliku - zmien nazwe pliku z logami"<<std::endl;
    std::cout<<"                (def. "<<incidenceGraphParams.minimizeSimplices<<")"<<std::endl;
    std::cout<<"    -ps size - wielkosc paczki dla obliczen rownoleglych"<<std::endl;
    std::cout<<"               (def. "<<parallelParams.packSize<<")"<<std::endl;
    std::cout<<"    -pc count - ilosc paczek dla obliczen rownoleglych"<<std::endl;
    std::cout<<"                jezeli == -1 to obliczana na podstawie rozmiaru paczki"<<std::endl;
    std::cout<<"                (def. "<<parallelParams.packsCount<<")"<<std::endl;
    std::cout<<"    -ppd [0|1] - przetworz dane przed podzialem na paczki"<<std::endl;
    std::cout<<"                 (def. "<<parallelParams.prepareData<<")"<<std::endl;
    std::cout<<"    -paso [0|1] - uzyj algorytmu online do obliczania podzadan"<<std::endl;
    std::cout<<"                  (def. "<<parallelParams.useAcyclicSubsetOnlineAlgorithm<<")"<<std::endl;
    std::cout<<"    -use_alg [0|1] - wykonaj obliczenia bez zadnych redukcji"<<std::endl;
    std::cout<<"                     (def. "<<useAlgebraic<<")"<<std::endl;
    std::cout<<"    -use_cored [0|1] - wykonaj obliczenia dla koredukcji"<<std::endl;
    std::cout<<"                       (def. "<<useCoreduction<<")"<<std::endl;
    std::cout<<"    -use_acsub [0|1] - wykonaj obliczenia dla podzbioru acyklicznego"<<std::endl;
    std::cout<<"                       (def. "<<useAcyclicSubset<<")"<<std::endl;
    std::cout<<"    -use_acsub_o [0|1] - wykonaj obliczenia dla podzbioru acyklicznego (wersja online)"<<std::endl;
    std::cout<<"                         (def. "<<useAcyclicSubsetOnline<<")"<<std::endl;
    std::cout<<"    -use_acsub_st [0|1] - wykonaj obliczenia dla podzbioru acyklicznego (wersja z drzewem rozpinajacym)"<<std::endl;
    std::cout<<"                         (def. "<<useAcyclicSubsetSpanningTree<<")"<<std::endl;
    std::cout<<"    -use_acsub_p [0|1] - wykonaj rownolegle obliczenia dla podzbioru acyklicznego"<<std::endl;
    std::cout<<"                         (def. "<<useAcyclicSubsetParallel<<")"<<std::endl;
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
            index += 2;
        }
        else if (CHECK_ARG_NEXT("-rr"))
        {
            testType = 1;
            simplicesCount = atoi(argv[index + 1]);
            index += 2;
        }
        else if (CHECK_ARG_NEXT("-i"))
        {
            testType = 2;
            inputFilename = argv[index + 1];
            index += 2;
        }
        else if (CHECK_ARG_NEXT("-act"))
        {
            testType = 3;
            nodeSimplicesCount = atoi(argv[index + 1]);
            nodesCount = atoi(argv[index + 2]);
            index += 3;
        }
        else if (CHECK_ARG_NEXT("-l"))
        {
            testType = 4;
            inputFilename = argv[index + 1];
            index += 2;
        }
        else if (CHECK_ARG_NEXT("-verts")) { vertsCount = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-test")) { incidenceGraphParams.acyclicTestNumber = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-dim")) { incidenceGraphParams.dim = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-ss")) { incidenceGraphParams.sortNodes = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-sv")) { sortVerts = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-min")) { incidenceGraphParams.minimizeSimplices = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-ps")) { parallelParams.packSize = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-pc")) { parallelParams.packsCount = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-ppd")) { parallelParams.prepareData = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-paso")) { parallelParams.useAcyclicSubsetOnlineAlgorithm = atoi(argv[index + 1]); index += 2; }
        else if (CHECK_ARG_NEXT("-log")) { logFilename = argv[index + 1]; index+= 2; }
        else if (CHECK_ARG_NEXT("-use_alg")) { useAlgebraic = atoi(argv[index + 1]); index+= 2; }
        else if (CHECK_ARG_NEXT("-use_cored")) { useCoreduction = atoi(argv[index + 1]); index+= 2; }
        else if (CHECK_ARG_NEXT("-use_acsub")) { useAcyclicSubset = atoi(argv[index + 1]); index+= 2; }
        else if (CHECK_ARG_NEXT("-use_acsub_o")) { useAcyclicSubsetOnline = atoi(argv[index + 1]); index+= 2; }
        else if (CHECK_ARG_NEXT("-use_acsub_st")) { useAcyclicSubsetSpanningTree = atoi(argv[index + 1]); index+= 2; }
        else if (CHECK_ARG_NEXT("-use_acsub_p")) { useAcyclicSubsetParallel = atoi(argv[index + 1]); index+= 2; }
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
    return (rt == RT_AcyclicSubset || rt == RT_AcyclicSubsetOnline || rt == RT_AcyclicSubsetSpanningTree || rt == RT_AcyclicSubsetParallel);
}

////////////////////////////////////////////////////////////////////////////////

void Tests::GenerateData(SimplexList &simplexList)
{
    switch (testType)
    {
        case 0:
            GenerateSimplexList(simplexList, simplicesCount, vertsCount, incidenceGraphParams.dim);
            break;
        case 1:
            GenerateReverseSimplexList(simplexList, simplicesCount, vertsCount,  incidenceGraphParams.dim + 1);
            break;
        case 2:
            ReadSimplexList(simplexList, inputFilename.c_str(), sortVerts);
            break;
        case 3:
            GenerateAcyclicTree(simplexList, nodeSimplicesCount, nodesCount);
            break;
        default:
            break;
    }
    Timer::Update("data generated");
    MemoryInfo::PrintInfo();
}

////////////////////////////////////////////////////////////////////////////////

void Tests::Test(SimplexList &simplexList, ReductionType reductionType)
{
    float total = 0;
    float t = 0;

    Timer::Update();
    Timer::Time timeStart = Timer::Now();

    MemoryInfo::Reset();

    AcyclicTest<IncidenceGraph::IntersectionFlags> *test = IsAcyclicSubsetReduction(reductionType) ? AcyclicTest<IncidenceGraph::IntersectionFlags>::Create(incidenceGraphParams.acyclicTestNumber, incidenceGraphParams.dim) : 0;
    IncidenceGraph *ig = 0;
    if (reductionType == RT_AcyclicSubset)
    {
        ig = IncidenceGraph::CreateAndCalculateAcyclicSubset(simplexList, incidenceGraphParams, test);
    }
    else if (reductionType == RT_AcyclicSubsetOnline)
    {
        ig = IncidenceGraph::CreateAndCalculateAcyclicSubsetOnline(simplexList, incidenceGraphParams, test);
    }
    else if (reductionType == RT_AcyclicSubsetSpanningTree)
    {
        ig = IncidenceGraph::CreateAndCalculateAcyclicSubsetSpanningTree(simplexList, incidenceGraphParams, test);
    }
    else if (reductionType == RT_AcyclicSubsetParallel)
    {
        ig = IncidenceGraph::CreateAndCalculateAcyclicSubsetParallel(simplexList, incidenceGraphParams, parallelParams, test, true);
    }
    else // (reductionType == RT_Coreduction || reductionType == RT_None)
    {
        ig = IncidenceGraph::Create(simplexList, incidenceGraphParams);
    }
    if (test)
    {
        delete test;
    }
    MemoryInfo::PrintInfo();
    Timer::Update();
    total = Timer::TimeFrom(timeStart, "total graph processing");
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
    MemoryInfo::PrintInfo();
    float t = Timer::Update("creating output");
    log<<"\t\t\t<output_graph>"<<t<<"</output_graph>"<<std::endl;
    totalTime += t;

    totalTime += ComputeHomology(og, reductionType == RT_Coreduction);
    std::cout<<"total: "<<totalTime<<std::endl;
    log<<"\t\t\t<total>"<<totalTime<<"</total>"<<std::endl;
    MemoryInfo::PrintInfo();
    
    delete og;
}

void Tests::TestAndCompare(SimplexList &simplexList)
{
    incidenceGraphParams.dim = simplexList[0].size() - 1;
    if (incidenceGraphParams.dim < 2) incidenceGraphParams.dim = 2;
    if (incidenceGraphParams.acyclicTestNumber == 0) // jezeli tablice, to gorne ograniczenie na wymiar == 4
    {
        if (incidenceGraphParams.dim > 4) incidenceGraphParams.dim = 4;
    }

    std::cout<<"simplices count: "<<simplexList.size()<<std::endl;
    std::cout<<"dim: "<<incidenceGraphParams.dim<<std::endl;

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

    if (useAcyclicSubset)
    {
        std::cout<<"acyclic subset:"<<std::endl;
        log<<"\t\t<acyclic_subset>"<<std::endl<<std::endl;
        Test(simplexList, RT_AcyclicSubset);
        log<<"\t\t</acyclic_subset>"<<std::endl<<std::endl;
    }

    if (useAcyclicSubsetOnline)
    {
        std::cout<<std::endl<<"acyclic subset online:"<<std::endl;
        log<<"\t\t<acyclic_subset_online>"<<std::endl<<std::endl;
        Test(simplexList, RT_AcyclicSubsetOnline);
        log<<"\t\t</acyclic_subset_online>"<<std::endl<<std::endl;
        cout<<std::endl<<std::endl;
    }

    if (useAcyclicSubsetSpanningTree)
    {
        std::cout<<std::endl<<"acyclic subset with spanning tree:"<<std::endl;
        log<<"\t\t<acyclic_subset_with_spanning_tree>"<<std::endl<<std::endl;
        Test(simplexList, RT_AcyclicSubsetSpanningTree);
        log<<"\t\t</acyclic_subset_with_spanning_tree>"<<std::endl<<std::endl;
    }

    if (useAcyclicSubsetParallel)
    {
        std::cout<<std::endl<<"acyclic subset parallel:"<<std::endl;
        log<<"\t\t<acyclic_subset_parallel>"<<std::endl<<std::endl;
        Test(simplexList, RT_AcyclicSubsetParallel);
        log<<"\t\t</acyclic_subset_parallel>"<<std::endl<<std::endl;
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
            sprintf(buff, "random set of size %d, randomly removed %d simplices", (int)simplexList.size(), simplicesCount);
            break;
        case 2:
            sprintf(buff, "testing %s", inputFilename.c_str());
            break;
        case 3:
            sprintf(buff, "acyclic tree of %d nodes %d simplices each, total: %d", nodesCount, nodeSimplicesCount, (int)simplexList.size());
            break;
        default:
            break;
    }

    std::cout<<buff<<std::endl;
    log<<"<input_size>"<<simplexList.size()<<"</input_size>"<<std::endl;
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
#ifdef DEBUG_MEMORY
    std::cout<<"sdf"<<std::endl;
    MemoryInfo::PrintInfo();
#endif
}

////////////////////////////////////////////////////////////////////////////////

int Tests::GetBetti(SimplexList &simplexList, int n)
{
    IncidenceGraph *ig = IncidenceGraph::Create(simplexList, incidenceGraphParams);
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
        MemoryInfo::PrintInfo();
    }

    Timer::Update();
    CRef<ReducibleFreeChainComplexType> RFCComplexCR = (ReducibleFreeChainComplexOverZFromSComplexAlgorithm<SComplex<SComplexDefaultTraits>, ReducibleFreeChainComplexType>(complex))();
    CRef<HomologySignature<int> > homSignCR = HomAlgFunctors<FreeModuleType>::homSignViaAR_Random(RFCComplexCR);
    t = Timer::Update("computing homology");
    log<<"\t\t\t<homology>"<<t<<"</homology>"<<std::endl;
    total += t;
    MemoryInfo::PrintInfo();
    
//    testsLog<<homSignCR();
    std::cout<<homSignCR();

    return total;
}

////////////////////////////////////////////////////////////////////////////////

void Tests::OpenLog()
{
    log.open(logFilename.c_str());
    log<<"<tests>"<<std::endl<<std::endl;
    log<<"<simplices_count>"<<simplicesCount<<"</simplices_count>"<<std::endl;
    log<<"<vertices_count>"<<vertsCount<<"</vertices_count>"<<std::endl;
    log<<"<input>"<<inputFilename<<"</input>"<<std::endl;
    log<<"<acyclic_test_number>"<<incidenceGraphParams.acyclicTestNumber<<"</acyclic_test_number>"<<std::endl;
    log<<"<dim>"<<incidenceGraphParams.dim<<"</dim>"<<std::endl;
    log<<"<minimalization>"<<incidenceGraphParams.minimizeSimplices<<"</minimalization>"<<std::endl;
}

void Tests::CloseLog()
{
    log<<"</tests>"<<std::endl;
    log.close();
}

////////////////////////////////////////////////////////////////////////////////
// eof
