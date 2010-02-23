/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Allen Kuo

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

/*! \file callablebondconstantvol.hpp
    \brief Constant callable-bond volatility
*/

#ifndef quantlib_callable_bond_constant_volatility_hpp
#define quantlib_callable_bond_constant_volatility_hpp

#include <ql/experimental/callablebonds/callablebondvolstructure.hpp>
#include <ql/time/period.hpp>

namespace QuantLib {

    class Quote;

    //! Constant callable-bond volatility, no time-strike dependence
    class CallableBondConstantVolatility
        : public CallableBondVolatilityStructure {
      public:
        CallableBondConstantVolatility(const Date& referenceDate,
                                       Volatility volatility,
                                       const DayCounter& dayCounter);
        CallableBondConstantVolatility(const Date& referenceDate,
                                       const Handle<Quote>& volatility,
                                       const DayCounter& dayCounter);
        CallableBondConstantVolatility(Natural settlementDays,
                                       const Calendar&,
                                       Volatility volatility,
                                       const DayCounter& dayCounter);
        CallableBondConstantVolatility(Natural settlementDays,
                                       const Calendar&,
                                       const Handle<Quote>& volatility,
                                       const DayCounter& dayCounter);
        //! \name TermStructure interface
        //@{
        DayCounter dayCounter() const { return dayCounter_; }
        Date maxDate() const { return Date::maxDate(); }
        //@}
        //! \name CallableBondConstantVolatility interface
        //@{
        const Period& maxBondTenor() const;
        Time maxBondLength() const;
        Real minStrike() const;
        Real maxStrike() const;
      protected:
        Volatility volatilityImpl(Time, Time, Rate) const;
        boost::shared_ptr<SmileSection> smileSectionImpl(Time optionTime,
                                                         Time bondLength) const;
        Volatility volatilityImpl(const Date&, const Period&, Rate) const;
        //@}
      private:
        Handle<Quote> volatility_;
        DayCounter dayCounter_;
        Period maxBondTenor_;
    };


    // inline definitions

    inline const Period& CallableBondConstantVolatility::maxBondTenor() const {
        return maxBondTenor_;
    }

    inline Time CallableBondConstantVolatility::maxBondLength() const {
        return QL_MAX_REAL;
    }

    inline Real CallableBondConstantVolatility::minStrike() const {
        return QL_MIN_REAL;
    }

    inline Real CallableBondConstantVolatility::maxStrike() const {
        return QL_MAX_REAL;
    }

}

#endif

