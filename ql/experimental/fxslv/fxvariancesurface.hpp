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
#include <ql/experimental/fxslv/fxsettlementforeigncurve.hpp>
#include <ql/experimental/fxslv/tradingtimetermstructure.hpp>
#include <ql/time/calendars/weekendsonly.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/math/interpolation.hpp>
#include <ql/experimental/fxslv/fxsmilesection.hpp>
#include <ql/experimental/fxslv/fxcostsmilesection.hpp>
#include <ql/quote.hpp>
#include <ql/pricingengines/blackdeltacalculator.hpp>
#include <ql/pricingengines/blackformula.hpp>
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
         * a parametric form for assigning trading time to each day (in the simplest case this
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
                          const FxSettleConvention& settlement,
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

        //! Forward for an option expiring on the given date.
        /*! It runs from the spot date of the reference date to the
            delivery date of the expiry, as in the smile sections.
        */
        Real fwd(const Date& expiry) const;
        //! Forward for an option expiring at a time from the reference date.
        /*! It is the forward of the expiry day containing the time, since
            every option expiring that day delivers on the same date; it
            steps at midnight.  An FxSettlementForeignCurve gives a
            Black-Scholes process the same forward.
        */
        Real fwd(Time t) const;

        // Introspection
        Handle<Quote> spot() const { return spot_; };
        const FxSettleConvention& settlement() const { return settlement_; }
        Handle<YieldTermStructure> domesticDiscountCurve() const { return domTs_; };
        Handle<YieldTermStructure> foreignDiscountCurve() const { return forTs_; };

      protected:
        //! Foreign and domestic discount factors from the spot date to the delivery date.
        std::pair<DiscountFactor, DiscountFactor> settlementDiscounts(const Date& expiry) const;

      private:
        // the nodes either side of a time: points of the grid, unless a pillar is closer
        struct Bracket {
            long index;      // grid point at or before the time
            Time ta, tb;     // times of the nodes
            Integer pa, pb;  // pillars at the nodes, or -1
        };
        Bracket bracket(Time t) const;
        // expiry date and time of a point of the grid
        Date nodeDate(long index) const;
        Time nodeTime(long index) const { return timeFromReference(nodeDate(index)); }
        Real nodeForward(Integer pillar, long index) const;
        void registerWithMarketData();
        // recomputes the pillar times, if stale
        void checkTradingTimes() const;
        // trading time to a time from the surface's reference date
        Time tradingTime(Time t) const;
        // total variance at a node: the reference date, a pillar or a point of the grid
        Real nodeVariance(Integer pillar, long index, Real strike) const;
        // interpolated smile at a point of the grid, built once and cached
        const Handle<T>& gridSmileSection(long index) const;

        // a smile expiring on the given date, at the given time from the reference date
        virtual Handle<T> interpolatedSmileSection(const Date& expiry, Time t, Real wInit, const T& ssInit,
                                                   Real wFinal, const T& ssFinal) const = 0;
        Real blackVarianceImpl(Time t, Real strike) const;  // smile variance

        const Handle<Quote> spot_;
        const FxSettleConvention settlement_;
        const std::vector<Date> pillars_;
        const Handle<YieldTermStructure> forTs_;
        const Handle<YieldTermStructure> domTs_;
        const Handle<TradingTimeTermStructure> timeTs_;

        Time nodeSpacing_;  // calendar time between points of the grid
        Date maxDate_;
        // computed lazily, after a notification, see checkTradingTimes()
        mutable bool tradingTimesStale_ = true;
        mutable std::vector<Time> tradingTimes_;  // trading times of the pillars, after a 0
        mutable std::vector<Time> pillarTimes_;   // calendar times of the pillars

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
                                            const FxSettleConvention& settlement,
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
            auto section = ext::make_shared<T>(pillars[j], spot_,
                                               ext::make_shared<FxRrBfQuotes>(atms[j], rrs[j], bfs[j], 
                                                                                deltas, flyType), 
                                               forTs_, domTs_, deltaType, atmType, Actual365Fixed(), 
                                               settlement, referenceDate);
            registerWith(section);
            smileSections_.push_back(section);
        }

        registerWithMarketData();
    }

    template<class T>
    inline void fxVarianceSurface<T>::update()
    {
        // recomputing here could throw in the middle of a notification,
        // e.g. when the evaluation date moves; do it when next needed
        smileCache_.clear();
        tradingTimesStale_ = true;
        BlackVarianceTermStructure::update();
    }

    template <class T>
    Real fxVarianceSurface<T>::atmVar(Date d) const {
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
        // each interpolated smile is built for an expiry date.  Both nodes
        // are taken at the same forward moneyness, since the forward keeps
        // moving with the carry when trading time does not.
        const Bracket b = bracket(t);
        const Real fa = nodeForward(b.pa, b.index);
        const Time tauA = tradingTime(b.ta), tauB = tradingTime(b.tb);
        if (tauB <= tauA)
            return nodeVariance(b.pa, b.index, strike * fa / fwd(t));  // no trading time between
        const Real fb = nodeForward(b.pb, b.index + 1);
        const Real moneyness = strike / fwd(t);
        const Real wa = nodeVariance(b.pa, b.index, moneyness * fa);
        const Real wb = nodeVariance(b.pb, b.index + 1, moneyness * fb);
        const Real lambda = (tradingTime(t) - tauA) / (tauB - tauA);
        return wa + lambda * (wb - wa);
    }

    template <class T>
    typename fxVarianceSurface<T>::Bracket fxVarianceSurface<T>::bracket(Time t) const {
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

    template <class T>
    Date fxVarianceSurface<T>::nodeDate(long index) const {
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

    template <class T>
    Real fxVarianceSurface<T>::nodeForward(Integer pillar, long index) const {
        return fwd(pillar >= 0 ? pillars_[pillar] : nodeDate(index));
    }

    template <class T>
    std::pair<DiscountFactor, DiscountFactor>
    fxVarianceSurface<T>::settlementDiscounts(const Date& expiry) const {
        const Date spotDate = settlement_.valueDate(referenceDate());
        const Date deliveryDate = settlement_.valueDate(expiry);
        return {forTs_->discount(deliveryDate) / forTs_->discount(spotDate),
                domTs_->discount(deliveryDate) / domTs_->discount(spotDate)};
    }

    template <class T>
    Real fxVarianceSurface<T>::fwd(const Date& expiry) const {
        const auto [dfor, ddom] = settlementDiscounts(expiry);
        return spot_->value() * dfor / ddom;
    }

    template <class T>
    Real fxVarianceSurface<T>::fwd(Time t) const {
        return fwd(detail::dayContaining(*this, t));
    }

    template <class T>
    Time fxVarianceSurface<T>::tradingTime(Time t) const {
        // the trading-time curve measures times from its own reference
        // date, so both must agree; a floating curve under a fixed surface
        // would drift apart as the evaluation date moves
        QL_REQUIRE(timeTs_->referenceDate() == referenceDate(),
                   "trading-time reference date (" << timeTs_->referenceDate()
                       << ") differs from the variance surface's (" << referenceDate() << ")");
        return timeTs_->tradingTime(t);
    }

    template <class T>
    Real fxVarianceSurface<T>::nodeVariance(Integer pillar, long index, Real strike) const {
        if (pillar >= 0)
            return smileSections_[pillar]->variance(strike);
        if (tradingTime(nodeTime(index)) <= 0.0)
            return 0.0;  // the reference date, or no trading time since
        return gridSmileSection(index)->variance(strike);
    }

    template <class T>
    const Handle<T>& fxVarianceSurface<T>::gridSmileSection(long index) const {
        auto it = smileCache_.find(index);
        if (it != smileCache_.end())
            return it->second;

        const Date expiry = nodeDate(index);
        const Time t = nodeTime(index);
        const Time tau = tradingTime(t);
        Handle<T> section;
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

    template <class T>
    void fxVarianceSurface<T>::registerWithMarketData() {
        // the smile sections are registered with as they are built
        registerWith(spot_);
        registerWith(forTs_);
        registerWith(domTs_);
        registerWith(timeTs_);
    }

    template <class T>
    void fxVarianceSurface<T>::checkTradingTimes() const {
        if (!tradingTimesStale_)
            return;
        tradingTimes_ = std::vector<Time>(pillars_.size() + 1); // these are market times, NOT real times!
        tradingTimes_[0] = 0.0;
        for (Size j = 1; j <= pillars_.size(); j++) {
            tradingTimes_[j] = timeTs_->tradingTime(referenceDate(), pillars_[j - 1]);
        }
        pillarTimes_.resize(pillars_.size());
        for (Size j = 0; j < pillars_.size(); ++j)
            pillarTimes_[j] = timeFromReference(pillars_[j]);
        tradingTimesStale_ = false;
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
                               const FxSettleConvention& settlement,
                               const Calendar& cal = WeekendsOnly(),
                               BusinessDayConvention bdc = Following,
                               bool forceMonotoneVariance = true,
                               Time nodeSpacing = 1.0 / 365);

      private:
        Handle<T> interpolatedSmileSection(const Date& expiry, Time t, Real wInit, const T& ssInit,
                                           Real wFinal, const T& ssFinal) const override;
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
                                                      const FxSettleConvention& settlement,
                                                      const Calendar& cal,
                                                      BusinessDayConvention bdc,
                                                      bool forceMonotoneVariance,
                                                      Time nodeSpacing)
    : fxVarianceSurface<T>(referenceDate, spot, pillars, atms, rrs, bfs, deltas, forTs, domTs,
                           timeTs, deltaType, atmType, flyType, settlement, cal, bdc, forceMonotoneVariance,
                           nodeSpacing) {}


    template <class T>
    Handle<T> fxVarianceSurfaceClark<T>::interpolatedSmileSection(const Date& expiry,
                                                                  Time t,
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
        return Handle<T>(ext::make_shared<T>(expiry, this->spot(),
                                             // butterflies built from smile vols are smile strangles
                                             ext::make_shared<FxRrBfQuotes>(
                                                 atm, std::vector<Handle<Quote>>{rr25, rr10},
                                                 std::vector<Handle<Quote>>{bf25, bf10}, std::vector<Real>{0.25, 0.10},
                                                 FxRrBfQuotes::SmileStrangle),
                                             this->foreignDiscountCurve(),
                                             this->domesticDiscountCurve(),
                                             ssInit.deltaType(), ssInit.atmType(),
                                             this->dayCounter(), this->settlement(),
                                             this->referenceDate()));
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
                             const FxSettleConvention& settlement,
                             const Calendar& cal = WeekendsOnly(),
                             BusinessDayConvention bdc = Following,
                             bool forceMonotoneVariance = true,
                             Time nodeSpacing = 1.0 / 365);

      private:
        Handle<T> interpolatedSmileSection(const Date& expiry, Time t, Real wInit, const T& ssInit,
                                           Real wFinal, const T& ssFinal) const override;
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
                                                  const FxSettleConvention& settlement,
                                                  const Calendar& cal,
                                                  BusinessDayConvention bdc,
                                                  bool forceMonotoneVariance,
                                                  Time nodeSpacing)
    : fxVarianceSurface<T>(referenceDate, spot, pillars, atms, rrs, bfs, deltas, forTs, domTs,
                           timeTs, deltaType, atmType, flyType, settlement, cal, bdc, forceMonotoneVariance,
                           nodeSpacing) {}


    template <class T>
    Handle<T> fxVarianceSurfaceNCP<T>::interpolatedSmileSection(const Date& expiry,
                                                                Time t,
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

            return Handle<T>(ext::make_shared<T>(expiry, this->spot(),
                                                 // butterflies built from smile vols are smile strangles
                                                 ext::make_shared<FxRrBfQuotes>(
                                                     atm, std::vector<Handle<Quote>>{rr25, rr10},
                                                     std::vector<Handle<Quote>>{bf25, bf10}, std::vector<Real>{0.25, 0.10},
                                                     FxRrBfQuotes::SmileStrangle),
                                                 this->foreignDiscountCurve(),
                                                 this->domesticDiscountCurve(),
                                                 ssInit.deltaType(), ssInit.atmType(),
                                                 this->dayCounter(), this->settlement(),
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
        return Handle<T>(ext::make_shared<T>(expiry, this->spot(),
                                             ext::make_shared<FxDeltaVolQuotes>(quotes),
                                             this->foreignDiscountCurve(),
                                             this->domesticDiscountCurve(),
                                             ssInit.deltaType(), ssInit.atmType(),
                                             this->dayCounter(), this->settlement(),
                                             this->referenceDate()));
    }
    //@}


}  // namespace QuantLib

#endif
