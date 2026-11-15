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

#include <ql/experimental/fxslv/fxsettlementforeigncurve.hpp>
#include <ql/experimental/fxslv/fxvariancesurface.hpp>
#include <ql/experimental/fxslv/tradingtime/tradingtimedaycounter.hpp>
#include <ql/pricingengines/blackdeltacalculator.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/quotes/simplequote.hpp>
#include <cmath>

namespace QuantLib {
    FxVarianceSurface::FxVarianceSurface(FxSmileModel model,
                                         const Date& referenceDate,
                                            const Handle<Quote>& spot,
                                            const std::vector<Date>& pillars,
                                            const std::vector<Handle<Quote>>& atms,
                                            const std::vector<std::vector<Handle<Quote>>>& rrs,
                                            const std::vector<std::vector<Handle<Quote>>>& bfs,
                                            const std::vector<Real>& deltas,
                                            const Handle<YieldTermStructure>& forTs,
                                            const Handle<YieldTermStructure>& domTs,
                                            const Handle<TradingTimeTermStructure>& timeTs,
                                            DeltaVolQuote::DeltaType deltaType,
                                            DeltaVolQuote::AtmType atmType,
                                            FxRrBfQuotes::FlyType flyType,
                                            const FxSettleConvention& settlement,
                                            const FxExpiryCut& cut,
                                            const Calendar& cal,
                                            BusinessDayConvention bdc,
                                            bool forceMonotoneVariance,
                                            Time nodeSpacing)
    : BlackVarianceTermStructure(referenceDate, cal, bdc, TradingTimeDayCounter(timeTs)),
      model_(model), spot_(spot),
      settlement_(settlement), cut_(cut),
      pillars_(pillars), forTs_(forTs), domTs_(domTs), timeTs_(timeTs), nodeSpacing_(nodeSpacing),
      maxDate_(pillars.back()) {
        QL_REQUIRE(pillars.size() == atms.size(), "mismatch between date vector and vol vector");
        QL_REQUIRE(pillars.size() == rrs.size(), "mismatch between date vector and rr vector");
        QL_REQUIRE(pillars.size() == bfs.size(), "mismatch between date vector and bf vector");

        // cannot have dates[0] = referenceDate, since vol at dates[0] must be 0!
        QL_REQUIRE(pillars[0] > referenceDate, "cannot have dates[0] <= referenceDate");
        QL_REQUIRE(nodeSpacing_ > 0.0, "positive node spacing required: " << nodeSpacing_);
#ifndef QL_HIGH_RESOLUTION_DATE
        // each node is an expiry date, so it falls within a day only with
        // high-resolution dates
        const Real spacingDays = nodeSpacing_ * 365.0;
        QL_REQUIRE(std::fabs(spacingDays - std::round(spacingDays)) < 1.0e-9,
                   "a node spacing of " << spacingDays << " days needs high-resolution dates");
#endif

        smileSections_.clear();
        for (Size j = 0; j < atms.size(); j++) {
            // this will also register the smile section with spot, vol quotes and discount curves
            auto section = makeFxSmileSection(
                model_, pillars[j], spot_,
                ext::make_shared<FxRrBfQuotes>(atms[j], rrs[j], bfs[j], deltas, flyType), forTs_,
                domTs_, deltaType, atmType, settlement, referenceDate);
            registerWith(section);
            smileSections_.push_back(section);
        }

        registerWithMarketData();
    }

    void FxVarianceSurface::update()
    {
        // recomputing here could throw in the middle of a notification,
        // e.g. when the evaluation date moves; do it when next needed
        smileCache_.clear();
        tradingTimesStale_ = true;
        BlackVarianceTermStructure::update();
    }

