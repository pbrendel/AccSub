/* 
 * File:   RedHomHelpers.hpp
 * Author: Piotr Brendel
 *         piotr.brendel@ii.uj.edu.pl
 *
 *         AccSub - constructing and removing acyclic subset
 *                  for simplicial complexes
 *         This code is a part of RedHom library
 *         http://redhom.ii.uj.edu.pl
 */

#ifndef REDHOMHELPERS_HPP
#define	REDHOMHELPERS_HPP

#include "../AcyclicSubset/IncidenceGraphHelpers.hpp"
#include "Utils.hpp"

////////////////////////////////////////////////////////////////////////////////
// RedHom stuff

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

#include <redHom/complex/simplicial/SimplexSComplex.hpp>

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

class RedHomHelpers
{
public:

    template <typename OutputGraph>
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

    template <typename OutputGraph>
    static void ComputeHomology(OutputGraph *og, bool performCoreductions)
    {
        Complex::Dims dims;
        Complex::KappaMap kappaMap;
        GetDimsAndKappaMap(og, dims, kappaMap);
        Complex complex(3, dims, kappaMap, 1);

        Timer::Update("creating complex");

        if (performCoreductions)
        {
            (*CoreductionAlgorithmFactory<Complex, int>::createDefault(complex))();
            Timer::Update("performing coreductions");
            MemoryInfo::Print();
        }

        CRef<ReducibleFreeChainComplexType> RFCComplexCR = (ReducibleFreeChainComplexOverZFromSComplexAlgorithm<Complex, ReducibleFreeChainComplexType>(complex))();
        CRef<HomologySignature<int> > homSignCR = HomAlgFunctors<FreeModuleType>::homSignViaAR_Random(RFCComplexCR);

        Timer::Update("computing homology");

        std::cout<<homSignCR();
    }

    template <typename SimplexList>
    static void ComputeHomology(SimplexList &simplexList, bool performCoreductions)
    {
        SimplexSComplex complex;
        for (typename SimplexList::iterator i = simplexList.begin(); i != simplexList.end(); i++)
        {
            std::set<int> simplex;
            simplex.insert(i->begin(), i->end());
            complex.addSimplex(simplex);
        }

        Timer::Update("creating complex");

        if (performCoreductions)
        {
            (*CoreductionAlgorithmFactory<SimplexSComplex, int>::createDefault(complex))();
            Timer::Update("performing coreductions");
            MemoryInfo::Print();
        }

        CRef<ReducibleFreeChainComplexType> RFCComplexCR = (ReducibleFreeChainComplexOverZFromSComplexAlgorithm<SimplexSComplex, ReducibleFreeChainComplexType>(complex))();
        CRef<HomologySignature<int> > homSignCR = HomAlgFunctors<FreeModuleType>::homSignViaAR_Random(RFCComplexCR);

        Timer::Update("computing homology");

        std::cout<<homSignCR();
    }

    template <typename SimplexList>
    static int GetBettiNumber(SimplexList &simplexList, int n)
    {
        SimplexSComplex complex;
        for (typename SimplexList::iterator i = simplexList.begin(); i != simplexList.end(); i++)
        {
            std::set<int> simplex;
            simplex.insert(i->begin(), i->end());
            complex.addSimplex(simplex);
        }

        (*CoreductionAlgorithmFactory<SimplexSComplex, float>::createDefault(complex))();
        CRef<ReducibleFreeChainComplexType> RFCComplexCR=
            (ReducibleFreeChainComplexOverZFromSComplexAlgorithm<SimplexSComplex, ReducibleFreeChainComplexType>(complex))();
        CRef<HomologySignature<int> > homSignCR = HomAlgFunctors<FreeModuleType>::homSignViaAR_Random(RFCComplexCR);

        return homSignCR().bettiNumber(n);
    }

    template <typename SimplexList>
    static bool IsTrivialHomology(SimplexList &simplexList)
    {
        SimplexSComplex complex;
        for (typename SimplexList::iterator i = simplexList.begin(); i != simplexList.end(); i++)
        {
            std::set<int> simplex;
            simplex.insert(i->begin(), i->end());
            complex.addSimplex(simplex);
        }

        CRef<ReducibleFreeChainComplexType> RFCComplexCR = (ReducibleFreeChainComplexOverZFromSComplexAlgorithm<SimplexSComplex, ReducibleFreeChainComplexType>(complex))();
        CRef<HomologySignature<int> > homSignCR = HomAlgFunctors<FreeModuleType>::homSignViaAR_Random(RFCComplexCR);
        return (homSignCR().topDim() < 0 || (homSignCR().topDim() == 0 && homSignCR().bettiNumber(0) == 1));
    }
};

#endif	/* REDHOMHELPERS_HPP */

