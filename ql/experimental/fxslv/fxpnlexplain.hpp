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

/*! \file fxpnlexplain.hpp
    \brief Day-on-day P&L explain of an FX vanilla option on an FX variance surface.
*/

#ifndef quantlib_fx_pnl_explain_hpp
#define quantlib_fx_pnl_explain_hpp

#include <ql/experimental/fxslv/fxeventvariance.hpp>
#include <ql/experimental/fxslv/fxtenorexpiry.hpp>
#include <ql/experimental/fxslv/fxvariancesurface.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/pricingengines/blackdeltacalculator.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <cmath>
#include <utility>
#include <vector>

namespace QuantLib {

    //! Quoting conventions of an FX vol surface
    struct FxVolConventions {
        std::vector<Real> deltas;  //!< risk-reversal and butterfly deltas, e.g. {0.25, 0.10}
        DeltaVolQuote::DeltaType deltaType;
        DeltaVolQuote::AtmType atmType;
        FxRrBfQuotes::FlyType flyType;  //!< of the butterflies given in the markets
        FxSettleConvention settlement;
        Real holidayWeight = 0.0;  //!< trading-time weight of non-business days
    };

    //! The FX vol market on a date: spot, curves, quotes by tenor and events
    /*! The quotes are given by tenor, in the order of the tenors of the
        explain, and the risk reversals and butterflies by delta, in the
        order of the conventions' deltas.  Events carry their own, fixed
        variance (see FxVolEvent); the ATM quotes include it.
    */
    struct FxVolMarket {
        Date date;
        Real spot;
        Handle<YieldTermStructure> foreignCurve, domesticCurve;
        std::vector<Volatility> atms;
        std::vector<std::vector<Volatility>> riskReversals, butterflies;
        std::vector<FxVolEvent> events;
    };

    //! How the P&L of a move in the forward is split between delta, gamma and vega
    /*! The total is the same for all three; only the breakdown changes.

        - Black: Black delta and gamma at the option's vol; the vol change
          from the smile moving with the forward (sticky delta) is reported
          as spot-smile vega and vanna.  Model independent.
        - StickyDelta: delta and gamma include the smile moving with the
          forward, \f$ \Delta + \nu \sigma_F \f$ and
          \f$ \Gamma + 2 \mbox{vanna}\, \sigma_F + \mbox{volga}\, \sigma_F^2 + \nu \sigma_{FF} \f$,
          with \f$ \sigma_F, \sigma_{FF} \f$ the derivatives of the option's
          vol in the forward with the quotes held in delta.  The surface
          gives these, whatever the smile model, so this is model
          independent too.
        - Bartlett: as StickyDelta, but the ATM vols also move with the
          forward as the smile model expects
          (FxSmileSection::atmVolForwardSensitivity()), so the delta
          includes the correlation of vol and spot that a delta-hedged
          book is hedged for, and the vega terms explain only the
          unexpected vol move.  Model dependent.
    */
    enum class FxPnlExplainType { Black, StickyDelta, Bartlett };

