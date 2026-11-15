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

/*! \file fxpairmarket.hpp
    \brief The market of an FX pair on a date: spot, curves, vol quotes and what is built from them
*/

#ifndef quantlib_fx_pair_market_hpp
#define quantlib_fx_pair_market_hpp

#include <ql/experimental/fxslv/conventions/fxpairconventions.hpp>
#include <ql/experimental/fxslv/fxeventvariance.hpp>
#include <ql/experimental/fxslv/fxvariancesurface.hpp>
#include <ql/experimental/fxslv/fxvolsurfacebuilder.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <utility>
#include <vector>

namespace QuantLib {

    //! ATM, risk-reversal and butterfly vols of a smile at one expiry
    /*! The butterflies are smile strangles; risk reversals and butterflies
        are by delta, in the order of the market's quoted deltas.
    */
    struct FxExpiryQuotes {
        Volatility atm;
        std::vector<Volatility> riskReversals, butterflies;
    };

    //! The market of an FX pair on a date
    /*! The pair's market data (spot, foreign and domestic curves, vol
        quotes by tenor and events), its conventions and the choices the
        vol surface is built with, together with what is built from them:
        the vol surface, the forward of any expiry, and the same market on
        the trading-time clock for local vol.  Every forward comes from
        fxSettlementDiscounts() on these curves, so that the surface, the
        processes and any analytics built from this market agree on it.

        The market is immutable: a change of market data is a new market
        (withSpot(), withCurves(), withQuotes(), withEvents(), rolled()),
        so that nothing built from this one goes stale.  It is meant to be
        built by an application's market (e.g. one that bundles several
        pairs and bootstraps their curves consistently, see
        bootstrapFxCurve()), and passed to the analytics: surfaces, the
        P&L explain and the greeks.

        What is built is built once, on first use, and shared by copies.
        Dates are fixed: the surface's reference date is the as-of date,
        whatever the evaluation date.  The curves are given; they should
        be fixed-date too.
    */
    class FxPairMarket {
      public:
        /*! \param asOf          the market's date.
            \param conventions   the pair's conventions.
            \param spot          spot rate, domestic per unit of foreign.
            \param foreignCurve  discount curve of the foreign currency.
            \param domesticCurve discount curve of the domestic currency,
                                 on the same collateral as the foreign one.
            \param quotes        vol quotes, with the tenors and deltas quoted.
            \param events        events with their variances, if any.
            \param config        how the vol surface is built.
        */
        FxPairMarket(const Date& asOf,
                     FxPairConventions conventions,
                     Real spot,
                     Handle<YieldTermStructure> foreignCurve,
                     Handle<YieldTermStructure> domesticCurve,
                     FxVolQuotes quotes,
                     std::vector<FxVolEvent> events = {},
                     FxVolSurfaceConfig config = FxVolSurfaceConfig());

        //! \name Market data
        //@{
        const Date& asOf() const { return data_->asOf; }
        const FxPairConventions& conventions() const { return data_->conventions; }
        Real spot() const { return data_->spot->value(); }
        //! The spot, as the quote the curves, surfaces and processes of this market use.
        const Handle<Quote>& spotQuote() const { return data_->spot; }
        const Handle<YieldTermStructure>& foreignCurve() const { return data_->foreignCurve; }
        const Handle<YieldTermStructure>& domesticCurve() const { return data_->domesticCurve; }
        const FxVolQuotes& quotes() const { return data_->quotes; }
        const std::vector<FxVolEvent>& events() const { return data_->events; }
        const FxVolSurfaceConfig& config() const { return data_->config; }
        //! The expiries of the quoted vol tenors.
        const std::vector<Date>& pillars() const { return data_->pillars; }
        //@}

        //! \name Forwards
        //@{
        //! Foreign and domestic discount factors from the spot date to the delivery of an expiry.
        std::pair<DiscountFactor, DiscountFactor> settlementDiscounts(const Date& expiry) const;
        //! The settlement forward of an expiry.
        Real forward(const Date& expiry) const;
        //@}

