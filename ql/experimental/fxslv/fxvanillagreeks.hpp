/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 Raghav Subbarao

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

/*! \file fxvanillagreeks.hpp
    \brief FX vanilla option risk metrics via bump-and-reval finite differences.
*/

#ifndef quantlib_fx_vanilla_greeks_hpp
#define quantlib_fx_vanilla_greeks_hpp

#include <ql/instruments/vanillaoption.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/experimental/fxslv/fxlocalvolsurface.hpp>
#include <ql/experimental/fxslv/fxpairmarket.hpp>
#include <ql/termstructures/volatility/equityfx/fixedlocalvolsurface.hpp>
#include <iostream>
#include <utility>
#include <vector>

namespace QuantLib {

    //! Premium-adjusted delta-type greeks of an FX vanilla position.
    /*!
        When the premium is paid in the foreign (base) currency, e.g. USD/JPY
        with the premium in USD, or EUR/USD with the premium in EUR, the
        value that matters is the value in that currency, \f$ W = V/S \f$, and
        the delta is the base-currency amount that hedges it:
        \f$ S\,\partial W/\partial S = \partial V/\partial S - V/S \f$, the
        plain delta less the premium, which is itself a spot exposure.  In
        the units of FxVanillaGreeks:
        \li \c spotDelta \f$ = N(\partial V/\partial S - V/S) \f$,
              i.e. FxVanillaGreeks::spotDelta \f$ - \f$ \c npv / \f$ S \f$;
        \li \c fwdDelta  \f$ = \f$ \c spotDelta / \f$ B_f \f$, the same
              hedge in forwards to the option's delivery, which is
              \f$ N F\,\partial(V_F/F)/\partial F \f$ on the forward value
              \f$ V_F = V/B_d \f$; (K/F) N(d2) for a unit call under Black;
        \li \c spotGamma: the change of \c spotDelta per 1% spot move,
              \f$ N(\partial^2 V/\partial S^2 - (\partial V/\partial S)/S
              + V/S^2)\,0.01\,S \f$;
        \li \c fwdGamma  \f$ = \f$ \c spotGamma / \f$ B_f \f$, the change
              of \c fwdDelta per 1% forward move;
        \li \c vanna: the change of \c spotDelta per 1% vol move, i.e.
              FxVanillaGreeks::vanna \f$ - \f$ \c vega / \f$ S \f$.

        Unlike the plain ones, these greeks do not satisfy simple put-call
        parity: since \f$ C - P = S B_f - K B_d \f$, the call's
        premium-adjusted spot delta exceeds the put's by \f$ N K B_d/S \f$,
        and its gamma falls short of the put's by \f$ 0.01\,N K B_d/S \f$.
        A call's premium-adjusted gamma is negative deep in the money.

        Values, vega and the other greeks do not depend on the premium
        currency, and are those of FxVanillaGreeks.
    */
    struct FxPremiumAdjustedGreeks {
        Real spotDelta;
        Real fwdDelta;
        Real spotGamma;
        Real fwdGamma;
        Real vanna;
    };

    //! Risk metrics for a single FX vanilla option position.
    /*!
        Values are in domestic currency and deltas are amounts of foreign
        (base) currency, all for the position's \c notional \f$ N \f$ in
        foreign currency.  Deltas are exposures: a positive delta is long
        foreign currency, hedged by selling that amount.

        Units, with EUR/USD as example (notional in EUR, premium in USD):
        \li \c npv        — USD, present value
        \li \c spotDelta  — EUR, \f$ N\,\partial V/\partial S \f$: the amount
                              of EUR, bought or sold spot, that hedges the
                              position
        \li \c fwdDelta   — EUR, the same hedge in forwards to the option's
                              delivery, \f$ N\,\partial V_F/\partial F \f$ on
                              the forward value \f$ V_F = V/B_d \f$, i.e.
                              \c spotDelta / \f$ B_f \f$; N(d1) per unit of
                              call notional under Black
        \li \c spotGamma  — EUR, the change of \c spotDelta per 1% spot move
        \li \c fwdGamma   — EUR, the change of \c fwdDelta per 1% forward
                              move (= \c spotGamma / \f$ B_f \f$)
        \li \c theta      — USD over the roll of the market to its next business
                              day (see FxPairMarket::rolled())
        \li \c vega       — USD per 1% (100 bp) parallel ATM vol move
        \li \c rega       — USD per 0.1% 1M-equivalent RR move
        \li \c sega       — USD per 0.01% 1M-equivalent BF move
        \li \c vanna      — EUR, the change of \c spotDelta per 1% ATM vol move
        \li \c navva      — USD, the change of \c vega per 1% spot move
        \li \c volga      — USD, the change of \c vega per 1% ATM vol move
        \li \c premiumAdjusted — delta, gamma and vanna for a premium paid
                              in EUR (see FxPremiumAdjustedGreeks)

        Here \f$ B_d \f$ and \f$ B_f \f$ are the domestic and settlement
        foreign discount factors to the expiry, with the forward
        \f$ F = S B_f/B_d \f$.

        \note \c vanna and \c navva are the same mixed derivative
        \f$ \partial^2 V/(\partial S\,\partial\sigma) \f$ in different units:
        vanna is the vol sensitivity of the delta, in EUR; navva the spot
        sensitivity of the vega, in USD per 1% spot move.  So
        \f$ \text{navva} = 0.01\,S \times \text{vanna} \f$: about a hundredth
        of it for EUR/USD, 1.5 times it for USD/JPY at 150.
    */
    struct FxVanillaGreeks {
        Real npv;
        Real spotDelta;
        Real fwdDelta;
        Real spotGamma;
        Real fwdGamma;
        Real theta;
        Real vega;
        Real rega;
        Real sega;
        Real vanna;
        Real navva;
        Real volga;
        //! The delta-type greeks with the premium in foreign currency.
        FxPremiumAdjustedGreeks premiumAdjusted;
    };

