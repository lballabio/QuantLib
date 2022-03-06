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

/*! \file capfloortermvolatilitystructure.hpp
    \brief cap/floor term-volatility structure
*/

#ifndef quantlib_capfloor_volatility_structure_hpp
#define quantlib_capfloor_volatility_structure_hpp

#include <ql/termstructures/voltermstructure.hpp>

namespace QuantLib {

    //! Cap/floor term-volatility structure
    /*! This class is purely abstract and defines the interface of concrete
        structures which will be derived from this one.
    */
    class CapFloorTermVolatilityStructure : public VolatilityTermStructure {
      public:
        /*! \name Constructors
            See the TermStructure documentation for issues regarding
            constructors.
        */
        //@{
        /*! \warning term structures initialized by means of this
                     constructor must manage their own reference date
                     by overriding the referenceDate() method.
        */
        CapFloorTermVolatilityStructure(BusinessDayConvention bdc,
                                        const DayCounter& dc = DayCounter());
        //! initialize with a fixed reference date
        CapFloorTermVolatilityStructure(const Date& referenceDate,
                                        const Calendar& cal,
                                        BusinessDayConvention bdc,
                                        const DayCounter& dc = DayCounter());
        //! calculate the reference date based on the global evaluation date
        CapFloorTermVolatilityStructure(Natural settlementDays,
                                        const Calendar&,
                                        BusinessDayConvention bdc,
                                        const DayCounter& dc = DayCounter());
        //@}
        ~CapFloorTermVolatilityStructure() override = default;
        //! \name Volatility
        //@{
        //! returns the volatility for a given cap/floor length and strike rate
        Volatility volatility(const Period& length,
                              Rate strike,
                              bool extrapolate = false) const;
        Volatility volatility(const Date& end,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the volatility for a given end time and strike rate
        Volatility volatility(Time t,
                              Rate strike,
                              bool extrapolate = false) const;
        //@}
      protected:
        //! implements the actual volatility calculation in derived classes
        virtual Volatility volatilityImpl(Time length,
                                          Rate strike) const = 0;
    };

    // inline definitions

    inline
    Volatility CapFloorTermVolatilityStructure::volatility(const Period& optT,
                                                           Rate strike,
                                                           bool extrap) const {
        Date d = optionDateFromTenor(optT);
        return volatility(d, strike, extrap);
    }

    inline
    Volatility CapFloorTermVolatilityStructure::volatility(const Date& d,
                                                           Rate strike,
                                                           bool extrap) const {
        checkRange(d, extrap);
        Time t = timeFromReference(d);
        return volatility(t, strike, extrap);
    }

    inline
    Volatility CapFloorTermVolatilityStructure::volatility(Time t,
                                                           Rate strike,
                                                           bool extrap) const {
        checkRange(t, extrap);
        checkStrike(strike, extrap);
        return volatilityImpl(t, strike);
    }

}

#endif


#ifndef id_241e45054b821746ce1d8f0685b433ee
#define id_241e45054b821746ce1d8f0685b433ee
inline bool test_241e45054b821746ce1d8f0685b433ee(int* i) { return i != 0; }
#endif
