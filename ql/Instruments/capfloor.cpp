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
#include "ql/CashFlows/floatingratecoupon.hpp"
#include "ql/Math/normaldistribution.hpp"

#include "ql/InterestRateModelling/onefactormodel.hpp"
#include "ql/Lattices/tree.hpp"

using std::cout;
using std::endl;

namespace QuantLib {

    namespace Instruments {

        using CashFlows::FloatingRateCoupon;
        using InterestRateModelling::OneFactorModel;

        void VanillaCapFloor::setupEngine() const {
            CapFloorParameters* parameters =
                dynamic_cast<CapFloorParameters*>(
                    engine_->parameters());
            QL_REQUIRE(parameters != 0,
                       "pricing engine does not supply needed parameters");

            parameters->type = type_;
            parameters->exerciseRates = exerciseRates_;

            parameters->startTimes.clear();
            parameters->endTimes.clear();
            parameters->nominals.clear();

            std::vector<Handle<CashFlow> > floatingLeg = swap_->floatingLeg();
            std::vector<Handle<CashFlow> >::const_iterator begin, end;
            begin = floatingLeg.begin();
            end   = floatingLeg.end();
            Date today = termStructure_->settlementDate();
            DayCounter counter = termStructure_->dayCounter();
            for (; begin != end; ++begin) {
                Handle<FloatingRateCoupon> coupon = *begin;
                QL_ENSURE(!coupon.isNull(), "not a floating rate coupon");
                Date beginDate = coupon->accrualStartDate();
                Time time = counter.yearFraction(today, beginDate);
                parameters->startTimes.push_back(time);
                time = counter.yearFraction(today, coupon->date());
                parameters->endTimes.push_back(time);
                parameters->nominals.push_back(coupon->nominal());
            }
        }

        void VanillaCapFloor::performCalculations() const {
            if (swap_->maturity() <= termStructure_->settlementDate()) {
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
                "Invalid pricing parameters");
        }

        const Results* CapFloorPricingEngine::results() const {
            return &results_;
        }

    }

}