    //! FX vanilla option risk calculator via bump-and-reval.
    /*!
        Prices a \c VanillaOption on an FX pair's market (FxPairMarket) and
        computes all standard FX option risk metrics using symmetric finite
        differences.  Each bump is a new market, built from the bumped
        quotes (FxPairMarket::withSpot(), FxPairMarket::withQuotes()), so
        that nothing is mutated: the market given, and anything else built
        from it, is left as it was.  Pricing is on the market's
        trading-time clock (FxPairMarket::blackProcess(),
        FxPairMarket::localVolProcess()).

        Two pricing modes are supported (selected at call time):
        - <b>Analytical Black-Scholes</b> (\c localVol=false): the implied vol is
          looked up from the FX variance surface at the option's (T, K) via
          interpolation; all Greeks are computed with the Black-Scholes formula.
          No finite-difference grid is used and no Dupire computation occurs.
        - <b>Local-vol Dupire FD</b> (\c localVol=true)

        The \c StickyType parameter controls how the volatility surface behaves
        when spot is bumped for Greek calculation:
        - <b>StickyType::Delta</b> (default): the FX variance surface re-anchors
          in delta space when spot moves (market convention), and the local vol
          is rebuilt from it.
        - <b>StickyType::Strike</b>: implied vols at fixed strikes do not move
          with spot.  A vanilla's price depends only on its own implied vol, so
          its greeks are Black's at that vol held as spot moves, whichever the
          \c localVol flag: delta and gamma at the option's implied vol,
          vega, vanna and volga with it read from the bumped surface, theta
          from the Black roll.
        - <b>StickyType::LocalVol</b>: the Dupire local vol is frozen in (t,K)
          space via \c FixedLocalVolSurface before bumping: the local-vol
          model's own delta.  It is not sticky strike: under local vol, implied
          vols at a fixed strike move with spot by about the skew
          \f$ \partial\sigma/\partial K \f$, the opposite of sticky delta's
          \f$ -\partial\sigma/\partial K \f$ (Hagan et al., 2002), so that with
          a put skew a call's local-vol delta is below its sticky-strike (Black)
          delta, and its sticky-delta one above.  Only effective when
          \c localVol=true.

        <b>Scaling conventions:</b>
        Greeks are reported per the standard FX desk conventions:
        - Deltas are hedge amounts in foreign currency; gammas are their
          changes per 1% move in the respective rate (see FxVanillaGreeks).
        - Vega, vanna, volga are per 1% (100 bp) absolute vol move.  They take
          central differences over a parallel bump of the ATM quotes, by
          default 1%: volga is a second difference, and with local vol smaller
          bumps leave it to the noise of the finite-difference and Dupire
          numerics, which it amplifies by \f$ 1/\text{volBump}^2 \f$ (at 0.1%, a
          volga of order 10 per 1% for a 3M ATM option, of either sign).
        - Rega bumps each RR pillar by \f$ \text{rrBump} \times \sqrt{T_{1M}/T_i} \f$,
          so the 1M pillar moves by \c rrBump (default 10 bp) and longer tenors
          move by a smaller amount proportional to \f$ 1/\sqrt{T} \f$.
        - Sega bumps each BF pillar similarly with \c bfBump (default 10 bp at 1M).
        - Theta is the P&L of the roll of the market to the next business
          day of its calendar, the option repriced on the rolled market
          (FxPairMarket::rolled()).  Holding today's surface fixed while the
          date moves is not defined: the surface is anchored to today's date.

        \ingroup vanillaengines
    */
    class FxVanillaBumpRisk {
      public:
        //! Controls how the vol surface behaves when spot is bumped.
        enum class StickyType {
            //! Sticky-delta (market convention): the FX variance surface
            //! re-anchors in delta space when spot moves.
            Delta,
            //! Sticky-strike: implied vols at fixed strikes do not move with
            //! spot; Black greeks at the option's implied vol.
            Strike,
            //! Local vol frozen: the Dupire local vol grid is frozen in (t,K)
            //! space before bumping spot, giving the local-vol model's own
            //! delta.  Only effective when \c localVol=true.
            LocalVol
        };

