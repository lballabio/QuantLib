
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/PricingEngines/CapFloor/blackcapfloorengine.hpp>

namespace QuantLib {

    void BlackCapFloorEngine::calculate() const {
        Real value = 0.0;
        CapFloor::Type type = arguments_.type;

        for (Size i=0; i<arguments_.startTimes.size(); i++) {
            Time fixing = arguments_.fixingTimes[i],
                end = arguments_.endTimes[i],
                accrualTime = arguments_.accrualTimes[i];
            if (end > 0.0) {    // discard expired caplets
                Real nominal = arguments_.nominals[i];
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
                    Real temp = q * accrualTime * nominal *
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

    Real BlackCapFloorEngine::capletValue(Time start, Rate forward,
                                          Rate strike, Volatility vol) const {
        if (start <= 0.0) {
            // the rate was fixed
            return std::max<Rate>(forward-strike,0.0);
        } else {
            // forecast
            return BlackModel::formula(forward, strike,
                                       vol*std::sqrt(start), 1);
        }
    }

    Real BlackCapFloorEngine::floorletValue(Time start, Rate forward,
                                            Rate strike, Volatility vol)
                                                                       const {
        if (start <= 0.0) {
            // the rate was fixed
            return std::max<Rate>(strike-forward,0.0);
        } else {
            // forecast
            return BlackModel::formula(forward, strike,
                                       vol*std::sqrt(start), -1);
        }
    }

}
