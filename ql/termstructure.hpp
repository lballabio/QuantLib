
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file termstructure.hpp
    \brief Term structure

    \fullpath
    ql/%termstructure.hpp
*/

// $Id$

#ifndef quantlib_term_structure_hpp
#define quantlib_term_structure_hpp

#include <ql/calendar.hpp>
#include <ql/daycounter.hpp>
#include <ql/dataformatters.hpp>
#include <ql/marketelement.hpp>
#include <vector>

/*! \namespace QuantLib::TermStructures
    \brief Concrete implementations of the TermStructure interface

    See sect. \ref termstructures
*/

namespace QuantLib {

    //! Term structure
    /*! This abstract class defines the interface of concrete
        rate structures which will be derived from this one.

        Rates are assumed to be annual continuos compounding.

        \todo add derived class ParSwapTermStructure similar to
              ZeroYieldTermStructure, DiscountStructure, ForwardRateStructure

        \todo allow for different compounding rules and compounding
              frequencies
    */
    class TermStructure : public Patterns::Observable {
      public:
        virtual ~TermStructure() {}
        //! \name Rates and discount
        //@{
        //! zero yield rate at a given date
        Rate zeroYield(const Date&, bool extrapolate = false) const;
        //! zero yield rate at a given time from settlement
        Rate zeroYield(Time, bool extrapolate = false) const;
        //! discount factor at a given date
        DiscountFactor discount(const Date&, bool extrapolate = false) const;
        //! discount factor at a given time from settlement
        DiscountFactor discount(Time, bool extrapolate = false) const;
        //! instantaneous forward rate at a given date
        Rate instantaneousForward(const Date&, bool extrapolate = false) const;
        //! instantaneous forward rate at a given time from settlement
        Rate instantaneousForward(Time, bool extrapolate = false) const;
        //! discrete forward rate between two dates
        Rate forward(const Date&, const Date&, bool extrapolate = false) const;
        //! discrete forward rate between two times
        Rate forward(Time, Time, bool extrapolate = false) const;
        //@}

        //! \name Dates
        //@{
        //! returns today's date
        virtual Date todaysDate() const = 0;
        //! returns the settlement date
        virtual Date settlementDate() const = 0;
        //! returns the day counter
        virtual DayCounter dayCounter() const = 0;
        //! returns the latest date for which the curve can return rates
        virtual Date maxDate() const = 0;
        //! returns the latest date for which the curve can return rates
        virtual Time maxTime() const;
        //@}

      protected:
        //! implements the actual zero yield calculation in derived classes
        virtual Rate zeroYieldImpl(Time,
            bool extrapolate = false) const = 0;
        //! implements the actual discount calculation in derived classes
        virtual DiscountFactor discountImpl(Time,
            bool extrapolate = false) const = 0;
        //! implements the actual forward rate calculation in derived classes
        virtual Rate forwardImpl(Time,
            bool extrapolate = false) const = 0;
    };

    //! Zero yield term structure
    /*! This abstract class acts as an adapter to TermStructure allowing the
        programmer to implement only the
        <tt>zeroYieldImpl(Time, bool)</tt> method in derived classes.

        Rates are assumed to be annual continuos compounding.
    */
    class ZeroYieldStructure : public TermStructure {
      public:
        virtual ~ZeroYieldStructure() {}
      protected:
        /*! Returns the discount factor for the given date calculating it
            from the zero yield.
        */
        DiscountFactor discountImpl(Time, bool extrapolate = false) const;
        /*! Returns the instantaneous forward rate for the given date
            calculating it from the zero yield.
        */
        Rate forwardImpl(Time, bool extrapolate = false) const;
    };

    //! Discount factor term structure
    /*! This abstract class acts as an adapter to TermStructure allowing the
        programmer to implement only the
        <tt>discountImpl(const Date&, bool)</tt> method in derived classes.

        Rates are assumed to be annual continuos compounding.
    */
    class DiscountStructure : public TermStructure {
      public:
        virtual ~DiscountStructure() {}
      protected:
        /*! Returns the zero yield rate for the given date calculating it
            from the discount.
        */
        Rate zeroYieldImpl(Time, bool extrapolate = false) const;
        /*! Returns the instantaneous forward rate for the given date
            calculating it from the discount.
        */
        Rate forwardImpl(Time, bool extrapolate = false) const;
    };

