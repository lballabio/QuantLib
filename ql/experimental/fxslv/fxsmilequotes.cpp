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

#include <ql/experimental/fxslv/fxsmilequotes.hpp>
#include <ql/experimental/fxslv/fxsmilesection.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/quotes/simplequote.hpp>
#include <cmath>
#include <utility>

namespace QuantLib {

    namespace {

        // n(d+), proportional to the vega of an option struck at K
        Real vegaWeight(Real forward, Rate strike, Real stdDev) {
            const Real dp = std::log(forward / strike) / stdDev + 0.5 * stdDev;
            return NormalDistribution()(dp);
        }

        // call and put strikes of the broker strangle, struck at the broker vol
        std::pair<Rate, Rate> brokerStrikes(const FxSmileSection& section, Real delta, Real stdDev) {
            const FxDeltaConverter conv = section.deltaConverter();
            return {conv.strike(Option::Call, delta, stdDev),
                    conv.strike(Option::Put, -delta, stdDev)};
        }

    }

    std::optional<std::pair<Rate, Volatility>>
    FxDeltaVolTarget::point(const FxSmileSection& section) const {
        // the quote's own vol and conventions fix its strike
        const DeltaVolQuote& q = **quote_;
        const Volatility vol = q.value();
        const Real w = vol * std::sqrt(section.exerciseTime());
        const FxDeltaConverter conv = section.deltaConverter().withType(q.deltaType());
        const Rate strike =
            q.atmType() == DeltaVolQuote::AtmNull ?
                conv.strike(q.delta() < 0 ? Option::Put : Option::Call, q.delta(), w) :
                conv.atmStrike(q.atmType(), w);
        return std::make_pair(strike, vol);
    }

    Real FxDeltaVolTarget::residual(const FxSmileSection& section) const {
        const auto p = *point(section);
        return section.volResidual(p.first, p.second);
    }

    Real FxDeltaVolTarget::weight(const FxSmileSection& section) const {
        const auto p = *point(section);
        return vegaWeight(section.forward(), p.first, p.second * std::sqrt(section.exerciseTime()));
    }

    Real FxRiskReversalTarget::weight(const FxSmileSection&) const {
        // d+ = N^{-1}(delta) for a forward call delta; both legs alike
        return NormalDistribution()(InverseCumulativeNormal()(delta_));
    }

    Real FxRiskReversalTarget::residual(const FxSmileSection& section) const {
        return section.volByDelta(delta_, Option::Call) - section.volByDelta(-delta_, Option::Put) -
               riskReversal_;
    }

    Real FxBrokerStrangleTarget::residual(const FxSmileSection& section) const {
        const Time tau = section.exerciseTime();
        const Real htau = std::sqrt(tau);

        const Real fwd = section.forward();

        const Real w = (atmVol_ + brokerFly_) * htau;
        const auto [callStrike, putStrike] = brokerStrikes(section, delta_, w);
        const BlackCalculator call(Option::Call, callStrike, fwd, w);
        const BlackCalculator put(Option::Put, putStrike, fwd, w);
        Real marketPrice = call.value() + put.value();
        Real marketVega = call.vega(tau) + put.vega(tau);
        QL_REQUIRE(marketVega > 0.0, "market strangle has no vega for delta " << delta_);

        const Real vc = section.volByStrike(callStrike);
        const Real vp = section.volByStrike(putStrike);
        const Real smilePrice = BlackCalculator(Option::Call, callStrike, fwd, vc * htau).value() +
                                BlackCalculator(Option::Put, putStrike, fwd, vp * htau).value();
        return (smilePrice - marketPrice) / marketVega;
    }


    Real FxBrokerStrangleTarget::weight(const FxSmileSection& section) const {
        const Real w = (atmVol_ + brokerFly_) * std::sqrt(section.exerciseTime());
        const Real fwd = section.forward();
        const auto [callStrike, putStrike] = brokerStrikes(section, delta_, w);
        return 0.5 * (vegaWeight(fwd, callStrike, w) + vegaWeight(fwd, putStrike, w));
    }

    void FxSmileQuotes::fit(const FxSmileSection& section, FxSmileTargets targets) {
        section.fitToTargets(std::move(targets));
    }

