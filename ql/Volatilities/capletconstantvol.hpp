
/*
 Copyright (C) 2004 StatPro Italia srl

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

/*! \file capletconstantvol.hpp
    \brief Constant caplet volatility
*/

#ifndef quantlib_caplet_constant_volatility_hpp
#define quantlib_caplet_constant_volatility_hpp

#include <ql/capvolstructures.hpp>
#include <ql/DayCounters/actual365.hpp>

namespace QuantLib {

    //! Constant caplet volatility, no time-strike dependence
    class CapletConstantVolatility : public CapletVolatilityStructure {
      public:
        CapletConstantVolatility(const Date& referenceDate,
                                 Volatility volatility,
                                 const DayCounter& dayCounter = Actual365());
        CapletConstantVolatility(const Date& referenceDate,
                                 const Handle<Quote>& volatility,
                                 const DayCounter& dayCounter = Actual365());
        CapletConstantVolatility(Integer settlementDays, const Calendar&,
                                 Volatility volatility,
                                 const DayCounter& dayCounter = Actual365());
        CapletConstantVolatility(Integer settlementDays, const Calendar&,
                                 const Handle<Quote>& volatility,
                                 const DayCounter& dayCounter = Actual365());
        //! \name BaseTermStructure interface
        //@{
        DayCounter dayCounter() const;
        //@}
      protected:
        //! \name CapletVolatilityStructure interface
        //@{
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
    : CapletVolatilityStructure(referenceDate), dayCounter_(dayCounter) {
        volatility_.linkTo(
                       boost::shared_ptr<Quote>(new SimpleQuote(volatility)));
        registerWith(volatility_);
    }

    inline CapletConstantVolatility::CapletConstantVolatility(
                                              const Date& referenceDate,
                                              const Handle<Quote>& volatility,
                                              const DayCounter& dayCounter)
    : CapletVolatilityStructure(referenceDate), volatility_(volatility),
      dayCounter_(dayCounter) {
        registerWith(volatility_);
    }

    inline CapletConstantVolatility::CapletConstantVolatility(
                                              Integer settlementDays,
                                              const Calendar& calendar,
                                              Volatility volatility,
                                              const DayCounter& dayCounter)
    : CapletVolatilityStructure(settlementDays,calendar),
      dayCounter_(dayCounter) {
        volatility_.linkTo(
                       boost::shared_ptr<Quote>(new SimpleQuote(volatility)));
        registerWith(volatility_);
    }

    inline CapletConstantVolatility::CapletConstantVolatility(
                                              Integer settlementDays,
                                              const Calendar& calendar,
                                              const Handle<Quote>& volatility,
                                              const DayCounter& dayCounter)
    : CapletVolatilityStructure(settlementDays,calendar),
      volatility_(volatility), dayCounter_(dayCounter) {
        registerWith(volatility_);
    }

    inline DayCounter CapletConstantVolatility::dayCounter() const {
        return dayCounter_;
    }

    inline Volatility CapletConstantVolatility::volatilityImpl(
                                                         Time, Real) const {
        return volatility_->value();
    }

}


#endif
