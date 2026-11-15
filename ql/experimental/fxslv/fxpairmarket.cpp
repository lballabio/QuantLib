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

#include <ql/experimental/fxslv/fxlocalvolsurface.hpp>
#include <ql/experimental/fxslv/fxpairmarket.hpp>
#include <ql/experimental/fxslv/fxsettlementforeigncurve.hpp>
#include <ql/experimental/fxslv/fxtradingtimevariancesurface.hpp>
#include <ql/experimental/fxslv/tradingtime/tradingtimedaycounter.hpp>
#include <ql/experimental/fxslv/tradingtime/tradingtimeyieldtermstructure.hpp>
#include <ql/pricingengines/blackdeltacalculator.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <cmath>
#include <utility>

namespace QuantLib {

    FxPairMarket::FxPairMarket(const Date& asOf,
                               FxPairConventions conventions,
                               Real spot,
                               Handle<YieldTermStructure> foreignCurve,
                               Handle<YieldTermStructure> domesticCurve,
                               FxVolQuotes quotes,
                               std::vector<FxVolEvent> events,
                               FxVolSurfaceConfig config)
    : FxPairMarket(Data{asOf, std::move(conventions), makeQuoteHandle(spot),
                        std::move(foreignCurve), std::move(domesticCurve), std::move(quotes),
                        std::move(events), config, {}}) {}

    FxPairMarket::FxPairMarket(Data data) : built_(ext::make_shared<Built>()) {
        const std::string name = data.conventions.name();
        QL_REQUIRE(data.asOf != Date(), name << ": no as-of date given");
        QL_REQUIRE(data.spot->value() > 0.0, name << ": non-positive spot " << data.spot->value());
        QL_REQUIRE(!data.foreignCurve.empty() && !data.domesticCurve.empty(),
                   name << ": both curves must be given");
        data.pillars = data.conventions.volPillars(data.asOf);
        data_ = ext::make_shared<const Data>(std::move(data));
    }

    std::pair<DiscountFactor, DiscountFactor>
    FxPairMarket::settlementDiscounts(const Date& expiry) const {
        return fxSettlementDiscounts(conventions().settlement(), asOf(), expiry, **foreignCurve(),
                                     **domesticCurve());
    }

    Real FxPairMarket::forward(const Date& expiry) const {
        const auto [dfor, ddom] = settlementDiscounts(expiry);
        return spot() * dfor / ddom;
    }

    const ext::shared_ptr<BlackVolTermStructure>& FxPairMarket::volSurface() const {
        if (!built_->surface) {
            built_->surface = makeFxVolSurface(asOf(), conventions(), spotQuote(), quotes(),
                                               foreignCurve(), domesticCurve(), events(), config());
            built_->fxSurface = dynamic_cast<const FxVolSurface*>(built_->surface.get());
            QL_ENSURE(built_->fxSurface, "the surface built is not an FX vol surface");
        }
        return built_->surface;
    }

    const FxVolSurface& FxPairMarket::fxVolSurface() const {
        volSurface();
        return *built_->fxSurface;
    }

    Volatility FxPairMarket::vol(const Date& expiry, Real strike) const {
        return volSurface()->blackVol(expiry, strike, true);
    }

    ext::shared_ptr<FxSmileSection>
    FxPairMarket::smileSection(const Date& expiry, const FxExpiryQuotes& quotes) const {
        return makeFxSmileSection(asOf(), conventions().withFlyType(FxRrBfQuotes::SmileStrangle),
                                  expiry, spotQuote(), quotes.atm, quotes.riskReversals,
                                  quotes.butterflies, foreignCurve(), domesticCurve(),
                                  config().model);
    }