    Real FxVarianceSurface::atmVar(Date d) const {
        checkTradingTimes();
        // quick routine for atm variance - avoids smile calibration!
        Time tau = timeTs_->tradingTime(referenceDate(), d);
        if (tau <= tradingTimes_.back()) {
            // before final expiry...

            if (tau <= tradingTimes_[1]) {
                // before first expiry
                Real vol = smileSections_.front()->atmVol();
                return (vol * vol * smileSections_.front()->exerciseTime()) * tau / tradingTimes_[1];
            } 
            else 
            {
                // interpolate between two smiles
                Size i = 0;
                while (tradingTimes_[i + 1] < tau)
                    i++;

                // times_ starts at 0 but there is no smile section
                // so adjust i accordingly when accessing smileSections_
                Real frontVol = smileSections_[i - 1]->atmVol();
                Real backVol = smileSections_[i]->atmVol();
                Real frontVar = frontVol * frontVol * smileSections_[i - 1]->exerciseTime();
                Real backVar = backVol * backVol * smileSections_[i]->exerciseTime();
                Real slopeVar = (backVar - frontVar) / (tradingTimes_[i + 1] - tradingTimes_[i]);
                return frontVar + slopeVar * (tau - tradingTimes_[i]);
            }
        } 
        else {
            // extrapolate with flat vol in trading time!
            // there might be events beyond the last expiry - this will 
            // account for that as we are working in trading time!
            Real vol = smileSections_.back()->atmVol();
            return (vol * vol * smileSections_.back()->exerciseTime()) * tau / tradingTimes_.back();
        }
    }

    Real FxVarianceSurface::atmVol(Date d) const {
        return std::sqrt(atmVar(d) / calendarTime(d));
    }

    Real FxVarianceSurface::blackVarianceImpl(Time tau, Real strike) const {
        // the time is a trading time; the calendar time finds the nodes, and
        // within an event, which takes no calendar time, the variance still
        // grows linearly in trading time
        tradingTime(0.0);  // checks the reference dates
        return varianceAt(timeTs_->calendarTime(tau), tau, strike);
    }

    Volatility FxVarianceSurface::marketVol(const Date& expiry, Real strike) const {
        const Time t = calendarTime(expiry);
        QL_REQUIRE(t > 0.0, "expiry " << expiry << " not after the reference date "
                                      << referenceDate());
        return std::sqrt(blackVariance(expiry, strike, true) / t);
    }

    Real FxVarianceSurface::varianceAt(Time t, Time tau, Real strike) const {
        // Total variance is interpolated between nodes: the reference date,
        // the pillars and the points of a grid in calendar time.  Between
        // two nodes it is linear in trading time, so it stays flat over a
        // weekend and jumps with trading time at an event, whatever the
        // spacing; the spacing only sets how closely the smile follows the
        // interpolation between pillars.  Nodes are in calendar time since
        // each interpolated smile is built for an expiry date.  Both nodes
        // are taken at the same forward moneyness, since the forward keeps
        // moving with the carry when trading time does not.
        const Bracket b = bracket(t);
        const Real fa = nodeForward(b.pa, b.index);
        const Time tauA = tradingTime(b.ta), tauB = tradingTime(b.tb);
        if (tauB <= tauA)  // no trading time between the nodes
            return nodeVariance(b.pa, b.index, strike * fa / calendarFwd(t));
        const Real fb = nodeForward(b.pb, b.index + 1);
        const Real moneyness = strike / calendarFwd(t);
        const Real wa = nodeVariance(b.pa, b.index, moneyness * fa);
        const Real wb = nodeVariance(b.pb, b.index + 1, moneyness * fb);
        const Real lambda = (tau - tauA) / (tauB - tauA);
        return wa + lambda * (wb - wa);
    }

    FxVarianceSurface::Bracket FxVarianceSurface::bracket(Time t) const {
        checkTradingTimes();
        // a time on a point of the grid, up to rounding, is taken to be on it
        const long j = static_cast<long>(std::floor(t / nodeSpacing_ + 1.0e-9));
        Bracket b{j, nodeTime(j), nodeTime(j + 1), -1, -1};
        for (Size i = 0; i < pillarTimes_.size(); ++i) {
            if (pillarTimes_[i] <= t && pillarTimes_[i] >= b.ta) {
                b.ta = pillarTimes_[i];
                b.pa = static_cast<Integer>(i);
            }
            if (pillarTimes_[i] > t && pillarTimes_[i] <= b.tb) {
                b.tb = pillarTimes_[i];
                b.pb = static_cast<Integer>(i);
            }
        }
        return b;
    }

