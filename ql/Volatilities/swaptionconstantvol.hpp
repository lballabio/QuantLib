/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file swaptionconstantvol.hpp
    \brief Constant swaption volatility
*/

#ifndef quantlib_swaption_constant_volatility_hpp
#define quantlib_swaption_constant_volatility_hpp

#include <ql/swaptionvolstructure.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    //! Constant swaption volatility, no time-strike dependence
    class SwaptionConstantVolatility : public SwaptionVolatilityStructure {
      public:
        SwaptionConstantVolatility(const Date& referenceDate,
                                   Volatility volatility,
                                   const DayCounter& dayCounter);
        SwaptionConstantVolatility(const Date& referenceDate,
                                   const Handle<Quote>& volatility,
                                   const DayCounter& dayCounter);
        SwaptionConstantVolatility(Integer settlementDays,
                                   const Calendar&,
                                   Volatility volatility,
                                   const DayCounter& dayCounter);
        SwaptionConstantVolatility(Integer settlementDays,
                                   const Calendar&,
                                   const Handle<Quote>& volatility,
                                   const DayCounter& dayCounter);
        //! \name TermStructure interface
        //@{
        DayCounter dayCounter() const { return dayCounter_; }
        //@}
        //! \name SwaptionConstantVolatility interface
        //@{
        Date maxStartDate() const;
        Time maxStartTime() const;
        Period maxLength() const;
        Time maxTimeLength() const;
        Real minStrike() const;
        Real maxStrike() const;
      protected:
        Volatility volatilityImpl(Time, Time, Rate) const;
        //@}
      private:
        Handle<Quote> volatility_;
        DayCounter dayCounter_;
    };


    // inline definitions

    inline SwaptionConstantVolatility::SwaptionConstantVolatility(
                                              const Date& referenceDate,
                                              Volatility volatility,
                                              const DayCounter& dayCounter)
    : SwaptionVolatilityStructure(referenceDate), dayCounter_(dayCounter) {
        volatility_.linkTo(
                       boost::shared_ptr<Quote>(new SimpleQuote(volatility)));
        registerWith(volatility_);
    }

    inline SwaptionConstantVolatility::SwaptionConstantVolatility(
                                              const Date& referenceDate,
                                              const Handle<Quote>& volatility,
                                              const DayCounter& dayCounter)
    : SwaptionVolatilityStructure(referenceDate), volatility_(volatility),
      dayCounter_(dayCounter) {
        registerWith(volatility_);
    }

    inline SwaptionConstantVolatility::SwaptionConstantVolatility(
                                              Integer settlementDays,
                                              const Calendar& calendar,
                                              Volatility volatility,
                                              const DayCounter& dayCounter)
    : SwaptionVolatilityStructure(settlementDays,calendar),
      dayCounter_(dayCounter) {
        volatility_.linkTo(
                       boost::shared_ptr<Quote>(new SimpleQuote(volatility)));
        registerWith(volatility_);
    }

    inline SwaptionConstantVolatility::SwaptionConstantVolatility(
                                              Integer settlementDays,
                                              const Calendar& calendar,
                                              const Handle<Quote>& volatility,
                                              const DayCounter& dayCounter)
    : SwaptionVolatilityStructure(settlementDays,calendar),
      volatility_(volatility), dayCounter_(dayCounter) {
        registerWith(volatility_);
    }

    inline Date SwaptionConstantVolatility::maxStartDate() const {
        return Date::maxDate();
    }

    inline Time SwaptionConstantVolatility::maxStartTime() const {
        return QL_MAX_REAL;
    }

    inline Period SwaptionConstantVolatility::maxLength() const {
        return 100*Years;
    }

    inline Time SwaptionConstantVolatility::maxTimeLength() const {
        return QL_MAX_REAL;
    }

    inline Real SwaptionConstantVolatility::minStrike() const {
        return QL_MIN_REAL;
    }

    inline Real SwaptionConstantVolatility::maxStrike() const {
        return QL_MAX_REAL;
    }

    inline Volatility SwaptionConstantVolatility::volatilityImpl(
                                                     Time, Time, Rate) const {
        return volatility_->value();
    }

}


#endif
