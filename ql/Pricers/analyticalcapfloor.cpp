
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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
*/

#include "ql/Pricers/analyticalcapfloor.hpp"

namespace QuantLib {

    namespace Pricers {

        using Instruments::CapFloor;

        void AnalyticalCapFloor::calculate() const {
            QL_REQUIRE(!model_.isNull(), 
                       "AnalyticalCapFloor: cannot price without model!");

            double value = 0.0;
            CapFloor::Type type = arguments_.type;
            Size nPeriods = arguments_.endTimes.size();
            for (Size i=0; i<nPeriods; i++) {
                Time maturity = arguments_.startTimes[i];
                Time bond = arguments_.endTimes[i];
                Time tenor = arguments_.accrualTimes[i];

                if ((type == CapFloor::Cap) ||
                    (type == CapFloor::Collar)) {
                    double temp = 1.0+arguments_.capRates[i]*tenor;
                    value += arguments_.nominals[i]*temp*
                        model_->discountBondOption(Option::Put, 1.0/temp, 
                                                   maturity, bond);
                }
                if ((type == CapFloor::Floor) ||
                    (type == CapFloor::Collar)) {
                    double temp = 1.0+arguments_.floorRates[i]*tenor;
                    double mult = (type == CapFloor::Floor) ? 1.0 : -1.0;
                    value += arguments_.nominals[i]*temp*mult*
                        model_->discountBondOption(Option::Call, 1.0/temp, 
                                                   maturity, bond);
                }

            }
            results_.value = value;
        }

    }

}
