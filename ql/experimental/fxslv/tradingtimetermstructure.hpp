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
    \brief Trading-time term structure with down-weighted holidays and events
*/

#ifndef quantlib_trading_time_term_structure_hpp
#define quantlib_trading_time_term_structure_hpp

#include <ql/handle.hpp>
#include <ql/quote.hpp>
#include <ql/shared_ptr.hpp>
#include <ql/termstructure.hpp>
#include <ql/time/calendars/weekendsonly.hpp>
#include <ql/time/date.hpp>
#include <utility>
#include <vector>

namespace QuantLib {

    //! An event adding trading time, released between its start and end
    /*! Instants are given as day numbers: the serial number of the date
        plus the fraction of the day passed.  The weight, in trading days,
        is a quote; released() gives the fraction of it released by an
        instant, from 0 at the start to 1 by the end.
    */
    class TradingTimeEvent {
      public:
        virtual ~TradingTimeEvent() = default;

        //! Instant the release starts, as a day number.
        Real start() const { return start_; }
        //! Instant the release ends, as a day number; equal to start() for a jump.
        Real end() const { return end_; }
        //! Weight, in trading days.
        const Handle<Quote>& weight() const { return weight_; }

        //! Fraction of the weight released by the given instant.
        virtual Real released(Real day) const = 0;
        //! Fraction of the weight released per day at the given instant; 0 for a jump.
        virtual Real releaseRate(Real day) const = 0;
        //! Whether the weight may be negative, e.g. to slow trading time down.
        virtual bool allowsNegativeWeight() const = 0;

      protected:
        TradingTimeEvent(Real start, Real end, Handle<Quote> weight);

      private:
        Real start_, end_;
        Handle<Quote> weight_;
    };

    //! An event adding trading time at an instant, e.g. a data release
    /*! The time of day must be explicit: a jump never happens at midnight
        by default.  The weight must not be negative.
    */
    class JumpTradingTimeEvent : public TradingTimeEvent {
      public:
        //! At the given time of day, a fraction of the day in (0, 1), on a date without a time.
        JumpTradingTimeEvent(const Date& date, Real timeOfDay, Handle<Quote> weight);
        //! At the date's own time of day, which must not be midnight (high-resolution dates).
        JumpTradingTimeEvent(const Date& dateTime, Handle<Quote> weight);

        Real released(Real day) const override { return day >= start() ? 1.0 : 0.0; }
        Real releaseRate(Real) const override { return 0.0; }
        bool allowsNegativeWeight() const override { return false; }
    };

    //! An event adding trading time evenly over an interval
    /*! The weight is released uniformly from the start to the end, e.g.
        through a press conference after a rate decision.  It may be
        negative, e.g. -0.5 over a business day for a half day, as long as
        trading time keeps running forwards.  Times of day are explicit; a
        time of 0 given explicitly is midnight, so whole days run from
        midnight to midnight.
    */
    class DiffuseTradingTimeEvent : public TradingTimeEvent {
      public:
        //! Between times of day, fractions of the day in [0, 1), on dates without a time.
        DiffuseTradingTimeEvent(const Date& startDate,
                                Real startTimeOfDay,
                                const Date& endDate,
                                Real endTimeOfDay,
                                Handle<Quote> weight);
        //! Between the dates' own times of day, neither at midnight (high-resolution dates).
        DiffuseTradingTimeEvent(const Date& startDateTime,
                                const Date& endDateTime,
                                Handle<Quote> weight);

        Real released(Real day) const override;
        Real releaseRate(Real day) const override;
        bool allowsNegativeWeight() const override { return true; }
    };

