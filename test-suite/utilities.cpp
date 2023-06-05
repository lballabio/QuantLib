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
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/time/calendars/nullcalendar.hpp>

#define CHECK_DOWNCAST(Derived,Description) { \
    ext::shared_ptr<Derived> hd = ext::dynamic_pointer_cast<Derived>(h); \
    if (hd) \
        return Description; \
}

namespace QuantLib {

    std::string payoffTypeToString(const ext::shared_ptr<Payoff>& h) {

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


    std::string exerciseTypeToString(const ext::shared_ptr<Exercise>& h) {

        CHECK_DOWNCAST(EuropeanExercise, "European");
        CHECK_DOWNCAST(AmericanExercise, "American");
        CHECK_DOWNCAST(BermudanExercise, "Bermudan");

        QL_FAIL("unknown exercise type");
    }


    ext::shared_ptr<YieldTermStructure>
    flatRate(const Date& today,
             const ext::shared_ptr<Quote>& forward,
             const DayCounter& dc) {
        return ext::shared_ptr<YieldTermStructure>(
                          new FlatForward(today, Handle<Quote>(forward), dc));
    }

    ext::shared_ptr<YieldTermStructure>
    flatRate(const Date& today, Rate forward, const DayCounter& dc) {
        return flatRate(
               today, ext::shared_ptr<Quote>(new SimpleQuote(forward)), dc);
    }

    ext::shared_ptr<YieldTermStructure>
    flatRate(const ext::shared_ptr<Quote>& forward,
             const DayCounter& dc) {
        return ext::shared_ptr<YieldTermStructure>(
              new FlatForward(0, NullCalendar(), Handle<Quote>(forward), dc));
    }

    ext::shared_ptr<YieldTermStructure>
    flatRate(Rate forward, const DayCounter& dc) {
        return flatRate(ext::shared_ptr<Quote>(new SimpleQuote(forward)),
                        dc);
    }


    ext::shared_ptr<BlackVolTermStructure>
    flatVol(const Date& today,
            const ext::shared_ptr<Quote>& vol,
            const DayCounter& dc) {
        return ext::shared_ptr<BlackVolTermStructure>(new
            BlackConstantVol(today, NullCalendar(), Handle<Quote>(vol), dc));
    }

    ext::shared_ptr<BlackVolTermStructure>
    flatVol(const Date& today, Volatility vol,
            const DayCounter& dc) {
        return flatVol(today,
                       ext::shared_ptr<Quote>(new SimpleQuote(vol)),
                       dc);
    }

    ext::shared_ptr<BlackVolTermStructure>
    flatVol(const ext::shared_ptr<Quote>& vol,
            const DayCounter& dc) {
        return ext::shared_ptr<BlackVolTermStructure>(new
            BlackConstantVol(0, NullCalendar(), Handle<Quote>(vol), dc));
    }

    ext::shared_ptr<BlackVolTermStructure>
    flatVol(Volatility vol,
            const DayCounter& dc) {
        return flatVol(ext::shared_ptr<Quote>(new SimpleQuote(vol)), dc);
    }


    Real relativeError(Real x1, Real x2, Real reference) {
        if (reference != 0.0)
            return std::fabs(x1-x2)/reference;
        else
            // fall back to absolute error
            return std::fabs(x1-x2);
    }
}
