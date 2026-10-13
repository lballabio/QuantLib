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

#include <ql/math/optimization/constraint.hpp>
#include <ql/math/optimization/endcriteria.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/experimental/fxslv/fxrootbracketing.hpp>
#include <ql/experimental/fxslv/fxsmilesectionbydelta.hpp>
#include <algorithm>
#include <cmath>

namespace QuantLib {

    FxSmileSectionByDelta::FxSmileSectionByDelta(const Date& exerciseDate,
                                                 const Handle<Quote>& spot,
                                                 const ext::shared_ptr<FxSmileQuotes>& quotes,
                                                 const Handle<YieldTermStructure>& foreignDiscount,
                                                 const Handle<YieldTermStructure>& domesticDiscount,
                                                 DeltaVolQuote::DeltaType deltaType,
                                                 DeltaVolQuote::AtmType atmType,
                                                 const DayCounter& dayCounter,
                                                 const FxSettleConvention& settlement,
                                                 const Date& referenceDate)
    : FxSmileSection(exerciseDate, spot, quotes,                     
                     foreignDiscount, domesticDiscount,
                     deltaType, atmType, dayCounter, settlement, referenceDate),
      params_() {}

    FxSmileSectionByDelta::FxSmileSectionByDelta(Time exerciseTime,
                                                 const Handle<Quote>& spot,
                                                 const ext::shared_ptr<FxSmileQuotes>& quotes,
                                                 const Handle<YieldTermStructure>& foreignDiscount,
                                                 const Handle<YieldTermStructure>& domesticDiscount,
                                                 DeltaVolQuote::DeltaType deltaType,
                                                 DeltaVolQuote::AtmType atmType,
                                                 const DayCounter& dayCounter)
    : FxSmileSection(exerciseTime, spot, quotes,
                     foreignDiscount, domesticDiscount,
                     deltaType, atmType, dayCounter),
      params_() {}

    Volatility FxSmileSectionByDelta::volByStrike(Rate strike) const 
    {
        calculate();

        Real delta = deltaByStrike(strike, Option::Type::Put);
        return volByDelta(delta, Option::Type::Put);
    }

    Volatility FxSmileSectionByDelta::volByDelta(Real delta, Option::Type parity) const 
    {
        calculate();

        if (parity == Option::Call) {
            // the smile is parameterised by put delta: convert the call delta
            const FxDeltaConverter conv = deltaConverter();
            const Real sqrtT = std::sqrt(exerciseTime());

            if (!conv.premiumAdjusted()) {
                // call minus put delta is a constant for unadjusted deltas
                QL_REQUIRE(delta > 0.0 && delta <= conv.callDeltaLimit(),
                           "call delta out of range: " << delta);
                delta -= conv.callMinusPutDelta(fwd_);
            } else {
                const Rate kPeak = peakCallDeltaStrike();
                const Volatility vPeak = volByStrike(kPeak);
                const Real maxCallDelta = conv.delta(Option::Call, kPeak, vPeak * sqrtT);
                QL_REQUIRE(delta <= maxCallDelta + QL_EPSILON,
                           "call delta " << delta << " out of range: the largest is "
                                         << maxCallDelta);
                if (std::fabs(delta - maxCallDelta) <= QL_EPSILON)
                    return vPeak;

                // Otherwise find the put delta d whose strike has the given
                // call delta: call delta - put delta = callMinusPutDelta(K).
                // Below the peak each call delta is attained at two strikes;
                // the quoted one is above the peak, i.e. at put deltas below
                // the peak's, where the call delta falls from its maximum to 0.
                auto deltaError = [&](Real d) {
                    const Volatility v = volByDelta(d, Option::Put);
                    const Rate k = conv.strike(Option::Put, d, v * sqrtT);
                    return delta - d - conv.callMinusPutDelta(k);
                };

                // the error is negative at the peak and grows as the put
                // delta falls (higher strikes)
                const Real dPeak = conv.delta(Option::Put, kPeak, vPeak * sqrtT);
                delta = detail::findRoot(deltaError, dPeak, std::max(std::fabs(dPeak), 0.01),
                                         detail::Slope::Decreasing, 1e-12);
            }
        }

        // got vol as a function of delta
        return volByDeltaImpl(delta, fwd_, params_);
    }

