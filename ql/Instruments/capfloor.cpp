
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

namespace QuantLib {

    namespace Instruments {

        using CashFlows::FloatingRateCoupon;

        void VanillaCapFloor::setupEngine() const {
            CapFloorArguments* arguments =
                dynamic_cast<CapFloorArguments*>(
                    engine_->arguments());
            QL_REQUIRE(arguments != 0,
                       "pricing engine does not supply needed arguments");

            arguments->type = type_;
            arguments->capRates.clear();
            arguments->floorRates.clear();
            arguments->startTimes.clear();
            arguments->endTimes.clear();
            arguments->accrualTimes.clear();
            arguments->forwards.clear();
            arguments->nominals.clear();

            Date settlement = termStructure_->settlementDate();
            DayCounter counter = termStructure_->dayCounter();

            std::vector<Handle<CashFlow> >::const_iterator begin =
                floatingLeg_.begin();
            std::vector<Rate>::const_iterator caps = capRates_.begin();
            std::vector<Rate>::const_iterator floors = floorRates_.begin();

            for (; begin != floatingLeg_.end(); ++begin) {
                Handle<FloatingRateCoupon> coupon = *begin;
                QL_ENSURE(!coupon.isNull(), 
                          "VanillaCapFloor::setupEngine Not a floating rate coupon");
                Date beginDate = coupon->accrualStartDate();
                Time time = counter.yearFraction(settlement, beginDate);
                arguments->startTimes.push_back(time);
                time = counter.yearFraction(settlement, coupon->date());
                arguments->endTimes.push_back(time);
                // this is passed explicitly for precision
                arguments->accrualTimes.push_back(coupon->accrualPeriod());
                // this is passed explicitly for precision
                if (arguments->endTimes.back() >= 0.0)  // but only if really needed 
                    arguments->forwards.push_back(coupon->fixing());
                else
                    arguments->forwards.push_back(Null<Rate>());
                arguments->nominals.push_back(coupon->nominal());
                if (caps == capRates_.end()) {
                    arguments->capRates.push_back(capRates_.back());
                } else {
                    arguments->capRates.push_back(*caps);
                    caps++;
                }
                if (floors == floorRates_.end()) {
                    arguments->floorRates.push_back(floorRates_.back());
                } else {
                    arguments->floorRates.push_back(*floors);
                    floors++;
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

        void CapFloorArguments::validate() const {
            QL_REQUIRE(
                endTimes.size() == startTimes.size(),
                "Invalid pricing arguments: size of startTimes (" +
                IntegerFormatter::toString(startTimes.size()) +
                ") different from that of endTimes (" +
                IntegerFormatter::toString(endTimes.size()) +
                ")");
            QL_REQUIRE(
                accrualTimes.size() == startTimes.size(),
                "Invalid pricing arguments: size of startTimes (" +
                IntegerFormatter::toString(startTimes.size()) +
                ") different from that of accrualTimes (" +
                IntegerFormatter::toString(accrualTimes.size()) +
                ")");
            QL_REQUIRE(
                capRates.size() == startTimes.size(),
                "Invalid pricing arguments: size of startTimes (" +
                IntegerFormatter::toString(startTimes.size()) +
                ") different from that of capRates (" +
                IntegerFormatter::toString(capRates.size()) +
                ")");
            QL_REQUIRE(
                floorRates.size() == startTimes.size(),
                "Invalid pricing arguments: size of startTimes (" +
                IntegerFormatter::toString(startTimes.size()) +
                ") different from that of floorRates (" +
                IntegerFormatter::toString(floorRates.size()) +
                ")");
            QL_REQUIRE(
                nominals.size() == startTimes.size(),
                "Invalid pricing arguments: size of startTimes (" +
                IntegerFormatter::toString(startTimes.size()) +
                ") different from that of nominals (" +
                IntegerFormatter::toString(nominals.size()) +
                ")");
        }

    }

}
