
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

/*! \file termstructure.hpp
    \brief Term structure
*/

#ifndef quantlib_yield_term_structure_hpp
#define quantlib_yield_term_structure_hpp

#include <ql/basetermstructure.hpp>
#include <ql/quote.hpp>
#include <ql/basicdataformatters.hpp>
#include <ql/Math/extrapolation.hpp>
#include <vector>

namespace QuantLib {

    //! Interest-rate term structure
    /*! This abstract class defines the interface of concrete
        rate structures which will be derived from this one.

        Rates are assumed to be annual continuous compounding.

        \todo add derived class ParSwapTermStructure similar to
              ZeroYieldTermStructure, DiscountStructure, ForwardRateStructure

        \todo allow for different compounding rules and compounding
              frequencies

        \ingroup yieldtermstructures

        \test observability against evaluation date changes is checked.
    */
    class YieldTermStructure : public BaseTermStructure,
                               public Extrapolator {
      public:
        /*! \name Constructors
            See the BaseTermStructure documentation for issues regarding
            constructors.
        */
        //@{
        #ifndef QL_DISABLE_DEPRECATED
        //! initialize with a fixed today's date and reference date
        /*! \deprecated use the constructor without today's date; set the
                        evaluation date through Settings::instance().
        */
        YieldTermStructure(const Date& todaysDate, const Date& referenceDate);
        #endif
        //! default constructor
        /*! \warning term structures initialized by means of this
                     constructor must manage their own reference date
                     by overriding the referenceDate() method.
        */
        YieldTermStructure();
        //! initialize with a fixed reference date
        YieldTermStructure(const Date& referenceDate);
        //! calculate the reference date based on the global evaluation date
        YieldTermStructure(Integer settlementDays, const Calendar&);
        //@}
        virtual ~YieldTermStructure() {}
        #ifndef QL_DISABLE_DEPRECATED
        //! the day counter used for date/time conversion
        virtual DayCounter dayCounter() const = 0;
        #endif
        /*! \name Rates and discount

            These methods are either function of dates or times.
            In the latter case, times are calculated as fraction
            of year from the reference date.
        */
        //@{
        //! zero-yield rate
        Rate zeroYield(const Date&, bool extrapolate = false) const;
        //! zero-yield rate
        Rate zeroYield(Time, bool extrapolate = false) const;
        //! discount factor
        DiscountFactor discount(const Date&, bool extrapolate = false) const;
        //! discount factor
        DiscountFactor discount(Time, bool extrapolate = false) const;
        //! instantaneous forward rate
        Rate instantaneousForward(const Date&, bool extrapolate = false) const;
        //! instantaneous forward rate
        Rate instantaneousForward(Time, bool extrapolate = false) const;
        //! instantaneous forward rate at a given compounding frequency
        Rate compoundForward(const Date&, Integer,
                             bool extrapolate = false) const;
        //! instantaneous forward rate at a given compounding frequency
        Rate compoundForward(Time, Integer, bool extrapolate = false) const;
        //! discrete forward rate between two dates
        Rate forward(const Date&, const Date&, bool extrapolate = false) const;
        //! discrete forward rate between two times
        Rate forward(Time, Time, bool extrapolate = false) const;
        //! zero-coupon rate
        Rate zeroCoupon(const Date&, Integer, bool extrapolate = false) const;
        //! zero-coupon rate
        Rate zeroCoupon(Time, Integer, bool extrapolate = false) const;
        //@}

        //! \name Dates
        //@{
        //! the latest date for which the curve can return rates
        virtual Date maxDate() const = 0;
        //! the latest time for which the curve can return rates
        virtual Time maxTime() const;
        //@}
      protected:
        /*! \name Calculations

            These methods must be implemented in derived classes to perform
            the actual discount and rate calculations. When they are called,
            range check has already been performed; therefore, they must
            assume that extrapolation is required.
        */
        //@{
        //! zero-yield calculation
        virtual Rate zeroYieldImpl(Time) const = 0;
        //! discount calculation
        virtual DiscountFactor discountImpl(Time) const = 0;
        //! instantaneous forward-rate calculation
        virtual Rate forwardImpl(Time) const = 0;
        //! compound forward-rate calculation
        virtual Rate compoundForwardImpl(Time, Integer) const = 0;
        //@}
      private:
        void checkRange(const Date&, bool extrapolate) const;
        void checkRange(Time, bool extrapolate) const;
    };

    #ifndef QL_DISABLE_DEPRECATED
    //! \deprecated renamed to YieldTermStructure
    typedef YieldTermStructure TermStructure;
    #endif


