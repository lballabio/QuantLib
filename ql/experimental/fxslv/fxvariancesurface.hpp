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

/*! \file fxvariancesurface.hpp
    \brief FX Black variance surface built from smile sections
*/

#ifndef quantlib_fx_variance_surface_hpp
#define quantlib_fx_variance_surface_hpp

#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/experimental/fxslv/tradingtimetermstructure.hpp>
#include <ql/time/calendars/weekendsonly.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/math/interpolation.hpp>
#include <ql/experimental/fxslv/fxsmilesection.hpp>
#include <ql/experimental/fxslv/fxcostsmilesection.hpp>
#include <ql/quote.hpp>
#include <ql/pricingengines/blackdeltacalculator.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/quotes/simplequote.hpp>
#include <map>

namespace QuantLib {

    template<class T>
    class fxVarianceSurface : public BlackVarianceTermStructure {
        /*
         * Implementation of a variance term structure for FXO trading. The aim is linearly
         * interpolate variance in trading time/market time as opposed to real time! This
         * allows the addition of events which are treated as jumps in trading time.
         *
         * All quotes first have to be converted into total variances using real time. We assume
         * a paramteric form for assigning trading time to each day (in the simplest case this
         * can be a piecewise constant function between pillar tenors) Events are treated as
         * multiples of the underlying trading time.
         *
         * This needs to be derived from a subclass of BlackVolTermStructure to allow the usage
         * of the AnalyticEuropeanEngine and therefore we derive from BlackVarianceTermStructure
         * as the interpolation is done in variance space.
         */

    static_assert(std::is_base_of_v<FxSmileSection, T>,
                      "Template parameter must be derived from fxSmileSection");

      public:
        fxVarianceSurface(const Date& referenceDate,
                          const Handle<Quote>& spot,
                          const std::vector<Date>& pillars,
                          const std::vector<Handle<Quote>>& atms,
                          const std::vector<std::vector<Handle<Quote>>>& rrs,
                          const std::vector<std::vector<Handle<Quote>>>& bfs,
                          const std::vector<Real>& deltas,
                          const Handle<YieldTermStructure>& forTs,
                          const Handle<YieldTermStructure>& domTs,
                          const Handle<TradingTimeTermStructure>& timesTs,
                          DeltaVolQuote::DeltaType deltaType,
                          DeltaVolQuote::AtmType atmType,
                          FxRrBfQuotes::FlyType flyType,
                          const FxSettlementConvention& settlement,
                          const Calendar& cal = WeekendsOnly(),
                          BusinessDayConvention bdc = Following,
                          bool forceMonotoneVariance = true,
                          Time nodeSpacing = 1.0 / 365);

        //! \name Observer interface
        //@{
        void update() override;
        //@}

        virtual Rate minStrike() const override { return QL_MIN_POSITIVE_REAL; };
        virtual Rate maxStrike() const override { return QL_MAX_REAL; };
        virtual Date maxDate() const override { return maxDate_; };

        Real atmVar(Date d) const;  // atm variance
        Real atmVol(Date d) const;  // atm vol

        // helper functions
        Real fwd(Date d) const { return spot_->value() * forTs_->discount(d) / domTs_->discount(d); };
        Real fwd(Time t) const { return spot_->value() * forTs_->discount(t) / domTs_->discount(t); };

        // Introspection
        Handle<Quote> spot() const { return spot_; };
        const FxSettlementConvention& settlement() const { return settlement_; }
        Handle<YieldTermStructure> domesticDiscountCurve() const { return domTs_; };
        Handle<YieldTermStructure> foreignDiscountCurve() const { return forTs_; };
        
      private:
        void registerWithMarketData();
        void getTradingTimes();
        // total variance at a node: the reference date, a pillar or a point of the grid
        Real nodeVariance(Integer pillar, long index, Real strike) const;
        // interpolated smile at a point of the grid, built once and cached
        const Handle<T>& gridSmileSection(long index) const;

        virtual Handle<T> interpolatedSmileSection(Time t, Real wInit, const T& ssInit, Real wFinal, const T& ssFinal) const = 0;
        Real blackVarianceImpl(Time t, Real strike) const;  // smile variance

        const Handle<Quote> spot_;
        const FxSettlementConvention settlement_;
        const std::vector<Date> pillars_;
        const Handle<YieldTermStructure> forTs_;
        const Handle<YieldTermStructure> domTs_;
        const Handle<TradingTimeTermStructure> timeTs_;

        Date maxDate_;
        std::vector<Time> tradingTimes_;  // vector of trading times for pillar smile sections
        std::vector<Time> pillarTimes_;   // calendar times of the pillars
        Time nodeSpacing_;                // calendar time between points of the grid

        mutable std::vector<ext::shared_ptr<T>> smileSections_;

