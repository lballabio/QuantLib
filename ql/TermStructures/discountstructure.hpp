
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

/*! \file discountstructure.hpp
    \brief Discount-based yield term structure
*/

#ifndef quantlib_discount_structure_hpp
#define quantlib_discount_structure_hpp

#include <ql/termstructure.hpp>

namespace QuantLib {

    #ifndef QL_DISABLE_DEPRECATED
    //! Discount factor term structure
    /*! This abstract class acts as an adapter to YieldTermStructure
        allowing the programmer to implement only the
        <tt>discountImpl(const Date&, bool)</tt> method in derived
        classes.
        Zero yield and forward are calculated from discounts.

        Rates are assumed to be annual continuous compounding.

        \deprecated use YieldTermStructure instead

        \ingroup yieldtermstructures
    */
    class DiscountStructure : public YieldTermStructure {
      public:
        /*! \name Constructors
            See the BaseTermStructure documentation for issues regarding
            constructors.
        */
        //@{
        DiscountStructure(const Date& todaysDate, const Date& referenceDate);
        DiscountStructure();
        DiscountStructure(const Date& referenceDate);
        DiscountStructure(Integer settlementDays, const Calendar&);
        //@}
        virtual ~DiscountStructure() {}
      protected:
        //! \name YieldTermStructure implementation
        //@{
        /*! Returns the zero yield rate for the given date calculating it
            from the discount.
        */
        Rate zeroYieldImpl(Time) const;
        /*! Returns the instantaneous forward rate for the given date
            calculating it from the discount.
        */
        Rate forwardImpl(Time) const;
        /*! Returns the forward rate at a specified compound frequency
	    for the given date calculating it from the zero yield.
        */
        virtual Rate compoundForwardImpl(Time, Integer) const;
        //@}
    };

    // inline definitions


    inline DiscountStructure::DiscountStructure(const Date& todaysDate,
                                                const Date& referenceDate)
    : YieldTermStructure(todaysDate,referenceDate) {}

    inline DiscountStructure::DiscountStructure() {}

    inline DiscountStructure::DiscountStructure(const Date& referenceDate)
    : YieldTermStructure(referenceDate) {}

    inline DiscountStructure::DiscountStructure(Integer settlementDays,
                                                const Calendar& calendar)
    : YieldTermStructure(settlementDays,calendar) {}

    inline Rate DiscountStructure::zeroYieldImpl(Time t) const {
        DiscountFactor df;
        if (t==0.0) {
            Time dt = 0.001;
            df = discountImpl(dt);
            return Rate(-QL_LOG(df)/dt);
        } else {
            df = discountImpl(t);
            return Rate(-QL_LOG(df)/t);
        }
    }

    inline Rate DiscountStructure::forwardImpl(Time t) const {
        // less than half day
        Time dt = 0.001;
        DiscountFactor df1 = discountImpl(t),
                       df2 = discountImpl(t+dt);
        return Rate(QL_LOG(df1/df2)/dt);
    }

    inline Rate DiscountStructure::compoundForwardImpl(Time t,
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
