
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

/*! \file blackcapfloor.cpp
    \brief European capfloor calculated using Black formula

    \fullpath
    ql/Pricers/%blackcapfloor.cpp
*/

// $Id$

#include "ql/Pricers/blackcapfloor.hpp"

namespace QuantLib {

    namespace Pricers {

        using Instruments::VanillaCapFloor;

        void BlackCapFloor::calculate() const {
            double value = 0.0;
            VanillaCapFloor::Type type = arguments_.type;

            for (Size i=0; i<arguments_.startTimes.size(); i++) {
                Time start = arguments_.startTimes[i],
                     end = arguments_.endTimes[i],
                     accrualTime = arguments_.accrualTimes[i];
                if (end > 0.0) {    // discard expired caplets
                    double nominal = arguments_.nominals[i];
                    DiscountFactor q = model_->termStructure()->discount(end);
                    Rate forward = arguments_.forwards[i];
                    // try and factorize the code below
                    if ((type == VanillaCapFloor::Cap) ||
                        (type == VanillaCapFloor::Collar)) {
                            value += q * accrualTime * nominal *
                                     capletValue(start,forward,
                                                 arguments_.capRates[i],
                                                 model_->volatility());
                    }
                    if ((type == VanillaCapFloor::Floor) ||
                        (type == VanillaCapFloor::Collar)) {
                            double temp = q * accrualTime * nominal *
                                          floorletValue(start,forward,
                                                 arguments_.floorRates[i],
                                                 model_->volatility());
                            if (type == VanillaCapFloor::Floor)
                                value += temp;
                            else
                                // a collar is long a cap and short a floor
                                value -= temp;
                    }
                }
            }
            results_.value = value;

        }

        double BlackCapFloor::capletValue(Time start, Rate forward,
                                          Rate strike, double vol) const {
            if (start <= 0.0) {
                // the rate was fixed
                return QL_MAX(forward-strike,0.0);
            } else {
                // forecast
                return BlackModel::formula(forward, strike, 
                                           vol*QL_SQRT(start), 1);
            }
        }
        
        double BlackCapFloor::floorletValue(Time start, Rate forward,
                                            Rate strike, double vol) const {
            if (start <= 0.0) {
                // the rate was fixed
                return QL_MAX(strike-forward,0.0);
            } else {
                // forecast
                return BlackModel::formula(forward, strike, 
                                           vol*QL_SQRT(start), -1);
            }
        }
        
    }

}
