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

        using Instruments::VanillaCapFloor;

        void AnalyticalCapFloor::calculate() const {
            QL_REQUIRE(!model_.isNull(), 
                       "AnalyticalCapFloor: cannot price without model!");

            double value = 0.0;
            VanillaCapFloor::Type type = parameters_.type;
            Size nPeriods = parameters_.endTimes.size();
            for (Size i=0; i<nPeriods; i++) {
                Time maturity = parameters_.startTimes[i];
                Time bond = parameters_.endTimes[i];
                Time tenor = parameters_.accrualTimes[i];

                if ((type == VanillaCapFloor::Cap) ||
                    (type == VanillaCapFloor::Collar)) {
                    double temp = 1.0+parameters_.capRates[i]*tenor;
                    value += parameters_.nominals[i]*temp*
                        model_->discountBondOption(Option::Put, 1.0/temp, 
                                                   maturity, bond);
                }
                if ((type == VanillaCapFloor::Floor) ||
                    (type == VanillaCapFloor::Collar)) {
                    double temp = 1.0+parameters_.floorRates[i]*tenor;
                    double mult = (type == VanillaCapFloor::Floor) ? 1.0 : -1.0;
                    value += parameters_.nominals[i]*temp*mult*
                        model_->discountBondOption(Option::Call, 1.0/temp, 
                                                   maturity, bond);
                }

            }
            results_.value = value;
        }

    }

}