    FxExpiryQuotes FxPairMarket::expiryQuotes(const Date& expiry) const {
        const auto [dfor, ddom] = settlementDiscounts(expiry);
        const Time t = Actual365Fixed().yearFraction(asOf(), expiry);
        const Real fwd = spot() * dfor / ddom;
        const auto& surface = volSurface();
        auto stdDev = [&](Real k) { return surface->blackVol(expiry, k, true) * std::sqrt(t); };
        auto calculator = [&](Option::Type type, Real k) {
            return BlackDeltaCalculator(type, conventions().deltaType(), spot(), ddom, dfor,
                                        stdDev(k));
        };
        // strikes at which the conventions hold at the smile's own vol, by
        // fixed-point iteration on the strike
        auto solve = [&](auto strikeAtVol) {
            Real k = fwd;
            for (Size n = 0; n < 200; ++n) {
                const Real next = strikeAtVol(k);
                if (std::fabs(next - k) < 1.0e-12 * fwd)
                    return next;
                k = next;
            }
            QL_FAIL("no strike found on the smile expiring " << expiry);
        };

        FxExpiryQuotes result;
        const Real kAtm = solve(
            [&](Real k) { return calculator(Option::Call, k).atmStrike(conventions().atmType()); });
        result.atm = stdDev(kAtm) / std::sqrt(t);
        for (Real d : conventions().deltas()) {
            const Real kc =
                solve([&](Real k) { return calculator(Option::Call, k).strikeFromDelta(d); });
            const Real kp =
                solve([&](Real k) { return calculator(Option::Put, k).strikeFromDelta(-d); });
            const Volatility call = stdDev(kc) / std::sqrt(t), put = stdDev(kp) / std::sqrt(t);
            result.riskReversals.push_back(call - put);
            result.butterflies.push_back((call + put) / 2.0 - result.atm);
        }
        return result;
    }

    void FxPairMarket::buildTradingTime() const {
        if (built_->tradingTimeBuilt)
            return;
        const auto& clock = fxVolSurface().tradingTimeStructure();
        built_->ttDomestic = Handle<YieldTermStructure>(
            ext::make_shared<TradingTimeYieldTermStructure>(domesticCurve(), clock));
        built_->ttForeign = Handle<YieldTermStructure>(
            ext::make_shared<TradingTimeYieldTermStructure>(foreignCurve(), clock));
        built_->ttSurface = Handle<BlackVolTermStructure>(
            ext::make_shared<FxTradingTimeVarianceSurface>(volSurface()));
        built_->tradingTimeBuilt = true;
    }

    const Handle<TradingTimeTermStructure>& FxPairMarket::tradingTime() const {
        return fxVolSurface().tradingTimeStructure();
    }

    DayCounter FxPairMarket::tradingTimeDayCounter() const {
        return TradingTimeDayCounter(tradingTime());
    }

    const Handle<YieldTermStructure>& FxPairMarket::tradingTimeDomesticCurve() const {
        buildTradingTime();
        return built_->ttDomestic;
    }

    const Handle<YieldTermStructure>& FxPairMarket::tradingTimeForeignCurve() const {
        buildTradingTime();
        return built_->ttForeign;
    }

    const Handle<BlackVolTermStructure>& FxPairMarket::tradingTimeVolSurface() const {
        buildTradingTime();
        return built_->ttSurface;
    }

    ext::shared_ptr<GeneralizedBlackScholesProcess> FxPairMarket::blackProcess() const {
        const Handle<YieldTermStructure> settlementForeign(
            ext::make_shared<FxSettlementForeignCurve>(
                tradingTimeForeignCurve(), tradingTimeDomesticCurve(), conventions().settlement()));
        return ext::make_shared<GeneralizedBlackScholesProcess>(
            spotQuote(), settlementForeign, tradingTimeDomesticCurve(), tradingTimeVolSurface());
    }

    ext::shared_ptr<GeneralizedBlackScholesProcess>
    FxPairMarket::localVolProcess(Volatility fallback) const {
        return makeFxLocalVolProcess(spotQuote(), tradingTimeForeignCurve(),
                                     tradingTimeDomesticCurve(), conventions().settlement(),
                                     tradingTimeVolSurface(), fallback);
    }

    FxPairMarket FxPairMarket::withSpot(Real spot) const {
        Data d = *data_;
        d.spot = makeQuoteHandle(spot);
        return FxPairMarket(std::move(d));
    }

    FxPairMarket FxPairMarket::withCurves(Handle<YieldTermStructure> foreignCurve,
                                          Handle<YieldTermStructure> domesticCurve) const {
        Data d = *data_;
        d.foreignCurve = std::move(foreignCurve);
        d.domesticCurve = std::move(domesticCurve);
        return FxPairMarket(std::move(d));
    }

    FxPairMarket FxPairMarket::withQuotes(FxVolQuotes quotes) const {
        Data d = *data_;
        d.quotes = std::move(quotes);
        return FxPairMarket(std::move(d));
    }

    FxPairMarket FxPairMarket::withEvents(std::vector<FxVolEvent> events) const {
        Data d = *data_;
        d.events = std::move(events);
        return FxPairMarket(std::move(d));
    }

