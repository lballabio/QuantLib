
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
        /*! \name zero rates

            These methods are either function of dates or times.
            In the latter case, times are calculated as fraction
            of year from the reference date.
        */
        //@{
        //! zero-yield rate
        /*! returns the implied zero-yield rate for a given date.
            The resulting InterestRate has the required daycounting rule.
        */
        InterestRate zeroRate(const Date& d,
                              const DayCounter& resultDayCounter,
                              Compounding comp,
                              Frequency freq = Annual,
                              bool extrapolate = false) const;

        //! zero-yield rate
        /*! returns the implied zero-yield rate for a given
            time.  The resulting InterestRate has the same
            day-counting rule used by the term structure. The same
            rule should be used for the calculating the time t.
        */
        InterestRate zeroRate(Time t,
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

        //! forward interest rate
        /*! returns the implied forward interest rate between two
            dates The resulting interest rate has the required
            day-counting rule.
        */
        InterestRate forwardRate(const Date& d1,
                                 const Date& d2,
                                 const DayCounter& resultDayCounter,
                                 Compounding comp,
                                 Frequency freq = Annual,
                                 bool extrapolate = false) const;

        //! forward interest rate
        /*! returns the implied forward interest rate between two
            times The resulting interest rate has the same
            day-counting rule used by the term structure. The same
            rule should be used for the calculating the time t.
        */
        InterestRate forwardRate(Time t1,
                                 Time t2,
                                 Compounding comp,
                                 Frequency freq = Annual,
                                 bool extrapolate = false) const;
        //@}

        /*! \name par rates

            These methods are either function of dates or times.
            In the latter case, times are calculated as fraction
            of year from the reference date.
        */
        //@{
        //! par rate
        /*! returns the implied par rate of a stylised swap starting at the
            effective date with a given tenor.

            \warning this par rate is not to be used for evaluation of a real
                     swap, since it does not take into account all the market
                     conventions' details.
        */
        Rate parRate(Year tenor,
                     const Date& effectiveDate,
                     Frequency freq = Annual,
                     bool extrapolate = false) const;

        //! par rate
        /*! returns the implied par rate of a stylised swap starting at the
            given time with a given tenor.

            \warning this par rate is not to be used for evaluation of a real
                     swap, since it does not take into account all the market
                     conventions' details.
        */
        Rate parRate(Year tenor,
                     Time t0,
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
        //! discount calculation
        virtual DiscountFactor discountImpl(Time) const = 0;
        //@}
      private:
        void checkRange(const Date& d,
                        bool extrapolate) const {
            checkRange(timeFromReference(d),extrapolate);
        }
        void checkRange(Time, bool extrapolate) const;
    };


    // inline definitions

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

    inline InterestRate YieldTermStructure::zeroRate(
                                                 const Date& d,
                                                 const DayCounter& dayCounter,
                                                 Compounding comp,
                                                 Frequency freq,
                                                 bool extrapolate) const {
        if(d==referenceDate()) {
            Time t = 0.0001;
            Real compound = 1.0/discount(t, extrapolate);
            return InterestRate::impliedRate(compound, t, dayCounter,
                                             comp, freq);
        }
        Real compound = 1.0/discount(d, extrapolate);
        return InterestRate::impliedRate(compound, referenceDate(), d,
                                         dayCounter, comp, freq);
    }

    inline InterestRate YieldTermStructure::zeroRate(Time t,
                                                     Compounding comp,
                                                     Frequency freq,
                                                     bool extrapolate) const {
        if (t==0.0) t = 0.0001;
        Real compound = 1.0/discount(t, extrapolate);
        return InterestRate::impliedRate(compound, t, dayCounter(),
                                         comp, freq);
    }


    // inline forward definitions

    inline InterestRate YieldTermStructure::forwardRate(
                                                const Date& d1,
                                                const Date& d2,
                                                const DayCounter& dayCounter,
                                                Compounding comp,
                                                Frequency freq,
                                                bool extrapolate) const {
        if (d1==d2) {
            Time t1 = timeFromReference(d1);
            Time t2 = t1 + 0.0001;
            Real compound =
                discount(t1, extrapolate)/discount(t2, extrapolate);
            return InterestRate::impliedRate(compound, t2-t1,
                                             dayCounter, comp, freq);
        }
        QL_REQUIRE(d1 < d2,
                   DateFormatter::toString(d1) +
                   " later than " +
                   DateFormatter::toString(d2));
        Real compound = discount(d1, extrapolate)/discount(d2, extrapolate);
        return InterestRate::impliedRate(compound,
                                         d1, d2, dayCounter,
                                         comp, freq);
    }

    inline InterestRate YieldTermStructure::forwardRate(
                                                   Time t1,
                                                   Time t2,
                                                   Compounding comp,
                                                   Frequency freq,
                                                   bool extrapolate) const {
        if (t2==t1) t2=t1+0.0001;
        QL_REQUIRE(t2>t1, "t2(" + DecimalFormatter::toString(t2) +
                          ")<t1(" + DecimalFormatter::toString(t2) +
                          ")");
        Real compound = discount(t1, extrapolate)/discount(t2, extrapolate);
        return InterestRate::impliedRate(compound, t2-t1,
                                         dayCounter(), comp, freq);
    }


    // inline par rate definitions

    inline Rate YieldTermStructure::parRate(Year tenor,
                                            const Date& effectiveDate,
                                            Frequency freq,
                                            bool extrap) const {
        return parRate(tenor, timeFromReference(effectiveDate), freq, extrap);
    }

    inline Rate YieldTermStructure::parRate(Year tenor, Time t0,
                                            Frequency freq,
                                            bool extrapolate) const {
        checkRange(t0+tenor, extrapolate);
        Real sum = 0.0;
        for (Year i=1; i<=tenor; i++)
            sum += discountImpl(t0+i);
        Real result = discountImpl(t0)-discountImpl(t0+tenor);
        result *=  Real(freq)/sum;
        return result;
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

}


#endif
