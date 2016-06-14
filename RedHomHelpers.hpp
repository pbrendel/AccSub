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

#include <cstdlib>
#include <iostream>
#include <string>

////////////////////////////////////////////////////////////////////////////////
// RedHom stuff

#include <capd/complex/SComplex.h>
#include <capd/complex/SComplexDefaultTraits.h>
#include <capd/complex/SimplexSComplex.hpp>
#include <capd/complex/Coreduction.h>
#include <capd/complex/AKQStrategy.hpp>
#include <capd/homAlgebra/HomologySignature.h>
#include <capd/complex/BettiNumbers.hpp>

////////////////////////////////////////////////////////////////////////////////

class RedHomHelpers
{
    typedef capd::complex::SComplex<capd::complex::SComplexDefaultTraits>   SComplex;
    typedef SComplex::Id                            Id;
    typedef int                                     Scalar;
    typedef capd::complex::SimplexSComplex          SimplexSComplex;
    typedef std::vector<int>                        Betti;
    typedef HomologySignature<Scalar, int>          Homology;

public:

    template <typename OutputGraph>
    static void ComputeBettiNumbers(OutputGraph *og, bool performCoreductions, bool performAllReductions)
    {
        SComplex::Dims dims;
        SComplex::KappaMap kappaMap;
        RedHomHelpers::GetDimsAndKappaMap(og, dims, kappaMap);
        SComplex complex(3, dims, kappaMap, 1);
        Timer::Update("creating complex");

        Betti betti;
        if (performCoreductions)
        {
            RedHomHelpers::GetBettiNumbersThroughCoreduction(complex, betti, performAllReductions, true);
        }
        else
        {
            RedHomHelpers::GetBettiNumbers(complex, betti, performAllReductions, true);
        }

        RedHomHelpers::PrintBettiNumbers(betti);
    }

    template <typename SimplexList>
    static void ComputeBettiNumbers(SimplexList &simplexList, bool performCoreductions, bool performAllReductions)
    {
        SimplexSComplex complex;
        RedHomHelpers::CreateSimplexSComplex(simplexList, complex);
        Timer::Update("creating complex");

        Betti betti;
        if (performCoreductions)
        {
            RedHomHelpers::GetBettiNumbersThroughCoreduction(complex, betti, performAllReductions, true);
        }
        else
        {
            RedHomHelpers::GetBettiNumbers(complex, betti, performAllReductions, true);
        }

        RedHomHelpers::PrintBettiNumbers(betti);
    }

    template <typename SimplexList>
    static int GetBettiNumber(SimplexList &simplexList, int n)
    {
        SimplexSComplex complex;
        RedHomHelpers::CreateSimplexSComplex(simplexList, complex);
        Betti betti;
        RedHomHelpers::GetBettiNumbers(complex, betti, true, false);
        return betti[n];
    }

    template <typename SimplexList>
    static bool IsTrivialHomology(SimplexList &simplexList)
    {
        SimplexSComplex complex;
        RedHomHelpers::CreateSimplexSComplex(simplexList, complex);
        Betti betti;
        RedHomHelpers::GetBettiNumbers(complex, betti, true, false);
        return (betti.size() == 0 || (betti.size() == 1 && betti[0] == 1));
    }

    template <typename SimplexList>
    static bool IsFullyReducible(SimplexList &simplexList)
    {
        typedef capd::complex::AKQReduceStrategy<SimplexSComplex, SComplex, Scalar> Strategy;
        typedef capd::complex::Coreduction<Strategy, Scalar, int>                   Coreduction;

        SimplexSComplex complex;
        RedHomHelpers::CreateSimplexSComplex(simplexList, complex);

        boost::shared_ptr<Coreduction> coreduction = boost::shared_ptr<Coreduction>(new Coreduction(new Strategy(complex)));
        (*coreduction)();

        Homology hom = coreduction->getExtractedSignature();
        return (hom.size() == 1 && hom.bettiNumber(0) == 1);
    }

private:

    template <typename SimplexList>
    static void CreateSimplexSComplex(SimplexList &simplexList, SimplexSComplex& complex)
    {
        for (typename SimplexList::iterator i = simplexList.begin(); i != simplexList.end(); i++)
        {
            std::set<int> simplex;
            simplex.insert(i->begin(), i->end());
            complex.addSimplex(simplex);
        }
    }

    template <typename OutputGraph>
    static void GetDimsAndKappaMap(OutputGraph *og, SComplex::Dims &dims, SComplex::KappaMap &kappaMap)
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

    template <typename SComplexType>
    static void GetBettiNumbers(SComplexType &complex, Betti &betti, bool performAllReductions, bool updateTimer)
    {
        typedef capd::complex::BettiNumbers<SComplexType, Scalar, int> BettiNumbersAlg;
        BettiNumbersAlg alg(complex, performAllReductions);
        betti = alg();
        if (updateTimer)
        {
            Timer::Update("computing homology");
        }
    }

    template <typename SComplexType>
    static void GetBettiNumbersThroughCoreduction(SComplexType& inputComplex, Betti &betti, bool performAllReductions, bool updateTimer)
    {
        typedef capd::complex::AKQReduceStrategy<SComplexType, SComplex, Scalar>   Strategy;
        typedef capd::complex::Coreduction<Strategy, Scalar, int>                  Coreduction;

        boost::shared_ptr<Coreduction> coreduction = boost::shared_ptr<Coreduction>(new Coreduction(new Strategy(inputComplex)));
        (*coreduction)();
        if (updateTimer)
        {
            Timer::Update("performing coreductions");
            MemoryInfo::Print();
        }

        typedef capd::complex::BettiNumbers<SComplex, Scalar, int> BettiNumbersAlg;
        BettiNumbersAlg alg(*coreduction->getStrategy()->getOutputComplex(), performAllReductions);
        betti = alg();
        if (updateTimer)
        {
            Timer::Update("computing homology");
        }
    }

    static void PrintBettiNumbers(const Betti& betti)
    {
        if (betti.size() == 0)
        {
            std::cout<<"H_0 = 0"<<std::endl;
        }
        for (size_t i = 0; i < betti.size(); i++)
        {
            if (betti[i] > 0)
            {
                std::cout<<"H_"<<i<<" = Z^"<<betti[i]<<std::endl;
            }
            else
            {
                std::cout<<"H_"<<i<<" = 0"<<std::endl;
            }
        }
    }
};

#endif	/* REDHOMHELPERS_HPP */