    //! Day-on-day P&L explain of an FX vanilla option, in domestic currency
    /*! The change in value from one date to the next is split by moving
        through the states

        - today's market;
        - the rolled market: the next date with the spot rolled along the
          forward, event variances held, and the eventless vol of each
          tenor held (see FxVanillaPnlExplain::rolled());
        - the rolled quotes with the next date's spot and curves;
        - then the next date's ATMs (and events), then its risk reversals,
          then its butterflies, which is the next date's market.

        Each step changes the option's implied vol \f$ \sigma \f$ at its
        own strike and expiry.  The P&L terms are a Taylor expansion with
        Black greeks taken at today's market, where \f$ dF \f$ is the
        change in the option's forward from the rolled to the next market:

        - theta: revaluation over the date change with today's vol, the
          spot on the forward;
        - vol roll: vega times the change in vol from today to the rolled
          market, split by replacing the ATM, then the risk reversals,
          then the butterflies of the option's fixed-expiry smile; the
          part not explained by the three (the smile re-anchored to the
          next date at unchanged quotes, and the fit of the fixed-expiry
          smile) is reported separately;
        - delta and gamma: \f$ \Delta\, dF \f$ and \f$ \frac{1}{2} \Gamma\, dF^2 \f$,
          with Black or smile-adjusted greeks (see FxPnlExplainType);
        - spot smile (Black type only): vega and vanna of the vol change
          from moving the spot and curves with the quotes held in delta;
          for the other types this move is in the delta and gamma, and
          for the Bartlett type the ATM vols of this step also move by
          their expected change given the forward's;
        - vega and vanna for each of the ATM, risk-reversal and butterfly
          steps: \f$ \nu\, d\sigma_k \f$ and
          \f$ \partial^2 V / \partial F \partial \sigma \, dF\, d\sigma_k \f$;
        - volga: \f$ \frac{1}{2} \partial^2 V / \partial \sigma^2 \, d\sigma^2 \f$
          with \f$ d\sigma \f$ the change from the rolled market (Black
          type), or from the market after the spot step (the other types,
          whose gamma includes the volga of that step), to the next market,
          so that the roll is excluded;
        - residual: the rest of the change in value.

        Values are present values at the spot date of each valuation date.
        Butterflies quoted as broker (market) strangles are converted to
        smile strangles on each pillar's calibrated smile, so that quotes
        from different dates can be combined.
    */
    struct FxPnlExplainResult {
        FxPnlExplainType type;
        Real total, theta;
        Real volRollAtm, volRollRiskReversal, volRollButterfly, volRollOther;
        Real delta, gamma;
        Real spotSmileVega, spotSmileVanna;
        Real vegaAtm, vegaRiskReversal, vegaButterfly;
        Real vannaAtm, vannaRiskReversal, vannaButterfly;
        Real volga, residual;

        //! \name The option's forward and implied vol in today's, the rolled and the next market
        //@{
        Real forwardToday, forwardRolled, forwardNext;
        Volatility volToday, volRolled, volNext;
        //@}
    };

    template <class Section, template <class> class Surface = fxVarianceSurfaceNCP>
    class FxVanillaPnlExplain {
      public:
        FxVanillaPnlExplain(std::vector<Period> tenors, FxVolConventions conventions);

        //! P&L explain of a European option held from today's market to the next.
        FxPnlExplainResult explain(Option::Type type,
                                   Real strike,
                                   const Date& expiry,
                                   Real notional,
                                   const FxVolMarket& today,
                                   const FxVolMarket& next,
                                   FxPnlExplainType explainType = FxPnlExplainType::Bartlett) const;

        //! \name Building blocks
        //@{
        //! Pillar expiries of the tenors on the given date.
        std::vector<Date> pillars(const Date& date) const;
        //! The market with its butterflies as smile strangles.
        FxVolMarket smileStrangles(const FxVolMarket& market) const;
        //! The market rolled to the next date
        /*! The spot rolls along today's forward to the next spot date and
            the curves are today's, so every forward to a delivery date is
            unchanged.  For each tenor and each point of the smile (ATM,
            and the calls and puts at the deltas), the total variance net
            of the pending events is divided by the eventless trading time
            to the pillar; this eventless vol is held for the tenor, and
            the events still pending at the next pillar are added back.
            The quotes returned are smile strangles.
        */
        FxVolMarket rolled(const FxVolMarket& today, const Date& next) const;
        //! The variance surface of a market quoted with smile strangles.
        ext::shared_ptr<Surface<Section>> surface(const FxVolMarket& market) const;
        //@}

      private:
        // ATM, and the call and put at each delta: vols of the smile points of a pillar
        std::vector<Volatility> smilePoints(const FxVolMarket& market, Size pillar) const;
        // smile-strangle quotes from the smile points of each pillar
        void setQuotes(FxVolMarket& market,
                       const std::vector<std::vector<Volatility>>& points) const;
        // a smile section at the expiry quoted with smile strangles
        ext::shared_ptr<Section> section(const FxVolMarket& market,
                                         const Date& expiry,
                                         Volatility atm,
                                         const std::vector<Volatility>& rrs,
                                         const std::vector<Volatility>& bfs) const;
        // ATM, risk reversals and butterflies (smile strangles) of a surface's smile at an expiry
        void fixedExpiryQuotes(const Surface<Section>& surface,
                               const FxVolMarket& market,
                               const Date& expiry,
                               Volatility& atm,
                               std::vector<Volatility>& rrs,
                               std::vector<Volatility>& bfs) const;
        // spot-date discount factors to delivery of an expiry, foreign and domestic
        std::pair<DiscountFactor, DiscountFactor> discounts(const FxVolMarket& market,
                                                            const Date& expiry) const;