        // interpolated smile sections at the points of the grid, keyed by index
        mutable std::map<long, Handle<T>> smileCache_;

    };

    template <class T>
    fxVarianceSurface<T>::fxVarianceSurface(const Date& referenceDate,
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
                                            const FxSettlementConvention& settlement,
                                            const Calendar& cal,
                                            BusinessDayConvention bdc,
                                            bool forceMonotoneVariance,
                                            Time nodeSpacing)
    : BlackVarianceTermStructure(referenceDate, cal, bdc, Actual365Fixed()), spot_(spot),
      settlement_(settlement),
      pillars_(pillars), forTs_(forTs), domTs_(domTs), timeTs_(timeTs), nodeSpacing_(nodeSpacing),
      maxDate_(pillars.back()) {
        QL_REQUIRE(pillars.size() == atms.size(), "mismatch between date vector and vol vector");
        QL_REQUIRE(pillars.size() == rrs.size(), "mismatch between date vector and rr vector");
        QL_REQUIRE(pillars.size() == bfs.size(), "mismatch between date vector and bf vector");

        // cannot have dates[0] = referenceDate, since vol at dates[0] must be 0!
        QL_REQUIRE(pillars[0] > referenceDate, "cannot have dates[0] <= referenceDate");
        QL_REQUIRE(nodeSpacing_ > 0.0, "positive node spacing required: " << nodeSpacing_);

        smileSections_.clear();
        for (Size j = 0; j < atms.size(); j++) {
            // this will also register the smile section with spot, vol quotes and discount curves
            auto section = ext::make_shared<T>(pillars[j], spot_,
                                               ext::make_shared<FxRrBfQuotes>(atms[j], rrs[j], bfs[j], 
                                                                                deltas, flyType), 
                                               forTs_, domTs_, deltaType, atmType, Actual365Fixed(), 
                                               settlement, referenceDate);
            registerWith(section);
            smileSections_.push_back(section);
        }

        registerWithMarketData();
        getTradingTimes();
    }

    template<class T>
    inline void fxVarianceSurface<T>::update()
    {
        smileCache_.clear();
        BlackVarianceTermStructure::update();
        getTradingTimes();
    }

