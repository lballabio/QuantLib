
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file flatforward.hpp
    \brief flat forward rate term structure
*/

#ifndef quantlib_flat_forward_curve_h
#define quantlib_flat_forward_curve_h

#include <ql/termstructure.hpp>
#include <ql/DayCounters/actual365.hpp>

namespace QuantLib {

    class FlatForward : public TermStructure, public Observer {
      public:
        // constructors
        FlatForward(const Date& todaysDate,
                    const Date& referenceDate,
                    Rate forward,
                    const DayCounter& dayCounter = Actual365());
        FlatForward(const Date& todaysDate,
                    const Date& referenceDate,
                    const RelinkableHandle<Quote>& forward,
                    const DayCounter& dayCounter = Actual365());
        // inspectors
        DayCounter dayCounter() const;
        Date todaysDate() const { return todaysDate_; }
        Date referenceDate() const;
        Date maxDate() const;
        // Observer interface
        void update();
      protected:
        Rate zeroYieldImpl(Time, bool extrapolate = false) const;
        DiscountFactor discountImpl(Time,
                                    bool extrapolate = false) const;
        Rate forwardImpl(Time, bool extrapolate = false) const;
        Rate compoundForwardImpl(Time t, int compFreq,
                                 bool extrapolate = false) const;
      private:
        Date todaysDate_, referenceDate_;
        DayCounter dayCounter_;
        RelinkableHandle<Quote> forward_;
    };

    // inline definitions

    inline FlatForward::FlatForward(const Date& todaysDate,
                                    const Date& referenceDate,
                                    Rate forward, 
                                    const DayCounter& dayCounter)
    : todaysDate_(todaysDate), referenceDate_(referenceDate),
      dayCounter_(dayCounter) {
        forward_.linkTo(boost::shared_ptr<Quote>(new SimpleQuote(forward)));
    }

    inline FlatForward::FlatForward(const Date& todaysDate,
                                    const Date& referenceDate,
                                    const RelinkableHandle<Quote>& forward,
                                    const DayCounter& dayCounter)
    : todaysDate_(todaysDate), referenceDate_(referenceDate),
      dayCounter_(dayCounter), forward_(forward) {
        registerWith(forward_);
    }

    inline DayCounter FlatForward::dayCounter() const {
        return dayCounter_;
    }

    inline Date FlatForward::referenceDate() const {
        return referenceDate_;
    }

    inline Date FlatForward::maxDate() const {
        return Date::maxDate();
    }

    inline void FlatForward::update() {
        notifyObservers();
    }

    inline Rate FlatForward::zeroYieldImpl(Time t, bool) const {
        // no forward limit on time
        QL_REQUIRE(t >= 0.0,
                   "FlatForward::zeroYieldImpl : "
                   "zero yield undefined for time (" +
                   DoubleFormatter::toString(t) + ")");
        return forward_->value();
    }

    inline DiscountFactor FlatForward::discountImpl(Time t, bool) const {
        // no forward limit on time
        QL_REQUIRE(t >= 0.0,
                   "FlatForward::discountImpl : "
                   "discount undefined for time (" +
                   DoubleFormatter::toString(t) + ")");
        return DiscountFactor(QL_EXP(-forward_->value()*t));
    }

    inline Rate FlatForward::forwardImpl(Time t, bool) const {
        // no forward limit on time
        QL_REQUIRE(t >= 0.0,
                   "FlatForward::forwardImpl : "
                   "forward undefined for time (" +
                   DoubleFormatter::toString(t) + ")");
        return forward_->value();
    }

    inline Rate FlatForward::compoundForwardImpl(Time t, int compFreq, 
                                                 bool extrapolate) const {
        double zy = zeroYieldImpl(t, extrapolate);
        if (compFreq == 0)
            return zy;
        if (t <= 1.0/compFreq)
            return (QL_EXP(zy*t)-1.0)/t;
        return (QL_EXP(zy*(1.0/compFreq))-1.0)*compFreq;
    }

}


#endif