    //! Trading time: calendar time with slower holidays, and events
    /*! Trading time runs at rate 1 on business days of the calendar and
        at rate \f$ w \f$ (the holiday weight, between 0 and 1) on every
        other day, weekends included; events add (or, if diffuse, possibly
        remove) trading time as they release their weights.  With \f$ x \f$
        the time in days since the reference instant,
        \f[
            D(x) = \int_0^x r(s)\,ds
                 + \sum_e W_e \left[ F_e(x_0 + x) - F_e(x_0) \right]
        \f]
        in days, where \f$ F_e \f$ is the fraction of the weight
        \f$ W_e \f$ released, so that the part of an event released by the
        reference instant has happened and does not count.  Trading time
        in years is \f$ D / 365 \f$; both tradingTime() forms evaluate
        this, so they agree.  Trading time must never run backwards: this
        is checked whenever the event weights or the reference date change.

        The reference instant is the reference date, with its time of day
        if it carries one; events earlier on the reference day count only
        if the reference date carries the valuation time, which needs
        high-resolution dates.

        Times are measured in Actual/365 (Fixed) from the reference date;
        dates before the reference date are not supported.

        \warning Results are cached lazily in mutable state, so an instance
                 must not be used from several threads at once.
    */
    class TradingTimeTermStructure : public TermStructure {
      public:
        //! \name Constructors
        //@{
        //! Fixed reference date.
        TradingTimeTermStructure(const Date& referenceDate,
                                 Calendar calendar = WeekendsOnly(),
                                 Real holidayWeight = 0.0,
                                 std::vector<ext::shared_ptr<TradingTimeEvent>> events = {});

        //! Reference date floating with the evaluation date.
        explicit TradingTimeTermStructure(Calendar calendar = WeekendsOnly(),
                                          Real holidayWeight = 0.0,
                                          std::vector<ext::shared_ptr<TradingTimeEvent>> events = {});
        //@}

        //! \name Observer interface
        //@{
        void update() override;
        //@}

        //! \name TermStructure interface
        //@{
        Date maxDate() const override { return Date::maxDate(); }

        //! the date at which discount = 1.0 and/or variance = 0.0
        const Date& referenceDate() const override;
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
        //! Weight of non-business days, weekends included.
        Real holidayWeight() const { return holidayWeight_; }
        const std::vector<ext::shared_ptr<TradingTimeEvent>>& events() const { return events_; }
        //! Start and end of the events still releasing, in years from the reference date.
        /*! A jump has equal start and end; an event in progress starts at 0. */
        std::vector<std::pair<Time, Time>> eventIntervals() const;
        //@}

      private:
        // days from the reference instant to the given date, with its time of day
        Real daysFromReference(const Date& d) const;
        // eventless trading days, from the start of the reference day to y days after
        Real baseDaysSinceReferenceDay(Real y) const;
        // trading days from the reference instant to x days after it
        Real tradingDaysSinceReference(Real x) const;
        // base rate of trading time on the given day
        Real baseRate(const Date& day) const;
        // rebuilds the computed state if the reference date or the weights have changed
        void checkReference() const;
        // fails if trading time would run backwards after the reference instant
        void checkRateIsNonNegative() const;

        Real holidayWeight_;
        std::vector<ext::shared_ptr<TradingTimeEvent>> events_;

        // the evaluation date, for a floating reference date
        mutable Date floatingReference_;

        // computed from the reference date and the weights, rebuilt when either changes
        mutable bool upToDate_ = false;
        mutable Date latestReference_;                 // reference value cumulativeBaseDays_ is built for
        mutable Date::serial_type referenceDay_ = 0;   // serial number of the reference day
        mutable Real referenceDayFraction_ = 0.0;      // fraction of the reference day passed
        mutable Real referenceInstant_ = 0.0;          // the two above, as a day number
        mutable std::vector<Size> pendingEvents_;      // events still to be released at or after reference
        mutable std::vector<Real> weights_;            // their weights
        mutable std::vector<Real> releasedAtReference_;  // their fractions released by the reference
        // base trading days before each day since the reference day; extended
        // lazily, and reset only when the reference day changes
        mutable std::vector<Real> cumulativeBaseDays_;
    };

}

#endif
