
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
        #ifndef QL_DISABLE_DEPRECATED
        /*! \deprecated use the constructor without today's date; set the
                        evaluation date through Settings::instance().
        */
        ZeroYieldStructure(const Date& todaysDate, const Date& referenceDate);
        #endif
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
        /*! Returns the instantaneous forward rate for the given date
            calculating it from the zero yield.
        */
        Rate forwardImpl(Time) const;
        #ifndef QL_DISABLE_DEPRECATED
        /*! Returns the forward rate at a specified compound frequency
	    for the given date calculating it from the zero yield.
        */
        Rate compoundForwardImpl(Time, Integer) const;
        #endif
        //@}
    };

    // inline definitions

    #ifndef QL_DISABLE_DEPRECATED
    inline ZeroYieldStructure::ZeroYieldStructure(const Date& todaysDate,
                                                  const Date& referenceDate)
    : YieldTermStructure(todaysDate,referenceDate) {}
    #endif

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

    inline Rate ZeroYieldStructure::forwardImpl(Time t) const {
        // less than half day
        Time dt = 0.001;
        Rate r1 = zeroYieldImpl(t),
             r2 = zeroYieldImpl(t+dt);
        return r2+t*(r2-r1)/dt;
    }

    #ifndef QL_DISABLE_DEPRECATED
    inline Rate ZeroYieldStructure::compoundForwardImpl(Time t,
                                                        Integer f) const {
        Rate zy = zeroYieldImpl(t);
        if (f == 0)
            return zy;
        if (t <= 1.0/f)
            return (QL_EXP(zy*t)-1.0)/t;
        return (QL_EXP(zy*(1.0/f))-1.0)*f;
    }
    #endif


}


#endif
