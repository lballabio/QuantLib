/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
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

/*! \file blackatmvolcurve.hpp
    \brief Black at-the-money (no-smile) volatility curve base class
*/

#ifndef quantlib_black_atm_vol_curve_hpp
#define quantlib_black_atm_vol_curve_hpp

#include <ql/termstructures/voltermstructure.hpp>
#include <ql/patterns/visitor.hpp>

namespace QuantLib {

    //! Black at-the-money (no-smile) volatility curve
    /*! This abstract class defines the interface of concrete
        Black at-the-money (no-smile) volatility curves which will be
        derived from this one.

        Volatilities are assumed to be expressed on an annual basis.
    */
    class BlackAtmVolCurve : public VolatilityTermStructure {
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
        BlackAtmVolCurve(BusinessDayConvention bdc = Following,
                         const DayCounter& dc = DayCounter());
        //! initialize with a fixed reference date
        BlackAtmVolCurve(const Date& referenceDate,
                         const Calendar& cal = Calendar(),
                         BusinessDayConvention bdc = Following,
                         const DayCounter& dc = DayCounter());
        //! calculate the reference date based on the global evaluation date
        BlackAtmVolCurve(Natural settlementDays,
                         const Calendar&,
                         BusinessDayConvention bdc = Following,
                         const DayCounter& dc = DayCounter());
        //@}
        virtual ~BlackAtmVolCurve() {}
        //! \name Black at-the-money spot volatility
        //@{
        //! spot at-the-money volatility
        Volatility atmVol(const Period& optionTenor,
                          bool extrapolate = false) const;
        //! spot at-the-money volatility
        Volatility atmVol(const Date& maturity,
                          bool extrapolate = false) const;
        //! spot at-the-money volatility
        Volatility atmVol(Time maturity,
                          bool extrapolate = false) const;
        //! spot at-the-money variance
        Real atmVariance(const Period& optionTenor,
                         bool extrapolate = false) const;
        //! spot at-the-money variance
        Real atmVariance(const Date& maturity,
                         bool extrapolate = false) const;
        //! spot at-the-money variance
        Real atmVariance(Time maturity,
                         bool extrapolate = false) const;
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      protected:
        /*! \name Calculations

            These methods must be implemented in derived classes to perform
            the actual volatility calculations. When they are called,
            range check has already been performed; therefore, they must
            assume that extrapolation is required.
        */
        //@{
        //! spot at-the-money variance calculation
        virtual Real atmVarianceImpl(Time t) const = 0;
        //! spot at-the-money volatility calculation
        virtual Volatility atmVolImpl(Time t) const = 0;
        //@}
    };

}

#endif
