/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2007 StatPro Italia srl

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

/*! \file capletconstantvol.hpp
    \brief Constant caplet volatility
*/

#ifndef quantlib_caplet_constant_volatility_hpp
#define quantlib_caplet_constant_volatility_hpp

#include <ql/termstructures/capvolstructures.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/time/calendars/nullcalendar.hpp>

namespace QuantLib {

    //! Constant caplet volatility, no time-strike dependence
    class CapletConstantVolatility : public CapletVolatilityStructure {
      public:
        CapletConstantVolatility(const Date& referenceDate,
                                 Volatility volatility,
                                 const DayCounter& dayCounter);
        CapletConstantVolatility(const Date& referenceDate,
                                 const Handle<Quote>& volatility,
                                 const DayCounter& dayCounter);
        CapletConstantVolatility(Volatility volatility,
                                 const DayCounter& dayCounter);
        CapletConstantVolatility(const Handle<Quote>& volatility,
                                 const DayCounter& dayCounter);
        //! \name TermStructure interface
        //@{
        DayCounter dayCounter() const { return dayCounter_; }
        Date maxDate() const;
        //@}
        //! \name CapletVolatilityStructure interface
        //@{
        Real minStrike() const;
        Real maxStrike() const;
      protected:
        Volatility volatilityImpl(Time t, Rate) const;
        //@}
      private:
        Handle<Quote> volatility_;
        DayCounter dayCounter_;
    };


    // inline definitions

    inline CapletConstantVolatility::CapletConstantVolatility(
                                              const Date& referenceDate,
                                              Volatility volatility,
                                              const DayCounter& dayCounter)
    : CapletVolatilityStructure(referenceDate),
      volatility_(boost::shared_ptr<Quote>(new SimpleQuote(volatility))),
      dayCounter_(dayCounter) {}

    inline CapletConstantVolatility::CapletConstantVolatility(
                                              const Date& referenceDate,
                                              const Handle<Quote>& volatility,
                                              const DayCounter& dayCounter)
    : CapletVolatilityStructure(referenceDate), volatility_(volatility),
      dayCounter_(dayCounter) {
        registerWith(volatility_);
    }

    inline CapletConstantVolatility::CapletConstantVolatility(
                                              Volatility volatility,
                                              const DayCounter& dayCounter)
    : CapletVolatilityStructure(0, NullCalendar()),
      volatility_(boost::shared_ptr<Quote>(new SimpleQuote(volatility))),
      dayCounter_(dayCounter) {}

    inline CapletConstantVolatility::CapletConstantVolatility(
                                              const Handle<Quote>& volatility,
                                              const DayCounter& dayCounter)
    : CapletVolatilityStructure(0, NullCalendar()), volatility_(volatility),
      dayCounter_(dayCounter) {
        registerWith(volatility_);
    }

    inline Date CapletConstantVolatility::maxDate() const {
        return Date::maxDate();
    }

    inline Real CapletConstantVolatility::minStrike() const {
        return QL_MIN_REAL;
    }

    inline Real CapletConstantVolatility::maxStrike() const {
        return QL_MAX_REAL;
    }

    inline Volatility CapletConstantVolatility::volatilityImpl(
                                                         Time, Rate) const {
        return volatility_->value();
    }

}

#endif
