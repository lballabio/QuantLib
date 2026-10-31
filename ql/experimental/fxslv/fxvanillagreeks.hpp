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
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/experimental/fxslv/fxlocalvolsurface.hpp>
#include <ql/termstructures/volatility/equityfx/fixedlocalvolsurface.hpp>
#include <iostream>
#include <utility>
#include <vector>

namespace QuantLib {

    //! Risk metrics for a single FX vanilla option position.
    /*!
        All monetary values are in domestic currency, scaled by \c notional.

        Reported units (using EUR/USD as example, notional in EUR):
        \li \c npv        — USD (present value)
        \li \c spotDelta  — USD per 1% spot move
        \li \c fwdDelta   — forward-USD per 1% forward move (= spotDelta / B_d)
        \li \c spotGamma  — USD (spot-delta change per 1% spot move)
        \li \c fwdGamma   — forward-USD (fwd-delta change per 1% fwd move)
        \li \c theta      — USD per calendar day
        \li \c vega       — USD per 1% (100 bp) parallel ATM vol move
        \li \c rega       — USD per 0.1% 1M-equivalent RR move
        \li \c sega       — USD per 0.01% 1M-equivalent BF move
        \li \c vanna      — USD (spot-delta change per 1% ATM vol move)
        \li \c navva      — USD (vega change per 1% spot move; equals \c vanna)
        \li \c volga      — USD (vega change per 1% ATM vol move)

        \note \c vanna and \c navva are identical numbers — they are both
        equal to \f$ \partial^2 V/(\partial S\,\partial\sigma) \f$ expressed
        in the same percent-scaled units.  The labels differ only in which
        first-order sensitivity is thought of as "changing": vanna is the
        vol-sensitivity of the delta; navva is the spot-sensitivity of the vega.
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
    };

    //! FX vanilla option risk calculator via bump-and-reval.
    /*!
        Prices a \c VanillaOption driven by a \c GeneralizedBlackScholesProcess
        backed by an FX variance surface (\c fxVarianceSurface) and computes all
        standard FX option risk metrics using symmetric finite differences.

        Two pricing modes are supported (selected at call time):
        - <b>Analytical Black-Scholes</b> (\c localVol=false): the implied vol is
          looked up from the FX variance surface at the option's (T, K) via
          interpolation; all Greeks are computed with the Black-Scholes formula.
          No finite-difference grid is used and no Dupire computation occurs.
        - <b>Local-vol Dupire FD</b> (\c localVol=true)

        When \c localVol=true, the \c StickyType parameter controls how the
        volatility surface behaves when spot is bumped for Greek calculation:
        - <b>StickyType::Delta</b> (default): the FX variance surface re-anchors
          in delta space when spot moves (market convention).
        - <b>StickyType::Strike</b>: the Dupire local vol is frozen in (t,K) space
          via \c FixedLocalVolSurface before bumping — model-consistent delta.

        <b>Scaling conventions:</b>
        Greeks are reported per the standard FX desk conventions:
        - Spot/fwd delta/gamma are per 1% move in the respective rate.
        - Vega, vanna, volga are per 1% (100 bp) absolute vol move.
        - Rega bumps each RR pillar by \f$ \text{rrBump} \times \sqrt{T_{1M}/T_i} \f$,
          so the 1M pillar moves by \c rrBump (default 10 bp) and longer tenors
          move by a smaller amount proportional to \f$ 1/\sqrt{T} \f$.
        - Sega bumps each BF pillar similarly with \c bfBump (default 10 bp at 1M).

        \ingroup vanillaengines
    */
    class FxVanillaBumpRisk {
      public:
        //! Controls how the vol surface behaves when spot is bumped.
        enum class StickyType {
            //! Sticky-delta (market convention): the FX variance surface
            //! re-anchors in delta space when spot moves.
            Delta,
            //! Sticky-strike (model-consistent): the Dupire local vol grid is
            //! frozen in (t,K) space before bumping spot.  Only effective when
            //! \c localVol=true.
            Strike
        };

