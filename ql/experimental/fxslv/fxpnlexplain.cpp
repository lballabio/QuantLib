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

#include <ql/experimental/fxslv/fxpnlexplain.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <cmath>

namespace QuantLib {

    FxPnlExplainResult fxVanillaPnlExplain(Option::Type type,
                                           Real strike,
                                           const Date& expiry,
                                           Real notional,
                                           const FxPairMarket& today,
                                           const FxPairMarket& next,
                                           FxPnlExplainType explainType) {
        QL_REQUIRE(today.conventions().name() == next.conventions().name(),
                   "the markets are of different pairs: " << today.conventions().name()
                                                          << " and " << next.conventions().name());
        QL_REQUIRE(today.quotes().tenors == next.quotes().tenors &&
                       today.quotes().deltas == next.quotes().deltas,
                   "the markets must quote the same vol tenors and deltas");
        QL_REQUIRE(next.asOf() > today.asOf(), "the next market must follow today's");
        QL_REQUIRE(expiry > next.asOf(), "the option must expire after the next date");

        // the states, all quoted with smile strangles
        const FxPairMarket m0 = today.smileStrangles();
        const FxPairMarket mN = next.smileStrangles();
        const FxPairMarket mR = today.rolled(next.asOf());
        const Size nPillars = m0.pillars().size();

        // the ATM vols' expected move per dF/F, by pillar, from today's smiles
        std::vector<Real> atmSensitivities(nPillars, 0.0);
        if (explainType == FxPnlExplainType::Bartlett)
            for (Size i = 0; i < nPillars; ++i)
                atmSensitivities[i] =
                    m0.fxVolSurface().pillarSmileSection(i)->atmVolForwardSensitivity();
        // a market with spot (and curves) moved, and the ATMs by their
        // expected change given each pillar's forward move
        auto movedSpot = [&](const FxPairMarket& before, Real spot,
                             const Handle<YieldTermStructure>& fCurve,
                             const Handle<YieldTermStructure>& dCurve) {
            const FxPairMarket moved = before.withSpot(spot).withCurves(fCurve, dCurve);
            FxVolQuotes quotes = moved.quotes();
            bool changed = false;
            for (Size i = 0; i < nPillars; ++i) {
                if (atmSensitivities[i] == 0.0)
                    continue;
                const Date& pillar = moved.pillars()[i];
                const Real r = moved.forward(pillar) / before.forward(pillar) - 1.0;
                quotes.atms[i] += atmSensitivities[i] * r;
                changed = true;
            }
            return changed ? moved.withQuotes(quotes) : moved;
        };

        // rolled quotes, next spot and curves (and expected ATM moves)
        const FxPairMarket mS =
            movedSpot(mR, mN.spot(), mN.foreignCurve(), mN.domesticCurve());
        FxVolQuotes quotesA = mS.quotes();  // next ATMs and events
        quotesA.atms = mN.quotes().atms;
        const FxPairMarket mA = mS.withQuotes(quotesA).withEvents(mN.events());
        FxVolQuotes quotesRR = quotesA;  // next risk reversals
        quotesRR.riskReversals = mN.quotes().riskReversals;
        const FxPairMarket mRR = mA.withQuotes(quotesRR);

        auto vol = [&](const FxPairMarket& m) { return m.marketVol(expiry, strike); };
        const Volatility vol0 = vol(m0), volR = vol(mR), volS = vol(mS), volA = vol(mA),
                         volRR = vol(mRR), volN = vol(mN);

        // Black value of the option in a market, at a vol
        const Actual365Fixed dc;
        auto value = [&](const FxPairMarket& m, Volatility v) {
            const auto [dfor, ddom] = m.settlementDiscounts(expiry);
            const Real fwd = m.spot() * dfor / ddom;
            const Real stdDev = v * std::sqrt(dc.yearFraction(m.asOf(), expiry));
            return notional * blackFormula(type, strike, fwd, stdDev, ddom);
        };

        // Black greeks in the forward and vol, at today's market
        const auto [dfor0, ddom0] = m0.settlementDiscounts(expiry);
        const Real f0 = m0.spot() * dfor0 / ddom0;
        const Time t0 = dc.yearFraction(m0.asOf(), expiry);
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
                return vol(movedSpot(m0, spot, m0.foreignCurve(), m0.domesticCurve()));
            };
            const Volatility up = volAtSpot(m0.spot() * (1.0 + h));
            const Volatility down = volAtSpot(m0.spot() * (1.0 - h));
            const Real volF = (up - down) / (2.0 * h * f0);
            const Real volFF = (up - 2.0 * vol0 + down) / (h * f0 * h * f0);
            deltaAdjusted = delta + vega * volF;
            gammaAdjusted = gamma + 2.0 * vanna * volF + volga * volF * volF + vega * volFF;
        }

        // split of the roll on the option's fixed-expiry smile, re-anchored
        // to the next date at the rolled spot
        const FxExpiryQuotes q0 = m0.expiryQuotes(expiry), qR = mR.expiryQuotes(expiry);
        auto rollVol = [&](Volatility atm, const std::vector<Volatility>& rrs,
                           const std::vector<Volatility>& bfs) {
            return mR.smileSection(expiry, FxExpiryQuotes{atm, rrs, bfs})->volByStrike(strike);
        };
        const Volatility rollStart = rollVol(q0.atm, q0.riskReversals, q0.butterflies);
        const Volatility rollAtm = rollVol(qR.atm, q0.riskReversals, q0.butterflies);
        const Volatility rollRr = rollVol(qR.atm, qR.riskReversals, q0.butterflies);
        const Volatility rollBf = rollVol(qR.atm, qR.riskReversals, qR.butterflies);

        const Real dF = mN.forward(expiry) - mR.forward(expiry);
        const Real v0 = value(m0, vol0), vN = value(mN, volN);

        FxPnlExplainResult r{};
        r.type = explainType;
        r.forwardToday = f0;
        r.forwardRolled = mR.forward(expiry);
        r.forwardNext = mN.forward(expiry);
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
