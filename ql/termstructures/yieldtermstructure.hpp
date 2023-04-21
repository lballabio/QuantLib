/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2009 Ferdinando Ametrano
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
#include <ql/quote.hpp>
#include <vector>

namespace QuantLib {

    //! Interest-rate term structure
    /*! This abstract class defines the interface of concrete
        interest rate structures which will be derived from this one.

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
        explicit YieldTermStructure(const DayCounter& dc = DayCounter(),
                                    bool extrapolate = false);
        YieldTermStructure(const Date& referenceDate,
                           const Calendar& cal = Calendar(),
                           const DayCounter& dc = DayCounter(),
                           std::vector<Handle<Quote> > jumps = {},
                           const std::vector<Date>& jumpDates = {},
                           bool extrapolate = false);
        YieldTermStructure(Natural settlementDays,
                           const Calendar& cal,
                           const DayCounter& dc = DayCounter(),
                           std::vector<Handle<Quote> > jumps = {},
                           const std::vector<Date>& jumpDates = {},
                           bool extrapolate = false);
        //@}

        /*! \name Discount factors

            These methods return the discount factor from a given date or time
            to the reference date.  In the latter case, the time is calculated
            as a fraction of year from the reference date.
        */
        //@{
        DiscountFactor discount(const Date& d,
                                bool extrapolate = false) const;
        /*! The same day-counting rule used by the term structure
            should be used for calculating the passed time t.
        */
        DiscountFactor discount(Time t,
                                bool extrapolate = false) const;
        //@}

        /*! \name Zero-yield rates

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

        /*! \name Forward rates

            These methods returns the forward interest rate between two dates
            or times.  In the former case, times are calculated as fractions
            of year from the reference date.

            If both dates (times) are equal the instantaneous forward rate is
            returned.
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
            for calculating the passed times t1 and t2.
        */
        InterestRate forwardRate(Time t1,
                                 Time t2,
                                 Compounding comp,
                                 Frequency freq = Annual,
                                 bool extrapolate = false) const;
        //@}

        //! \name Jump inspectors
        //@{
        const std::vector<Date>& jumpDates() const;
        const std::vector<Time>& jumpTimes() const;
        //@}

        //! \name Observer interface
        //@{
        void update() override;
        //@}
      protected:
        /*! \name Calculations

            This method must be implemented in derived classes to
            perform the actual calculations. When it is called,
            range check has already been performed; therefore, it
            must assume that extrapolation is required.
        */
        //@{
        //! discount factor calculation
        virtual DiscountFactor discountImpl(Time) const = 0;
        //@}
      private:
        // methods
        void setJumps(const Date& referenceDate);
        // data members
        std::vector<Handle<Quote> > jumps_;
        std::vector<Date> jumpDates_;
        std::vector<Time> jumpTimes_;
        Size nJumps_ = 0;
        Date latestReference_;
    };

    // inline definitions

    inline
    DiscountFactor YieldTermStructure::discount(const Date& d,
                                                bool extrapolate) const {
        return discount(timeFromReference(d), extrapolate);
    }

    inline
    InterestRate YieldTermStructure::forwardRate(const Date& d,
                                                 const Period& p,
                                                 const DayCounter& dayCounter,
                                                 Compounding comp,
                                                 Frequency freq,
                                                 bool extrapolate) const {
        return forwardRate(d, d+p, dayCounter, comp, freq, extrapolate);
    }

    inline const std::vector<Date>& YieldTermStructure::jumpDates() const {
        return this->jumpDates_;
    }

    inline const std::vector<Time>& YieldTermStructure::jumpTimes() const {
        return this->jumpTimes_;
    }

}

#endif
