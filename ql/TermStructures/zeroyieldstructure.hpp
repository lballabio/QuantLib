
/*
 Copyright (C) 2000-2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file zeroyieldstructure.hpp
    \brief Zero-yield based term structure
*/

#ifndef quantlib_zero_yield_structure_hpp
#define quantlib_zero_yield_structure_hpp

#include <ql/termstructure.hpp>

namespace QuantLib {

    //! Zero-yield term structure
    /*! This abstract class acts as an adapter to YieldTermStructure
        allowing the programmer to implement only the
        <tt>zeroYieldImpl(Time, bool)</tt> method in derived classes.
        Discount and forward are calculated from zero yields.

        Rates are assumed to be annual continuous compounding.

        \ingroup yieldtermstructures
    */
    class ZeroYieldStructure : public YieldTermStructure {
      public:
        /*! \name Constructors
            See the BaseTermStructure documentation for issues regarding
            constructors.
        */
        //@{
        ZeroYieldStructure();
        ZeroYieldStructure(const Date& referenceDate);
        ZeroYieldStructure(Integer settlementDays, const Calendar&);
        //@}
        virtual ~ZeroYieldStructure() {}
      protected:
        //! \name YieldTermStructure implementation
        //@{
        /*! Returns the discount factor for the given date calculating it
            from the zero yield.
        */
        DiscountFactor discountImpl(Time) const;
        //! zero-yield calculation
        virtual Rate zeroYieldImpl(Time) const = 0;
        //@}
    };

    // inline definitions

    inline ZeroYieldStructure::ZeroYieldStructure() {}

    inline ZeroYieldStructure::ZeroYieldStructure(const Date& referenceDate)
    : YieldTermStructure(referenceDate) {}

    inline ZeroYieldStructure::ZeroYieldStructure(Integer settlementDays,
                                                  const Calendar& calendar)
    : YieldTermStructure(settlementDays,calendar) {}

    inline DiscountFactor ZeroYieldStructure::discountImpl(Time t) const {
        Rate r = zeroYieldImpl(t);
        return DiscountFactor(QL_EXP(-r*t));
    }

}


#endif
