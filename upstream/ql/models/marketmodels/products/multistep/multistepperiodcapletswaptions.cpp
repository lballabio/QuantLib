/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Mark Joshi

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

#include <ql/instruments/payoffs.hpp>
#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/products/multistep/multistepperiodcapletswaptions.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <utility>

namespace QuantLib {

    MultiStepPeriodCapletSwaptions::MultiStepPeriodCapletSwaptions(
        const std::vector<Time>& rateTimes,
        const std::vector<Time>& forwardOptionPaymentTimes,
        const std::vector<Time>& swaptionPaymentTimes,
        std::vector<ext::shared_ptr<StrikedTypePayoff> > forwardPayOffs,
        std::vector<ext::shared_ptr<StrikedTypePayoff> > swapPayOffs,
        Size period,
        Size offset)
    : MultiProductMultiStep(rateTimes), paymentTimes_(forwardOptionPaymentTimes),
      forwardOptionPaymentTimes_(forwardOptionPaymentTimes),
      swaptionPaymentTimes_(swaptionPaymentTimes), forwardPayOffs_(std::move(forwardPayOffs)),
      swapPayOffs_(std::move(swapPayOffs)), period_(period), offset_(offset) {
        QL_REQUIRE(rateTimes.size() >=2,
                       "we need at least two rate times in MultiStepPeriodCapletSwaptions ");

        checkIncreasingTimes(forwardOptionPaymentTimes);
        checkIncreasingTimes(swaptionPaymentTimes);
        for (Real& swaptionPaymentTime : swaptionPaymentTimes_)
            paymentTimes_.push_back(swaptionPaymentTime);
        lastIndex_ = rateTimes.size()-1;
        numberFRAs_ = rateTimes.size()-1;
        numberBigFRAs_ = (numberFRAs_-offset_)/period_;


        QL_REQUIRE(offset_< period_,
                       "the offset must be less then the period in MultiStepPeriodCapletSwaptions ");
        QL_REQUIRE(numberBigFRAs_ > 0,
                       "we must have at least one FRA after the periodizing in  MultiStepPeriodCapletSwaptions ");

        QL_REQUIRE(forwardOptionPaymentTimes_.size() == numberBigFRAs_,
                       "we must have precisely one payment time for each forward option  MultiStepPeriodCapletSwaptions ");

         QL_REQUIRE(forwardPayOffs_.size() == numberBigFRAs_,
                       "we must have precisely one payoff  for each forward option  MultiStepPeriodCapletSwaptions ");

        QL_REQUIRE(swaptionPaymentTimes_.size() == numberBigFRAs_,
                       "we must have precisely one payment time for each swaption in MultiStepPeriodCapletSwaptions ");

         QL_REQUIRE(swapPayOffs_.size() == numberBigFRAs_,
                       "we must have precisely one payoff  for each swaption in  MultiStepPeriodCapletSwaptions ");
    }

    bool MultiStepPeriodCapletSwaptions::nextTimeStep(
            const CurveState& currentState,
            std::vector<Size>& numberCashFlowsThisStep,
            std::vector<std::vector<MarketModelMultiProduct::CashFlow> >&
                                                               genCashFlows)
    {

        for (Size & i : numberCashFlowsThisStep)
            i = 0UL;

        if (currentIndex_ >= offset_ && (currentIndex_ - offset_) % period_ == 0) {
            // caplet first
            Real df = currentState.discountRatio(currentIndex_ + period_, currentIndex_);
            Time tau = rateTimes_[currentIndex_ + period_] - rateTimes_[currentIndex_];
            Real forward = (1.0 / df - 1.0) / tau;
            Real value = (*forwardPayOffs_[productIndex_])(forward);
            value *= tau * currentState.discountRatio(currentIndex_ + period_, currentIndex_);

            if (value > 0) {
                numberCashFlowsThisStep[productIndex_] = 1UL;
                genCashFlows[productIndex_][0].amount = value;
                genCashFlows[productIndex_][0].timeIndex = productIndex_;
            }

            // now swaption

            unsigned long numberPeriods = numberBigFRAs_ - productIndex_;
            Real B = 0.0;
            double P0 = 1.0; // i.e currentState.discountRatio(currentIndex_,currentIndex_);
            Real Pn =
                currentState.discountRatio(currentIndex_ + numberPeriods * period_, currentIndex_);
            for (unsigned long i = 0; i < numberPeriods; ++i) {
                Time tau = rateTimes_[currentIndex_ + (i + 1) * period_] -
                           rateTimes_[currentIndex_ + i * period_];
                B += tau *
                     currentState.discountRatio(currentIndex_ + (i + 1) * period_, currentIndex_);
            }


            Real swapRate = (P0 - Pn) / B;

            Real swaptionValue = (*swapPayOffs_[productIndex_])(swapRate);
            swaptionValue *= B;

            if (swaptionValue > 0) {
                numberCashFlowsThisStep[productIndex_ + numberBigFRAs_] = 1UL;
                genCashFlows[productIndex_ + numberBigFRAs_][0].amount = swaptionValue;
                genCashFlows[productIndex_ + numberBigFRAs_][0].timeIndex =
                    productIndex_ + numberBigFRAs_;
            }

            ++productIndex_;

    }

    ++currentIndex_;

    bool terminate =(productIndex_ >= numberBigFRAs_);


    return terminate;
    }

    std::unique_ptr<MarketModelMultiProduct>
    MultiStepPeriodCapletSwaptions::clone() const {
        return std::unique_ptr<MarketModelMultiProduct>(new MultiStepPeriodCapletSwaptions(*this));
    }

}