    //! Forward rate term structure
    /*! This abstract class acts as an adapter to TermStructure allowing the
        programmer to implement only the
        <tt>forwardImpl(const Date&, bool)</tt> method in derived classes.

        Rates are assumed to be annual continuos compounding.
    */
    class ForwardRateStructure : public TermStructure {
      public:
        virtual ~ForwardRateStructure() {}
      protected:
        /*! Returns the zero yield rate for the given date calculating it
            from the instantaneous forward rate.
            \warning This is just a default, highly inefficient
                implementation. Derived classes should implement their own
                zeroYield method.
        */
        Rate zeroYieldImpl(Time, bool extrapolate = false) const;
        /*! Returns the discount factor for the given date calculating it
            from the instantaneous forward rate.
        */
        DiscountFactor discountImpl(Time, bool extrapolate = false) const;
    };




    // inline definitions

    inline Rate TermStructure::zeroYield(const Date& d,
        bool extrapolate) const {
            Time t = dayCounter().yearFraction(settlementDate(),d);
            return zeroYieldImpl(t,extrapolate);
    }

    inline Rate TermStructure::zeroYield(Time t, bool extrapolate) const {
        return zeroYieldImpl(t,extrapolate);
    }

    inline DiscountFactor TermStructure::discount(const Date& d,
        bool extrapolate) const {
            Time t = dayCounter().yearFraction(settlementDate(),d);
            return discountImpl(t,extrapolate);
    }

    inline DiscountFactor TermStructure::discount(Time t,
        bool extrapolate) const {
            return discountImpl(t,extrapolate);
    }

    inline Rate TermStructure::instantaneousForward(const Date& d,
        bool extrapolate) const {
            Time t = dayCounter().yearFraction(settlementDate(),d);
            return forwardImpl(t,extrapolate);
    }

    inline Rate TermStructure::instantaneousForward(Time t,
        bool extrapolate) const {
        return forwardImpl(t,extrapolate);
    }

    inline Rate TermStructure::forward(const Date& d1,
        const Date& d2, bool extrapolate) const {
            Time t1 = dayCounter().yearFraction(settlementDate(),d1);
            Time t2 = dayCounter().yearFraction(settlementDate(),d2);
            return QL_LOG( discountImpl(t1,extrapolate)/
                           discountImpl(t2,extrapolate)  ) / (t2-t1);
    }

    inline Rate TermStructure::forward(Time t1, Time t2,
        bool extrapolate) const {
            return QL_LOG( discountImpl(t1,extrapolate)/
                           discountImpl(t2,extrapolate)  ) / (t2-t1);
    }

    inline Time TermStructure::maxTime() const {
            return dayCounter().yearFraction(settlementDate(), maxDate());
    }

    // curve deriving discount and forward from zero yield

    inline DiscountFactor ZeroYieldStructure::discountImpl(Time t,
        bool extrapolate) const {
            Rate r = zeroYieldImpl(t, extrapolate);
            return DiscountFactor(QL_EXP(-r*t));
    }

    inline Rate ZeroYieldStructure::forwardImpl(Time t,
        bool extrapolate) const {
            // less than half day
            Time dt = 0.001;
            Rate r1 = zeroYieldImpl(t, extrapolate),
                 r2 = zeroYieldImpl(t+dt, true);
            return r2+t*(r2-r1)/dt;
    }


    // curve deriving zero yield and forward from discount

    inline Rate DiscountStructure::zeroYieldImpl(Time t,
        bool extrapolate) const {
            DiscountFactor df = discountImpl(t, extrapolate);
            return Rate(-QL_LOG(df)/t);
    }

    inline Rate DiscountStructure::forwardImpl(Time t,
        bool extrapolate) const {
            // less than half day
            Time dt = 0.001;
            DiscountFactor df1 = discountImpl(t, extrapolate),
                           df2 = discountImpl(t+dt, true);
            return Rate(QL_LOG(df1/df2)/dt);
    }


    // curve deriving zero yield and discount from forward

    inline Rate ForwardRateStructure::zeroYieldImpl(Time t,
        bool extrapolate) const {
            QL_REQUIRE(t >= 0.0,
                "negative time (" + DoubleFormatter::toString(t) +
                ") not allowed");
            if (t == 0.0)
                return forwardImpl(0.0);
            double sum = 0.5*forwardImpl(0.0);
            Size N = 1000;
            double dt = t/N;
            for (Time i=dt; i<t; i+=dt)
                sum += forwardImpl(i, extrapolate);
            sum += 0.5*forwardImpl(t, extrapolate);
            return Rate(sum*dt/t);
    }

    inline DiscountFactor ForwardRateStructure::discountImpl(Time t,
        bool extrapolate) const {
            Rate r = zeroYieldImpl(t, extrapolate);
            return DiscountFactor(QL_EXP(-r*t));
    }

}


#endif
