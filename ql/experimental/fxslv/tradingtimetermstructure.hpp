/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Raghav Subbarao

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file tradingtimetermstructure.hpp
    \brief Trading-time term structure with down-weighted weekends
*/

#ifndef quantlib_trading_time_term_structure_hpp
#define quantlib_trading_time_term_structure_hpp

#include <ql/handle.hpp>
#include <ql/quote.hpp>
#include <ql/termstructure.hpp>
#include <ql/time/calendars/weekendsonly.hpp>
#include <ql/time/date.hpp>
#include <vector>

namespace QuantLib {

    //! Trading time: calendar time with slower weekends and holidays, and event jumps
    /*! Trading time runs at rate 1 on business days of the calendar and
        at rate \f$ w \f$ (the weekend weight, between 0 and 1) on
        weekends and holidays, so it is continuous and never decreases;
        jumps add trading time at an instant, of a size given by a quote
        in days.  With \f$ x \f$ the time in days since the reference date,
        \f[
            D(x) = \int_0^x r(s)\,ds + \sum_{0 < x_e \le x} J_e
        \f]
        in days, and trading time in years is \f$ D / 365 \f$.  Both
        tradingTime() forms evaluate this, so they agree.

        A jump happens at an explicit time of day, never by default at
        midnight; a jump at or before the reference instant has happened
        and does not count.

        Times are measured in Actual/365 (Fixed) from the reference date;
        dates before the reference date are not supported.
    */
    class tradingTimeTermStructure : public TermStructure {
      public:
        //! An event that adds trading time at an instant
        class Jump {
          public:
            //! At the given time of day, a fraction of the day in (0, 1), on a date without a time.
            Jump(const Date& date, Real timeOfDay, Handle<Quote> size);
            //! At the date's own time of day, which must not be midnight (high-resolution dates).
            Jump(const Date& dateTime, Handle<Quote> size);

            const Date& date() const { return date_; }
            Real timeOfDay() const { return timeOfDay_; }
            //! Size in days; it must not be negative when used.
            const Handle<Quote>& size() const { return size_; }

          private:
            Date date_;
            Real timeOfDay_;
            Handle<Quote> size_;
        };

        //! \name Constructors
        //@{
        //! Fixed reference date.
        tradingTimeTermStructure(const Date& referenceDate,
                                 Calendar calendar = WeekendsOnly(),
                                 Real weekendWeight = 0.0,
                                 std::vector<Jump> jumps = {});

        //! Reference date floating with the evaluation date.
        explicit tradingTimeTermStructure(Natural settlementDays = 0,
                                          Calendar calendar = WeekendsOnly(),
                                          Real weekendWeight = 0.0,
                                          std::vector<Jump> jumps = {});
        //@}

        //! \name Observer interface
        //@{
        void update() override;
        //@}

        //! \name TermStructure interface
        //@{
        Date maxDate() const override { return Date::maxDate(); }
        //@}

        //! \name Trading time
        //@{
        //! Trading time between two dates, in years.
        Time tradingTime(const Date& d1, const Date& d2) const;
        //! Trading time from the reference date to the given date, in years.
        Time tradingTime(const Date& d) const { return tradingTime(referenceDate(), d); }
        //! Trading time from the reference date to the given Actual/365 time, in years.
        Time tradingTime(Time t) const;
        //@}

        //! \name Inspectors
        //@{
        Real weekendWeight() const { return weekendWeight_; }
        //! Times of the jumps still to happen, in years from the reference date.
        std::vector<Time> jumpTimes() const;
        //@}

      private:
        // trading days from the start of the reference day to y days after it
        Real tradingDays(Real y) const;
        // trading days of the jumps in (x1, x2], in days since the reference instant
        Real jumpDays(Real x1, Real x2) const;
        // days from the start of the reference day to the given date, with its time of day
        Real daysFromReferenceDay(const Date& d) const;
        // rebuilds the computed state if the reference date has moved
        void checkReference() const;

        Real weekendWeight_;
        std::vector<Jump> jumps_;

        // computed from the reference date, rebuilt when it changes
        mutable Date latestReference_;
        mutable Real referenceTimeOfDay_ = 0.0;  // fraction of the reference day already passed
        mutable std::vector<Real> jumpDays_;     // jump times, in days since the reference instant
        mutable std::vector<Real> cumulative_;   // trading days before each day since the reference day
    };

}

#endif
