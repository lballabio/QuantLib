/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

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
/*! \file analyticalcapfloor.cpp
    \brief Analytical pricer for caps/floors

    \fullpath
    ql/Pricers/%analyticalcapfloor.cpp
*/

// $Id$

#include "ql/Pricers/analyticalcapfloor.hpp"

namespace QuantLib {

    namespace Pricers {

        void AnalyticalCapFloor::calculate() const {
            QL_REQUIRE(!model_.isNull(), "Cannot price without model!");

            QL_REQUIRE(model_->hasDiscountBondOptionFormula(),
                "No analytical formula for discount bond options");

            Option::Type optionType;
            switch (parameters_.type) {
              case Instruments::VanillaCapFloor::Cap:
                optionType = Option::Put;
                break;
              case Instruments::VanillaCapFloor::Floor:
                optionType = Option::Call;
                break;
              default:
                throw Error("Invalid cap/floor type");
            }

            double value = 0.0;
            Size nPeriods = parameters_.endTimes.size();
            for (Size i=0; i<nPeriods; i++) {
                Rate exerciseRate = parameters_.exerciseRates[i];

                Time maturity = parameters_.startTimes[i];
                Time bond = parameters_.endTimes[i];
                Time tenor = bond - maturity;
                double optionStrike = 1.0/(1.0+exerciseRate*tenor);

                double optionValue = model_->discountBondOption(
                    optionType, optionStrike, maturity, bond);

                double capletValue = parameters_.nominals[i]*
                    (1.0+exerciseRate*tenor)*optionValue;
                value += capletValue;
            }
            results_.value = value;
        }

    }

}
