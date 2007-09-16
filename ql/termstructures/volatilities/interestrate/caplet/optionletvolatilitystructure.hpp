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

/*! \file optionletvolatilitystructure.hpp
    \brief optionlet (caplet/floorlet) volatility structure
*/

#ifndef quantlib_optionlet_volatility_structure_hpp
#define quantlib_optionlet_volatility_structure_hpp

#include <ql/termstructures/voltermstructure.hpp>

namespace QuantLib {

    //! Optionlet (caplet/floorlet) volatility structure
    /*! This class is purely abstract and defines the interface of
        concrete structures which will be derived from this one.
    */
    class OptionletVolatilityStructure : public VolatilityTermStructure {
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
        OptionletVolatilityStructure(BusinessDayConvention bdc = Following,
                                     const DayCounter& dc = Actual365Fixed());
        //! initialize with a fixed reference date
        OptionletVolatilityStructure(const Date& referenceDate,
                                     const Calendar& cal = Calendar(),
                                     BusinessDayConvention bdc = Following,
                                     const DayCounter& dc = Actual365Fixed());
        //! calculate the reference date based on the global evaluation date
        OptionletVolatilityStructure(Natural settlementDays,
                                     const Calendar&,
                                     BusinessDayConvention bdc = Following,
                                     const DayCounter& dc = Actual365Fixed());
        //@}
        virtual ~OptionletVolatilityStructure() {}
        //! \name Volatility and Variance
        //@{
        //! returns the volatility for a given exercise date and strike rate
        Volatility volatility(const Date& exerciseDate,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the volatility for a given exercise time and strike rate
        Volatility volatility(Time t,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the volatility for a given option tenor and strike rate
        Volatility volatility(const Period& optionTenor,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the Black variance for a given exercise date and strike rate
        Real blackVariance(const Date& exerciseDate,
                           Rate strike,
                           bool extrapolate = false) const;
        //! returns the Black variance for a given start time and strike rate
        Real blackVariance(Time t,
                           Rate strike,
                           bool extrapolate = false) const;
        //! returns the Black variance for a given option tenor and strike rate
        Volatility blackVariance(const Period& optionTenor,
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
        virtual Volatility volatilityImpl(Time length,
                                          Rate strike) const = 0;
      private:
        void checkRange(Time, Rate strike, bool extrapolate) const;
    };


    // inline definitions

    inline Volatility
    OptionletVolatilityStructure::volatility(const Date& start,
                                             Rate strike,
                                             bool extrapolate) const {
        Time t = timeFromReference(start);
        checkRange(t, strike, extrapolate);
        return volatilityImpl(t,strike);
    }

    inline Volatility
    OptionletVolatilityStructure::volatility(Time t,
                                             Rate strike,
                                             bool extrapolate) const {
        checkRange(t, strike, extrapolate);
        return volatilityImpl(t,strike);
    }

    inline Volatility
    OptionletVolatilityStructure::volatility(const Period& optionTenor,
                                             Rate strike,
                                             bool extrapolate) const {
        Date exerciseDate = optionDateFromTenor(optionTenor);
        return volatility(exerciseDate, strike, extrapolate);
    }

    inline Volatility
    OptionletVolatilityStructure::blackVariance(const Date& start,
                                                Rate strike,
                                                bool extrap) const {
        Time t = timeFromReference(start);
        checkRange(t, strike, extrap);
        Volatility vol = volatilityImpl(t, strike);
        return vol*vol*t;
    }

    inline Volatility
    OptionletVolatilityStructure::blackVariance(Time t,
                                                Rate strike,
                                                bool extrap) const {
        checkRange(t, strike, extrap);
        Volatility vol = volatilityImpl(t, strike);
        return vol*vol*t;
    }

    inline Volatility
    OptionletVolatilityStructure::blackVariance(const Period& optionT,
                                                Rate strike,
                                                bool extrap) const {
        Date exerciseDate = optionDateFromTenor(optionT);
        return blackVariance(exerciseDate, strike, extrap);
    }

    inline void
    OptionletVolatilityStructure::checkRange(Time t,
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
