
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
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
            if (parameters_.type==Instruments::VanillaCapFloor::Cap)
                optionType = Option::Put;
            else
                optionType = Option::Call;

            double value = 0.0;
            size_t nPeriods = parameters_.endTimes.size();
            const std::vector<Rate>& exerciseRates = parameters_.exerciseRates;
            for (size_t i=0; i<nPeriods; i++) {
                Rate exerciseRate;
                if (i<exerciseRates.size())
                    exerciseRate = exerciseRates[i];
                else
                    exerciseRate = exerciseRates.back();

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