    //! Zero-yield term structure
    /*! This abstract class acts as an adapter to YieldTermStructure
        allowing the programmer to implement only the
        <tt>zeroYieldImpl(Time, bool)</tt> method in derived classes.

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
        /*! Returns the forward rate at a specified compound frequency
	    for the given date calculating it from the zero yield.
        */
        Rate compoundForwardImpl(Time, Integer) const;
        //@}
    };

    //! Discount factor term structure
    /*! This abstract class acts as an adapter to YieldTermStructure
        allowing the programmer to implement only the
        <tt>discountImpl(const Date&, bool)</tt> method in derived
        classes.

        Rates are assumed to be annual continuous compounding.

        \ingroup yieldtermstructures
    */
    class DiscountStructure : public YieldTermStructure {
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
        DiscountStructure(const Date& todaysDate, const Date& referenceDate);
        #endif
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
        Rate compoundForwardImpl(Time, Integer) const;
        //@}
    };

    //! Forward rate term structure
    /*! This abstract class acts as an adapter to TermStructure allowing the
        programmer to implement only the
        <tt>forwardImpl(const Date&, bool)</tt> method in derived classes.

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
    inline YieldTermStructure::YieldTermStructure(const Date& todaysDate,
                                                  const Date& referenceDate)
    : BaseTermStructure(todaysDate, referenceDate) {}
    #endif

    inline YieldTermStructure::YieldTermStructure() {}

    inline YieldTermStructure::YieldTermStructure(const Date& referenceDate)
    : BaseTermStructure(referenceDate) {}

    inline YieldTermStructure::YieldTermStructure(Integer settlementDays,
                                                  const Calendar& calendar)
    : BaseTermStructure(settlementDays, calendar) {}

    inline Rate YieldTermStructure::zeroYield(const Date& d,
                                              bool extrapolate) const {
        checkRange(d, extrapolate);
        return zeroYieldImpl(timeFromReference(d));
    }

    inline Rate YieldTermStructure::zeroYield(Time t, bool extrapolate) const {
        checkRange(t, extrapolate);
        return zeroYieldImpl(t);
    }

    inline DiscountFactor YieldTermStructure::discount(const Date& d,
                                                       bool extrapolate)
                                                                       const {
        checkRange(d, extrapolate);
        return discountImpl(timeFromReference(d));
    }

    inline DiscountFactor YieldTermStructure::discount(Time t,
                                                       bool extrapolate)
                                                                       const {
        checkRange(t, extrapolate);
        return discountImpl(t);
    }

    inline Rate YieldTermStructure::instantaneousForward(const Date& d,
                                                         bool extrapolate)
                                                                       const {
        checkRange(d, extrapolate);
        return forwardImpl(timeFromReference(d));
    }

    inline Rate YieldTermStructure::instantaneousForward(Time t,
                                                         bool extrapolate)
                                                                       const {
        checkRange(t, extrapolate);
        return forwardImpl(t);
    }

    inline Rate YieldTermStructure::compoundForward(const Date& d, Integer f,
                                                    bool extrapolate) const {
        checkRange(d, extrapolate);
        return compoundForwardImpl(timeFromReference(d),f);
    }

    inline Rate YieldTermStructure::compoundForward(Time t, Integer f,
                                                    bool extrapolate) const {
        checkRange(t, extrapolate);
        return compoundForwardImpl(t,f);
    }

    inline Rate YieldTermStructure::forward(const Date& d1, const Date& d2,
                                            bool extrapolate) const {
        QL_REQUIRE(d1 <= d2,
                   DateFormatter::toString(d1) +
                   " later than " +
                   DateFormatter::toString(d2));
        checkRange(d2, extrapolate);
        return forward(timeFromReference(d1), timeFromReference(d2));
    }

    inline Rate YieldTermStructure::forward(Time t1, Time t2,
                                            bool extrapolate) const {
        QL_REQUIRE(t1 <= t2,
                   DecimalFormatter::toString(t1) +
                   " later than " +
                   DecimalFormatter::toString(t2));
        checkRange(t2, extrapolate);
        if (t2==t1)
	        return instantaneousForward(t1);
        else
            return QL_LOG(discountImpl(t1)/discountImpl(t2))/(t2-t1);
    }

    inline Rate YieldTermStructure::zeroCoupon(const Date& d, Integer f,
                                               bool extrapolate) const {
        return zeroCoupon(timeFromReference(d),f,extrapolate);
    }

    inline Rate YieldTermStructure::zeroCoupon(Time t, Integer f,
                                               bool extrapolate) const {
        checkRange(t, extrapolate);
        DiscountFactor df = discountImpl(t);
        if (t == 0.0)
            return zeroYieldImpl(0.0); // limit for t -> 0
	    else if (f > 0 && t <= (1.0/f))
            return Rate((1.0/df-1.0)/t);
        else
            return Rate((QL_POW(1.0/df,1.0/(t*f))-1.0)*f);
    }

    inline Time YieldTermStructure::maxTime() const {
        return timeFromReference(maxDate());
    }

    inline void YieldTermStructure::checkRange(const Date& d,
                                               bool extrapolate) const {
        checkRange(timeFromReference(d),extrapolate);
    }

    inline void YieldTermStructure::checkRange(Time t,
                                               bool extrapolate) const {
        QL_REQUIRE(t >= 0.0,
                   "negative time (" +
                   DecimalFormatter::toString(t) +
                   ") given");
        QL_REQUIRE(extrapolate || allowsExtrapolation() || t <= maxTime(),
                   "time (" +
                   DecimalFormatter::toString(t) +
                   ") is past max curve time (" +
                   DecimalFormatter::toString(maxTime()) + ")");
    }


    // curve deriving discount and forward from zero yield

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

    inline Rate ZeroYieldStructure::compoundForwardImpl(Time t,
                                                        Integer f) const {
        Rate zy = zeroYieldImpl(t);
        if (f == 0)
            return zy;
        if (t <= 1.0/f)
            return (QL_EXP(zy*t)-1.0)/t;
        return (QL_EXP(zy*(1.0/f))-1.0)*f;
    }

    // curve deriving zero yield and forward from discount

    #ifndef QL_DISABLE_DEPRECATED
    inline DiscountStructure::DiscountStructure(const Date& todaysDate,
                                                const Date& referenceDate)
    : YieldTermStructure(todaysDate,referenceDate) {}
    #endif

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

    // curve deriving zero yield and discount from forward

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
