
/*
 Copyright (C) 2004 Ferdinando Ametrano
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
#include <ql/basicdataformatters.hpp>
#include <ql/interestrate.hpp>
#include <ql/relinkablehandle.hpp>
#include <ql/Math/extrapolation.hpp>

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
        YieldTermStructure(const Date& todaysDate,
                           const Date& referenceDate);
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
        YieldTermStructure(Integer settlementDays,
                           const Calendar&);
        //@}
        virtual ~YieldTermStructure() {}
        #ifndef QL_DISABLE_DEPRECATED
        //! the day counter used for date/time conversion
        virtual DayCounter dayCounter() const = 0;
        #endif
        /*! \name zero rates

            These methods are either function of dates or times.
            In the latter case, times are calculated as fraction
            of year from the reference date.
        */
        //@{
        #ifndef QL_DISABLE_DEPRECATED
        //! zero-yield rate
        /*! \deprecated use zeroRate(const Date& d, const DayCounter& dc,
                                     Continuous, Annual, bool extrapolate)
                        instead
        */
        Rate zeroYield(const Date&,
                       bool extrapolate = false) const;

        //! zero-yield rate
        /*! \deprecated use zeroRate(Time t, Continuous, Annual,
                                     bool extrapolate)
                        instead
        */
        Rate zeroYield(Time t,
                       bool extrapolate = false) const;

        //! zero-coupon rate
        /*! \deprecated use zeroRate(const Date&, const DayCounter&,
                                     Compounding, Frequency, bool) instead
        */
        Rate zeroCoupon(const Date&,
                        Integer,
                        bool extrapolate = false) const;

        //! zero-coupon rate
        /*! \deprecated use zeroRate(Time, Compounding, Frequency, bool)
                        instead
        */
        Rate zeroCoupon(Time,
                        Integer,
                        bool extrapolate = false) const;
        #endif

        //! zero-yield Rate
        /*! returns the implied zero-yield Rate for a given date.
            The resulting Rate has the required daycounting rule.
        */
        Rate zeroRate(const Date& d,
                      const DayCounter& resultDayCounter,
                      Compounding comp,
                      Frequency freq = Annual,
                      bool extrapolate = false) const;

        //! zero-yield Rate
        /*! returns the implied zero-yield Rate for a given time.
            The resulting Rate is calculated implicitly assuming
            the same daycounting rule used for the time t measure.
        */
        Rate zeroRate(Time t,
                      Compounding comp,
                      Frequency freq = Annual,
                      bool extrapolate = false) const;

        //! zero-yield InterestRate
        /*! returns the implied zero-yield InterestRate for a given date.
            The resulting InterestRate has the required daycounting rule.
        */
        InterestRate zeroInterestRate(const Date& d,
                                      const DayCounter& resultDayCounter,
                                      Compounding comp,
                                      Frequency freq = Annual,
                                      bool extrapolate = false) const;

        //! zero-yield InterestRate
        /*! returns the implied zero-yield InterestRate for a given time.
            The resulting InterestRate has the same daycounting rule that
            should have been used for the time measure:
            Settings::instance().dayCounter()
        */
        InterestRate zeroInterestRate(Time t,
                                      Compounding comp,
                                      Frequency freq = Annual,
                                      bool extrapolate = false) const;
        //@}

        /*! \name discount factors

            These methods are either function of dates or times.
            In the latter case, times are calculated as fraction
            of year from the reference date.
        */
        //@{
        //! discount factor
        DiscountFactor discount(const Date&,
                                bool extrapolate = false) const;
        //! discount factor
        DiscountFactor discount(Time,
                                bool extrapolate = false) const;
        //@}

        /*! \name forward rates

            These methods are either function of dates or times.
            In the latter case, times are calculated as fraction
            of year from the reference date.
        */
        //@{
        //! instantaneous forward rate
        Rate instantaneousForward(const Date&,
                                  bool extrapolate = false) const;
        //! instantaneous forward rate
        Rate instantaneousForward(Time,
                                  bool extrapolate = false) const;
        //! instantaneous forward rate at a given compounding frequency
        Rate compoundForward(const Date&,
                             Integer,
                             bool extrapolate = false) const;
        //! instantaneous forward rate at a given compounding frequency
        Rate compoundForward(Time,
                             Integer,
                             bool extrapolate = false) const;
        //! discrete forward rate between two dates
        Rate forward(const Date&,
                     const Date&,
                     bool extrapolate = false) const;
        //! discrete forward rate between two times
        Rate forward(Time,
                     Time,
                     bool extrapolate = false) const;

        //! forward Rate
        /*! returns the implied forward Rate between two dates
            The resulting Rate has the required daycounting rule.
        */
        Rate forwardRate(const Date& d1,
                         const Date& d2,
                         const DayCounter& resultDayCounter,
                         Compounding comp,
                         Frequency freq = Annual,
                         bool extrapolate = false) const;

        //! forward Rate
        /*! returns the implied forward Rate between two times
            The resulting Rate is calculated implicitly assuming
            the same daycounting rule used for the time measures.
        */
        Rate forwardRate(Time t1,
                         Time t2,
                         Compounding comp,
                         Frequency freq = Annual,
                         bool extrapolate = false) const;

        //! forward InterestRate
        /*! returns the implied forward InterestRate between two dates
            The resulting InterestRate has the required daycounting rule.
        */
        InterestRate forwardInterestRate(const Date& d1,
                                         const Date& d2,
                                         const DayCounter& resultDayCounter,
                                         Compounding comp,
                                         Frequency freq = Annual,
                                         bool extrapolate = false) const;

        //! forward InterestRate
        /*! returns the implied forward InterestRate between two times
            The resulting InterestRate has the same daycounting rule that
            should have been used for the time measure:
            Settings::instance().dayCounter()
        */
        InterestRate forwardInterestRate(Time t,
                                         Time t2,
                                         Compounding comp,
                                         Frequency freq = Annual,
                                         bool extrapolate = false) const;
        //@}

        //! \name Dates
        //@{
        //! the latest date for which the curve can return rates
        virtual Date maxDate() const = 0;

        //! the latest time for which the curve can return rates
        virtual Time maxTime() const { return timeFromReference(maxDate()); }
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
        void checkRange(const Date& d,
                        bool extrapolate) const {
            checkRange(timeFromReference(d),extrapolate);
        }
        void checkRange(Time, bool extrapolate) const;
    };

    #ifndef QL_DISABLE_DEPRECATED
    //! \deprecated renamed to YieldTermStructure
    typedef YieldTermStructure TermStructure;
    #endif



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


    // inline zero definitions

    inline Rate YieldTermStructure::zeroRate(const Date& d,
                                             const DayCounter& dayCounter,
                                             Compounding comp,
                                             Frequency freq,
                                             bool extrapolate) const {
        if(d==referenceDate()) {
            Time t = 0.0001;
            Real compound = 1.0/discount(t, extrapolate);
            return InterestRate::impliedRate(compound, t, comp, freq);
        }
        Real compound = 1.0/discount(d, extrapolate);
        return InterestRate::impliedRate(compound,
                                         referenceDate(), d, dayCounter,
                                         comp, freq);
    }

    inline Rate YieldTermStructure::zeroRate(Time t,
                                             Compounding comp,
                                             Frequency freq,
                                             bool extrapolate) const {

        if (t==0.0) t = 0.0001;
        Real compound = 1.0/discount(t, extrapolate);
        return InterestRate::impliedRate(compound, t, comp, freq);
    }

    inline InterestRate YieldTermStructure::zeroInterestRate(
                                                 const Date& d,
                                                 const DayCounter& dayCounter,
                                                 Compounding comp,
                                                 Frequency freq,
                                                 bool extrapolate) const {

        if(d==referenceDate()) {
            Time t = 0.0001;
            Real compound = 1.0/discount(t, extrapolate);
            return InterestRate::impliedInterestRate(compound, t, dayCounter,
                                                     comp, freq);
        }
        Real compound = 1.0/discount(d, extrapolate);
        return InterestRate::impliedInterestRate(compound,
                                                 referenceDate(), d,
                                                 dayCounter, comp, freq);
    }

    inline InterestRate YieldTermStructure::zeroInterestRate(
                                                     Time t,
                                                     Compounding comp,
                                                     Frequency freq,
                                                     bool extrapolate) const {

        Real r = zeroRate(t, comp, freq, extrapolate);
        return InterestRate(r, Settings::instance().dayCounter(), comp, freq);
    }



    // inline forward definitions

    inline Rate YieldTermStructure::forwardRate(const Date& d1,
                                                const Date& d2,
                                                const DayCounter& dayCounter,
                                                Compounding comp,
                                                Frequency freq,
                                                bool extrapolate) const {

        if (d1==d2) {
            Time t1 = timeFromReference(d1);
            Time t2 = t1 + 0.0001;
            Real compound = discount(t1, extrapolate)/discount(t2, extrapolate);
            return InterestRate::impliedRate(compound, t2-t1, comp, freq);
        }
        Real compound = discount(d1, extrapolate)/discount(d2, extrapolate);
        return InterestRate::impliedRate(compound,
                                         d1, d2, dayCounter,
                                         comp, freq);
    }

    inline Rate YieldTermStructure::forwardRate(Time t1,
                                                Time t2,
                                                Compounding comp,
                                                Frequency freq,
                                                bool extrapolate) const {

        if (t2==t1) t2=t1+0.0001;
        QL_REQUIRE(t2>t1, "t2(" + DecimalFormatter::toString(t2) +
                          ")<t1(" + DecimalFormatter::toString(t2) +
                          ")");
        Real compound = discount(t1, extrapolate)/discount(t2, extrapolate);
        return InterestRate::impliedRate(compound, t2-t1, comp, freq);
    }

    inline InterestRate YieldTermStructure::forwardInterestRate(
                                                 const Date& d1,
                                                 const Date& d2,
                                                 const DayCounter& dayCounter,
                                                 Compounding comp,
                                                 Frequency freq,
                                                 bool extrapolate) const {

        if (d1==d2) {
            Time t1 = timeFromReference(d1);
            Time t2 = t1 + 0.0001;
            Real compound = discount(t1, extrapolate)/discount(t2, extrapolate);
            return InterestRate::impliedInterestRate(compound, t2-t1,
                                                     dayCounter, comp, freq);
        }
        Real compound = discount(d1, extrapolate)/discount(d2, extrapolate);
        return InterestRate::impliedInterestRate(compound,
                                                 d1, d2, dayCounter,
                                                 comp, freq);
    }

    inline InterestRate YieldTermStructure::forwardInterestRate(
                                                     Time t1,
                                                     Time t2,
                                                     Compounding comp,
                                                     Frequency freq,
                                                     bool extrapolate) const {

        if (t2==t1) t2=t1+0.0001;
        QL_REQUIRE(t2>t1, "t2(" + DecimalFormatter::toString(t2) +
                          ")<t1(" + DecimalFormatter::toString(t2) +
                          ")");
        Real r = forwardRate(t1, t2, comp, freq, extrapolate);
        return InterestRate(r, Settings::instance().dayCounter(), comp, freq);
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





    // inline discount definitions

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


    // inline deprecated (or to be deprecated) definitions



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

    #ifndef QL_DISABLE_DEPRECATED
    inline Rate YieldTermStructure::zeroYield(const Date& d,
                                              bool extrapolate) const {
        checkRange(d, extrapolate);
        return zeroYieldImpl(timeFromReference(d));
    }

    inline Rate YieldTermStructure::zeroYield(Time t, bool extrapolate) const {
        checkRange(t, extrapolate);
        return zeroYieldImpl(t);
    }

    inline Rate YieldTermStructure::zeroCoupon(const Date& d, Integer f,
                                               bool extrapolate) const {
        // return zeroCoupon(timeFromReference(d),f,extrapolate);
       return zeroRate(timeFromReference(d), SimpleThenCompounded,
           Frequency(f), extrapolate);
    }

    inline Rate YieldTermStructure::zeroCoupon(Time t, Integer f,
                                               bool extrapolate) const {
       return zeroRate(t, SimpleThenCompounded, Frequency(f), extrapolate);
    }
    #endif


}


#endif
