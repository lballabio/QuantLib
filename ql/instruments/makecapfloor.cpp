/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2007 StatPro Italia srl

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

#include <ql/instruments/makecapfloor.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>

using boost::shared_ptr;
using boost::dynamic_pointer_cast;

namespace QuantLib {

    MakeCapFloor::MakeCapFloor(CapFloor::Type capFloorType,
                               const Period& tenor,
                               const shared_ptr<IborIndex>& iborIndex,
                               Rate strike,
                               const Period& forwardStart)
    : capFloorType_(capFloorType), strike_(strike),
      firstCapletExcluded_(forwardStart==0*Days), asOptionlet_(false),
      makeVanillaSwap_(MakeVanillaSwap(tenor, iborIndex, 0.0, forwardStart)) {}

    MakeCapFloor::operator CapFloor() const {
        shared_ptr<CapFloor> capfloor = *this;
        return *capfloor;
    }

    MakeCapFloor::operator shared_ptr<CapFloor>() const {

        VanillaSwap swap = makeVanillaSwap_;

        Leg leg = swap.floatingLeg();
        if (firstCapletExcluded_)
            leg.erase(leg.begin());

        // only leaves the last coupon
        if (asOptionlet_ && leg.size() > 1) {
            Leg::iterator end = leg.end();  // Sun Studio needs an lvalue
            leg.erase(leg.begin(), --end);
        }

        std::vector<Rate> strikeVector(1, strike_);
        if (strike_ == Null<Rate>()) {

            // temporary patch...
            // should be fixed for every CapFloor::Engine
            shared_ptr<BlackCapFloorEngine> temp = 
                dynamic_pointer_cast<BlackCapFloorEngine>(engine_);
            QL_REQUIRE(temp,
                       "cannot calculate ATM without a BlackCapFloorEngine");
            Handle<YieldTermStructure> discountCurve = temp->termStructure();
            strikeVector[0] = CashFlows::atmRate(leg,
                                                 **discountCurve,
                                                 false,
                                                 discountCurve->referenceDate());
        }

        shared_ptr<CapFloor> capFloor(new
            CapFloor(capFloorType_, leg, strikeVector));
        capFloor->setPricingEngine(engine_);
        return capFloor;
    }

    MakeCapFloor& MakeCapFloor::withNominal(Real n) {
        makeVanillaSwap_.withNominal(n);
        return *this;
    }

    MakeCapFloor& MakeCapFloor::withEffectiveDate(const Date& effectiveDate,
                                                  bool firstCapletExcluded) {
        makeVanillaSwap_.withEffectiveDate(effectiveDate);
        firstCapletExcluded_ = firstCapletExcluded;
        return *this;
    }

    MakeCapFloor& MakeCapFloor::withTenor(const Period& t) {
        makeVanillaSwap_.withFixedLegTenor(t);
        makeVanillaSwap_.withFloatingLegTenor(t);
        return *this;
    }


    MakeCapFloor& MakeCapFloor::withCalendar(const Calendar& cal) {
        makeVanillaSwap_.withFixedLegCalendar(cal);
        makeVanillaSwap_.withFloatingLegCalendar(cal);
        return *this;
    }


    MakeCapFloor& MakeCapFloor::withConvention(BusinessDayConvention bdc) {
        makeVanillaSwap_.withFixedLegConvention(bdc);
        makeVanillaSwap_.withFloatingLegConvention(bdc);
        return *this;
    }


    MakeCapFloor&
    MakeCapFloor::withTerminationDateConvention(BusinessDayConvention bdc) {
        makeVanillaSwap_.withFixedLegTerminationDateConvention(bdc);
        makeVanillaSwap_.withFloatingLegTerminationDateConvention(bdc);
        return *this;
    }


    MakeCapFloor& MakeCapFloor::withRule(DateGeneration::Rule r) {
        makeVanillaSwap_.withFixedLegRule(r);
        makeVanillaSwap_.withFloatingLegRule(r);
        return *this;
    }

    MakeCapFloor& MakeCapFloor::withEndOfMonth(bool flag) {
        makeVanillaSwap_.withFixedLegEndOfMonth(flag);
        makeVanillaSwap_.withFloatingLegEndOfMonth(flag);
        return *this;
    }


    MakeCapFloor& MakeCapFloor::withFirstDate(const Date& d) {
        makeVanillaSwap_.withFixedLegFirstDate(d);
        makeVanillaSwap_.withFloatingLegFirstDate(d);
        return *this;
    }

    MakeCapFloor& MakeCapFloor::withNextToLastDate(const Date& d) {
        makeVanillaSwap_.withFixedLegNextToLastDate(d);
        makeVanillaSwap_.withFloatingLegNextToLastDate(d);
        return *this;
    }

    MakeCapFloor& MakeCapFloor::withDayCount(const DayCounter& dc) {
        makeVanillaSwap_.withFixedLegDayCount(dc);
        makeVanillaSwap_.withFloatingLegDayCount(dc);
        return *this;
    }

    MakeCapFloor& MakeCapFloor::asOptionlet(bool b) {
        asOptionlet_ = b;
        return *this;
    }

    MakeCapFloor& MakeCapFloor::withPricingEngine(
                             const shared_ptr<PricingEngine>& engine) {
        engine_ = engine;
        return *this;
    }

}