        std::vector<Period> tenors_;
        FxVolConventions conventions_;
    };


    // template definitions

    template <class Section, template <class> class Surface>
    FxVanillaPnlExplain<Section, Surface>::FxVanillaPnlExplain(std::vector<Period> tenors,
                                                               FxVolConventions conventions)
    : tenors_(std::move(tenors)), conventions_(std::move(conventions)) {
        QL_REQUIRE(!tenors_.empty(), "no tenors given");
        QL_REQUIRE(!conventions_.deltas.empty(), "no deltas given");
    }

    template <class Section, template <class> class Surface>
    std::vector<Date> FxVanillaPnlExplain<Section, Surface>::pillars(const Date& date) const {
        return fxExpiryDates(date, tenors_, conventions_.settlement);
    }

    template <class Section, template <class> class Surface>
    std::pair<DiscountFactor, DiscountFactor>
    FxVanillaPnlExplain<Section, Surface>::discounts(const FxVolMarket& market,
                                                    const Date& expiry) const {
        const Date spotDate = conventions_.settlement.valueDate(market.date);
        const Date delivery = conventions_.settlement.valueDate(expiry);
        return {market.foreignCurve->discount(delivery) / market.foreignCurve->discount(spotDate),
                market.domesticCurve->discount(delivery) /
                    market.domesticCurve->discount(spotDate)};
    }

    template <class Section, template <class> class Surface>
    ext::shared_ptr<Section>
    FxVanillaPnlExplain<Section, Surface>::section(const FxVolMarket& market,
                                                  const Date& expiry,
                                                  Volatility atm,
                                                  const std::vector<Volatility>& rrs,
                                                  const std::vector<Volatility>& bfs) const {
        std::vector<Handle<Quote>> rrQuotes, bfQuotes;
        for (Size j = 0; j < rrs.size(); ++j) {
            rrQuotes.push_back(makeQuoteHandle(rrs[j]));
            bfQuotes.push_back(makeQuoteHandle(bfs[j]));
        }
        return ext::make_shared<Section>(
            expiry, makeQuoteHandle(market.spot),
            ext::make_shared<FxRrBfQuotes>(makeQuoteHandle(atm), rrQuotes, bfQuotes,
                                           conventions_.deltas, FxRrBfQuotes::SmileStrangle),
            market.foreignCurve, market.domesticCurve, conventions_.deltaType,
            conventions_.atmType, Actual365Fixed(), conventions_.settlement, market.date);
    }

    template <class Section, template <class> class Surface>
    std::vector<Volatility>
    FxVanillaPnlExplain<Section, Surface>::smilePoints(const FxVolMarket& market,
                                                      Size pillar) const {
        const Size nd = conventions_.deltas.size();
        QL_REQUIRE(market.riskReversals[pillar].size() == nd &&
                       market.butterflies[pillar].size() == nd,
                   "tenor " << tenors_[pillar] << " needs a risk reversal and a butterfly "
                            << "for each of the " << nd << " deltas");
        const Volatility atm = market.atms[pillar];
        std::vector<Volatility> points = {atm};
        if (conventions_.flyType == FxRrBfQuotes::SmileStrangle) {
            for (Size j = 0; j < nd; ++j) {
                const Volatility rr = market.riskReversals[pillar][j];
                const Volatility bf = market.butterflies[pillar][j];
                points.push_back(atm + bf + rr / 2.0);
                points.push_back(atm + bf - rr / 2.0);
            }
            return points;
        }
        // broker strangles are not points on the smile: read the points off
        // the pillar's calibrated smile
        std::vector<Handle<Quote>> rrQuotes, bfQuotes;
        for (Size j = 0; j < nd; ++j) {
            rrQuotes.push_back(makeQuoteHandle(market.riskReversals[pillar][j]));
            bfQuotes.push_back(makeQuoteHandle(market.butterflies[pillar][j]));
        }
        const Section ss(pillars(market.date)[pillar], makeQuoteHandle(market.spot),
                         ext::make_shared<FxRrBfQuotes>(makeQuoteHandle(atm), rrQuotes, bfQuotes,
                                                        conventions_.deltas,
                                                        conventions_.flyType),
                         market.foreignCurve, market.domesticCurve, conventions_.deltaType,
                         conventions_.atmType, Actual365Fixed(), conventions_.settlement,
                         market.date);
        points.front() = ss.atmVol();
        for (Real d : conventions_.deltas) {
            points.push_back(ss.volByDelta(d, Option::Call));
            points.push_back(ss.volByDelta(-d, Option::Put));
        }
        return points;
    }

