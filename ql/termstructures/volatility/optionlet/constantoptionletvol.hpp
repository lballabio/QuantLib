/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Ferdinando Ametrano
 Copyright (C) 2004, 2005, 2007 StatPro Italia srl
 Copyright (C) 2015 Peter Caspers

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

/*! \file constantoptionletvol.hpp
    \brief Constant caplet/floorlet volatility
*/

#ifndef quantlib_caplet_constant_volatility_hpp
#define quantlib_caplet_constant_volatility_hpp

#include <ql/termstructures/volatility/optionlet/optionletvolatilitystructure.hpp>

namespace QuantLib {

    class Quote;

    //! Constant caplet volatility, no time-strike dependence
    class ConstantOptionletVolatility : public OptionletVolatilityStructure {
      public:
        //! floating reference date, floating market data
        ConstantOptionletVolatility(Natural settlementDays,
                                    const Calendar& cal,
                                    BusinessDayConvention bdc,
                                    Handle<Quote> volatility,
                                    const DayCounter& dc,
                                    VolatilityType type = ShiftedLognormal,
                                    Real displacement = 0.0);
        //! fixed reference date, floating market data
        ConstantOptionletVolatility(const Date& referenceDate,
                                    const Calendar& cal,
                                    BusinessDayConvention bdc,
                                    Handle<Quote> volatility,
                                    const DayCounter& dc,
                                    VolatilityType type = ShiftedLognormal,
                                    Real displacement = 0.0);
        //! floating reference date, fixed market data
        ConstantOptionletVolatility(Natural settlementDays, const Calendar &cal,
                                    BusinessDayConvention bdc,
                                    Volatility volatility, const DayCounter &dc,
                                    VolatilityType type = ShiftedLognormal,
                                    Real displacement = 0.0);
        //! fixed reference date, fixed market data
        ConstantOptionletVolatility(const Date &referenceDate,
                                    const Calendar &cal,
                                    BusinessDayConvention bdc,
                                    Volatility volatility, const DayCounter &dc,
                                    VolatilityType type = ShiftedLognormal,
                                    Real displacement = 0.0);
        //! \name TermStructure interface
        //@{
        Date maxDate() const override;
        //@}
        //! \name VolatilityTermStructure interface
        //@{
        Real minStrike() const override;
        Real maxStrike() const override;
        //@}
        VolatilityType volatilityType() const override;
        Real displacement() const override;

      protected:
        std::shared_ptr<SmileSection> smileSectionImpl(const Date& d) const override;
        std::shared_ptr<SmileSection> smileSectionImpl(Time) const override;
        Volatility volatilityImpl(Time, Rate) const override;

      private:
        Handle<Quote> volatility_;
        VolatilityType type_;
        Real displacement_;
    };


    // inline definitions

    inline Date ConstantOptionletVolatility::maxDate() const {
        return Date::maxDate();
    }

    inline Real ConstantOptionletVolatility::minStrike() const {
        return QL_MIN_REAL;
    }

    inline Real ConstantOptionletVolatility::maxStrike() const {
        return QL_MAX_REAL;
    }

    inline VolatilityType
    ConstantOptionletVolatility::volatilityType() const {
        return type_;
    }

    inline Real ConstantOptionletVolatility::displacement() const {
        return displacement_;
    }
}

#endif
