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

/*! \file fxeventvariance.hpp
    \brief FX vol events carrying a fixed variance, and their trading time.
*/

#ifndef quantlib_fx_event_variance_hpp
#define quantlib_fx_event_variance_hpp

#include <ql/experimental/fxslv/fxexpirycut.hpp>
#include <ql/experimental/fxslv/tradingtimetermstructure.hpp>
#include <ql/time/calendar.hpp>
#include <vector>

namespace QuantLib {

    //! An event releasing a fixed ATM total variance
    /*! The market assigns an event a variance of its own, which does not
        move with the vol around it: bumping the ATM vols, or rolling the
        surface, keeps it constant.  The variance is in total-variance
        units (vol squared times years) and the time of day, a fraction of
        the day in (0, 1) on the New York clock (see FxExpiryCut), places
        the release within its date: an option expiring on the event's
        date sees the event only if it comes before the option's cut.

        Valuations are taken at the end of the day: an event is released
        by every valuation date on or after its date.
    */
    struct FxVolEvent {
        Date date;
        Real timeOfDay = 0.5;
        Real variance;
    };

    //! Events not yet released at the end of the valuation date.
    std::vector<FxVolEvent> fxPendingEvents(const std::vector<FxVolEvent>& events,
                                            const Date& valuationDate);

    //! Whether an event comes before the cut on the expiry date, i.e. the option sees it.
    bool fxEventPrecedesExpiry(const FxVolEvent& event,
                               const Date& expiry,
                               const FxExpiryCut& cut);

    //! ATM variance of the events released after the valuation date and before the expiry cut.
    Real fxEventVariance(const std::vector<FxVolEvent>& events,
                         const Date& valuationDate,
                         const Date& expiry,
                         const FxExpiryCut& cut);

    //! Trading time whose events release their variances on a surface with the given ATM pillars
    /*! The variance surface interpolates total variance linearly in
        trading time, so an event of weight \f$ W \f$ between pillars
        \f$ i-1 \f$ and \f$ i \f$ releases \f$ r_i W \f$ with \f$ r_i \f$
        the variance per unit of trading time there.  The ATM pillars fix
        \f$ r_i \f$ given the event variances:
        \f[
            r_i = \frac{w_i - w_{i-1} - \sum_{e \in (i-1, i]} v_e}{\tau_i - \tau_{i-1}},
        \f]
        where an event belongs to the first pillar whose cut it precedes.
        with \f$ w_i \f$ the ATM total variance of pillar \f$ i \f$ and
        \f$ \tau \f$ the trading time without events, and each weight is
        \f$ W_e = v_e / r_i \f$.  Events after the last pillar use its
        rate.  The weights hold exactly at the ATM; other deltas release
        the variance in proportion to their own rates between the pillars.

        Only events pending at the end of the reference date are used.
        The holiday weight and calendar are those of the trading time.

        The trading time measures an expiry to the start of its date, so
        its events are placed on the clock of the cut, New York time less
        the cut's time of day: an event at time of day \f$ s \f$ on
        date \f$ D \f$ is released at \f$ D + s - c \f$ for a cut at
        \f$ c \f$, and an expiry on \f$ D \f$ sees it exactly when it
        precedes the cut.  An event at the cut itself is released just
        after the start of its date.  The base trading time does not move:
        days still run from midnight to midnight.
    */
    ext::shared_ptr<TradingTimeTermStructure>
    makeFxEventTradingTime(const Date& referenceDate,
                           const std::vector<Date>& pillars,
                           const std::vector<Volatility>& atmVols,
                           const std::vector<FxVolEvent>& events,
                           const FxExpiryCut& cut,
                           const Calendar& calendar,
                           Real holidayWeight);

}

#endif