    Date FxVarianceSurface::nodeDate(long index) const {
#ifdef QL_HIGH_RESOLUTION_DATE
        const auto offset = boost::posix_time::microseconds(
            std::llround(index * nodeSpacing_ * 365.0 * 86400.0e6));
        return Date(referenceDate().dateTime() + offset);
#else
        // whole days, as checked on construction
        return referenceDate() +
               static_cast<Date::serial_type>(index * std::lround(nodeSpacing_ * 365.0));
#endif
    }

    Real FxVarianceSurface::nodeForward(Integer pillar, long index) const {
        return fwd(pillar >= 0 ? pillars_[pillar] : nodeDate(index));
    }

    std::pair<DiscountFactor, DiscountFactor>
    FxVarianceSurface::settlementDiscounts(const Date& expiry) const {
        return fxSettlementDiscounts(settlement_, referenceDate(), expiry, **forTs_, **domTs_);
    }

    Real FxVarianceSurface::fwd(const Date& expiry) const {
        const auto [dfor, ddom] = settlementDiscounts(expiry);
        return spot_->value() * dfor / ddom;
    }

    Real FxVarianceSurface::fwd(Time t) const {
        return fwd(detail::dayContaining(*this, t));
    }

    Real FxVarianceSurface::calendarFwd(Time t) const {
        return fwd(detail::dayContaining(referenceDate(), Actual365Fixed(), t));
    }

    Time FxVarianceSurface::tradingTime(Time t) const {
        // the trading-time curve measures times from its own reference
        // date, so both must agree; a floating curve under a fixed surface
        // would drift apart as the evaluation date moves
        QL_REQUIRE(timeTs_->referenceDate() == referenceDate(),
                   "trading-time reference date (" << timeTs_->referenceDate()
                       << ") differs from the variance surface's (" << referenceDate() << ")");
        return timeTs_->tradingTime(t);
    }

    Real FxVarianceSurface::nodeVariance(Integer pillar, long index, Real strike) const {
        if (pillar >= 0)
            return smileSections_[pillar]->variance(strike);
        if (tradingTime(nodeTime(index)) <= 0.0)
            return 0.0;  // the reference date, or no trading time since
        return gridSmileSection(index)->variance(strike);
    }

    const Handle<FxSmileSection>& FxVarianceSurface::gridSmileSection(long index) const {
        auto it = smileCache_.find(index);
        if (it != smileCache_.end())
            return it->second;

        const Date expiry = nodeDate(index);
        const Time t = nodeTime(index);
        const Time tau = tradingTime(t);
        Handle<FxSmileSection> section;
        if (tau <= tradingTimes_[1]) {
            // before the first pillar: scale the first smile
            section = interpolatedSmileSection(expiry, t, 0.0, *smileSections_.front(),
                                               tau / tradingTimes_[1], *smileSections_.front());
        } else if (tau <= tradingTimes_.back()) {
            // between two pillars; tradingTimes_ starts at the reference
            // date, so pillar i is at tradingTimes_[i + 1]
            Size i = 1;
            while (tradingTimes_[i + 1] < tau)
                ++i;
            const Real w = (tau - tradingTimes_[i]) / (tradingTimes_[i + 1] - tradingTimes_[i]);
            section = interpolatedSmileSection(expiry, t, 1.0 - w, *smileSections_[i - 1],
                                               w, *smileSections_[i]);
        } else {
            // after the last pillar: scale the last smile
            section = interpolatedSmileSection(expiry, t, tau / tradingTimes_.back(), *smileSections_.back(),
                                               0.0, *smileSections_.back());
        }
        return smileCache_.emplace(index, section).first->second;
    }