    std::vector<Volatility> FxPairMarket::smilePoints(Size pillar) const {
        const std::vector<Real>& deltas = conventions().deltas();
        const Volatility atm = quotes().atms[pillar];
        std::vector<Volatility> points = {atm};
        if (conventions().flyType() == FxRrBfQuotes::SmileStrangle) {
            for (Size j = 0; j < deltas.size(); ++j) {
                const Volatility rr = quotes().riskReversals[pillar][j];
                const Volatility bf = quotes().butterflies[pillar][j];
                points.push_back(atm + bf + rr / 2.0);
                points.push_back(atm + bf - rr / 2.0);
            }
            return points;
        }
        // broker strangles are not points on the smile: read the points off
        // the pillar's calibrated smile
        const auto section = makeFxSmileSection(
            asOf(), conventions(), pillars()[pillar], spotQuote(), atm,
            quotes().riskReversals[pillar], quotes().butterflies[pillar], foreignCurve(),
            domesticCurve(), config().model);
        points.front() = section->atmVol();
        for (Real d : deltas) {
            points.push_back(section->volByDelta(d, Option::Call));
            points.push_back(section->volByDelta(-d, Option::Put));
        }
        return points;
    }

    FxVolQuotes
    FxPairMarket::quotesFromPoints(const std::vector<std::vector<Volatility>>& points) const {
        const Size nd = conventions().deltas().size();
        FxVolQuotes result;
        result.atms.resize(points.size());
        result.riskReversals.assign(points.size(), std::vector<Volatility>(nd));
        result.butterflies.assign(points.size(), std::vector<Volatility>(nd));
        for (Size i = 0; i < points.size(); ++i) {
            result.atms[i] = points[i][0];
            for (Size j = 0; j < nd; ++j) {
                const Volatility call = points[i][1 + 2 * j], put = points[i][2 + 2 * j];
                result.riskReversals[i][j] = call - put;
                result.butterflies[i][j] = (call + put) / 2.0 - result.atms[i];
            }
        }
        return result;
    }

    FxPairMarket FxPairMarket::smileStrangles() const {
        if (conventions().flyType() == FxRrBfQuotes::SmileStrangle)
            return *this;
        std::vector<std::vector<Volatility>> points;
        for (Size i = 0; i < pillars().size(); ++i)
            points.push_back(smilePoints(i));
        Data d = *data_;
        d.quotes = quotesFromPoints(points);
        d.conventions = conventions().withFlyType(FxRrBfQuotes::SmileStrangle);
        return FxPairMarket(std::move(d));
    }

    FxPairMarket FxPairMarket::rolled(const Date& next) const {
        QL_REQUIRE(next > asOf(), "the next date must follow the market's");
        const FxPairMarket market = smileStrangles();
        const FxPairConventions& conv = market.conventions();
        const std::vector<Date>& pillarsToday = market.pillars();
        const std::vector<Date> pillarsNext = conv.volPillars(next);
        const Calendar& cal = conv.settlement().calendar();
        const TradingTimeTermStructure baseToday(asOf(), cal, config().holidayWeight);
        const TradingTimeTermStructure baseNext(next, cal, config().holidayWeight);
        const Actual365Fixed dc;

        std::vector<std::vector<Volatility>> points;
        for (Size i = 0; i < pillarsToday.size(); ++i) {
            const Time t0 = dc.yearFraction(asOf(), pillarsToday[i]);
            const Time t1 = dc.yearFraction(next, pillarsNext[i]);
            const Time tau0 = baseToday.tradingTime(asOf(), pillarsToday[i]);
            const Time tau1 = baseNext.tradingTime(next, pillarsNext[i]);
            const Real events0 = fxEventVariance(events(), asOf(), pillarsToday[i], conv.cut());
            const Real events1 = fxEventVariance(events(), next, pillarsNext[i], conv.cut());

            std::vector<Volatility> p = market.smilePoints(i);
            for (auto& v : p) {
                const Real eventless = v * v * t0 - events0;
                QL_REQUIRE(eventless > 0.0, "events before " << pillarsToday[i]
                                                << " carry more variance than the smile");
                v = std::sqrt((eventless / tau0 * tau1 + events1) / t1);
            }
            points.push_back(p);
        }

        Data d = *market.data_;
        d.asOf = next;
        // the spot rolls along today's forward to the next spot date
        const Date spot0 = conv.settlement().valueDate(asOf());
        const Date spot1 = conv.settlement().valueDate(next);
        d.spot = makeQuoteHandle(
            spot() * (foreignCurve()->discount(spot1) / foreignCurve()->discount(spot0)) /
            (domesticCurve()->discount(spot1) / domesticCurve()->discount(spot0)));
        d.quotes = market.quotesFromPoints(points);
        return FxPairMarket(std::move(d));
    }

}
