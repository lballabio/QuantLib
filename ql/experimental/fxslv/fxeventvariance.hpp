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

#include <ql/experimental/fxslv/tradingtimetermstructure.hpp>
#include <ql/time/calendar.hpp>
#include <vector>

namespace QuantLib {

    //! An event releasing a fixed ATM total variance
    /*! The market assigns an event a variance of its own, which does not
        move with the vol around it: bumping the ATM vols, or rolling the
        surface, keeps it constant.  The variance is in total-variance
        units (vol squared times years) and the time of day, a fraction of
        the day in (0, 1), places the release within its date.

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

    //! ATM variance of the events released after the valuation date and before the expiry.
    /*! An event on the expiry date comes after the expiry, as the
        surface's trading time measures to the start of the expiry date.
    */
    Real fxEventVariance(const std::vector<FxVolEvent>& events,
                         const Date& valuationDate,
                         const Date& expiry);

    //! Trading time whose events release their variances on a surface with the given ATM pillars
    /*! The variance surface interpolates total variance linearly in
        trading time, so an event of weight \f$ W \f$ between pillars
        \f$ i-1 \f$ and \f$ i \f$ releases \f$ r_i W \f$ with \f$ r_i \f$
        the variance per unit of trading time there.  The ATM pillars fix
        \f$ r_i \f$ given the event variances:
        \f[
            r_i = \frac{w_i - w_{i-1} - \sum_{e \in (i-1, i]} v_e}{\tau_i - \tau_{i-1}},
        \f]
        with \f$ w_i \f$ the ATM total variance of pillar \f$ i \f$ and
        \f$ \tau \f$ the trading time without events, and each weight is
        \f$ W_e = v_e / r_i \f$.  Events after the last pillar use its
        rate.  The weights hold exactly at the ATM; other deltas release
        the variance in proportion to their own rates between the pillars.

        Only events pending at the end of the reference date are used.
        The holiday weight and calendar are those of the trading time.
    */
    ext::shared_ptr<TradingTimeTermStructure>
    makeFxEventTradingTime(const Date& referenceDate,
                           const std::vector<Date>& pillars,
                           const std::vector<Volatility>& atmVols,
                           const std::vector<FxVolEvent>& events,
                           const Calendar& calendar,
                           Real holidayWeight);

}

#endif
