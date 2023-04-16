/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2007 StatPro Italia srl
 Copyright (C) 2009 Chris Kenyon

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
 */

#include <ql/cashflows/cashflows.hpp>
#include <ql/instruments/makeyoyinflationcapfloor.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <utility>

namespace QuantLib {

    MakeYoYInflationCapFloor::MakeYoYInflationCapFloor(YoYInflationCapFloor::Type capFloorType,
                                                       std::shared_ptr<YoYInflationIndex> index,
                                                       const Size& length,
                                                       Calendar cal,
                                                       const Period& observationLag)
    : capFloorType_(capFloorType), length_(length), calendar_(std::move(cal)),
      index_(std::move(index)), observationLag_(observationLag), strike_(Null<Rate>()),

      dayCounter_(Thirty360(Thirty360::BondBasis)) {}

    MakeYoYInflationCapFloor::operator YoYInflationCapFloor() const {
        std::shared_ptr<YoYInflationCapFloor> capfloor = *this;
        return *capfloor;
    }

    MakeYoYInflationCapFloor::operator std::shared_ptr<YoYInflationCapFloor>() const {

        Date startDate;
        if (effectiveDate_ != Date()) {
            startDate = effectiveDate_;
        } else {
            Date referenceDate = Settings::instance().evaluationDate();
            Date spotDate = calendar_.advance(referenceDate,
                                              fixingDays_*Days);
            startDate = spotDate+forwardStart_;
        }

        Date endDate = calendar_.advance(startDate,length_*Years,Unadjusted);
        Schedule schedule(startDate, endDate, Period(Annual), calendar_,
                          Unadjusted, Unadjusted, // ref periods & acc periods
                          DateGeneration::Forward, false);
        Leg leg = yoyInflationLeg(schedule, calendar_, index_,
                                  observationLag_)
        .withPaymentAdjustment(roll_)
        .withPaymentDayCounter(dayCounter_)
        .withNotionals(nominal_)
        ;

        if (firstCapletExcluded_)
            leg.erase(leg.begin());

        // only leaves the last coupon
        if (asOptionlet_ && leg.size() > 1) {
            auto end = leg.end(); // Sun Studio needs an lvalue
            leg.erase(leg.begin(), --end);
        }

        std::vector<Rate> strikeVector(1, strike_);
        if (strike_ == Null<Rate>()) {
            // ATM on the forecasting curve
            strikeVector[0] = CashFlows::atmRate(leg, **nominalTermStructure_,
                                                 false, nominalTermStructure_->referenceDate());
        }

        std::shared_ptr<YoYInflationCapFloor> capFloor(new
                    YoYInflationCapFloor(capFloorType_, leg, strikeVector));
        capFloor->setPricingEngine(engine_);
        return capFloor;
    }

    MakeYoYInflationCapFloor& MakeYoYInflationCapFloor::withNominal(Real n) {
        nominal_ = n;
        return *this;
    }

    MakeYoYInflationCapFloor& MakeYoYInflationCapFloor::withEffectiveDate(
                                            const Date& effectiveDate) {
        effectiveDate_ = effectiveDate;
        return *this;
    }

    MakeYoYInflationCapFloor&
    MakeYoYInflationCapFloor::withPaymentAdjustment(BusinessDayConvention bdc) {
        roll_ = bdc;
        return *this;
    }

    MakeYoYInflationCapFloor&
    MakeYoYInflationCapFloor::withPaymentDayCounter(const DayCounter& dc) {
        dayCounter_ = dc;
        return *this;
    }

    MakeYoYInflationCapFloor&
    MakeYoYInflationCapFloor::withFixingDays(Natural n) {
        fixingDays_ = n;
        return *this;
    }

    MakeYoYInflationCapFloor& MakeYoYInflationCapFloor::asOptionlet(bool b) {
        asOptionlet_ = b;
        return *this;
    }

    MakeYoYInflationCapFloor& MakeYoYInflationCapFloor::withPricingEngine(
        const std::shared_ptr<PricingEngine>& engine) {
        engine_ = engine;
        return *this;
    }

    MakeYoYInflationCapFloor&
    MakeYoYInflationCapFloor::withStrike(Rate strike) {
        QL_REQUIRE(nominalTermStructure_.empty(), "ATM strike already given");
        strike_ = strike;
        return *this;
    }

    MakeYoYInflationCapFloor&
    MakeYoYInflationCapFloor::withAtmStrike(
                      const Handle<YieldTermStructure>& nominalTermStructure) {
        QL_REQUIRE(strike_ == Null<Rate>(), "explicit strike already given");
        nominalTermStructure_ = nominalTermStructure;
        return *this;
    }

    MakeYoYInflationCapFloor&
    MakeYoYInflationCapFloor::withForwardStart(Period forwardStart) {
        forwardStart_ = forwardStart;
        return *this;
    }

}

