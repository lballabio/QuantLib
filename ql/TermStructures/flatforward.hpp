
/*
 Copyright (C) 2000-2005 StatPro Italia srl

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

#ifndef quantlib_flat_forward_curve_hpp
#define quantlib_flat_forward_curve_hpp

#include <ql/yieldtermstructure.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    //! Flat interest-rate curve
    /*! \ingroup yieldtermstructures */
    class FlatForward : public YieldTermStructure {
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
        FlatForward(Integer settlementDays, const Calendar& calendar,
                    const Handle<Quote>& forward,
                    const DayCounter& dayCounter,
                    Compounding compounding = Continuous,
                    Frequency frequency = Annual);
        FlatForward(Integer settlementDays, const Calendar& calendar,
                    Rate forward,
                    const DayCounter& dayCounter,
                    Compounding compounding = Continuous,
                    Frequency frequency = Annual);
        // inspectors
        DayCounter dayCounter() const { return dayCounter_; }
        Compounding compounding() const { return compounding_; }
        Frequency compoundingFrequency() const { return frequency_; }
        Date maxDate() const;
        void update();
      private:
        DiscountFactor discountImpl(Time) const;
        void updateRate();
        DayCounter dayCounter_;
        Handle<Quote> forward_;
        Compounding compounding_;
        Frequency frequency_;
        InterestRate rate_;
    };

    // inline definitions

    inline FlatForward::FlatForward(const Date& referenceDate,
                                    const Handle<Quote>& forward,
                                    const DayCounter& dayCounter,
                                    Compounding compounding,
                                    Frequency frequency)
    : YieldTermStructure(referenceDate), dayCounter_(dayCounter),
      forward_(forward), compounding_(compounding), frequency_(frequency) {
        registerWith(forward_);
        updateRate();
    }

    inline FlatForward::FlatForward(const Date& referenceDate,
                                    Rate forward,
                                    const DayCounter& dayCounter,
                                    Compounding compounding,
                                    Frequency frequency)
    : YieldTermStructure(referenceDate), dayCounter_(dayCounter),
      compounding_(compounding), frequency_(frequency) {
        forward_.linkTo(boost::shared_ptr<Quote>(new SimpleQuote(forward)));
        updateRate();
    }

    inline FlatForward::FlatForward(Integer settlementDays,
                                    const Calendar& calendar,
                                    const Handle<Quote>& forward,
                                    const DayCounter& dayCounter,
                                    Compounding compounding,
                                    Frequency frequency)
    : YieldTermStructure(settlementDays,calendar), dayCounter_(dayCounter),
      forward_(forward), compounding_(compounding), frequency_(frequency) {
        registerWith(forward_);
        updateRate();
    }

    inline FlatForward::FlatForward(Integer settlementDays,
                                    const Calendar& calendar,
                                    Rate forward,
                                    const DayCounter& dayCounter,
                                    Compounding compounding,
                                    Frequency frequency)
    : YieldTermStructure(settlementDays,calendar), dayCounter_(dayCounter),
      compounding_(compounding), frequency_(frequency) {
        forward_.linkTo(boost::shared_ptr<Quote>(new SimpleQuote(forward)));
        updateRate();
    }

    inline Date FlatForward::maxDate() const {
        return Date::maxDate();
    }

    inline void FlatForward::update() {
        updateRate();
        YieldTermStructure::update();
    }

    inline DiscountFactor FlatForward::discountImpl(Time t) const {
        return rate_.discountFactor(t);
    }

    inline void FlatForward::updateRate() {
        rate_ = InterestRate(forward_->value(), dayCounter_,
                             compounding_, frequency_);
    }

}


#endif

