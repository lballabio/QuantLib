
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2002, 2003 Sad Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file binomialvanillaengine.cpp
    \brief Vanilla option engine using binomial tree

    \fullpath
    ql/Pricers/%binomialvanillaengine.cpp
*/

// $Id$

#include <ql/PricingEngines/discretizedvanillaoption.hpp>
#include <ql/Lattices/binomialtree.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>

using namespace QuantLib::Lattices;
using namespace QuantLib::TermStructures;
using namespace QuantLib::VolTermStructures;

namespace QuantLib {

    namespace PricingEngines {

        void BinomialVanillaEngine::calculate() const {
            double s0 = arguments_.underlying;
            double v = arguments_.volTS->blackVol(arguments_.maturity, s0);
            Rate r = arguments_.riskFreeTS->zeroYield(arguments_.maturity);
            Rate q = arguments_.dividendTS->zeroYield(arguments_.maturity);
            Date referenceDate = arguments_.riskFreeTS->referenceDate();
            Date todaysDate    = arguments_.riskFreeTS->todaysDate();
            DayCounter dc      = arguments_.riskFreeTS->dayCounter();

            // binomial trees with constant coefficient
            RelinkableHandle<TermStructure> flatRiskFree(
                Handle<TermStructure>(new
                FlatForward(todaysDate, referenceDate, r, dc)));
            RelinkableHandle<TermStructure> flatDividends(
                Handle<TermStructure>(new
                FlatForward(todaysDate, referenceDate, q, dc)));
            RelinkableHandle<BlackVolTermStructure> flatVol(
                Handle<BlackVolTermStructure>(new
                BlackConstantVol(referenceDate, v, dc)));


            
            
            Handle<Lattices::Tree> tree;
            Handle<DiffusionProcess> bs(new
                BlackScholesProcess(flatRiskFree, flatDividends, flatVol,s0));
            switch(type_) {
                case CoxRossRubinstein:
                    tree = Handle<Tree>(new
                        Lattices::CoxRossRubinstein(bs, arguments_.maturity, steps_));
                    break;
                case JarrowRudd:
                    tree = Handle<Tree>(new
                        Lattices::JarrowRudd(bs, arguments_.maturity, steps_));
                    break;
                case EQP:
                    tree = Handle<Tree>(new
                        Lattices::AdditiveEQPBinomialTree(bs, arguments_.maturity, steps_));
                    break;
                case Trigeorgis:
                    tree = Handle<Tree>(new
                        Lattices::Trigeorgis(bs, arguments_.maturity, steps_));
                    break;
                case Tian:
                    tree = Handle<Tree>(new
                        Lattices::Tian(bs, arguments_.maturity, steps_));
                    break;
                default:
                    throw IllegalArgumentError(
                        "BinomialVanillaEngine::calculate :"
                        "undefined binomial model");
            }

            Handle<Lattice> lattice(new
                BlackScholesLattice(tree, r, arguments_.maturity, steps_));

            Handle<DiscretizedAsset> option(
                new DiscretizedVanillaOption(lattice,arguments_));

            lattice->initialize(option, arguments_.maturity);
            lattice->rollback(option, 0.0);
            results_.value = lattice->presentValue(option);
        }

    }

}

