
/*
 Copyright (C) 2003, 2004 StatPro Italia srl

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

#include "utilities.hpp"
#include <ql/Instruments/payoffs.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>

#if defined(HAVE_BOOST)
#define CHECK_DOWNCAST(Derived,Description) { \
    Handle<Derived> hd = boost::dynamic_pointer_cast<Derived>(h); \
    if (hd) \
        return Description; \
}
#else
#define CHECK_DOWNCAST(Derived,Description) { \
    try { \
        Handle<Derived> hd = h; \
        return Description; \
    } catch (...) {} \
}
#endif

namespace QuantLib {

    std::string payoffTypeToString(const Handle<Payoff>& h) {

        CHECK_DOWNCAST(PlainVanillaPayoff, "plain-vanilla");
        CHECK_DOWNCAST(CashOrNothingPayoff, "cash-or-nothing");
        CHECK_DOWNCAST(AssetOrNothingPayoff, "asset-or-nothing");
        CHECK_DOWNCAST(SuperSharePayoff, "super-share");
        CHECK_DOWNCAST(GapPayoff, "gap");

        QL_FAIL("payoffTypeToString : unknown payoff type");
    }


    std::string exerciseTypeToString(const Handle<Exercise>& h) {

        CHECK_DOWNCAST(EuropeanExercise, "European");
        CHECK_DOWNCAST(AmericanExercise, "American");
        CHECK_DOWNCAST(BermudanExercise, "Bermudan");

        QL_FAIL("exerciseTypeToString : unknown exercise type");
    }


    Handle<TermStructure> makeFlatCurve(const Handle<Quote>& forward,
                                        DayCounter dc) {
        Date today = Date::todaysDate();
        return Handle<TermStructure>(
                       new FlatForward(today, today, 
                                       RelinkableHandle<Quote>(forward), dc));
    }

    Handle<BlackVolTermStructure> makeFlatVolatility(const Handle<Quote>& vol,
                                                     DayCounter dc) {
        Date today = Date::todaysDate();
        return Handle<BlackVolTermStructure>(
                      new BlackConstantVol(today, 
                                           RelinkableHandle<Quote>(vol), dc));
    }

    double relativeError(double x1, double x2, double reference) {
        if (reference != 0.0)
            return QL_FABS(x1-x2)/reference;
        else
            // fall back to absolute error
            return QL_FABS(x1-x2);
    }

}


