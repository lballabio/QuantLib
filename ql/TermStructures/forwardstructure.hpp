
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

/*! \file forwardstructure.hpp
    \brief Forward-based yield term structure
*/

#ifndef quantlib_forward_structure_hpp
#define quantlib_forward_structure_hpp

#include <ql/termstructure.hpp>

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
            See the BaseTermStructure documentation for issues regarding
            constructors.
        */
        //@{
        #ifndef QL_DISABLE_DEPRECATED
        /*! \deprecated use the constructor without today's date; set the
                        evaluation date through Settings::instance().
        */
        ForwardRateStructure(const Date& todaysDate,
                             const Date& referenceDate);
        #endif
        ForwardRateStructure();
        ForwardRateStructure(const Date& referenceDate);
        ForwardRateStructure(Integer settlementDays, const Calendar&);
        //@}
        virtual ~ForwardRateStructure() {}
      protected:
        //! \name YieldTermStructure implementation
        //@{
        /*! Returns the zero yield rate for the given date calculating it
            from the instantaneous forward rate.

            \warning This is just a default, highly inefficient
                     implementation. Derived classes should implement
                     their own zeroYield method.
        */
        Rate zeroYieldImpl(Time) const;
        /*! Returns the discount factor for the given date calculating it
            from the instantaneous forward rate.
        */
        DiscountFactor discountImpl(Time) const;
        /*! Returns the forward rate at a specified compound frequency
	    for the given date calculating it from the zero yield.
        */
        Rate compoundForwardImpl(Time, Integer) const;
        //@}
    };


    // inline definitions

    #ifndef QL_DISABLE_DEPRECATED
    inline ForwardRateStructure::ForwardRateStructure(
                                                    const Date& todaysDate,
                                                    const Date& referenceDate)
    : YieldTermStructure(todaysDate,referenceDate) {}
    #endif

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
        return DiscountFactor(QL_EXP(-r*t));
    }

    inline Rate ForwardRateStructure::compoundForwardImpl(Time t,
                                                          Integer f) const {
        Rate zy = zeroYieldImpl(t);
        if (f == 0)
            return zy;
        if (t <= 1.0/f)
            return (QL_EXP(zy*t)-1.0)/t;
        return (QL_EXP(zy*(1.0/f))-1.0)*f;
    }

}


#endif
