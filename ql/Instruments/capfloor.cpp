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
/*! \file capfloor.cpp
    \brief European cap and floor class

    \fullpath
    ql/Instruments/%capfloor.cpp
*/

// $Id$

#include "ql/Instruments/capfloor.hpp"
#include "ql/dataformatters.hpp"
#include "ql/CashFlows/floatingratecoupon.hpp"

namespace QuantLib {

    namespace Instruments {

        using CashFlows::FloatingRateCouponVector;
        using CashFlows::FloatingRateCoupon;

        void VanillaCapFloor::setupEngine() const {
            CapFloorParameters* parameters =
                dynamic_cast<CapFloorParameters*>(
                    engine_->parameters());
            QL_REQUIRE(parameters != 0,
                       "pricing engine does not supply needed parameters");

            parameters->type = type_;
            parameters->exerciseRates.clear();
            parameters->startTimes.clear();
            parameters->endTimes.clear();
            parameters->nominals.clear();

            Date settlement = termStructure_->settlementDate();
            DayCounter counter = termStructure_->dayCounter();

            std::vector<Handle<CashFlow> >::const_iterator begin =
                floatingLeg_.begin();
            std::vector<Rate>::const_iterator rates = exerciseRates_.begin();

            for (; begin != floatingLeg_.end(); ++begin) {
                Handle<FloatingRateCoupon> coupon = *begin;
                QL_ENSURE(!coupon.isNull(), 
                          "VanillaCapFloor::setupEngine floating rate coupon is Null");
                Date beginDate = coupon->accrualStartDate();
                Time time = counter.yearFraction(settlement, beginDate);
                parameters->startTimes.push_back(time);
                time = counter.yearFraction(settlement, coupon->date());
                parameters->endTimes.push_back(time);
                parameters->nominals.push_back(coupon->nominal());
                if (rates == exerciseRates_.end()) {
                    parameters->exerciseRates.push_back(exerciseRates_.back());
                } else {
                    parameters->exerciseRates.push_back(*rates);
                    rates++;
                }

            }
        }

        void VanillaCapFloor::performCalculations() const {
            if (floatingLeg_.back()->date()<termStructure_->settlementDate()) {
                isExpired_ = true;
                NPV_ = 0.0;
            } else {
                isExpired_ = false;
                Option::performCalculations();
            }
            QL_ENSURE(isExpired_ || NPV_ != Null<double>(),
                      "null value returned from cap/floor pricer");
        }
    
    }

    namespace Pricers {

        Arguments* CapFloorPricingEngine::parameters() {
            return &parameters_;
        }

        void CapFloorPricingEngine::validateParameters() const {
            QL_REQUIRE(
                parameters_.endTimes.size() == parameters_.startTimes.size(),
                "Invalid pricing parameters: size of startTimes(" +
                IntegerFormatter::toString(parameters_.startTimes.size()) +
                ") different from that of endTimes(" +
                IntegerFormatter::toString(parameters_.endTimes.size()) +
                ")");
            QL_REQUIRE(
                parameters_.exerciseRates.size()==parameters_.startTimes.size(),
                "Invalid pricing parameters");
        }

        const Results* CapFloorPricingEngine::results() const {
            return &results_;
        }

    }

}
