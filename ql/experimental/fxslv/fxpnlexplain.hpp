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

#include <ql/experimental/fxslv/fxpairmarket.hpp>
#include <ql/option.hpp>
#include <utility>

namespace QuantLib {

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
          tenor held (see FxPairMarket::rolled());
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

    //! Day-on-day P&L explain of an FX vanilla option from today's market to the next
    /*! The two markets are of the same pair, quoting the same vol tenors
        and deltas; their surfaces, and those of the intermediate states, are
        built with today's market's choices (FxPairMarket::config()).
    */
    FxPnlExplainResult
    fxVanillaPnlExplain(Option::Type type,
                        Real strike,
                        const Date& expiry,
                        Real notional,
                        const FxPairMarket& today,
                        const FxPairMarket& next,
                        FxPnlExplainType explainType = FxPnlExplainType::Bartlett);

}

#endif
