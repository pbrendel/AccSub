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

#include "IncidenceGraphHelpers.hpp"
#include "Utils.hpp"

////////////////////////////////////////////////////////////////////////////////
// RedHom stuff

#include <cstdlib>
#include <iostream>
#include <string>

#include <capd/complex/SComplex.h>
#include <capd/complex/SComplexDefaultTraits.h>
#include <capd/complex/SimplexSComplex.hpp>

////////////////////////////////////////////////////////////////////////////////
// RedHom setup

using namespace capd::complex;

typedef SComplex<SComplexDefaultTraits> Complex;
typedef Complex::Id Id;
typedef int ScalarType;

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

//        if (performCoreductions)
//        {
//            (*CoreductionAlgorithmFactory<Complex, ScalarType>::createDefault(complex))();
//            Timer::Update("performing coreductions");
//            MemoryInfo::Print();
//        }
//
//        CRef<HomologySignature<int> > homSignCR = GetHomologySignature(complex);
//        Timer::Update("computing homology");
//        std::cout<<homSignCR();
    }

    template <typename SimplexList>
    static void ComputeHomology(SimplexList &simplexList, bool performCoreductions)
    {
//        SimplexSComplex complex;
//        for (typename SimplexList::iterator i = simplexList.begin(); i != simplexList.end(); i++)
//        {
//            std::set<int> simplex;
//            simplex.insert(i->begin(), i->end());
//            complex.addSimplex(simplex);
//        }
//
//        Timer::Update("creating complex");
//
//        if (performCoreductions)
//        {
//            (*CoreductionAlgorithmFactory<SimplexSComplex, ScalarType>::createDefault(complex))();
//            Timer::Update("performing coreductions");
//            MemoryInfo::Print();
//        }
//
//        CRef<HomologySignature<int> > homSignCR = GetHomologySignature(complex);
//        Timer::Update("computing homology");
//        std::cout<<homSignCR();
    }

    template <typename SimplexList>
    static int GetBettiNumber(SimplexList &simplexList, int n)
    {
//        SimplexSComplex complex;
//        for (typename SimplexList::iterator i = simplexList.begin(); i != simplexList.end(); i++)
//        {
//            std::set<int> simplex;
//            simplex.insert(i->begin(), i->end());
//            complex.addSimplex(simplex);
//        }
//
//        (*CoreductionAlgorithmFactory<SimplexSComplex, ScalarType>::createDefault(complex))();
//        CRef<HomologySignature<int> > homSignCR = GetHomologySignature(complex);
//        return homSignCR().bettiNumber(n);
    }

    template <typename SimplexList>
    static bool IsTrivialHomology(SimplexList &simplexList)
    {
//        SimplexSComplex complex;
//        for (typename SimplexList::iterator i = simplexList.begin(); i != simplexList.end(); i++)
//        {
//            std::set<int> simplex;
//            simplex.insert(i->begin(), i->end());
//            complex.addSimplex(simplex);
//        }
//
//        CRef<HomologySignature<int> > homSignCR = GetHomologySignature(complex);
//        return (homSignCR().topDim() < 0 || (homSignCR().topDim() == 0 && homSignCR().bettiNumber(0) == 1));
    }

    template <typename SimplexList>
    static bool IsFullyReducible(SimplexList &simplexList)
    {
//        SimplexSComplex complex;
//        for (typename SimplexList::iterator i = simplexList.begin(); i != simplexList.end(); i++)
//        {
//            std::set<int> simplex;
//            simplex.insert(i->begin(), i->end());
//            complex.addSimplex(simplex);
//        }
//
//        typedef typename CoreductionAlgorithmFactory<SimplexSComplex, ScalarType>::DefaultAlgorithm Coreduction;
//        boost::shared_ptr<Coreduction> algorithm = CoreductionAlgorithmFactory<SimplexSComplex, ScalarType>::createDefault(complex);
//        algorithm->setStoreReducedCells(true);
//        (*algorithm)();
//
//        if (complex.size(1) == 0)
//        {
//            return ((*algorithm).getExtractedCells().size() == 1);
//        }
        return false;
    }

private:

//    template <typename ComplexType>
//    static CRef<HomologySignature<int> > GetHomologySignature(ComplexType &complex)
//    {
//        typedef typename SComplexFiltrT<ComplexType>::CellFreeModule FreeModuleType;
//        typedef ReducibleFreeChainComplex<FreeModuleType> ReducibleFreeChainComplexType;
//
//        ReducibleFreeChainComplexOverZFromSComplexAlgorithm<ComplexType, ScalarType> rfcBuilder(complex);
//        CRef<ReducibleFreeChainComplexType> rfcComplexCR = rfcBuilder.template build<ReducibleFreeChainComplexType>();
//        return HomAlgFunctors<FreeModuleType>::homSignViaAR_Random(rfcComplexCR);
//    }
};

#endif	/* REDHOMHELPERS_HPP */