    void FxVarianceSurface::registerWithMarketData() {
        // the smile sections are registered with as they are built
        registerWith(spot_);
        registerWith(forTs_);
        registerWith(domTs_);
        registerWith(timeTs_);
    }

    void FxVarianceSurface::checkTradingTimes() const {
        if (!tradingTimesStale_)
            return;
        tradingTimes_ = std::vector<Time>(pillars_.size() + 1); // these are market times, NOT real times!
        tradingTimes_[0] = 0.0;
        for (Size j = 1; j <= pillars_.size(); j++) {
            tradingTimes_[j] = timeTs_->tradingTime(referenceDate(), pillars_[j - 1]);
        }
        pillarTimes_.resize(pillars_.size());
        for (Size j = 0; j < pillars_.size(); ++j)
            pillarTimes_[j] = calendarTime(pillars_[j]);
        tradingTimesStale_ = false;
    }

    FxVarianceSurfaceClark::FxVarianceSurfaceClark(FxSmileModel model,
                                                   const Date& referenceDate, const Handle<Quote>& spot,
                                                      const std::vector<Date>& pillars, 
                                                      const std::vector<Handle<Quote>>& atms,
                                                      const std::vector<std::vector<Handle<Quote>>>& rrs,
                                                      const std::vector<std::vector<Handle<Quote>>>& bfs,
                                                      const std::vector<Real>& deltas,
                                                      const Handle<YieldTermStructure>& forTs,
                                                      const Handle<YieldTermStructure>& domTs,
                                                      const Handle<TradingTimeTermStructure>& timeTs,
                                                      DeltaVolQuote::DeltaType deltaType,
                                                      DeltaVolQuote::AtmType atmType,
                                                      FxRrBfQuotes::FlyType flyType,
                                                      const FxSettleConvention& settlement,
                                                      const FxExpiryCut& cut,
                                                      const Calendar& cal,
                                                      BusinessDayConvention bdc,
                                                      bool forceMonotoneVariance,
                                                      Time nodeSpacing)
    : FxVarianceSurface(model, referenceDate, spot, pillars, atms, rrs, bfs, deltas, forTs, domTs,
                           timeTs, deltaType, atmType, flyType, settlement, cut, cal, bdc,
                           forceMonotoneVariance, nodeSpacing) {}

    Handle<FxSmileSection> FxVarianceSurfaceClark::interpolatedSmileSection(const Date& expiry,
                                                                  Time t,
                                                                  Real wInit,
                                                                  const FxSmileSection& ssInit,
                                                                  Real wFinal,
                                                                  const FxSmileSection& ssFinal) const {
        // Implementation of flat forward smile interpolation in variance [Clark].
        // Interpolate ATM, 25d and 10d strikes for given weights to get five
        // points on the interpolated smile. Calibrate and return the smile.
        //
        // For interpolation between two pillars: wInit = 1-w, wFinal = w.
        // For scaling a single smile (before first / after last pillar):
        // one weight is 0, the other is the variance scaling factor.
        const Time t1 = ssInit.exerciseTime(), t2 = ssFinal.exerciseTime();
        auto flatFwdVar = [wInit, wFinal, t1, t2, t](Volatility v1, Volatility v2) {
            return std::sqrt((wInit * v1 * v1 * t1 + wFinal * v2 * v2 * t2) / t);
        };

        Handle<Quote> atm = makeQuoteHandle(flatFwdVar(ssInit.atmVol(),
                                                       ssFinal.atmVol()));

        // 25 delta
        Volatility v25c = flatFwdVar(ssInit.volByDelta(0.25, Option::Call),
                                     ssFinal.volByDelta(0.25, Option::Call));
        Volatility v25p = flatFwdVar(ssInit.volByDelta(-0.25, Option::Put),
                                     ssFinal.volByDelta(-0.25, Option::Put));
        Handle<Quote> rr25 = makeQuoteHandle(v25c - v25p);
        Handle<Quote> bf25 = makeQuoteHandle((v25c + v25p) / 2. - atm->value());

        // 10 delta
        Volatility v10c = flatFwdVar(ssInit.volByDelta(0.10, Option::Call),
                                     ssFinal.volByDelta(0.10, Option::Call));
        Volatility v10p = flatFwdVar(ssInit.volByDelta(-0.10, Option::Put),
                                     ssFinal.volByDelta(-0.10, Option::Put));
        Handle<Quote> rr10 = makeQuoteHandle(v10c - v10p);
        Handle<Quote> bf10 = makeQuoteHandle((v10c + v10p) / 2. - atm->value());

        // create and return the smile section
        return Handle<FxSmileSection>(makeFxSmileSection(this->smileModel(), expiry, this->spot(),
                                             // butterflies built from smile vols are smile strangles
                                             ext::make_shared<FxRrBfQuotes>(
                                                 atm, std::vector<Handle<Quote>>{rr25, rr10},
                                                 std::vector<Handle<Quote>>{bf25, bf10}, std::vector<Real>{0.25, 0.10},
                                                 FxRrBfQuotes::SmileStrangle),
                                             this->foreignDiscountCurve(),
                                             this->domesticDiscountCurve(),
                                             ssInit.deltaType(), ssInit.atmType(),
                                             this->settlement(),
                                             this->referenceDate()));
    }

