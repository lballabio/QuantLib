
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

/*! \file europeanbinomialengine.cpp
    \brief European option engine using binomial methods

    \fullpath
    ql/Pricers/%europeanbinomialengine.cpp
*/

// $Id$

#include <ql/PricingEngines/discretizedvanillaoption.hpp>

using QuantLib::Pricers::ExercisePayoff;

namespace QuantLib {

    namespace PricingEngines {

        void EuropeanBinomialEngine::calculate() const {
            double s0 = arguments_.underlying;
            Date referenceDate = arguments_.riskFreeTS->referenceDate();
            double v = arguments_.volTS->blackVol(referenceDate, s0);
            Rate r = arguments_.riskFreeTS->zeroYield(referenceDate);
            Rate q = arguments_.dividendTS->zeroYield(referenceDate);
            Time t = arguments_.riskFreeTS->dayCounter().yearFraction(
                referenceDate, arguments_.exercise.date());

            Handle<Lattices::Tree> tree;
            switch(type_) {
                case CoxRossRubinstein:
                    tree = Handle<Lattices::Tree>(
                        new Lattices::CoxRossRubinstein(v, r, q, 
                                                        s0, t, steps_));
                    break;
                case JarrowRudd:
                    tree = Handle<Lattices::Tree>(
                        new Lattices::JarrowRudd(v, r, q, s0, t, steps_));
                    break;
            }

            Handle<Lattices::Lattice> lattice(
                new Lattices::BlackScholesLattice(tree, r, t, steps_));

            Handle<DiscretizedAsset> option(
                new DiscretizedVanillaOption(lattice, arguments_));

            lattice->initialize(option, t);
            lattice->rollback(option, 0.0);
            results_.value = lattice->presentValue(option);
        }

    }

}