    template <class T>
    Real fxVarianceSurface<T>::atmVar(Date d) const {
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

    template <class T>
    Real fxVarianceSurface<T>::atmVol(Date d) const {
        return std::sqrt(atmVar(d) / timeFromReference(d));
    }

    template <class T>
    Real fxVarianceSurface<T>::blackVarianceImpl(Time t, Real strike) const {
        // Total variance is interpolated between nodes: the reference date,
        // the pillars and the points of a grid in calendar time.  Between
        // two nodes it is linear in trading time, so it stays flat over a
        // weekend and jumps with trading time at an event, whatever the
        // spacing; the spacing only sets how closely the smile follows the
        // interpolation between pillars.  Nodes are in calendar time since
        // each interpolated smile is built at a calendar time.  Both nodes
        // are taken at the same forward moneyness, since the forward keeps
        // moving with the carry when trading time does not.
        // a time on a point of the grid, up to rounding, is taken to be on it
        const long j = static_cast<long>(std::floor(t / nodeSpacing_ + 1.0e-9));
        Time ta = j * nodeSpacing_, tb = (j + 1) * nodeSpacing_;
        Integer pa = -1, pb = -1;  // the pillars at the nodes, if any
        for (Size i = 0; i < pillarTimes_.size(); ++i) {
            if (pillarTimes_[i] <= t && pillarTimes_[i] >= ta) {
                ta = pillarTimes_[i];
                pa = static_cast<Integer>(i);
            }
            if (pillarTimes_[i] > t && pillarTimes_[i] <= tb) {
                tb = pillarTimes_[i];
                pb = static_cast<Integer>(i);
            }
        }

        const Real moneyness = strike / fwd(t);
        const Real wa = nodeVariance(pa, j, moneyness * fwd(ta));
        const Time tauA = timeTs_->tradingTime(ta), tauB = timeTs_->tradingTime(tb);
        if (tauB <= tauA)
            return wa;  // no trading time between the nodes
        const Real wb = nodeVariance(pb, j + 1, moneyness * fwd(tb));
        const Real lambda = (timeTs_->tradingTime(t) - tauA) / (tauB - tauA);
        return wa + lambda * (wb - wa);
    }

    template <class T>
    Real fxVarianceSurface<T>::nodeVariance(Integer pillar, long index, Real strike) const {
        if (pillar >= 0)
            return smileSections_[pillar]->variance(strike);
        if (timeTs_->tradingTime(index * nodeSpacing_) <= 0.0)
            return 0.0;  // the reference date, or no trading time since
        return gridSmileSection(index)->variance(strike);
    }

    template <class T>
    const Handle<T>& fxVarianceSurface<T>::gridSmileSection(long index) const {
        auto it = smileCache_.find(index);
        if (it != smileCache_.end())
            return it->second;

        const Time t = index * nodeSpacing_;
        const Time tau = timeTs_->tradingTime(t);
        Handle<T> section;
        if (tau <= tradingTimes_[1]) {
            // before the first pillar: scale the first smile
            section = interpolatedSmileSection(t, 0.0, *smileSections_.front(),
                                               tau / tradingTimes_[1], *smileSections_.front());
        } else if (tau <= tradingTimes_.back()) {
            // between two pillars; tradingTimes_ starts at the reference
            // date, so pillar i is at tradingTimes_[i + 1]
            Size i = 1;
            while (tradingTimes_[i + 1] < tau)
                ++i;
            const Real w = (tau - tradingTimes_[i]) / (tradingTimes_[i + 1] - tradingTimes_[i]);
            section = interpolatedSmileSection(t, 1.0 - w, *smileSections_[i - 1],
                                               w, *smileSections_[i]);
        } else {
            // after the last pillar: scale the last smile
            section = interpolatedSmileSection(t, tau / tradingTimes_.back(), *smileSections_.back(),
                                               0.0, *smileSections_.back());
        }
        return smileCache_.emplace(index, section).first->second;
    }

    template <class T>
    void fxVarianceSurface<T>::registerWithMarketData() {
        // the smile sections are registered with as they are built
        registerWith(spot_);
        registerWith(forTs_);
        registerWith(domTs_);
        registerWith(timeTs_);
    }

    template <class T>
    void fxVarianceSurface<T>::getTradingTimes() {
        tradingTimes_ = std::vector<Time>(pillars_.size() + 1); // these are market times, NOT real times!
        tradingTimes_[0] = 0.0;
        for (Size j = 1; j <= pillars_.size(); j++) {
            tradingTimes_[j] = timeTs_->tradingTime(referenceDate(), pillars_[j - 1]);
        }
        pillarTimes_.resize(pillars_.size());
        for (Size j = 0; j < pillars_.size(); ++j)
            pillarTimes_[j] = timeFromReference(pillars_[j]);
    }


    //! \name fxVarianceSurfaceClark
    //@{
    template<class T>
    class fxVarianceSurfaceClark : public fxVarianceSurface<T> {
      public:
        fxVarianceSurfaceClark(const Date& referenceDate,
                               const Handle<Quote>& spot,
                               const std::vector<Date>& pillars,
                               const std::vector<Handle<Quote>>& atms,
                               const std::vector<std::vector<Handle<Quote>>>& rrs,
                               const std::vector<std::vector<Handle<Quote>>>& bfs,
                               const std::vector<Real>& deltas,
                               const Handle<YieldTermStructure>& forTs,
                               const Handle<YieldTermStructure>& domTs,
                               const Handle<TradingTimeTermStructure>& timesTs,
                               DeltaVolQuote::DeltaType deltaType,
                               DeltaVolQuote::AtmType atmType,
                               FxRrBfQuotes::FlyType flyType,
                               const FxSettlementConvention& settlement,
                               const Calendar& cal = WeekendsOnly(),
                               BusinessDayConvention bdc = Following,
                               bool forceMonotoneVariance = true,
                               Time nodeSpacing = 1.0 / 365);

      private:
        Handle<T> interpolatedSmileSection(Time t, Real wInit, const T& ssInit, Real wFinal, const T& ssFinal) const;
    };

    template <class T>
    fxVarianceSurfaceClark<T>::fxVarianceSurfaceClark(const Date& referenceDate, const Handle<Quote>& spot,
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
                                                      const FxSettlementConvention& settlement,
                                                      const Calendar& cal,
                                                      BusinessDayConvention bdc,
                                                      bool forceMonotoneVariance,
                                                      Time nodeSpacing)
    : fxVarianceSurface<T>(referenceDate, spot, pillars, atms, rrs, bfs, deltas, forTs, domTs,
                           timeTs, deltaType, atmType, flyType, settlement, cal, bdc, forceMonotoneVariance,
                           nodeSpacing) {}


    template <class T>
    Handle<T> fxVarianceSurfaceClark<T>::interpolatedSmileSection(Time t,
                                                                  Real wInit,
                                                                  const T& ssInit,
                                                                  Real wFinal,
                                                                  const T& ssFinal) const {
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
        return Handle<T>(ext::make_shared<T>(t, this->spot(),
                                             // butterflies built from smile vols are smile strangles
                                             ext::make_shared<FxRrBfQuotes>(
                                                 atm, std::vector<Handle<Quote>>{rr25, rr10},
                                                 std::vector<Handle<Quote>>{bf25, bf10}, std::vector<Real>{0.25, 0.10},
                                                 FxRrBfQuotes::SmileStrangle),
                                             this->foreignDiscountCurve(),
                                             this->domesticDiscountCurve(),
                                             ssInit.deltaType(), ssInit.atmType(),
                                             this->dayCounter()));
    }
    //@}


    //! \name fxVarianceSurfaceNCP
    //@{
    template<class T>
    class fxVarianceSurfaceNCP : public fxVarianceSurface<T> {
      public:
        fxVarianceSurfaceNCP(const Date& referenceDate,
                             const Handle<Quote>& spot,
                             const std::vector<Date>& pillars,
                             const std::vector<Handle<Quote>>& atms,
                             const std::vector<std::vector<Handle<Quote>>>& rrs,
                             const std::vector<std::vector<Handle<Quote>>>& bfs,
                             const std::vector<Real>& deltas,
                             const Handle<YieldTermStructure>& forTs,
                             const Handle<YieldTermStructure>& domTs,
                             const Handle<TradingTimeTermStructure>& timesTs,
                             DeltaVolQuote::DeltaType deltaType,
                             DeltaVolQuote::AtmType atmType,
                             FxRrBfQuotes::FlyType flyType,
                             const FxSettlementConvention& settlement,
                             const Calendar& cal = WeekendsOnly(),
                             BusinessDayConvention bdc = Following,
                             bool forceMonotoneVariance = true,
                             Time nodeSpacing = 1.0 / 365);

      private:
        Handle<T> interpolatedSmileSection(Time t, Real wInit, const T& ssInit, Real wFinal, const T& ssFinal) const;
    };


    template <class T>
    fxVarianceSurfaceNCP<T>::fxVarianceSurfaceNCP(const Date& referenceDate, const Handle<Quote>& spot,
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
                                                  const FxSettlementConvention& settlement,
                                                  const Calendar& cal,
                                                  BusinessDayConvention bdc,
                                                  bool forceMonotoneVariance,
                                                  Time nodeSpacing)
    : fxVarianceSurface<T>(referenceDate, spot, pillars, atms, rrs, bfs, deltas, forTs, domTs,
                           timeTs, deltaType, atmType, flyType, settlement, cal, bdc, forceMonotoneVariance,
                           nodeSpacing) {}


    template <class T>
    Handle<T> fxVarianceSurfaceNCP<T>::interpolatedSmileSection(Time t,
                                                                Real wInit,
                                                                const T& ssInit,
                                                                Real wFinal,
                                                                const T& ssFinal) const {
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

            return Handle<T>(ext::make_shared<T>(t, this->spot(),
                                                 // butterflies built from smile vols are smile strangles
                                                 ext::make_shared<FxRrBfQuotes>(
                                                     atm, std::vector<Handle<Quote>>{rr25, rr10},
                                                     std::vector<Handle<Quote>>{bf25, bf10}, std::vector<Real>{0.25, 0.10},
                                                     FxRrBfQuotes::SmileStrangle),
                                                 this->foreignDiscountCurve(),
                                                 this->domesticDiscountCurve(),
                                                 ssInit.deltaType(), ssInit.atmType(),
                                                 this->dayCounter()));
        }

        // Interpolation in probability space using normed call prices [Gope, Fries 2011].
        Real ddom = this->domesticDiscountCurve()->discount(t);
        Real dfor = this->foreignDiscountCurve()->discount(t);
        Real spt = this->spot()->value();

        auto interpNcp = [t, wInit, wFinal, ssInit, ssFinal, spt, ddom, dfor](Real k2) {
            Real m2 = k2 / ssFinal.forward();                // long moneyness
            Real p = ssFinal.exerciseProbability(m2);        // its exercise probability
            Real m1 = ssInit.moneynessFromProbability(p);   // short moneyness, same probability

            Real ncp1 = ssInit.normedCallPrice(m1);  // ncp at shorter smile
            Real ncp2 = ssFinal.normedCallPrice(m2); // ncp at longer smile

            // interpolated values
            Real fwd = spt * dfor / ddom;
            Real k = fwd * (wInit * m1 + wFinal * m2);     // strike
            Real c = fwd * (wInit * ncp1 + wFinal * ncp2); // prem

            Brent solver;
            solver.setMaxEvaluations(10000);
            Real iw = solver.solve([&](Real w) { return BlackCalculator(Option::Call, k, fwd, w).value() - c; },
                                    1e-12, 0.1, 0.01, 0.5);

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
        return Handle<T>(ext::make_shared<T>(t, this->spot(),
                                             ext::make_shared<FxDeltaVolQuotes>(quotes),
                                             this->foreignDiscountCurve(),
                                             this->domesticDiscountCurve(),
                                             ssInit.deltaType(), ssInit.atmType(),
                                             this->dayCounter()));
    }
    //@}


}  // namespace QuantLib

#endif