    FxVarianceSurfaceNCP::FxVarianceSurfaceNCP(FxSmileModel model,
                                               const Date& referenceDate, const Handle<Quote>& spot,
                                                  const std::vector<Date>& pillars,
                                                  const std::vector<Handle<Quote>>& atms,
                                                  const std::vector<std::vector<Handle<Quote>>>& rrs,
                                                  const std::vector<std::vector<Handle<Quote>>>& bfs,
                                                  const std::vector<Real>& deltas,
                                                  const Handle<YieldTermStructure>& forTs,
                                                  const Handle<YieldTermStructure>& domTs,
                                                  const Handle<TradingTimeTermStructure>& timeTs,
                                                  DeltaVolQuote::DeltaType deltaType,
                                                  DeltaVolQuote::AtmType atmType,
                                                  FxRrBfQuotes::FlyType flyType,
                                                  const FxSettleConvention& settlement,
                                                  const FxExpiryCut& cut,
                                                  const Calendar& cal,
                                                  BusinessDayConvention bdc,
                                                  bool forceMonotoneVariance,
                                                  Time nodeSpacing)
    : FxVarianceSurface(model, referenceDate, spot, pillars, atms, rrs, bfs, deltas, forTs, domTs,
                           timeTs, deltaType, atmType, flyType, settlement, cut, cal, bdc,
                           forceMonotoneVariance, nodeSpacing) {}