    template <class Section, template <class> class Surface>
    void FxVanillaPnlExplain<Section, Surface>::setQuotes(
        FxVolMarket& market, const std::vector<std::vector<Volatility>>& points) const {
        const Size nd = conventions_.deltas.size();
        market.atms.resize(points.size());
        market.riskReversals.assign(points.size(), std::vector<Volatility>(nd));
        market.butterflies.assign(points.size(), std::vector<Volatility>(nd));
        for (Size i = 0; i < points.size(); ++i) {
            market.atms[i] = points[i][0];
            for (Size j = 0; j < nd; ++j) {
                const Volatility call = points[i][1 + 2 * j], put = points[i][2 + 2 * j];
                market.riskReversals[i][j] = call - put;
                market.butterflies[i][j] = (call + put) / 2.0 - market.atms[i];
            }
        }
    }

    template <class Section, template <class> class Surface>
    FxVolMarket
    FxVanillaPnlExplain<Section, Surface>::smileStrangles(const FxVolMarket& market) const {
        QL_REQUIRE(market.atms.size() == tenors_.size() &&
                       market.riskReversals.size() == tenors_.size() &&
                       market.butterflies.size() == tenors_.size(),
                   "market on " << market.date << " needs quotes for each of the "
                                << tenors_.size() << " tenors");
        if (conventions_.flyType == FxRrBfQuotes::SmileStrangle)
            return market;
        std::vector<std::vector<Volatility>> points;
        for (Size i = 0; i < tenors_.size(); ++i)
            points.push_back(smilePoints(market, i));
        FxVolMarket result = market;
        setQuotes(result, points);
        return result;
    }

    template <class Section, template <class> class Surface>
    FxVolMarket FxVanillaPnlExplain<Section, Surface>::rolled(const FxVolMarket& today,
                                                             const Date& next) const {
        QL_REQUIRE(next > today.date, "the next date must follow today");
        const FxVolMarket market = smileStrangles(today);
        const std::vector<Date> pillarsToday = pillars(today.date), pillarsNext = pillars(next);
        const Calendar& cal = conventions_.settlement.calendar();
        const TradingTimeTermStructure baseToday(today.date, cal, conventions_.holidayWeight);
        const TradingTimeTermStructure baseNext(next, cal, conventions_.holidayWeight);
        const Actual365Fixed dc;

        std::vector<std::vector<Volatility>> points;
        for (Size i = 0; i < tenors_.size(); ++i) {
            const Time t0 = dc.yearFraction(today.date, pillarsToday[i]);
            const Time t1 = dc.yearFraction(next, pillarsNext[i]);
            const Time tau0 = baseToday.tradingTime(today.date, pillarsToday[i]);
            const Time tau1 = baseNext.tradingTime(next, pillarsNext[i]);
            const Real events0 = fxEventVariance(today.events, today.date, pillarsToday[i]);
            const Real events1 = fxEventVariance(today.events, next, pillarsNext[i]);

            std::vector<Volatility> p = smilePoints(market, i);
            for (auto& v : p) {
                const Real eventless = v * v * t0 - events0;
                QL_REQUIRE(eventless > 0.0, "events before " << pillarsToday[i]
                                                << " carry more variance than the smile");
                v = std::sqrt((eventless / tau0 * tau1 + events1) / t1);
            }
            points.push_back(p);
        }

        FxVolMarket result = market;
        result.date = next;
        // the spot rolls along today's forward to the next spot date
        const Date spot0 = conventions_.settlement.valueDate(today.date);
        const Date spot1 = conventions_.settlement.valueDate(next);
        result.spot = today.spot *
                      (today.foreignCurve->discount(spot1) / today.foreignCurve->discount(spot0)) /
                      (today.domesticCurve->discount(spot1) / today.domesticCurve->discount(spot0));
        setQuotes(result, points);
        return result;
    }

