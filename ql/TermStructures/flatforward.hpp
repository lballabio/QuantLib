
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
#ifndef QL_DISABLE_DEPRECATED
#   include <ql/DayCounters/actual365fixed.hpp>
#endif

namespace QuantLib {

    //! Flat interest-rate curve
    /*! \ingroup yieldtermstructures */
    class FlatForward : public YieldTermStructure {
      public:
        // constructors
        #ifndef QL_DISABLE_DEPRECATED
        /*! \deprecated use one of the non-deprecated constructors. */
        FlatForward(const Date& todaysDate,
                    const Date& referenceDate,
                    Rate forward,
                    const DayCounter& dayCounter);
        /*! \deprecated use one of the non-deprecated constructors. */
        FlatForward(const Date& todaysDate,
                    const Date& referenceDate,
                    const Handle<Quote>& forward,
                    const DayCounter& dayCounter);
        #endif
        FlatForward(const Date& referenceDate,
                    const Handle<Quote>& forward,
                    const DayCounter& dayCounter);
        FlatForward(const Date& referenceDate,
                    Rate forward,
                    const DayCounter& dayCounter);

        FlatForward(Integer settlementDays, const Calendar& calendar,
                    const Handle<Quote>& forward,
                    const DayCounter& dayCounter);
        FlatForward(Integer settlementDays, const Calendar& calendar,
                    Rate forward,
                    const DayCounter& dayCounter);

        // inspectors
        #ifndef QL_DISABLE_DEPRECATED
        DayCounter dayCounter() const { return dayCounter_; }
        #endif
        Date maxDate() const;
      protected:
        Rate zeroYieldImpl(Time) const;
        DiscountFactor discountImpl(Time) const;
        Rate forwardImpl(Time) const;
#ifndef QL_DISABLE_DEPRECATED
        Rate compoundForwardImpl(Time t, Integer compFreq) const;
#endif
      private:
        DayCounter dayCounter_;
        Handle<Quote> forward_;
    };

    // inline definitions

    #ifndef QL_DISABLE_DEPRECATED
    inline FlatForward::FlatForward(const Date& todaysDate,
                                    const Date& referenceDate,
                                    Rate forward,
                                    const DayCounter& dayCounter)
    : YieldTermStructure(todaysDate, referenceDate),
      dayCounter_(dayCounter) {
        forward_.linkTo(boost::shared_ptr<Quote>(new SimpleQuote(forward)));
    }

    inline FlatForward::FlatForward(const Date& todaysDate,
                                    const Date& referenceDate,
                                    const Handle<Quote>& forward,
                                    const DayCounter& dayCounter)
    : YieldTermStructure(todaysDate, referenceDate),
      dayCounter_(dayCounter), forward_(forward) {
        registerWith(forward_);
    }
    #endif

    inline FlatForward::FlatForward(const Date& referenceDate,
                                    const Handle<Quote>& forward,
                                    const DayCounter& dayCounter)
    : YieldTermStructure(referenceDate), dayCounter_(dayCounter),
      forward_(forward) {
        registerWith(forward_);
    }

    inline FlatForward::FlatForward(const Date& referenceDate,
                                    Rate forward,
                                    const DayCounter& dayCounter)
    : YieldTermStructure(referenceDate), dayCounter_(dayCounter) {
        forward_.linkTo(boost::shared_ptr<Quote>(new SimpleQuote(forward)));
    }

    inline FlatForward::FlatForward(Integer settlementDays,
                                    const Calendar& calendar,
                                    const Handle<Quote>& forward,
                                    const DayCounter& dayCounter)
    : YieldTermStructure(settlementDays,calendar), dayCounter_(dayCounter),
      forward_(forward) {
        registerWith(forward_);
    }

    inline FlatForward::FlatForward(Integer settlementDays,
                                    const Calendar& calendar,
                                    Rate forward,
                                    const DayCounter& dayCounter)
    : YieldTermStructure(settlementDays,calendar), dayCounter_(dayCounter) {
        forward_.linkTo(boost::shared_ptr<Quote>(new SimpleQuote(forward)));
    }

    inline Date FlatForward::maxDate() const {
        return Date::maxDate();
    }

    inline Rate FlatForward::zeroYieldImpl(Time t) const {
        return forward_->value();
    }

    inline DiscountFactor FlatForward::discountImpl(Time t) const {
        return DiscountFactor(QL_EXP(-forward_->value()*t));
    }

    inline Rate FlatForward::forwardImpl(Time t) const {
        return forward_->value();
    }

    #ifndef QL_DISABLE_DEPRECATED
    inline Rate FlatForward::compoundForwardImpl(Time t,
                                                 Integer compFreq) const {
        Rate zy = zeroYieldImpl(t);
        if (compFreq == 0)
            return zy;
        if (t <= 1.0/compFreq)
            return (QL_EXP(zy*t)-1.0)/t;
        return (QL_EXP(zy*(1.0/compFreq))-1.0)*compFreq;
    }
    #endif
}


#endif

