/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 StatPro Italia srl

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

/*! \file swaptionconstantvol.hpp
    \brief Constant swaption volatility
*/

#ifndef quantlib_swaption_constant_volatility_hpp
#define quantlib_swaption_constant_volatility_hpp

#include <ql/termstructures/voltermstructures/swaptionvolstructure.hpp>
#include <ql/time/period.hpp>

namespace QuantLib {

    class Quote;

    //! Constant swaption volatility, no time-strike dependence
    class SwaptionConstantVolatility : public SwaptionVolatilityStructure {
      public:
        SwaptionConstantVolatility(const Date& referenceDate,
                                   Volatility volatility,
                                   const DayCounter& dayCounter);
        SwaptionConstantVolatility(const Date& referenceDate,
                                   const Handle<Quote>& volatility,
                                   const DayCounter& dayCounter);
        SwaptionConstantVolatility(Natural settlementDays,
                                   const Calendar&,
                                   Volatility volatility,
                                   const DayCounter& dayCounter);
        SwaptionConstantVolatility(Natural settlementDays,
                                   const Calendar&,
                                   const Handle<Quote>& volatility,
                                   const DayCounter& dayCounter);
        //! \name TermStructure interface
        //@{
        DayCounter dayCounter() const { return dayCounter_; }
        Date maxDate() const { return Date::maxDate(); }
        //@}
        //! \name SwaptionConstantVolatility interface
        //@{
        const Period& maxSwapTenor() const;
        Time maxSwapLength() const;
        Real minStrike() const;
        Real maxStrike() const;
      protected:
        Volatility volatilityImpl(Time, Time, Rate) const;
        boost::shared_ptr<SmileSection> smileSectionImpl(Time optionTime,
                                                         Time swapLength) const;
        Volatility volatilityImpl(const Date&, const Period&, Rate) const;
        //@}
      private:
        Handle<Quote> volatility_;
        DayCounter dayCounter_;
        Period maxSwapTenor_;
    };


    // inline definitions

    inline const Period& SwaptionConstantVolatility::maxSwapTenor() const {
        return maxSwapTenor_;
    }

    inline Time SwaptionConstantVolatility::maxSwapLength() const {
        return QL_MAX_REAL;
    }

    inline Real SwaptionConstantVolatility::minStrike() const {
        return QL_MIN_REAL;
    }

    inline Real SwaptionConstantVolatility::maxStrike() const {
        return QL_MAX_REAL;
    }

}

#endif
