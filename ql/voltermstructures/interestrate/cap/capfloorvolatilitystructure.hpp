/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

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

/*! \file capfloorvolatilitystructure.hpp
    \brief cap/floor volatility structure
*/

#ifndef quantlib_capfloor_volatility_structure_hpp
#define quantlib_capfloor_volatility_structure_hpp

#include <ql/voltermstructure.hpp>

namespace QuantLib {

    //! Cap/floor term-volatility structure
    /*! This class is purely abstract and defines the interface of concrete
        structures which will be derived from this one.
    */
    class CapFloorVolatilityStructure : public VolatilityTermStructure {
      public:
        /*! \name Constructors
            See the TermStructure documentation for issues regarding
            constructors.
        */
        //@{
        //! default constructor
        /*! \warning term structures initialized by means of this
                     constructor must manage their own reference date
                     by overriding the referenceDate() method.
        */
        CapFloorVolatilityStructure(BusinessDayConvention bdc = Following,
                                    const DayCounter& dc = DayCounter());
        //! initialize with a fixed reference date
        CapFloorVolatilityStructure(const Date& referenceDate,
                                    const Calendar& cal = Calendar(),
                                    BusinessDayConvention bdc = Following,
                                    const DayCounter& dc = DayCounter());
        //! calculate the reference date based on the global evaluation date
        CapFloorVolatilityStructure(Natural settlementDays,
                                    const Calendar&,
                                    BusinessDayConvention bdc = Following,
                                    const DayCounter& dc = DayCounter());
        //@}
        virtual ~CapFloorVolatilityStructure() {}
        //! \name Volatility
        //@{
        Volatility volatility(const Date& end,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the volatility for a given cap/floor length and strike rate
        Volatility volatility(const Period& length,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the volatility for a given end time and strike rate
        Volatility volatility(Time t,
                              Rate strike,
                              bool extrapolate = false) const;
        //@}
        //! \name Limits
        //@{
        //! the minimum strike for which the term structure can return vols
        virtual Real minStrike() const = 0;
        //! the maximum strike for which the term structure can return vols
        virtual Real maxStrike() const = 0;
        //@}
      protected:
        //! implements the actual volatility calculation in derived classes
        virtual Volatility volatilityImpl(Time length, Rate strike) const = 0;
      private:
        void checkRange(Time, Rate strike, bool extrapolate) const;
    };

    // inline definitions

    inline
    Volatility CapFloorVolatilityStructure::volatility(const Date& end,
                                                  Rate strike,
                                                  bool extrapolate) const {
        Time t = timeFromReference(end);
        checkRange(t,strike,extrapolate);
        return volatilityImpl(t,strike);
    }

    inline
    Volatility CapFloorVolatilityStructure::volatility(Time t,
                                                  Rate strike,
                                                  bool extrapolate) const {
        checkRange(t,strike,extrapolate);
        return volatilityImpl(t,strike);
    }

    inline
    Volatility CapFloorVolatilityStructure::volatility(const Period& optionT,
                                                       Rate strike,
                                                       bool extrap) const {
        Date exerciseDate = optionDateFromTenor(optionT);
        return volatility(exerciseDate, strike, extrap);
    }

    inline
    void CapFloorVolatilityStructure::checkRange(Time t,
                                            Rate k,
                                            bool extrapolate) const {
        TermStructure::checkRange(t, extrapolate);
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   (k >= minStrike() && k <= maxStrike()),
                   "strike (" << k << ") is outside the curve domain ["
                   << minStrike() << "," << maxStrike()<< "]");
    }

}

#endif
