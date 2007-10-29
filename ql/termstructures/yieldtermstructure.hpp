/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

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

/*! \file yieldtermstructure.hpp
    \brief Interest-rate term structure
*/

#ifndef quantlib_yield_term_structure_hpp
#define quantlib_yield_term_structure_hpp

#include <ql/termstructure.hpp>
#include <ql/interestrate.hpp>
#include <vector>

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
    class YieldTermStructure : public TermStructure {
      public:
        /*! \name Constructors
            See the TermStructure documentation for issues regarding
            constructors.
        */
        //@{
        //! default constructor
        /*! \warning term structures initialized by means of this
                     constructor must manage their own reference date
                     by overriding the referenceDate() method.
        */
        YieldTermStructure(const DayCounter& dc = DayCounter());
        //! initialize with a fixed reference date
        YieldTermStructure(const Date& referenceDate,
                           const Calendar& cal = Calendar(),
                           const DayCounter& dc = DayCounter());
        //! calculate the reference date based on the global evaluation date
        YieldTermStructure(Natural settlementDays,
                           const Calendar& cal,
                           const DayCounter& dc = DayCounter());
        //@}
        virtual ~YieldTermStructure() {}
        /*! \name zero-yield rates

            These methods return the implied zero-yield rate for a
            given date or time.  In the former case, the time is
            calculated as a fraction of year from the reference date.
        */
        //@{
        /*! The resulting interest rate has the required daycounting
            rule.
        */
        InterestRate zeroRate(const Date& d,
                              const DayCounter& resultDayCounter,
                              Compounding comp,
                              Frequency freq = Annual,
                              bool extrapolate = false) const;

        /*! The resulting interest rate has the same day-counting rule
            used by the term structure. The same rule should be used
            for calculating the passed time t.
        */
        InterestRate zeroRate(Time t,
                              Compounding comp,
                              Frequency freq = Annual,
                              bool extrapolate = false) const;
        //@}

        /*! \name discount factors

            These methods return the discount factor for a given date
            or time.  In the former case, the time is calculated as a
            fraction of year from the reference date.
        */
        //@{
        DiscountFactor discount(const Date&,
                                bool extrapolate = false) const;
        /*! The same day-counting rule used by the term structure
            should be used for calculating the passed time t.
        */
        DiscountFactor discount(Time,
                                bool extrapolate = false) const;
        //@}

        /*! \name forward rates

            These methods returns the implied forward interest rate
            between two dates or times.  In the former case, times are
            calculated as fractions of year from the reference date.
        */
        //@{
        /*! The resulting interest rate has the required day-counting
            rule.
        */
        InterestRate forwardRate(const Date& d1,
                                 const Date& d2,
                                 const DayCounter& resultDayCounter,
                                 Compounding comp,
                                 Frequency freq = Annual,
                                 bool extrapolate = false) const;
        /*! The resulting interest rate has the required day-counting
            rule.
            \warning dates are not adjusted for holidays
        */
        InterestRate forwardRate(const Date& d,
                                 const Period& p,
                                 const DayCounter& resultDayCounter,
                                 Compounding comp,
                                 Frequency freq = Annual,
                                 bool extrapolate = false) const;

        /*! The resulting interest rate has the same day-counting rule
            used by the term structure. The same rule should be used
            for the calculating the passed times t1 and t2.
        */
        InterestRate forwardRate(Time t1,
                                 Time t2,
                                 Compounding comp,
                                 Frequency freq = Annual,
                                 bool extrapolate = false) const;
        //@}

        /*! \name par rates

            These methods returns the implied par rate for a given
            sequence of payments at the given dates or times.  In the
            former case, times are calculated as fractions of year
            from the reference date.

            \warning though somewhat related to a swap rate, this
                     method is not to be used for the fair rate of a
                     real swap, since it does not take into account
                     all the market conventions' details. The correct
                     way to evaluate such rate is to instantiate a
                     SimpleSwap with the correct conventions, pass it
                     the term structure and call the swap's fairRate()
                     method.
        */
        //@{
        Rate parRate(Integer tenor,
                     const Date& startDate,
                     Frequency freq = Annual,
                     bool extrapolate = false) const;

        /*! the first date in the vector must equal the start date;
            the following dates must equal the payment dates.
        */
        Rate parRate(const std::vector<Date>& dates,
                     Frequency freq = Annual,
                     bool extrapolate = false) const;

        /*! the first time in the vector must equal the start time;
            the following times must equal the payment times.
        */
        Rate parRate(const std::vector<Time>& times,
                     Frequency freq = Annual,
                     bool extrapolate = false) const;

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
    };

    // inline definitions

    inline
    InterestRate YieldTermStructure::forwardRate(const Date& d,
                                                 const Period& p,
                                                 const DayCounter& dayCounter,
                                                 Compounding comp,
                                                 Frequency freq,
                                                 bool extrapolate) const {
        return forwardRate(d, d+p, dayCounter, comp, freq, extrapolate);
    }

    inline
    DiscountFactor YieldTermStructure::discount(const Date& d,
                                                bool extrapolate) const {
        return discount(timeFromReference(d), extrapolate);
    }

    inline
    DiscountFactor YieldTermStructure::discount(Time t,
                                                bool extrapolate) const {
        checkRange(t, extrapolate);
        return discountImpl(t);
    }

}

#endif