    Rate FxSmileSectionByDelta::strikeByDelta(Real delta, Option::Type parity) const 
    {
        calculate();

        const Volatility v = volByDelta(delta, parity);
        return deltaConverter().strike(parity, delta, v * std::sqrt(exerciseTime()));
    }

    Real FxSmileSectionByDelta::deltaByStrike(Rate strike, Option::Type parity) const 
    {
        calculate();

        // The smile is parameterised by put delta, so find the put delta at
        // the strike first: the fixed point d = putDelta(strike, vol(d)).
        // This only needs delta from strike, never the inverse.  The put
        // delta at a fixed strike lies in a range independent of the vol.
        const FxDeltaConverter conv = deltaConverter();
        const Real sqrtT = std::sqrt(exerciseTime());
        const Real dmin = conv.putDeltaLimit(strike);
        // The upper end is 0 rather than -epsilon: a deep out-of-the-money
        // put can have a delta smaller in magnitude than epsilon, and the
        // fixed-point form never needs to invert delta -> strike at 0.
        const Real dmax = 0.0;
        Real d0 = conv.delta(Option::Put, strike, referenceVol() * sqrtT);
        if (!(d0 > dmin && d0 < dmax))
            d0 = 0.5 * (dmin + dmax);

        auto deltaError = [&](Real delta) {
            const Volatility v = volByDelta(delta, Option::Put);
            return conv.delta(Option::Put, strike, v * sqrtT) - delta;
        };

        Brent solver;
        Real d = solver.solve(deltaError, 1e-12, d0, dmin, dmax);

        if (parity == Option::Call)
            d = conv.delta(Option::Call, strike, volByDelta(d, Option::Put) * sqrtT);

        return d;
    }

    Real FxSmileSectionByDelta::volResidual(Rate strike, Volatility vol) const
    {
        // the model vol at the point's put delta, computed with the point's own vol
        const Real putDelta =
            deltaConverter().delta(Option::Put, strike, vol * std::sqrt(exerciseTime()));
        return volByDelta(putDelta, Option::Put) - vol;
    }


    //! \name Quadratic smile section (delta-parameterized)
    //@{
    FxQuadraticSmileSection::FxQuadraticSmileSection(const Date& exerciseDate,
                                                 const Handle<Quote>& spot,
                                                 const ext::shared_ptr<FxSmileQuotes>& quotes,
                                                 const Handle<YieldTermStructure>& foreignDiscount,
                                                 const Handle<YieldTermStructure>& domesticDiscount,
                                                 DeltaVolQuote::DeltaType deltaType,
                                                 DeltaVolQuote::AtmType atmType,
                                                 const DayCounter& dayCounter,
                                                 const FxSettleConvention& settlement,
                                                 const Date& referenceDate)
    : FxSmileSectionByDelta(exerciseDate, spot, quotes,
                            foreignDiscount, domesticDiscount,
                            deltaType, atmType, dayCounter, settlement, referenceDate)
    {
        params_.reserve(3);
    }

    FxQuadraticSmileSection::FxQuadraticSmileSection(Time exerciseTime,
                                                 const Handle<Quote>& spot,
                                                 const ext::shared_ptr<FxSmileQuotes>& quotes,
                                                 const Handle<YieldTermStructure>& foreignDiscount,
                                                 const Handle<YieldTermStructure>& domesticDiscount,
                                                 DeltaVolQuote::DeltaType deltaType,
                                                 DeltaVolQuote::AtmType atmType,
                                                 const DayCounter& dayCounter)
    : FxSmileSectionByDelta(exerciseTime, spot, quotes,
                            foreignDiscount, domesticDiscount,
                            deltaType, atmType, dayCounter)
    {
        params_.reserve(3);
    }

    Array FxQuadraticSmileSection::initialParams() const
    {
        // vol = a*delta^2 + b*delta + c
        // ATM put delta is conventionally -0.5, so at ATM:
        //   atm_vol = a*0.25 + b*(-0.5) + c
        // Start with a=0, b=0, c=atm_vol
        Array guess(3);
        guess[0] = 0.0;
        guess[1] = 0.0;
        guess[2] = referenceVol();
        return guess;
    }

    Volatility FxQuadraticSmileSection::volByDeltaImpl(Real delta,
                                                       Real fwd,
                                                       const std::vector<Real>& params) const
    {
        return params[0] * delta * delta + params[1] * delta + params[2];
    }
    //@}

}
