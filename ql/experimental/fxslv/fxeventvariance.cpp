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

#include <ql/errors.hpp>
#include <ql/experimental/fxslv/fxeventvariance.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <utility>

namespace QuantLib {

    namespace {

        // the event on the clock of the cut, New York time less the cut,
        // as a date and a time of day in (0, 1)
        std::pair<Date, Real> onCutClock(const FxVolEvent& e, const FxExpiryCut& cut) {
            const Real s = e.timeOfDay - cut.timeOfDay();
            if (s > FxExpiryCut::sameInstant)
                return {e.date, s};
            if (s < -FxExpiryCut::sameInstant)
                return {e.date - 1, s + 1.0};
            // at the cut: after the expiry on its date, so just after its start
            return {e.date, FxExpiryCut::sameInstant};
        }

    }

    std::vector<FxVolEvent> fxPendingEvents(const std::vector<FxVolEvent>& events,
                                            const Date& valuationDate) {
        std::vector<FxVolEvent> pending;
        for (const auto& e : events) {
            QL_REQUIRE(e.variance >= 0.0, "event on " << e.date << " has a negative variance");
            QL_REQUIRE(e.timeOfDay > 0.0 && e.timeOfDay < 1.0,
                       "event on " << e.date << " needs a time of day in (0, 1)");
            if (e.date > valuationDate)
                pending.push_back(e);
        }
        return pending;
    }

    bool fxEventPrecedesExpiry(const FxVolEvent& event,
                               const Date& expiry,
                               const FxExpiryCut& cut) {
        return cut.precedes(event.date, event.timeOfDay, expiry);
    }

    Real fxEventVariance(const std::vector<FxVolEvent>& events,
                         const Date& valuationDate,
                         const Date& expiry,
                         const FxExpiryCut& cut) {
        Real variance = 0.0;
        for (const auto& e : fxPendingEvents(events, valuationDate))
            if (fxEventPrecedesExpiry(e, expiry, cut))
                variance += e.variance;
        return variance;
    }

    ext::shared_ptr<TradingTimeTermStructure>
    makeFxEventTradingTime(const Date& referenceDate,
                           const std::vector<Date>& pillars,
                           const std::vector<Volatility>& atmVols,
                           const std::vector<FxVolEvent>& events,
                           const FxExpiryCut& cut,
                           const Calendar& calendar,
                           Real holidayWeight) {
        QL_REQUIRE(!pillars.empty(), "no pillars given");
        QL_REQUIRE(pillars.size() == atmVols.size(),
                   pillars.size() << " pillars but " << atmVols.size() << " ATM vols");
        QL_REQUIRE(pillars.front() > referenceDate, "pillars must follow the reference date");
        for (Size i = 1; i < pillars.size(); ++i)
            QL_REQUIRE(pillars[i] > pillars[i - 1], "pillars must be increasing");

        const std::vector<FxVolEvent> pending = fxPendingEvents(events, referenceDate);
        if (pending.empty())
            return ext::make_shared<TradingTimeTermStructure>(referenceDate, calendar,
                                                              holidayWeight);

        const TradingTimeTermStructure base(referenceDate, calendar, holidayWeight);
        const Actual365Fixed dc;

        // base variance rate in each pillar interval, net of its events
        const Size n = pillars.size();
        std::vector<Real> rates(n);
        Real wPrev = 0.0;
        Date datePrev = referenceDate;
        for (Size i = 0; i < n; ++i) {
            const Real w = atmVols[i] * atmVols[i] * dc.yearFraction(referenceDate, pillars[i]);
            Real eventVariance = 0.0;
            for (const auto& e : pending)
                if (!fxEventPrecedesExpiry(e, datePrev, cut) &&
                    fxEventPrecedesExpiry(e, pillars[i], cut))
                    eventVariance += e.variance;
            const Time dtau = base.tradingTime(datePrev, pillars[i]);
            QL_REQUIRE(dtau > 0.0,
                       "no trading time between " << datePrev << " and " << pillars[i]);
            rates[i] = (w - wPrev - eventVariance) / dtau;
            QL_REQUIRE(rates[i] > 0.0, "the events before " << pillars[i]
                                           << " carry more variance than the ATM vols allow");
            wPrev = w;
            datePrev = pillars[i];
        }

        std::vector<ext::shared_ptr<TradingTimeEvent>> ttEvents;
        for (const auto& e : pending) {
            if (e.variance == 0.0)
                continue;
            Size i = 0;
            while (i < n - 1 && !fxEventPrecedesExpiry(e, pillars[i], cut))
                ++i;
            // weights are in trading days
            const Real weight = e.variance / rates[i] * 365.0;
            const auto [date, timeOfDay] = onCutClock(e, cut);
            ttEvents.push_back(ext::make_shared<JumpTradingTimeEvent>(
                date, timeOfDay, Handle<Quote>(ext::make_shared<SimpleQuote>(weight))));
        }
        return ext::make_shared<TradingTimeTermStructure>(referenceDate, calendar, holidayWeight,
                                                          std::move(ttEvents));
    }

}
