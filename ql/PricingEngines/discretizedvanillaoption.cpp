/*
 Copyright (C) 2002, 2003 Sadruddin Rejeb

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
/*! \file discretizedvanillaoption.cpp
    \brief Discretized Vanilla Option

    \fullpath
    ql/PricingEngines/%discretizedvanillaoption.cpp
*/

// $Id$

#include <ql/PricingEngines/discretizedvanillaoption.hpp>

namespace QuantLib {

    namespace PricingEngines {

        void DiscretizedVanillaOption::reset(Size size) {
            values_ = Array(size, 0.0);
            adjustValues();
        }

        void DiscretizedVanillaOption::adjustValues() {
            Date referenceDate = arguments_.riskFreeTS->referenceDate();
            double residualTime = 
                arguments_.riskFreeTS->dayCounter().yearFraction(
                    referenceDate, arguments_.exercise.date());
            if (isOnTime(residualTime)) {
                applySpecificCondition();
            }
        }

        void DiscretizedVanillaOption::applySpecificCondition() {
            Handle<Lattices::BlackScholesLattice> lattice(method());
            QL_REQUIRE(!lattice.isNull(),
                       "Must use a BlackScholes lattice");
            Handle<Lattices::Tree> tree(lattice->tree());
            Size i = tree->nColumns() - 1;
            double strike = arguments_.strike;

            Array prices(values_.size(), 0.0);
            for (Size j=0; j<values_.size(); j++) {
                prices[j] = Pricers::ExercisePayoff(arguments_.type,
                    tree->underlying(i, j), strike);
                values_[j] = QL_MAX(values_[j], prices[j]);
            }
        }


    }

}

