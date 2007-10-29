/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2007 StatPro Italia srl

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

/*! \file flatforward.hpp
    \brief flat forward rate term structure
*/

#ifndef quantlib_flat_forward_curve_hpp
#define quantlib_flat_forward_curve_hpp

#include <ql/patterns/lazyobject.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/quotes/simplequote.hpp>

namespace QuantLib {

    //! Flat interest-rate curve
    /*! \ingroup yieldtermstructures */
    class FlatForward : public YieldTermStructure,
                        public LazyObject {
      public:
        // constructors
        FlatForward(const Date& referenceDate,
                    const Handle<Quote>& forward,
                    const DayCounter& dayCounter,
                    Compounding compounding = Continuous,
                    Frequency frequency = Annual);
        FlatForward(const Date& referenceDate,
                    Rate forward,
                    const DayCounter& dayCounter,
                    Compounding compounding = Continuous,
                    Frequency frequency = Annual);
        FlatForward(Natural settlementDays,
                    const Calendar& calendar,
                    const Handle<Quote>& forward,
                    const DayCounter& dayCounter,
                    Compounding compounding = Continuous,
                    Frequency frequency = Annual);
        FlatForward(Natural settlementDays,
                    const Calendar& calendar,
                    Rate forward,
                    const DayCounter& dayCounter,
                    Compounding compounding = Continuous,
                    Frequency frequency = Annual);
        // inspectors
        Compounding compounding() const { return compounding_; }
        Frequency compoundingFrequency() const { return frequency_; }
        virtual void performCalculations() const;
        Date maxDate() const;
        void update();
      private:
        DiscountFactor discountImpl(Time) const;
        Handle<Quote> forward_;
        Compounding compounding_;
        Frequency frequency_;
        mutable InterestRate rate_;
    };

    // inline definitions
    inline FlatForward::FlatForward(const Date& referenceDate,
                                    const Handle<Quote>& forward,
                                    const DayCounter& dayCounter,
                                    Compounding compounding,
                                    Frequency frequency)
    : YieldTermStructure(referenceDate, Calendar(), dayCounter),
      forward_(forward), compounding_(compounding), frequency_(frequency) {
        registerWith(forward_);
    }

    inline FlatForward::FlatForward(const Date& referenceDate,
                                    Rate forward,
                                    const DayCounter& dayCounter,
                                    Compounding compounding,
                                    Frequency frequency)
    : YieldTermStructure(referenceDate, Calendar(), dayCounter),
      forward_(boost::shared_ptr<Quote>(new SimpleQuote(forward))),
      compounding_(compounding), frequency_(frequency) {}

    inline FlatForward::FlatForward(Natural settlementDays,
                                    const Calendar& calendar,
                                    const Handle<Quote>& forward,
                                    const DayCounter& dayCounter,
                                    Compounding compounding,
                                    Frequency frequency)
    : YieldTermStructure(settlementDays, calendar, dayCounter),
      forward_(forward), compounding_(compounding), frequency_(frequency) {
        registerWith(forward_);
    }

    inline FlatForward::FlatForward(Natural settlementDays,
                                    const Calendar& calendar,
                                    Rate forward,
                                    const DayCounter& dayCounter,
                                    Compounding compounding,
                                    Frequency frequency)
    : YieldTermStructure(settlementDays, calendar, dayCounter),
      forward_(boost::shared_ptr<Quote>(new SimpleQuote(forward))),
      compounding_(compounding), frequency_(frequency) {}

    inline Date FlatForward::maxDate() const {
        return Date::maxDate();
    }

    inline void FlatForward::update() {
        LazyObject::update();
        YieldTermStructure::update();
    }

    inline DiscountFactor FlatForward::discountImpl(Time t) const {
        calculate();
        return rate_.discountFactor(t);
    }
  
    inline void FlatForward::performCalculations() const {
        rate_ = InterestRate(forward_->value(), dayCounter(),
                             compounding_, frequency_);
    }

}

#endif
