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

#include <ql/experimental/fxslv/conventions/fxtenorexpiry.hpp>
#include <ql/experimental/fxslv/fxvariancesurface.hpp>
#include <ql/experimental/fxslv/fxvolsurfacebuilder.hpp>
#include <ql/quotes/simplequote.hpp>

namespace QuantLib {

    void checkFxVolQuotes(const FxVolQuotes& quotes, const std::string& pair) {
        const Size nt = quotes.tenors.size(), nd = quotes.deltas.size();
        QL_REQUIRE(nt > 0, pair << ": no vol tenors quoted");
        QL_REQUIRE(nd > 0, pair << ": no risk-reversal deltas quoted");
        for (Real d : quotes.deltas)
            QL_REQUIRE(d > 0.0 && d < 0.5, pair << ": delta " << d << " not in (0, 0.5)");
        QL_REQUIRE(quotes.atms.size() == nt && quotes.riskReversals.size() == nt &&
                       quotes.butterflies.size() == nt,
                   pair << ": quotes needed for each of the " << nt << " vol tenors");
        for (Size i = 0; i < nt; ++i)
            QL_REQUIRE(quotes.riskReversals[i].size() == nd && quotes.butterflies[i].size() == nd,
                       pair << ": tenor " << quotes.tenors[i]
                            << " needs a risk reversal and a butterfly for each of the " << nd
                            << " deltas");
    }

    namespace {

        std::vector<Handle<Quote>> quoteHandles(const std::vector<Volatility>& values) {
            std::vector<Handle<Quote>> handles;
            handles.reserve(values.size());
            for (Volatility v : values)
                handles.push_back(makeQuoteHandle(v));
            return handles;
        }

    }

    ext::shared_ptr<FxVarianceSurface>
    makeFxVolSurface(const Date& asOf,
                     const FxPairConventions& conventions,
                     const Handle<Quote>& spot,
                     const FxVolQuotes& quotes,
                     const Handle<YieldTermStructure>& foreignCurve,
                     const Handle<YieldTermStructure>& domesticCurve,
                     const std::vector<FxVolEvent>& events,
                     const FxVolSurfaceConfig& config) {
        checkFxVolQuotes(quotes, conventions.name());
        const std::vector<Date> pillars =
            fxExpiryDates(asOf, quotes.tenors, conventions.settlement());
        std::vector<std::vector<Handle<Quote>>> rrs, bfs;
        for (Size i = 0; i < pillars.size(); ++i) {
            rrs.push_back(quoteHandles(quotes.riskReversals[i]));
            bfs.push_back(quoteHandles(quotes.butterflies[i]));
        }
        const Calendar& calendar = conventions.settlement().calendar();
        RelinkableHandle<TradingTimeTermStructure> clock(
            ext::make_shared<TradingTimeTermStructure>(asOf, calendar, config.holidayWeight));

        ext::shared_ptr<FxVarianceSurface> surface;
        switch (config.interpolation) {
          case FxVolInterpolation::Ncp:
            surface = ext::make_shared<FxVarianceSurfaceNCP>(
                config.model, asOf, spot, pillars, quoteHandles(quotes.atms), rrs, bfs,
                quotes.deltas, foreignCurve, domesticCurve, clock, conventions.deltaType(),
                conventions.atmType(), conventions.flyType(), conventions.settlement(),
                conventions.cut(), calendar, Following, true);
            break;
          case FxVolInterpolation::Clark:
            surface = ext::make_shared<FxVarianceSurfaceClark>(
                config.model, asOf, spot, pillars, quoteHandles(quotes.atms), rrs, bfs,
                quotes.deltas, foreignCurve, domesticCurve, clock, conventions.deltaType(),
                conventions.atmType(), conventions.flyType(), conventions.settlement(),
                conventions.cut(), calendar, Following, true);
            break;
          default:
            QL_FAIL("unknown vol interpolation");
        }
        surface->enableExtrapolation();

        if (!fxPendingEvents(events, asOf).empty()) {
            // the events release their variance on the surface's ATM
            // pillars, i.e. the fitted smiles' ATMs, which need not
            // reprice the quotes
            std::vector<Volatility> fitted;
            for (const auto& d : pillars)
                fitted.push_back(surface->atmVol(d));
            clock.linkTo(makeFxEventTradingTime(asOf, pillars, fitted, events, conventions.cut(),
                                                calendar, config.holidayWeight));
        }
        return surface;
    }

}