        /*! \param option            Vanilla option to price.
            \param market            the pair's market; its as-of date must be
                                     the evaluation date when calculating.
            \param notional          Option notional in foreign currency.
            \param spotBump          Relative spot bump for delta/gamma (e.g. 0.001).
            \param volBump           Absolute ATM vol bump for vega/vanna/volga
                                     (see above for why it is not smaller).
            \param rrBump            Reference (1M) RR bump for rega; other tenors
                                     scale as \f$ \text{rrBump}\,\sqrt{T_{1M}/T_i}\f$.
            \param bfBump            Reference (1M) BF bump for sega.
            \param tGrid             FD time steps.
            \param xGrid             FD spot grid points.
            \param lvTimePts         Time points in the auto-sized local vol grid
                                     (frozen local-vol mode only).
            \param lvStrikePts       Strike points in the local vol grid.
            \param lvStrikeSpread    Half-width of local vol strike grid in units of
                                     \f$ \sigma_{ATM}\sqrt{T} \f$.
        */
        FxVanillaBumpRisk(ext::shared_ptr<VanillaOption> option,
                          FxPairMarket market,
                          Real notional = 1.0e6,
                          Real spotBump = 0.001,
                          Real volBump = 0.01,
                          Real rrBump = 0.001,
                          Real bfBump = 0.001,
                          Size tGrid = 100,
                          Size xGrid = 100,
                          Size lvTimePts = 25,
                          Size lvStrikePts = 50,
                          Real lvStrikeSpread = 3.5);

        //! Use the given volatility where Dupire local vol is not defined.
        /*! By default there is none, and pricing fails there; the number
            of substitutions in the base market's local vol is reported by
            localVolFallbacks().
        */
        void setLocalVolFallback(Volatility fallbackVol);
        //! Number of times the base market's local vol used the fallback.
        Size localVolFallbacks() const { return localVol_->fallbacks(); }

        //! Compute all Greeks via bump-and-reval.
        FxVanillaGreeks calculate(bool localVol = true,
                                  StickyType sticky = StickyType::Delta) const;

        //! Build a \c FixedLocalVolSurface by sampling the Dupire local vol on a grid.
        ext::shared_ptr<FixedLocalVolSurface>
        buildFixedLocalVolSurface(const std::vector<Time>& times,
                                  const std::vector<Real>& strikes) const;

        //! Print a table comparing Dupire vs. \c FixedLocalVolSurface local vol values.
        void printLocalVolComparison(const std::vector<Time>& times,
                                     const std::vector<Real>& strikes,
                                     std::ostream& out = std::cout) const;

        const FxPairMarket& market() const { return market_; }

      private:
        // the finite-difference grid for the option in the given market
        ext::shared_ptr<FdmMesher>
        makeMesher(const ext::shared_ptr<GeneralizedBlackScholesProcess>& proc) const;
        // value of the position in a market, with Black or local vol on the given grid
        Real value(const FxPairMarket& market,
                   bool localVol,
                   const ext::shared_ptr<FdmMesher>& mesher) const;
        // value of the position with the given process (local vol frozen or Black)
        Real value(const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
                   bool localVol,
                   const ext::shared_ptr<FdmMesher>& mesher) const;
        // P&L of the roll of the market to its next business day, from today's value
        Real rollTheta(bool localVol, Real todaysValue) const;

        std::pair<std::vector<Time>, std::vector<Real>> buildLvGrid() const;
        ext::shared_ptr<FixedLocalVolSurface>
        frozenLocalVol(const FxPairMarket& market,
                       const std::vector<Time>& times,
                       const std::vector<Real>& strikes) const;

        ext::shared_ptr<VanillaOption> option_;
        FxPairMarket market_;
        Real notional_;
        Real spotBump_, volBump_, rrBump_, bfBump_;
        Size tGrid_, xGrid_;
        Size lvTimePts_, lvStrikePts_;
        Real lvStrikeSpread_;
        Volatility localVolFallback_ = Null<Volatility>();
        // the base market's local vol, for the fallback count and sampling
        ext::shared_ptr<FxLocalVolSurface> localVol_;
    };

}

#endif
