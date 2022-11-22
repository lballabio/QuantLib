/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004 StatPro Italia srl
 Copyright (C) 2009 Ferdinando Ametrano

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

/*! \file forwardstructure.hpp
    \brief Forward-based yield term structure
*/

#ifndef quantlib_forward_structure_hpp
#define quantlib_forward_structure_hpp

#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! %Forward-rate term structure
    /*! This abstract class acts as an adapter to YieldTermStructure allowing
        the programmer to implement only the <tt>forwardImpl(Time)</tt> method
        in derived classes.

        Zero yields and discounts are calculated from forwards.

        Forward rates are assumed to be annual continuous compounding.

        \ingroup yieldtermstructures
    */
    class ForwardRateStructure : public YieldTermStructure {
      public:
        /*! \name Constructors
            See the TermStructure documentation for issues regarding
            constructors.
        */
        //@{
        explicit ForwardRateStructure(
            const DayCounter& dayCounter = DayCounter());
        explicit ForwardRateStructure(
            const Date& referenceDate,
            const Calendar& cal = Calendar(),
            const DayCounter& dayCounter = DayCounter(),
            const std::vector<Handle<Quote> >& jumps = {},
            const std::vector<Date>& jumpDates = {});
        ForwardRateStructure(
            Natural settlementDays,
            const Calendar& cal,
            const DayCounter& dayCounter = DayCounter(),
            const std::vector<Handle<Quote> >& jumps = {},
            const std::vector<Date>& jumpDates = {});
        //@}
      protected:
        /*! \name Calculations

            These methods must be implemented in derived classes to
            perform the actual calculations. When they are called,
            range check has already been performed; therefore, they
            must assume that extrapolation is required.
        */
        //@{
        //! instantaneous forward-rate calculation
        virtual Rate forwardImpl(Time) const = 0;
        /*! Returns the zero yield rate for the given date calculating it
            from the instantaneous forward rate \f$ f(t) \f$ as
            \f[
            z(t) = \int_0^t f(\tau) d\tau
            \f]

            \warning This default implementation uses an highly inefficient
                     and possibly wildly inaccurate numerical integration.
                     Derived classes should override it if a more efficient
                     implementation is available.
        */
        virtual Rate zeroYieldImpl(Time) const;
        //@}

        //! \name YieldTermStructure implementation
        //@{
        /*! Returns the discount factor for the given date calculating it
            from the zero rate as \f$ d(t) = \exp \left( -z(t) t \right) \f$
        */
        DiscountFactor discountImpl(Time) const override;
        //@}
    };


    // inline definitions

    inline DiscountFactor ForwardRateStructure::discountImpl(Time t) const {
        if (t == 0.0)     // this acts as a safe guard in cases where
            return 1.0;   // zeroYieldImpl(0.0) would throw.

        Rate r = zeroYieldImpl(t);
        return DiscountFactor(std::exp(-r*t));
    }

}

#endif