    FxRrBfQuotes::FxRrBfQuotes(Handle<Quote> atm,
                               std::vector<Handle<Quote>> riskReversals,
                               std::vector<Handle<Quote>> butterflies,
                               std::vector<Real> deltas,
                               FlyType flyType)
    : atm_(std::move(atm)), riskReversals_(std::move(riskReversals)),
      butterflies_(std::move(butterflies)), deltas_(std::move(deltas)), flyType_(flyType) {
        QL_REQUIRE(!atm_.empty(), "no ATM quote given");
        QL_REQUIRE(riskReversals_.size() == deltas_.size(),
                   "risk reversal quotes must be the same size as deltas");
        QL_REQUIRE(butterflies_.size() == deltas_.size(),
                   "butterfly quotes must be the same size as deltas");
        for (Size i = 0; i < deltas_.size(); ++i) {
            QL_REQUIRE(deltas_[i] > 0.0 && deltas_[i] < 0.5,
                       "deltas must be in (0, 0.5): " << deltas_[i]);
            for (Size j = 0; j < i; ++j)
                QL_REQUIRE(deltas_[i] != deltas_[j], "duplicate delta: " << deltas_[i]);
            QL_REQUIRE(!riskReversals_[i].empty(),
                       "no risk reversal quote given for delta " << deltas_[i]);
            QL_REQUIRE(!butterflies_[i].empty(),
                       "no butterfly quote given for delta " << deltas_[i]);
        }
        registerWith(atm_);
        for (const auto& rr : riskReversals_)
            registerWith(rr);
        for (const auto& bf : butterflies_)
            registerWith(bf);
    }

    void FxRrBfQuotes::calibrate(const FxSmileSection& section) const {
        const Time tau = section.exerciseTime();
        const DeltaVolQuote::DeltaType deltaType = section.deltaType();

        FxSmileTargets targets;

        // atm vol target
        targets.push_back(ext::make_shared<FxDeltaVolTarget>(Handle<DeltaVolQuote>(
            ext::make_shared<DeltaVolQuote>(atm_, deltaType, tau, section.atmType()))));

        for (Size i = 0; i < deltas_.size(); ++i) {
            const Real d = deltas_[i];
            const Real rr = riskReversals_[i]->value();
            const Real bf = butterflies_[i]->value();

            if (flyType_ == SmileStrangle) {
                // smile strangles convert algebraically to points on the smile
                const Volatility cVol = atm_->value() + bf + rr / 2.;
                const Volatility pVol = atm_->value() + bf - rr / 2.;
                targets.push_back(ext::make_shared<FxDeltaVolTarget>(Handle<DeltaVolQuote>(
                    ext::make_shared<DeltaVolQuote>(d, makeQuoteHandle(cVol), tau, deltaType))));
                targets.push_back(ext::make_shared<FxDeltaVolTarget>(Handle<DeltaVolQuote>(
                    ext::make_shared<DeltaVolQuote>(-d, makeQuoteHandle(pVol), tau, deltaType))));
            } else {
                // broker strangles: risk reversal at the smile's deltas plus 
                // the strangle premium, fitted jointly with the ATM
                targets.push_back(ext::make_shared<FxRiskReversalTarget>(d, rr));
                targets.push_back(ext::make_shared<FxBrokerStrangleTarget>(atm_->value(), bf, d));
            }
        }

        fit(section, std::move(targets));
    }


    FxDeltaVolQuotes::FxDeltaVolQuotes(std::vector<Handle<DeltaVolQuote>> quotes)
    : quotes_(std::move(quotes)) {
        QL_REQUIRE(!quotes_.empty(), "no delta-vol quotes given");
        for (const auto& q : quotes_) {
            QL_REQUIRE(!q.empty(), "empty delta-vol quote handle given");
            QL_REQUIRE(q->atmType() != DeltaVolQuote::AtmNull || q->delta() != 0.0,
                       "delta-vol quote needs a non-zero delta or an ATM convention");
            registerWith(q);
        }
    }

    Volatility FxDeltaVolQuotes::referenceVol() const {
        Real sumVol = 0.0;
        for (const auto& q : quotes_)
            sumVol += q->value();
        return sumVol / static_cast<Real>(quotes_.size());
    }

    void FxDeltaVolQuotes::calibrate(const FxSmileSection& section) const {
        FxSmileTargets targets;
        targets.reserve(quotes_.size());
        for (const auto& q : quotes_)
            targets.push_back(ext::make_shared<FxDeltaVolTarget>(q));
        fit(section, std::move(targets));
    }

}
