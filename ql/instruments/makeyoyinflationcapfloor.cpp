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

#include <ql/instruments/makeyoyinflationcapfloor.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/time/daycounters/thirty360.hpp>

namespace QuantLib {

    MakeYoYInflationCapFloor::MakeYoYInflationCapFloor(YoYInflationCapFloor::Type capFloorType,
                                const Size& length, const Calendar& cal,
                                const boost::shared_ptr<YoYInflationIndex>& index,
                                const Period& observationLag, Rate strike,
                                const Period& forwardStart)
    : capFloorType_(capFloorType), length_(length),
      calendar_(cal), index_(index), observationLag_(observationLag),
      strike_(strike), firstCapletExcluded_(false),
      asOptionlet_(false), effectiveDate_(Date()), forwardStart_(forwardStart),
      dayCounter_(Thirty360()), roll_(ModifiedFollowing), fixingDays_(0),
      nominal_(1000000.0)
     {}

    MakeYoYInflationCapFloor::operator YoYInflationCapFloor() const {
        boost::shared_ptr<YoYInflationCapFloor> capfloor = *this;
        return *capfloor;
    }

    MakeYoYInflationCapFloor::operator boost::shared_ptr<YoYInflationCapFloor>() const {

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
            Leg::iterator end = leg.end();  // Sun Studio needs an lvalue
            leg.erase(leg.begin(), --end);
        }

        std::vector<Rate> strikeVector(1, strike_);
        if (strike_ == Null<Rate>()) {
            // ATM on the forecasting curve
            QL_REQUIRE(!index_->yoyInflationTermStructure().empty(),
                       "no forecasting yoy term structure set for " <<
                       index_->name());
            Handle<YieldTermStructure> fc
            = index_->yoyInflationTermStructure()->nominalTermStructure();
            strikeVector[0] = CashFlows::atmRate(leg,**fc,
                                                 false, fc->referenceDate());
        }

        boost::shared_ptr<YoYInflationCapFloor> capFloor(new
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
        const boost::shared_ptr<PricingEngine>& engine) {
        engine_ = engine;
        return *this;
    }

}

