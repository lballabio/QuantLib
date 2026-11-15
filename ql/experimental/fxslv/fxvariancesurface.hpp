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

#include <ql/experimental/fxslv/conventions/fxexpirycut.hpp>
#include <ql/experimental/fxslv/conventions/fxsettlementconvention.hpp>
#include <ql/experimental/fxslv/smilesections/fxsmilesection.hpp>
#include <ql/experimental/fxslv/smilesections/fxsmilesectionfactory.hpp>
#include <ql/experimental/fxslv/tradingtime/tradingtimetermstructure.hpp>
#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/calendars/weekendsonly.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <map>
#include <vector>

namespace QuantLib {

    //! FX Black variance surface built from smile sections at pillar expiries
    /*! Total variance is interpolated linearly in trading time, not in
        calendar time, between the pillars and the points of a daily grid,
        so that weekends, holidays and events, which are jumps in trading
        time, shape the term structure; all quotes are first converted to
        total variances with calendar times, as they are quoted.

        The smile at each pillar is a section of the given model
        (FxSmileModel), built by makeFxSmileSection(); the surface holds
        them as FxSmileSection, so the model is chosen at run time.  How
        smiles are interpolated between pillars is up to the subclass:
        FxVarianceSurfaceNCP (in normalised call price) or
        FxVarianceSurfaceClark (flat forward variance at fixed deltas).

        The quotes refer to options expiring at the given cut, and the
        events of the trading time must be placed for it, as
        makeFxEventTradingTime() does when given the same cut: an option
        expiring on an event's date then sees the event only if it
        precedes the cut.

        The surface's time axis is trading time: its day counter is a
        TradingTimeDayCounter on its clock, so that blackVariance(t, K)
        takes a trading time and Dupire local vol on it is smooth over
        weekends and events.  Total variance at a date is the same
        whatever the axis; blackVol() is per year of trading time,
        marketVol() per year of calendar time, as quoted.  Processes on
        the surface need curves on the same axis
        (TradingTimeYieldTermStructure); FxCalendarTimeVolSurface gives
        the calendar axis.
    */
    class FxVarianceSurface : public BlackVarianceTermStructure {

      public:
        FxVarianceSurface(FxSmileModel model,
                          const Date& referenceDate,
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
                          const FxExpiryCut& cut,
                          const Calendar& cal = WeekendsOnly(),
                          BusinessDayConvention bdc = Following,
                          bool forceMonotoneVariance = true,
                          Time nodeSpacing = 1.0 / 365);

        //! \name Observer interface
        //@{
        void update() override;
    
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
        //! Forward for an option expiring at a trading time from the reference date.
        /*! It is the forward of the expiry day containing the time, since
            every option expiring that day delivers on the same date; it
            steps at midnight.  An FxSettlementForeignCurve gives a
            Black-Scholes process the same forward.
        */
        Real fwd(Time t) const;

        // Introspection
        //! The model of the smile sections.
        FxSmileModel smileModel() const { return model_; }
        Handle<Quote> spot() const { return spot_; };
        //! The calibrated smile sections at the pillars.
        const std::vector<ext::shared_ptr<FxSmileSection>>& pillarSmileSections() const {
            return smileSections_;
        }
        const std::vector<Date>& pillarDates() const { return pillars_; }
        ext::shared_ptr<FxSmileSection> pillarSmileSection(Size pillar) const {
            QL_REQUIRE(pillar < smileSections_.size(), "no pillar " << pillar);
            return smileSections_[pillar];
        }
        const FxSettleConvention& settlement() const { return settlement_; }
        //! The cut the quotes, pillars and trading-time events refer to.
        const FxExpiryCut& expiryCut() const { return cut_; }

        Volatility marketVol(const Date& expiry, Real strike) const;
        const Handle<TradingTimeTermStructure>& tradingTimeStructure() const {
            return timeTs_;
        }
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
        Time nodeTime(long index) const { return calendarTime(nodeDate(index)); }
        // calendar time (Actual/365 Fixed) of a date from the reference date, on
        // which the nodes, the pillars and the smile sections are placed
        Time calendarTime(const Date& d) const {
            return Actual365Fixed().yearFraction(referenceDate(), d);
        }
        // forward of the expiry day containing a calendar time
        Real calendarFwd(Time t) const;
        Real nodeForward(Integer pillar, long index) const;
        void registerWithMarketData();
        // recomputes the pillar times, if stale
        void checkTradingTimes() const;
        // trading time to a time from the surface's reference date
        Time tradingTime(Time t) const;
        // total variance at a node: the reference date, a pillar or a point of the grid
        Real nodeVariance(Integer pillar, long index, Real strike) const;
        // interpolated smile at a point of the grid, built once and cached
        const Handle<FxSmileSection>& gridSmileSection(long index) const;

        // a smile expiring on the given date, at the given time from the reference date
        virtual Handle<FxSmileSection> interpolatedSmileSection(const Date& expiry, Time t, Real wInit, const FxSmileSection& ssInit,
                                                   Real wFinal, const FxSmileSection& ssFinal) const = 0;
        Real blackVarianceImpl(Time t, Real strike) const override;  // smile variance
        // smile variance at a calendar time and its trading time
        Real varianceAt(Time t, Time tau, Real strike) const;

        const FxSmileModel model_;
        const Handle<Quote> spot_;
        const FxSettleConvention settlement_;
        const FxExpiryCut cut_;
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

        mutable std::vector<ext::shared_ptr<FxSmileSection>> smileSections_;

        // interpolated smile sections at the points of the grid, keyed by index
        mutable std::map<long, Handle<FxSmileSection>> smileCache_;

    };

    //! FX variance surface interpolating smiles in flat forward variance at fixed deltas [Clark]
    class FxVarianceSurfaceClark : public FxVarianceSurface {
      public:
        FxVarianceSurfaceClark(FxSmileModel model,
                               const Date& referenceDate,
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
                               const FxExpiryCut& cut,
                               const Calendar& cal = WeekendsOnly(),
                               BusinessDayConvention bdc = Following,
                               bool forceMonotoneVariance = true,
                               Time nodeSpacing = 1.0 / 365);

      private:
        Handle<FxSmileSection> interpolatedSmileSection(const Date& expiry, Time t, Real wInit, const FxSmileSection& ssInit,
                                           Real wFinal, const FxSmileSection& ssFinal) const override;
    };

    //! FX variance surface interpolating smiles in normalised call price [Gope, Fries 2011]
    class FxVarianceSurfaceNCP : public FxVarianceSurface {
      public:
        FxVarianceSurfaceNCP(FxSmileModel model,
                             const Date& referenceDate,
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
                             const FxExpiryCut& cut,
                             const Calendar& cal = WeekendsOnly(),
                             BusinessDayConvention bdc = Following,
                             bool forceMonotoneVariance = true,
                             Time nodeSpacing = 1.0 / 365);

      private:
        Handle<FxSmileSection> interpolatedSmileSection(const Date& expiry, Time t, Real wInit, const FxSmileSection& ssInit,
                                           Real wFinal, const FxSmileSection& ssFinal) const override;
    };

}

#endif