        /*! \param option            Vanilla option to price.
            \param process           GBS process (spot, domestic/foreign curves,
                                     FX variance surface).  For its forward to
                                     agree with the surface's, its foreign
                                     curve should be an FxSettlementForeignCurve.
                                     Local vol is taken from an FxLocalVolSurface
                                     on its surface; where it fails, pricing fails.
            \param spotQuote         \c SimpleQuote underlying the spot handle —
                                     bumped for delta/gamma/vanna.
            \param atmVolQuotes      All pillar ATM \c SimpleQuote objects — bumped
                                     in parallel for vega, vanna and volga.
            \param rrPillarQuotes    Per-tenor RR \c SimpleQuote vectors, e.g.
                                     {{rr25_1M, rr10_1M}, {rr25_3M, rr10_3M}, ...}.
                                     May be empty (rega will be zero).
            \param rrPillarTimes     Year-fraction time for each RR pillar (one
                                     entry per outer vector of \p rrPillarQuotes).
            \param bfPillarQuotes    Per-tenor BF \c SimpleQuote vectors.
            \param bfPillarTimes     Year-fraction time for each BF pillar.
            \param notional          Option notional in foreign currency.
            \param spotBump          Relative spot bump for delta/gamma (e.g. 0.001).
            \param volBump           Absolute ATM vol bump for vega/vanna/volga.
            \param rrBump            Reference (1M) RR bump for rega; other tenors
                                     scale as \f$ \text{rrBump}\,\sqrt{T_{1M}/T_i}\f$.
            \param bfBump            Reference (1M) BF bump for sega.
            \param tGrid             FD time steps.
            \param xGrid             FD spot grid points.
            \param lvTimePts         Time points in the auto-sized local vol grid
                                     (sticky-strike mode only).
            \param lvStrikePts       Strike points in the local vol grid.
            \param lvStrikeSpread    Half-width of local vol strike grid in units of
                                     \f$ \sigma_{ATM}\sqrt{T} \f$.
        */
        FxVanillaBumpRisk(
            ext::shared_ptr<VanillaOption> option,
            ext::shared_ptr<GeneralizedBlackScholesProcess> process,
            ext::shared_ptr<SimpleQuote> spotQuote,
            std::vector<ext::shared_ptr<SimpleQuote>> atmVolQuotes,
            std::vector<std::vector<ext::shared_ptr<SimpleQuote>>> rrPillarQuotes = {},
            std::vector<Time> rrPillarTimes = {},
            std::vector<std::vector<ext::shared_ptr<SimpleQuote>>> bfPillarQuotes = {},
            std::vector<Time> bfPillarTimes = {},
            Real notional = 1.0e6,
            Real spotBump = 0.001,
            Real volBump = 0.001,
            Real rrBump = 0.001,
            Real bfBump = 0.001,
            Size tGrid = 100,
            Size xGrid = 100,
            Size lvTimePts = 25,
            Size lvStrikePts = 50,
            Real lvStrikeSpread = 3.5);

        //! Use the given volatility where Dupire local vol is not defined.
        /*! By default there is none, and pricing fails there; the number
            of substitutions is reported by localVolFallbacks().
        */
        void setLocalVolFallback(Volatility fallbackVol);
        //! Number of times the local vol fallback was used.
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

      private:
        ext::shared_ptr<PricingEngine>
        makeEngineFor(const ext::shared_ptr<GeneralizedBlackScholesProcess>& proc,
                      bool localVol) const;

        ext::shared_ptr<PricingEngine> makeEngine(bool localVol) const;
        // Dupire local vol of the process's surface, allowing periods without variance
        void buildLocalVol(Volatility fallbackVol);
        ext::shared_ptr<PricingEngine> makeAnalyticEngine() const;

        std::pair<std::vector<Time>, std::vector<Real>> buildLvGrid() const;

        ext::shared_ptr<GeneralizedBlackScholesProcess>
        makeStickyProcess(const std::vector<Time>& times,
                          const std::vector<Real>& strikes) const;

        ext::shared_ptr<VanillaOption> option_;
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        ext::shared_ptr<SimpleQuote> spotQuote_;
        std::vector<ext::shared_ptr<SimpleQuote>> atmVolQuotes_;
        std::vector<std::vector<ext::shared_ptr<SimpleQuote>>> rrPillarQuotes_;
        std::vector<Time> rrPillarTimes_;
        std::vector<std::vector<ext::shared_ptr<SimpleQuote>>> bfPillarQuotes_;
        std::vector<Time> bfPillarTimes_;
        Real notional_;
        Real spotBump_, volBump_, rrBump_, bfBump_;
        Size tGrid_, xGrid_;
        Size lvTimePts_, lvStrikePts_;
        Real lvStrikeSpread_;
        // the local vol, and the process with it given explicitly
        ext::shared_ptr<FxLocalVolSurface> localVol_;
        ext::shared_ptr<GeneralizedBlackScholesProcess> localVolProcess_;
    };

}

#endif
