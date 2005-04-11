/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/PricingEngines/CapFloor/analyticcapfloorengine.hpp>

namespace QuantLib {

    void AnalyticCapFloorEngine::calculate() const {
        QL_REQUIRE(model_, "null model");

        Real value = 0.0;
        CapFloor::Type type = arguments_.type;
        Size nPeriods = arguments_.endTimes.size();

        for (Size i=0; i<nPeriods; i++) {

            Time fixingTime = arguments_.fixingTimes[i];
            Time bond = arguments_.endTimes[i];

            #if QL_TODAYS_PAYMENTS
            if (bond >= 0.0) {
            #else
            if (bond > 0.0) {
            #endif
                Time tenor = arguments_.accrualTimes[i];
                Rate fixing = arguments_.forwards[i];
                if (fixingTime <= 0.0) {
                    if (type == CapFloor::Cap || type == CapFloor::Collar) {
                        DiscountFactor discount = model_->discount(bond);
                        Rate strike = arguments_.capRates[i];
                        value += discount * arguments_.nominals[i] * tenor
                               * std::max(0.0, fixing - strike);
                    }
                    if (type == CapFloor::Floor || type == CapFloor::Collar) {
                        DiscountFactor discount = model_->discount(bond);
                        Rate strike = arguments_.floorRates[i];
                        Real mult = (type == CapFloor::Floor)?1.0:-1.0;
                        value += discount * arguments_.nominals[i] * tenor
                              * mult * std::max(0.0, strike - fixing);
                    }
                } else {
                    Time maturity = arguments_.startTimes[i];
                    if (type == CapFloor::Cap || type == CapFloor::Collar) {
                        Real temp = 1.0+arguments_.capRates[i]*tenor;
                        value += arguments_.nominals[i]*temp*
                            model_->discountBondOption(Option::Put, 1.0/temp,
                                                       maturity, bond);
                    }
                    if (type == CapFloor::Floor || type == CapFloor::Collar) {
                        Real temp = 1.0+arguments_.floorRates[i]*tenor;
                        Real mult = (type == CapFloor::Floor) ? 1.0 : -1.0;
                        value += arguments_.nominals[i]*temp*mult*
                            model_->discountBondOption(Option::Call, 1.0/temp,
                                                       maturity, bond);
                    }
                }
            }
        }

        results_.value = value;
    }

}