    template <class Section, template <class> class Surface>
    ext::shared_ptr<Surface<Section>>
    FxVanillaPnlExplain<Section, Surface>::surface(const FxVolMarket& market) const {
        const std::vector<Date> dates = pillars(market.date);
        std::vector<Handle<Quote>> atms;
        std::vector<std::vector<Handle<Quote>>> rrs, bfs;
        for (Size i = 0; i < dates.size(); ++i) {
            atms.push_back(makeQuoteHandle(market.atms[i]));
            std::vector<Handle<Quote>> rr, bf;
            for (Size j = 0; j < conventions_.deltas.size(); ++j) {
                rr.push_back(makeQuoteHandle(market.riskReversals[i][j]));
                bf.push_back(makeQuoteHandle(market.butterflies[i][j]));
            }
            rrs.push_back(rr);
            bfs.push_back(bf);
        }
        const Calendar& cal = conventions_.settlement.calendar();
        RelinkableHandle<TradingTimeTermStructure> timeTs(
            ext::make_shared<TradingTimeTermStructure>(market.date, cal,
                                                       conventions_.holidayWeight));
        auto result = ext::make_shared<Surface<Section>>(
            market.date, makeQuoteHandle(market.spot), dates, atms, rrs, bfs,
            conventions_.deltas, market.foreignCurve, market.domesticCurve, timeTs,
            conventions_.deltaType, conventions_.atmType, FxRrBfQuotes::SmileStrangle,
            conventions_.settlement, cal, Following, true);
        result->enableExtrapolation();
        if (!fxPendingEvents(market.events, market.date).empty()) {
            // the events release their variance on the surface's ATM pillars,
            // i.e. the fitted smiles' ATMs, which need not reprice the quotes
            std::vector<Volatility> fitted;
            for (const auto& d : dates)
                fitted.push_back(result->atmVol(d));
            timeTs.linkTo(makeFxEventTradingTime(market.date, dates, fitted, market.events, cal,
                                                 conventions_.holidayWeight));
        }
        return result;
    }

    template <class Section, template <class> class Surface>
    void FxVanillaPnlExplain<Section, Surface>::fixedExpiryQuotes(
        const Surface<Section>& surface,
        const FxVolMarket& market,
        const Date& expiry,
        Volatility& atm,
        std::vector<Volatility>& rrs,
        std::vector<Volatility>& bfs) const {
        const auto [dfor, ddom] = discounts(market, expiry);
        const Time t = Actual365Fixed().yearFraction(market.date, expiry);
        const Real fwd = market.spot * dfor / ddom;
        auto stdDev = [&](Real k) { return surface.blackVol(expiry, k, true) * std::sqrt(t); };
        auto calculator = [&](Option::Type type, Real k) {
            return BlackDeltaCalculator(type, conventions_.deltaType, market.spot, ddom, dfor,
                                        stdDev(k));
        };
        // strikes at which the smile's conventions hold at the smile's own vol,
        // by fixed-point iteration on the strike
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

        const Real kAtm = solve(
            [&](Real k) { return calculator(Option::Call, k).atmStrike(conventions_.atmType); });
        atm = stdDev(kAtm) / std::sqrt(t);
        rrs.clear();
        bfs.clear();
        for (Real d : conventions_.deltas) {
            const Real kc =
                solve([&](Real k) { return calculator(Option::Call, k).strikeFromDelta(d); });
            const Real kp =
                solve([&](Real k) { return calculator(Option::Put, k).strikeFromDelta(-d); });
            const Volatility call = stdDev(kc) / std::sqrt(t), put = stdDev(kp) / std::sqrt(t);
            rrs.push_back(call - put);
            bfs.push_back((call + put) / 2.0 - atm);
        }
    }