    Handle<FxSmileSection> FxVarianceSurfaceNCP::interpolatedSmileSection(const Date& expiry,
                                                                Time t,
                                                                Real wInit,
                                                                const FxSmileSection& ssInit,
                                                                Real wFinal,
                                                                const FxSmileSection& ssFinal) const {
        // Scaling case: same smile on both sides — scale variance rather than
        // interpolating in probability space.
        if (&ssInit == &ssFinal) {
            Real s = std::sqrt((wInit + wFinal) * ssInit.exerciseTime() / t); // vol scaling factor

            Handle<Quote> atm = makeQuoteHandle(ssInit.atmVol() * s);

            // 25 delta
            Volatility v25c = ssInit.volByDelta(0.25, Option::Call) * s;
            Volatility v25p = ssInit.volByDelta(-0.25, Option::Put) * s;
            Handle<Quote> rr25 = makeQuoteHandle(v25c - v25p);
            Handle<Quote> bf25 = makeQuoteHandle((v25c + v25p) / 2. - atm->value());

            // 10 delta
            Volatility v10c = ssInit.volByDelta(0.10, Option::Call) * s;
            Volatility v10p = ssInit.volByDelta(-0.10, Option::Put) * s;
            Handle<Quote> rr10 = makeQuoteHandle(v10c - v10p);
            Handle<Quote> bf10 = makeQuoteHandle((v10c + v10p) / 2. - atm->value());

            return Handle<FxSmileSection>(makeFxSmileSection(this->smileModel(), expiry, this->spot(),
                                                 // butterflies built from smile vols are smile strangles
                                                 ext::make_shared<FxRrBfQuotes>(
                                                     atm, std::vector<Handle<Quote>>{rr25, rr10},
                                                     std::vector<Handle<Quote>>{bf25, bf10}, std::vector<Real>{0.25, 0.10},
                                                     FxRrBfQuotes::SmileStrangle),
                                                 this->foreignDiscountCurve(),
                                                 this->domesticDiscountCurve(),
                                                 ssInit.deltaType(), ssInit.atmType(),
                                                 this->settlement(),
                                             this->referenceDate()));
        }

        // Interpolation in probability space using normed call prices [Gope, Fries 2011].
        // discount factors and forward from the spot date to the delivery date
        const auto [dfor, ddom] = this->settlementDiscounts(expiry);
        Real spt = this->spot()->value();

        // used only here, while the sections are alive, so capture by reference
        auto interpNcp = [&](Real k2) {
            Real m2 = k2 / ssFinal.forward();                // long moneyness
            Real p = ssFinal.exerciseProbability(m2);        // its exercise probability
            Real m1 = ssInit.moneynessFromProbability(p);   // short moneyness, same probability

            Real ncp1 = ssInit.normedCallPrice(m1);  // ncp at shorter smile
            Real ncp2 = ssFinal.normedCallPrice(m2); // ncp at longer smile

            // interpolated values
            Real fwd = spt * dfor / ddom;
            Real k = fwd * (wInit * m1 + wFinal * m2);     // strike
            Real c = fwd * (wInit * ncp1 + wFinal * ncp2); // prem

            // undiscounted price, so a unit discount; this inverts the
            // out-of-the-money option, and bounds the std dev only by [0, 24]
            Real iw = blackFormulaImpliedStdDev(Option::Call, k, fwd, c, 1.0, 0.0, Null<Real>(),
                                                1.0e-10, 100);

            Real v = iw / std::sqrt(t); // black calculator uses total vol so scale by sqrt(time)

            // convert the (strike, vol) to a (delta, vol) to create a DeltaVolQuote
            Real d;
            if (k > fwd) {
                d = BlackDeltaCalculator(Option::Call, ssFinal.deltaType(), spt, ddom, dfor, iw).deltaFromStrike(k);
            } else {
                d = BlackDeltaCalculator(Option::Put, ssFinal.deltaType(), spt, ddom, dfor, iw).deltaFromStrike(k);
            }
            return Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(d, makeQuoteHandle(v), t, ssFinal.deltaType()));
        };

        std::vector<Handle<DeltaVolQuote>> quotes;

        // 10d put
        Real k_p10 = ssFinal.strikeByDelta(-0.10, Option::Put);
        quotes.push_back(interpNcp(k_p10));

        // 25d put
        Real k_p25 = ssFinal.strikeByDelta(-0.25, Option::Put);
        quotes.push_back(interpNcp(k_p25));

        // atm
        Real k_atm = ssFinal.atmStrike();
        quotes.push_back(interpNcp(k_atm));

        // 25c call
        Real k_c25 = ssFinal.strikeByDelta(0.25, Option::Call);
        quotes.push_back(interpNcp(k_c25));

        // 10c call
        Real k_c10 = ssFinal.strikeByDelta(0.10, Option::Call);
        quotes.push_back(interpNcp(k_c10));

        // create and return the smile section
        return Handle<FxSmileSection>(makeFxSmileSection(this->smileModel(), expiry, this->spot(),
                                             ext::make_shared<FxDeltaVolQuotes>(quotes),
                                             this->foreignDiscountCurve(),
                                             this->domesticDiscountCurve(),
                                             ssInit.deltaType(), ssInit.atmType(),
                                             this->settlement(),
                                             this->referenceDate()));
    }

}
