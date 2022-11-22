/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Ferdinando Ametrano

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

/*! \file constantcapfloortermvol.hpp
    \brief Constant cap/floor term volatility
*/

#ifndef quantlib_constant_capfloor_term_volatility_hpp
#define quantlib_constant_capfloor_term_volatility_hpp

#include <ql/termstructures/volatility/capfloor/capfloortermvolatilitystructure.hpp>

namespace QuantLib {

    class Quote;

    //! Constant caplet volatility, no time-strike dependence
    class ConstantCapFloorTermVolatility : public CapFloorTermVolatilityStructure {
      public:
        //! floating reference date, floating market data
        ConstantCapFloorTermVolatility(Natural settlementDays,
                                       const Calendar& cal,
                                       BusinessDayConvention bdc,
                                       Handle<Quote> volatility,
                                       const DayCounter& dc);
        //! fixed reference date, floating market data
        ConstantCapFloorTermVolatility(const Date& referenceDate,
                                       const Calendar& cal,
                                       BusinessDayConvention bdc,
                                       Handle<Quote> volatility,
                                       const DayCounter& dc);
        //! floating reference date, fixed market data
        ConstantCapFloorTermVolatility(Natural settlementDays,
                                      const Calendar& cal,
                                      BusinessDayConvention bdc,
                                      Volatility volatility,
                                      const DayCounter& dc);
        //! fixed reference date, fixed market data
        ConstantCapFloorTermVolatility(const Date& referenceDate,
                                       const Calendar& cal,
                                       BusinessDayConvention bdc,
                                       Volatility volatility,
                                       const DayCounter& dc);
        //! \name TermStructure interface
        //@{
        Date maxDate() const override;
        //@}
        //! \name VolatilityTermStructure interface
        //@{
        Real minStrike() const override;
        Real maxStrike() const override;
        //@}
      protected:
        Volatility volatilityImpl(Time, Rate) const override;

      private:
        Handle<Quote> volatility_;
    };


    // inline definitions

    inline Date ConstantCapFloorTermVolatility::maxDate() const {
        return Date::maxDate();
    }

    inline Real ConstantCapFloorTermVolatility::minStrike() const {
        return QL_MIN_REAL;
    }

    inline Real ConstantCapFloorTermVolatility::maxStrike() const {
        return QL_MAX_REAL;
    }

}

#endif
