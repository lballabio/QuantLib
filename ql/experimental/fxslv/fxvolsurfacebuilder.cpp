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
#include <ql/experimental/fxslv/smilesections/fxsmilesectionbydelta.hpp>
#include <ql/experimental/fxslv/smilesections/fxsmilesectionbystrike.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>

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

        template <class Section, template <class> class Surface>
        ext::shared_ptr<BlackVolTermStructure>
        build(const Date& asOf,
              const FxPairConventions& conventions,
              const Handle<Quote>& spot,
              const FxVolQuotes& quotes,
              const Handle<YieldTermStructure>& foreignCurve,
              const Handle<YieldTermStructure>& domesticCurve,
              const std::vector<FxVolEvent>& events,
              Real holidayWeight) {
            const std::vector<Date> pillars =
                fxExpiryDates(asOf, quotes.tenors, conventions.settlement());
            std::vector<std::vector<Handle<Quote>>> rrs, bfs;
            for (Size i = 0; i < pillars.size(); ++i) {
                rrs.push_back(quoteHandles(quotes.riskReversals[i]));
                bfs.push_back(quoteHandles(quotes.butterflies[i]));
            }
            const Calendar& calendar = conventions.settlement().calendar();
            RelinkableHandle<TradingTimeTermStructure> clock(
                ext::make_shared<TradingTimeTermStructure>(asOf, calendar, holidayWeight));
            auto surface = ext::make_shared<Surface<Section>>(
                asOf, spot, pillars, quoteHandles(quotes.atms), rrs, bfs, quotes.deltas,
                foreignCurve, domesticCurve, clock, conventions.deltaType(), conventions.atmType(),
                conventions.flyType(), conventions.settlement(), conventions.cut(), calendar,
                Following, true);
            surface->enableExtrapolation();
            if (!fxPendingEvents(events, asOf).empty()) {
                // the events release their variance on the surface's ATM
                // pillars, i.e. the fitted smiles' ATMs, which need not
                // reprice the quotes
                std::vector<Volatility> fitted;
                for (const auto& d : pillars)
                    fitted.push_back(surface->atmVol(d));
                clock.linkTo(makeFxEventTradingTime(asOf, pillars, fitted, events,
                                                    conventions.cut(), calendar, holidayWeight));
            }
            return surface;
        }

        template <template <class> class Surface>
        ext::shared_ptr<BlackVolTermStructure>
        buildWithModel(FxSmileModel model,
                       const Date& asOf,
                       const FxPairConventions& conventions,
                       const Handle<Quote>& spot,
                       const FxVolQuotes& quotes,
                       const Handle<YieldTermStructure>& foreignCurve,
                       const Handle<YieldTermStructure>& domesticCurve,
                       const std::vector<FxVolEvent>& events,
                       Real holidayWeight) {
            switch (model) {
              case FxSmileModel::Sabr:
                return build<FxSabrSmileSection, Surface>(asOf, conventions, spot, quotes,
                                                          foreignCurve, domesticCurve, events,
                                                          holidayWeight);
              case FxSmileModel::Svi:
                return build<FxSviSmileSection, Surface>(asOf, conventions, spot, quotes,
                                                         foreignCurve, domesticCurve, events,
                                                         holidayWeight);
              case FxSmileModel::Polynomial:
                return build<FxPolynomialSmileSection, Surface>(asOf, conventions, spot, quotes,
                                                                foreignCurve, domesticCurve,
                                                                events, holidayWeight);
              case FxSmileModel::Quadratic:
                return build<FxQuadraticSmileSection, Surface>(asOf, conventions, spot, quotes,
                                                               foreignCurve, domesticCurve,
                                                               events, holidayWeight);
            }
            QL_FAIL("unknown smile model");
        }

        template <class Section>
        ext::shared_ptr<FxSmileSection>
        section(const Date& asOf,
                const FxPairConventions& conventions,
                const Date& expiry,
                const Handle<Quote>& spot,
                const ext::shared_ptr<FxSmileQuotes>& quotes,
                const Handle<YieldTermStructure>& foreignCurve,
                const Handle<YieldTermStructure>& domesticCurve) {
            return ext::make_shared<Section>(expiry, spot, quotes, foreignCurve, domesticCurve,
                                             conventions.deltaType(), conventions.atmType(),
                                             Actual365Fixed(), conventions.settlement(), asOf);
        }

    }

    ext::shared_ptr<BlackVolTermStructure>
    makeFxVolSurface(const Date& asOf,
                     const FxPairConventions& conventions,
                     const Handle<Quote>& spot,
                     const FxVolQuotes& quotes,
                     const Handle<YieldTermStructure>& foreignCurve,
                     const Handle<YieldTermStructure>& domesticCurve,
                     const std::vector<FxVolEvent>& events,
                     const FxVolSurfaceConfig& config) {
        checkFxVolQuotes(quotes, conventions.name());
        switch (config.interpolation) {
          case FxVolInterpolation::Ncp:
            return buildWithModel<fxVarianceSurfaceNCP>(config.model, asOf, conventions, spot,
                                                        quotes, foreignCurve, domesticCurve,
                                                        events, config.holidayWeight);
          case FxVolInterpolation::Clark:
            return buildWithModel<fxVarianceSurfaceClark>(config.model, asOf, conventions, spot,
                                                          quotes, foreignCurve, domesticCurve,
                                                          events, config.holidayWeight);
        }
        QL_FAIL("unknown vol interpolation");
    }

    ext::shared_ptr<FxSmileSection>
    makeFxSmileSection(const Date& asOf,
                       const FxPairConventions& conventions,
                       const Date& expiry,
                       const Handle<Quote>& spot,
                       const std::vector<Real>& deltas,
                       Volatility atm,
                       const std::vector<Volatility>& riskReversals,
                       const std::vector<Volatility>& butterflies,
                       const Handle<YieldTermStructure>& foreignCurve,
                       const Handle<YieldTermStructure>& domesticCurve,
                       FxSmileModel model) {
        const auto quotes = ext::make_shared<FxRrBfQuotes>(
            makeQuoteHandle(atm), quoteHandles(riskReversals), quoteHandles(butterflies),
            deltas, conventions.flyType());
        switch (model) {
          case FxSmileModel::Sabr:
            return section<FxSabrSmileSection>(asOf, conventions, expiry, spot, quotes,
                                               foreignCurve, domesticCurve);
          case FxSmileModel::Svi:
            return section<FxSviSmileSection>(asOf, conventions, expiry, spot, quotes,
                                              foreignCurve, domesticCurve);
          case FxSmileModel::Polynomial:
            return section<FxPolynomialSmileSection>(asOf, conventions, expiry, spot, quotes,
                                                     foreignCurve, domesticCurve);
          case FxSmileModel::Quadratic:
            return section<FxQuadraticSmileSection>(asOf, conventions, expiry, spot, quotes,
                                                    foreignCurve, domesticCurve);
        }
        QL_FAIL("unknown smile model");
    }

}
