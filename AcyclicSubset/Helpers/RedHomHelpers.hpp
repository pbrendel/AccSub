/* 
 * File:   RedHomHelpers.hpp
 * Author: Piotr Brendel
 */

#ifndef REDHOMHELPERS_HPP
#define	REDHOMHELPERS_HPP

#include "../AcyclicSubset/IncidenceGraphHelpers.hpp"
#include "Utils.hpp"

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

template <typename OutputGraph>
class RedHomHelpers
{
public:

    static void GetDimsAndKappaMap(OutputGraph *og, Complex::Dims &dims, Complex::KappaMap &kappaMap)
    {
        for (typename OutputGraph::Nodes::iterator i = og->nodes.begin(); i != og->nodes.end(); i++)
        {
            dims.push_back((*i)->simplex.size() - 1);
            int index = (*i)->index;
            std::vector<int>::iterator k = (*i)->kappa.begin();
            for (typename OutputGraph::Nodes::iterator j = (*i)->subnodes.begin(); j != (*i)->subnodes.end(); j++)
            {
                kappaMap.push_back(boost::tuple<Id, Id, int>(index, (*j)->index, (*k)));
                k++;
            }
        }
    }

    static void ComputeHomology(OutputGraph *og, bool performCoreductions)
    {
        Complex::Dims dims;
        Complex::KappaMap kappaMap;
        GetDimsAndKappaMap(og, dims, kappaMap);
        Complex complex(3, dims, kappaMap, 1);

        Timer::Update("creating complex");

        if (performCoreductions)
        {
            (*CoreductionAlgorithmFactory<Complex>::createDefault(complex))();
            Timer::Update("performing coreductions");
            MemoryInfo::Print();
        }

        CRef<ReducibleFreeChainComplexType> RFCComplexCR = (ReducibleFreeChainComplexOverZFromSComplexAlgorithm<SComplex<SComplexDefaultTraits>, ReducibleFreeChainComplexType>(complex))();
        CRef<HomologySignature<int> > homSignCR = HomAlgFunctors<FreeModuleType>::homSignViaAR_Random(RFCComplexCR);

        Timer::Update("computing homology");

        std::cout<<homSignCR();
    }
    
    static int GetBettiNumber(typename OutputGraph::SimplexList &simplexList, int n)
    {
        typedef typename OutputGraph::IncidenceGraph IncidenceGraph;
        
        IncidenceGraph *ig = IncidenceGraphHelpers<IncidenceGraph>::Create(simplexList);
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
};

#endif	/* REDHOMHELPERS_HPP */

