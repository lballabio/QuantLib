/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2018 Sebastian Schlenkrich

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

/*! \file tenorswaptionvts.cpp
    \brief swaption volatility term structure based on volatility transformation
*/


#include <ql/experimental/basismodels/tenorswaptionvts.hpp>
#include <ql/experimental/basismodels/swaptioncfs.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/exercise.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/math/rounding.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/time/dategenerationrule.hpp>


namespace QuantLib {

    TenorSwaptionVTS::TenorSwaptionSmileSection::TenorSwaptionSmileSection(
        const TenorSwaptionVTS& volTS, Time optionTime, Time swapLength)
    : SmileSection(optionTime, volTS.baseVTS_->dayCounter(), Normal, 0.0) {
        baseSmileSection_ = volTS.baseVTS_->smileSection(optionTime, swapLength, true);
        // first we need the swap start and end date
        Real oneDayAsYear =
            volTS.dayCounter().yearFraction(volTS.referenceDate(), volTS.referenceDate() + 1);
        Date exerciseDate =
            volTS.referenceDate() + ((BigInteger)ClosestRounding(0)(optionTime / oneDayAsYear));
        Date effectiveDate = volTS.baseIndex_->fixingCalendar().advance(
            exerciseDate, volTS.baseIndex_->fixingDays() * Days);
        Date maturityDate = volTS.baseIndex_->fixingCalendar().advance(
            effectiveDate, ((BigInteger)swapLength * 12.0) * Months, Unadjusted, false);
        // now we can set up the schedules
        Schedule baseFixedSchedule(effectiveDate, maturityDate, volTS.baseFixedFreq_,
                                   volTS.baseIndex_->fixingCalendar(), ModifiedFollowing,
                                   Unadjusted, DateGeneration::Backward, false);
        Schedule finlFixedSchedule(effectiveDate, maturityDate, volTS.targFixedFreq_,
                                   volTS.targIndex_->fixingCalendar(), ModifiedFollowing,
                                   Unadjusted, DateGeneration::Backward, false);
        Schedule baseFloatSchedule(effectiveDate, maturityDate, volTS.baseIndex_->tenor(),
                                   volTS.baseIndex_->fixingCalendar(), ModifiedFollowing,
                                   Unadjusted, DateGeneration::Backward, false);
        Schedule targFloatSchedule(effectiveDate, maturityDate, volTS.targIndex_->tenor(),
                                   volTS.baseIndex_->fixingCalendar(), ModifiedFollowing,
                                   Unadjusted, DateGeneration::Backward, false);
        // and swaps
        auto baseSwap = ext::make_shared<VanillaSwap>(
            Swap::Payer, 1.0, baseFixedSchedule, 1.0, volTS.baseFixedDC_, baseFloatSchedule,
            volTS.baseIndex_, 0.0, volTS.baseIndex_->dayCounter());
        auto targSwap = ext::make_shared<VanillaSwap>(
            Swap::Payer, 1.0, baseFixedSchedule, 1.0, volTS.baseFixedDC_, targFloatSchedule,
            volTS.targIndex_, 0.0, volTS.targIndex_->dayCounter());
        auto finlSwap = ext::make_shared<VanillaSwap>(
            Swap::Payer, 1.0, finlFixedSchedule, 1.0, volTS.targFixedDC_, targFloatSchedule,
            volTS.targIndex_, 0.0, volTS.targIndex_->dayCounter());
        // adding engines
        baseSwap->setPricingEngine(
            ext::shared_ptr<PricingEngine>(new DiscountingSwapEngine(volTS.discountCurve_)));
        targSwap->setPricingEngine(
            ext::shared_ptr<PricingEngine>(new DiscountingSwapEngine(volTS.discountCurve_)));
        finlSwap->setPricingEngine(
            ext::shared_ptr<PricingEngine>(new DiscountingSwapEngine(volTS.discountCurve_)));
        // swap rates
        swapRateBase_ = baseSwap->fairRate();
        swapRateTarg_ = targSwap->fairRate();
        swapRateFinl_ = finlSwap->fairRate();
        SwaptionCashFlows cfs(
            ext::make_shared<Swaption>(
                baseSwap, ext::shared_ptr<Exercise>(new EuropeanExercise(exerciseDate))),
            volTS.discountCurve_);
        SwaptionCashFlows cf2(
            ext::make_shared<Swaption>(
                targSwap, ext::shared_ptr<Exercise>(new EuropeanExercise(exerciseDate))),
            volTS.discountCurve_);
        // calculate affine TSR model u and v
        // Sum tau_j   (fixed leg)
        Real sumTauj = 0.0;
        for (Real k : cfs.annuityWeights())
            sumTauj += k;
        // Sum tau_j (T_M - T_j)   (fixed leg)
        Real sumTaujDeltaT = 0.0;
        for (Size k = 0; k < cfs.annuityWeights().size(); ++k)
            sumTaujDeltaT +=
                cfs.annuityWeights()[k] * (cfs.fixedTimes().back() - cfs.fixedTimes()[k]);
        // Sum w_i   (float leg)
        Real sumWi = 0.0;
        for (Real k : cfs.floatWeights())
            sumWi += k;
        // Sum w_i (T_N - T_i)    (float leg)
        Real sumWiDeltaT = 0.0;
        for (Size k = 0; k < cfs.floatWeights().size(); ++k)
            sumWiDeltaT += cfs.floatWeights()[k] * (cfs.floatTimes().back() - cfs.floatTimes()[k]);
        // assemble u, v and a(T_p)
        Real den = sumTaujDeltaT * sumWi - sumWiDeltaT * sumTauj;
        Real u = -sumTauj / den;
        Real v = sumTaujDeltaT / den;
        Actual365Fixed dc;
        // a(T_p) = u (T_p - T_N) + v
        Real T_N = cfs.fixedTimes().back();
        Real sumBase = 0.0;
        Real sumTarg = 0.0;
        // we skip the first and last weight as they represent the notional flows
        for (Size k = 1; k < cfs.floatWeights().size() - 1; ++k)
            sumBase += cfs.floatWeights()[k] * (u * (T_N - cfs.floatTimes()[k]) + v);
        for (Size k = 1; k < cf2.floatWeights().size() - 1; ++k)
            sumTarg += cf2.floatWeights()[k] * (u * (T_N - cf2.floatTimes()[k]) + v);
        lambda_ = sumTarg - sumBase;
        // Annuity scaling
        annuityScaling_ = targSwap->fixedLegBPS() / finlSwap->fixedLegBPS();
    }

    Volatility TenorSwaptionVTS::TenorSwaptionSmileSection::volatilityImpl(Rate strike) const {
        Real strikeBase = (strike - (swapRateTarg_ - (1.0 + lambda_) * swapRateBase_)) /
                          (1.0 + lambda_) / annuityScaling_;
        Real volBase = baseSmileSection_->volatility(strikeBase, Normal, 0.0);
        Real volTarg = annuityScaling_ * (1.0 + lambda_) * volBase;
        return volTarg;
    }


}