    template <class Section, template <class> class Surface>
    FxPnlExplainResult
    FxVanillaPnlExplain<Section, Surface>::explain(Option::Type type,
                                                  Real strike,
                                                  const Date& expiry,
                                                  Real notional,
                                                  const FxVolMarket& today,
                                                  const FxVolMarket& next,
                                                  FxPnlExplainType explainType) const {
        QL_REQUIRE(next.date > today.date, "the next market must follow today's");
        QL_REQUIRE(expiry > next.date, "the option must expire after the next date");

        // the states, all quoted with smile strangles
        const FxVolMarket m0 = smileStrangles(today);
        const FxVolMarket mN = smileStrangles(next);
        const FxVolMarket mR = rolled(today, next.date);

        // the ATM vols' expected move per dF/F, by pillar, from today's smiles
        const auto s0 = surface(m0);
        std::vector<Real> atmSensitivities(tenors_.size(), 0.0);
        if (explainType == FxPnlExplainType::Bartlett)
            for (Size i = 0; i < tenors_.size(); ++i)
                atmSensitivities[i] =
                    s0->pillarSmileSections()[i]->atmVolForwardSensitivity();
        // a market with spot (and curves) moved, and the ATMs by their
        // expected change given each pillar's forward move
        auto movedSpot = [&](FxVolMarket m, Real spot, const Handle<YieldTermStructure>& fCurve,
                             const Handle<YieldTermStructure>& dCurve) {
            const FxVolMarket before = m;
            m.spot = spot;
            m.foreignCurve = fCurve;
            m.domesticCurve = dCurve;
            const std::vector<Date> dates = pillars(m.date);
            for (Size i = 0; i < dates.size(); ++i) {
                if (atmSensitivities[i] == 0.0)
                    continue;
                const auto [f1, d1] = discounts(before, dates[i]);
                const auto [f2, d2] = discounts(m, dates[i]);
                const Real r = (m.spot * f2 / d2) / (before.spot * f1 / d1) - 1.0;
                m.atms[i] += atmSensitivities[i] * r;
            }
            return m;
        };

        // rolled quotes, next spot and curves (and expected ATM moves)
        const FxVolMarket mS = movedSpot(mR, mN.spot, mN.foreignCurve, mN.domesticCurve);
        FxVolMarket mA = mS;  // next ATMs and events
        mA.atms = mN.atms;
        mA.events = mN.events;
        FxVolMarket mRR = mA;  // next risk reversals
        mRR.riskReversals = mN.riskReversals;

        const auto sR = surface(mR), sS = surface(mS), sA = surface(mA), sRR = surface(mRR),
                   sN = surface(mN);
        auto vol = [&](const Surface<Section>& s) { return s.blackVol(expiry, strike, true); };
        const Volatility vol0 = vol(*s0), volR = vol(*sR), volS = vol(*sS), volA = vol(*sA),
                         volRR = vol(*sRR), volN = vol(*sN);

        // Black value of the option in a market, at a vol
        const Actual365Fixed dc;
        auto value = [&](const FxVolMarket& m, Volatility v) {
            const auto [dfor, ddom] = discounts(m, expiry);
            const Real fwd = m.spot * dfor / ddom;
            const Real stdDev = v * std::sqrt(dc.yearFraction(m.date, expiry));
            return notional * blackFormula(type, strike, fwd, stdDev, ddom);
        };
        auto forward = [&](const FxVolMarket& m) {
            const auto [dfor, ddom] = discounts(m, expiry);
            return m.spot * dfor / ddom;
        };

        // Black greeks in the forward and vol, at today's market
        const auto [dfor0, ddom0] = discounts(m0, expiry);
        const Real f0 = m0.spot * dfor0 / ddom0;
        const Time t0 = dc.yearFraction(m0.date, expiry);
        const Real sd = vol0 * std::sqrt(t0);
        const Real d1 = std::log(f0 / strike) / sd + sd / 2.0, d2 = d1 - sd;
        const NormalDistribution phi;
        const CumulativeNormalDistribution cdf;
        const Real scale = notional * ddom0;
        const Real delta = scale * (type == Option::Call ? cdf(d1) : cdf(d1) - 1.0);
        const Real gamma = scale * phi(d1) / (f0 * sd);
        const Real vega = scale * f0 * phi(d1) * std::sqrt(t0);
        const Real vanna = -scale * phi(d1) * d2 / vol0;
        const Real volga = vega * d1 * d2 / vol0;

        // smile-adjusted delta and gamma: the option's vol along the moves
        // of today's forward, by central differences in the forward
        Real deltaAdjusted = delta, gammaAdjusted = gamma;
        if (explainType != FxPnlExplainType::Black) {
            const Real h = 1.0e-3;
            auto volAtSpot = [&](Real spot) {
                return vol(*surface(movedSpot(m0, spot, m0.foreignCurve, m0.domesticCurve)));
            };
            const Volatility up = volAtSpot(m0.spot * (1.0 + h));
            const Volatility down = volAtSpot(m0.spot * (1.0 - h));
            const Real volF = (up - down) / (2.0 * h * f0);
            const Real volFF = (up - 2.0 * vol0 + down) / (h * f0 * h * f0);
            deltaAdjusted = delta + vega * volF;
            gammaAdjusted = gamma + 2.0 * vanna * volF + volga * volF * volF + vega * volFF;
        }

        // split of the roll on the option's fixed-expiry smile, re-anchored
        // to the next date at the rolled spot
        Volatility atm0, atmR;
        std::vector<Volatility> rr0, bf0, rrR, bfR;
        fixedExpiryQuotes(*s0, m0, expiry, atm0, rr0, bf0);
        fixedExpiryQuotes(*sR, mR, expiry, atmR, rrR, bfR);
        const Volatility rollStart = section(mR, expiry, atm0, rr0, bf0)->volByStrike(strike);
        const Volatility rollAtm = section(mR, expiry, atmR, rr0, bf0)->volByStrike(strike);
        const Volatility rollRr = section(mR, expiry, atmR, rrR, bf0)->volByStrike(strike);
        const Volatility rollBf = section(mR, expiry, atmR, rrR, bfR)->volByStrike(strike);

        const Real dF = forward(mN) - forward(mR);
        const Real v0 = value(m0, vol0), vN = value(mN, volN);

        FxPnlExplainResult r{};
        r.type = explainType;
        r.forwardToday = f0;
        r.forwardRolled = forward(mR);
        r.forwardNext = forward(mN);
        r.volToday = vol0;
        r.volRolled = volR;
        r.volNext = volN;

        r.total = vN - v0;
        r.theta = value(mR, vol0) - v0;
        r.volRollAtm = vega * (rollAtm - rollStart);
        r.volRollRiskReversal = vega * (rollRr - rollAtm);
        r.volRollButterfly = vega * (rollBf - rollRr);
        r.volRollOther = vega * (volR - vol0) - r.volRollAtm - r.volRollRiskReversal -
                         r.volRollButterfly;
        r.delta = deltaAdjusted * dF;
        r.gamma = 0.5 * gammaAdjusted * dF * dF;
        if (explainType == FxPnlExplainType::Black) {
            r.spotSmileVega = vega * (volS - volR);
            r.spotSmileVanna = vanna * dF * (volS - volR);
        }
        r.vegaAtm = vega * (volA - volS);
        r.vegaRiskReversal = vega * (volRR - volA);
        r.vegaButterfly = vega * (volN - volRR);
        r.vannaAtm = vanna * dF * (volA - volS);
        r.vannaRiskReversal = vanna * dF * (volRR - volA);
        r.vannaButterfly = vanna * dF * (volN - volRR);
        // the smile-adjusted gamma contains the volga of the spot step
        const Volatility volgaStart = explainType == FxPnlExplainType::Black ? volR : volS;
        r.volga = 0.5 * volga * (volN - volgaStart) * (volN - volgaStart);
        r.residual = r.total - (r.theta + r.volRollAtm + r.volRollRiskReversal +
                                r.volRollButterfly + r.volRollOther + r.delta + r.gamma +
                                r.spotSmileVega + r.spotSmileVanna + r.vegaAtm +
                                r.vegaRiskReversal + r.vegaButterfly + r.vannaAtm +
                                r.vannaRiskReversal + r.vannaButterfly + r.volga);
        return r;
    }

}

#endif
