
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

/*! \file blackcapfloor.cpp
    \brief European capfloor calculated using Black formula
*/

#include "ql/Pricers/blackcapfloor.hpp"

namespace QuantLib {

    namespace Pricers {

        using Instruments::CapFloor;

        void BlackCapFloor::calculate() const {
            double value = 0.0;
            CapFloor::Type type = arguments_.type;

            for (Size i=0; i<arguments_.startTimes.size(); i++) {
                Time fixing = arguments_.fixingTimes[i],
                     end = arguments_.endTimes[i],
                     accrualTime = arguments_.accrualTimes[i];
                if (end > 0.0) {    // discard expired caplets
                    double nominal = arguments_.nominals[i];
                    DiscountFactor q = model_->termStructure()->discount(end);
                    Rate forward = arguments_.forwards[i];
                    // try and factorize the code below
                    if ((type == CapFloor::Cap) ||
                        (type == CapFloor::Collar)) {
                            value += q * accrualTime * nominal *
                                     capletValue(fixing,forward,
                                                 arguments_.capRates[i],
                                                 model_->volatility());
                    }
                    if ((type == CapFloor::Floor) ||
                        (type == CapFloor::Collar)) {
                            double temp = q * accrualTime * nominal *
                                          floorletValue(fixing,forward,
                                                 arguments_.floorRates[i],
                                                 model_->volatility());
                            if (type == CapFloor::Floor)
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
                #if defined(QL_PATCH_MICROSOFT)
                // numerical inaccuracies can yield a negative answer
                return QL_MAX(BlackModel::formula(forward, strike, 
                                                  vol*QL_SQRT(start), 1),
                              0.0);
                #else
                return BlackModel::formula(forward, strike, 
                                           vol*QL_SQRT(start), 1);
                #endif
            }
        }
        
        double BlackCapFloor::floorletValue(Time start, Rate forward,
                                            Rate strike, double vol) const {
            if (start <= 0.0) {
                // the rate was fixed
                return QL_MAX(strike-forward,0.0);
            } else {
                // forecast
                #if defined(QL_PATCH_MICROSOFT)
                // numerical inaccuracies can yield a negative answer
                return QL_MAX(BlackModel::formula(forward, strike, 
                                                  vol*QL_SQRT(start), -1),
                              0.0);
                #else
                return BlackModel::formula(forward, strike, 
                                           vol*QL_SQRT(start), -1);
                #endif
            }
        }
        
    }

}
