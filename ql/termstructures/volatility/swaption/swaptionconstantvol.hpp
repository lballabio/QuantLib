/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Ferdinando Ametrano
 Copyright (C) 2006, 2007 StatPro Italia srl
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

/*! \file swaptionconstantvol.hpp
    \brief Constant swaption volatility
*/

#ifndef quantlib_swaption_constant_volatility_hpp
#define quantlib_swaption_constant_volatility_hpp

#include <ql/termstructures/volatility/swaption/swaptionvolstructure.hpp>
#include <ql/time/period.hpp>

namespace QuantLib {

    class Quote;

    //! Constant swaption volatility, no time-strike dependence
    class ConstantSwaptionVolatility : public SwaptionVolatilityStructure {
      public:
        //! floating reference date, floating market data
        ConstantSwaptionVolatility(Natural settlementDays,
                                   const Calendar& cal,
                                   BusinessDayConvention bdc,
                                   Handle<Quote> volatility,
                                   const DayCounter& dc,
                                   VolatilityType type = ShiftedLognormal,
                                   Real shift = 0.0);
        //! fixed reference date, floating market data
        ConstantSwaptionVolatility(const Date& referenceDate,
                                   const Calendar& cal,
                                   BusinessDayConvention bdc,
                                   Handle<Quote> volatility,
                                   const DayCounter& dc,
                                   VolatilityType type = ShiftedLognormal,
                                   Real shift = 0.0);
        //! floating reference date, fixed market data
        ConstantSwaptionVolatility(Natural settlementDays,
                                   const Calendar& cal,
                                   BusinessDayConvention bdc,
                                   Volatility volatility,
                                   const DayCounter& dc,
                                   VolatilityType type = ShiftedLognormal,
                                   Real shift = 0.0);
        //! fixed reference date, fixed market data
        ConstantSwaptionVolatility(const Date& referenceDate,
                                   const Calendar& cal,
                                   BusinessDayConvention bdc,
                                   Volatility volatility,
                                   const DayCounter& dc,
                                   VolatilityType type = ShiftedLognormal,
                                   Real shift = 0.0);
        //! \name TermStructure interface
        //@{
        Date maxDate() const override;
        //@}
        //! \name VolatilityTermStructure interface
        //@{
        Real minStrike() const override;
        Real maxStrike() const override;
        //@}
        //! \name SwaptionVolatilityStructure interface
        //@{
        const Period& maxSwapTenor() const override;
        //@}
        //! volatility type
        VolatilityType volatilityType() const override;

      protected:
        ext::shared_ptr<SmileSection> smileSectionImpl(const Date&, const Period&) const override;
        ext::shared_ptr<SmileSection> smileSectionImpl(Time, Time) const override;
        Volatility volatilityImpl(const Date&, const Period&, Rate) const override;
        Volatility volatilityImpl(Time, Time, Rate) const override;
        Real shiftImpl(Time optionTime, Time swapLength) const override;

      private:
        Handle<Quote> volatility_;
        Period maxSwapTenor_;
        VolatilityType volatilityType_;
        Real shift_;
    };


    // inline definitions

    inline Date ConstantSwaptionVolatility::maxDate() const {
        return Date::maxDate();
    }

    inline Real ConstantSwaptionVolatility::minStrike() const {
        return QL_MIN_REAL;
    }

    inline Real ConstantSwaptionVolatility::maxStrike() const {
        return QL_MAX_REAL;
    }

    inline const Period& ConstantSwaptionVolatility::maxSwapTenor() const {
        return maxSwapTenor_;
    }

    inline VolatilityType ConstantSwaptionVolatility::volatilityType() const {
        return volatilityType_;
    }

    inline Real ConstantSwaptionVolatility::shiftImpl(Time optionTime, Time swapLength) const {
        // consistency check
        SwaptionVolatilityStructure::shiftImpl(optionTime, swapLength);
        return shift_;
    }

}

#endif


#ifndef id_2fcd18e440d29bff9ad291e7b54c6058
#define id_2fcd18e440d29bff9ad291e7b54c6058
inline bool test_2fcd18e440d29bff9ad291e7b54c6058(int* i) { return i != 0; }
#endif
