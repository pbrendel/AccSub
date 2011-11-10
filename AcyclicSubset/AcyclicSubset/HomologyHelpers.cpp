/* 
 * File:   HomologyHelpers.cpp
 * Author: Piotr Brendel
 */

#include "HomologyHelpers.h"
#include "IncidenceGraph.h"
#include "IncidenceGraphHelpers.h"
#include "OutputGraph.h"

#ifdef USE_HELPERS
#include "../Helpers/Utils.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// RedHom stuff

long double powl(long double x, int y)
{
    return pow(x, y);
}

ofstreamcout fcout;

////////////////////////////////////////////////////////////////////////////////

void HomologyHelpers::GetDimsAndKappaMap(OutputGraph *og, Complex::Dims &dims, Complex::KappaMap &kappaMap)
{
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
}

////////////////////////////////////////////////////////////////////////////////

void HomologyHelpers::ComputeHomology(OutputGraph *og, bool performCoreductions)
{
    Complex::Dims dims;
    Complex::KappaMap kappaMap;
    GetDimsAndKappaMap(og, dims, kappaMap);
    Complex complex(3, dims, kappaMap, 1);    
#ifdef USE_HELPERS
    Timer::Update("creating complex");
#endif    
    
    if (performCoreductions)
    {
        (*CoreductionAlgorithmFactory<Complex>::createDefault(complex))();
#ifdef USE_HELPERS
        Timer::Update("performing coreductions");  
        MemoryInfo::Print();
#endif
    }

    CRef<ReducibleFreeChainComplexType> RFCComplexCR = (ReducibleFreeChainComplexOverZFromSComplexAlgorithm<SComplex<SComplexDefaultTraits>, ReducibleFreeChainComplexType>(complex))();
    CRef<HomologySignature<int> > homSignCR = HomAlgFunctors<FreeModuleType>::homSignViaAR_Random(RFCComplexCR);

#ifdef USE_HELPERS
    Timer::Update("computing homology");
#endif
    
    std::cout<<homSignCR();    
}

////////////////////////////////////////////////////////////////////////////////

int HomologyHelpers::GetBettiNumber(SimplexList &simplexList, int n)
{
    IncidenceGraph *ig = IncidenceGraphHelpers::Create(simplexList);
    OutputGraph *og = new OutputGraph(ig);

    Complex::Dims dims;
    Complex::KappaMap kappaMap;
    GetDimsAndKappaMap(og, dims, kappaMap);

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
// eof
