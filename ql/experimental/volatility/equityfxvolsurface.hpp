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

/*! \file equityfxvolsurface.hpp
    \brief Equity/FX vol (smile) surface
*/

#ifndef quantlib_equity_fx_vol_surface_hpp
#define quantlib_equity_fx_vol_surface_hpp

#include <ql/experimental/volatility/blackvolsurface.hpp>

namespace QuantLib {

    //! Equity/FX volatility (smile) surface
    /*! This abstract class defines the interface of concrete
        Equity/FX volatility (smile) surfaces which will
        be derived from this one.

        Volatilities are assumed to be expressed on an annual basis.

        It's only in absence of smile that the concept of (at-the-money)
        forward volatility makes sense.
    */
    class EquityFXVolSurface : public BlackVolSurface {
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
        explicit EquityFXVolSurface(BusinessDayConvention bdc = Following,
                                    const DayCounter& dc = DayCounter());
        //! initialize with a fixed reference date
        explicit EquityFXVolSurface(const Date& referenceDate,
                                    const Calendar& cal = Calendar(),
                                    BusinessDayConvention bdc = Following,
                                    const DayCounter& dc = DayCounter());
        //! calculate the reference date based on the global evaluation date
        EquityFXVolSurface(Natural settlementDays,
                           const Calendar&,
                           BusinessDayConvention bdc = Following,
                           const DayCounter& dc = DayCounter());
        //@}
        //! \name Black Volatility
        //@{
        //! forward (at-the-money) volatility
        Volatility atmForwardVol(const Date& date1,
                                 const Date& date2,
                                 bool extrapolate = false) const;
        //! forward (at-the-money) volatility
        Volatility atmForwardVol(Time time1,
                                 Time time2,
                                 bool extrapolate = false) const;
        //! forward (at-the-money) variance
        Real atmForwardVariance(const Date& date1,
                                const Date& date2,
                                bool extrapolate = false) const;
        //! forward (at-the-money) variance
        Real atmForwardVariance(Time time1,
                                Time time2,
                                bool extrapolate = false) const;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
    };

}

#endif
