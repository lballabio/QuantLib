/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 StatPro Italia srl

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

#include "utilities.hpp"
#include <ql/instruments/payoffs.hpp>
#include <ql/indexes/indexmanager.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/time/calendars/nullcalendar.hpp>

#define CHECK_DOWNCAST(Derived,Description) { \
    std::shared_ptr<Derived> hd = std::dynamic_pointer_cast<Derived>(h); \
    if (hd) \
        return Description; \
}

namespace QuantLib {

    std::string payoffTypeToString(const std::shared_ptr<Payoff>& h) {

        CHECK_DOWNCAST(PlainVanillaPayoff, "plain-vanilla");
        CHECK_DOWNCAST(CashOrNothingPayoff, "cash-or-nothing");
        CHECK_DOWNCAST(AssetOrNothingPayoff, "asset-or-nothing");
        CHECK_DOWNCAST(SuperSharePayoff, "super-share");
        CHECK_DOWNCAST(SuperFundPayoff, "super-fund");
        CHECK_DOWNCAST(PercentageStrikePayoff, "percentage-strike");
        CHECK_DOWNCAST(GapPayoff, "gap");
        CHECK_DOWNCAST(FloatingTypePayoff, "floating-type");

        QL_FAIL("unknown payoff type");
    }


    std::string exerciseTypeToString(const std::shared_ptr<Exercise>& h) {

        CHECK_DOWNCAST(EuropeanExercise, "European");
        CHECK_DOWNCAST(AmericanExercise, "American");
        CHECK_DOWNCAST(BermudanExercise, "Bermudan");

        QL_FAIL("unknown exercise type");
    }


    std::shared_ptr<YieldTermStructure>
    flatRate(const Date& today,
             const std::shared_ptr<Quote>& forward,
             const DayCounter& dc) {
        return std::shared_ptr<YieldTermStructure>(
                          new FlatForward(today, Handle<Quote>(forward), dc));
    }

    std::shared_ptr<YieldTermStructure>
    flatRate(const Date& today, Rate forward, const DayCounter& dc) {
        return flatRate(
               today, std::shared_ptr<Quote>(new SimpleQuote(forward)), dc);
    }

    std::shared_ptr<YieldTermStructure>
    flatRate(const std::shared_ptr<Quote>& forward,
             const DayCounter& dc) {
        return std::shared_ptr<YieldTermStructure>(
              new FlatForward(0, NullCalendar(), Handle<Quote>(forward), dc));
    }

    std::shared_ptr<YieldTermStructure>
    flatRate(Rate forward, const DayCounter& dc) {
        return flatRate(std::shared_ptr<Quote>(new SimpleQuote(forward)),
                        dc);
    }


    std::shared_ptr<BlackVolTermStructure>
    flatVol(const Date& today,
            const std::shared_ptr<Quote>& vol,
            const DayCounter& dc) {
        return std::shared_ptr<BlackVolTermStructure>(new
            BlackConstantVol(today, NullCalendar(), Handle<Quote>(vol), dc));
    }

    std::shared_ptr<BlackVolTermStructure>
    flatVol(const Date& today, Volatility vol,
            const DayCounter& dc) {
        return flatVol(today,
                       std::shared_ptr<Quote>(new SimpleQuote(vol)),
                       dc);
    }

    std::shared_ptr<BlackVolTermStructure>
    flatVol(const std::shared_ptr<Quote>& vol,
            const DayCounter& dc) {
        return std::shared_ptr<BlackVolTermStructure>(new
            BlackConstantVol(0, NullCalendar(), Handle<Quote>(vol), dc));
    }

    std::shared_ptr<BlackVolTermStructure>
    flatVol(Volatility vol,
            const DayCounter& dc) {
        return flatVol(std::shared_ptr<Quote>(new SimpleQuote(vol)), dc);
    }


    Real relativeError(Real x1, Real x2, Real reference) {
        if (reference != 0.0)
            return std::fabs(x1-x2)/reference;
        else
            // fall back to absolute error
            return std::fabs(x1-x2);
    }


    IndexHistoryCleaner::~IndexHistoryCleaner() {
        IndexManager::instance().clearHistories();
    }

}
