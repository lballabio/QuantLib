/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000-2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file forwardstructure.hpp
    \brief Forward-based yield term structure
*/

#ifndef quantlib_forward_structure_hpp
#define quantlib_forward_structure_hpp

#include <ql/yieldtermstructure.hpp>

namespace QuantLib {

    //! Forward rate term structure
    /*! This abstract class acts as an adapter to TermStructure allowing the
        programmer to implement only the
        <tt>forwardImpl(const Date&, bool)</tt> method in derived classes.
        Zero yields and discounts are calculated from forwards.

        Rates are assumed to be annual continuous compounding.

        \ingroup yieldtermstructures
    */
    class ForwardRateStructure : public YieldTermStructure {
      public:
        /*! \name Constructors
            See the TermStructure documentation for issues regarding
            constructors.
        */
        //@{
        ForwardRateStructure();
        ForwardRateStructure(const Date& referenceDate);
        ForwardRateStructure(Integer settlementDays, const Calendar&);
        //@}
        virtual ~ForwardRateStructure() {}
      protected:
        //! \name YieldTermStructure implementation
        //@{
        /*! Returns the discount factor for the given date calculating it
            from the instantaneous forward rate.
        */
        DiscountFactor discountImpl(Time) const;
        //! instantaneous forward-rate calculation
        virtual Rate forwardImpl(Time) const = 0;
        /*! Returns the zero yield rate for the given date calculating it
            from the instantaneous forward rate.

            \warning This is just a default, highly inefficient and
                     possibly wildly inaccurate implementation. Derived
                     classes should implement their own zeroYield method.
        */
        virtual Rate zeroYieldImpl(Time) const;
        //@}
    };


    // inline definitions

    inline ForwardRateStructure::ForwardRateStructure() {}

    inline ForwardRateStructure::ForwardRateStructure(
                                                    const Date& referenceDate)
    : YieldTermStructure(referenceDate) {}

    inline ForwardRateStructure::ForwardRateStructure(Integer settlementDays,
                                                      const Calendar& calendar)
    : YieldTermStructure(settlementDays,calendar) {}

    inline Rate ForwardRateStructure::zeroYieldImpl(Time t) const {
        if (t == 0.0)
            return forwardImpl(0.0);
        // implement smarter integration if plan to use the following code
        Rate sum = 0.5*forwardImpl(0.0);
        Size N = 1000;
        Time dt = t/N;
        for (Time i=dt; i<t; i+=dt)
            sum += forwardImpl(i);
        sum += 0.5*forwardImpl(t);
        return Rate(sum*dt/t);
    }

    inline DiscountFactor ForwardRateStructure::discountImpl(Time t) const {
        Rate r = zeroYieldImpl(t);
        return DiscountFactor(std::exp(-r*t));
    }

}


#endif