        //! \name Vols
        //@{
        //! The vol surface, on the trading-time axis (see FxVarianceSurface).
        const ext::shared_ptr<FxVarianceSurface>& volSurface() const;
        //! The implied vol of an expiry and strike, per year of calendar time, as quoted.
        Volatility marketVol(const Date& expiry, Real strike) const;
        //! A smile section of the market's model at an expiry, with smile-strangle quotes.
        ext::shared_ptr<FxSmileSection> smileSection(const Date& expiry,
                                                     const FxExpiryQuotes& quotes) const;
        //! The ATM, risk reversals and butterflies of the surface's smile at an expiry.
        /*! Read off the surface at the strikes at which the pair's
            conventions hold at the smile's own vols; the butterflies are
            smile strangles.
        */
        FxExpiryQuotes expiryQuotes(const Date& expiry) const;
        //@}

        //! \name The market on the trading-time clock
        /*! Local vol, and the variance and leverage of stochastic local
            vol, follow the variance surface: in calendar time they drop
            over weekends and holidays and spike at events.  On the
            trading-time clock of the surface, its own axis, the same
            market is smooth: the curves here have the same discount
            factor at every date as the given ones, with trading times.
            Prices are those of the calendar-time market for vanillas.
            Model parameters calibrated on it are per year of trading time.
        */
        //@{
        //! The trading-time clock of the surface.
        const Handle<TradingTimeTermStructure>& tradingTime() const;
        //! Year fractions in trading time.
        DayCounter tradingTimeDayCounter() const;
        const Handle<YieldTermStructure>& tradingTimeDomesticCurve() const;
        const Handle<YieldTermStructure>& tradingTimeForeignCurve() const;
        //! A Black process, with the settlement forward, for analytic pricing.
        ext::shared_ptr<GeneralizedBlackScholesProcess> blackProcess() const;
        //! A local-vol process with the settlement forward and Dupire local vol
        /*! Its local vol is an FxLocalVolSurface with the given fallback
            where local vol is not defined; with none, pricing fails there.
            The fallback is the caller's choice, not market data, so each
            call builds a new local vol.
        */
        ext::shared_ptr<GeneralizedBlackScholesProcess>
        localVolProcess(Volatility fallback = Null<Volatility>()) const;
        //@}

        //! \name Other markets
        //@{
        FxPairMarket withSpot(Real spot) const;
        FxPairMarket withCurves(Handle<YieldTermStructure> foreignCurve,
                                Handle<YieldTermStructure> domesticCurve) const;
        FxPairMarket withQuotes(FxVolQuotes quotes) const;
        FxPairMarket withEvents(std::vector<FxVolEvent> events) const;
        //! The same market, its butterflies converted to smile strangles
        /*! Broker strangles are not points on the smile: the smile points
            are read off each pillar's calibrated smile.  Smile strangles
            from different markets can be combined, broker ones cannot.
        */
        FxPairMarket smileStrangles() const;
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
        FxPairMarket rolled(const Date& next) const;
        //@}

      private:
        struct Data {
            Date asOf;
            FxPairConventions conventions;
            Handle<Quote> spot;
            Handle<YieldTermStructure> foreignCurve, domesticCurve;
            FxVolQuotes quotes;
            std::vector<FxVolEvent> events;
            FxVolSurfaceConfig config;
            std::vector<Date> pillars;
        };
        // what is built from the data, on first use
        struct Built {
            ext::shared_ptr<FxVarianceSurface> surface;
            bool tradingTimeBuilt = false;
            Handle<YieldTermStructure> ttDomestic, ttForeign;
        };
        FxPairMarket(Data data);
        void buildTradingTime() const;
        // ATM and the call and put at each delta of a pillar's smile
        std::vector<Volatility> smilePoints(Size pillar) const;
        // a smile section of the market's model and conventions at an expiry
        ext::shared_ptr<FxSmileSection> section(const FxPairConventions& conventions,
                                                const Date& expiry,
                                                Volatility atm,
                                                const std::vector<Volatility>& riskReversals,
                                                const std::vector<Volatility>& butterflies) const;
        // smile-strangle quotes from the smile points of each pillar
        FxVolQuotes quotesFromPoints(const std::vector<std::vector<Volatility>>& points) const;

        ext::shared_ptr<const Data> data_;
        ext::shared_ptr<Built> built_;
    };

}

#endif
