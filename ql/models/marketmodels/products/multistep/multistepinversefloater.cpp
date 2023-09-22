/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2006 Giorgio Facchinetti

This file is part of QuantLib, a free-software/open-source library
for financial quantitative analysts and developers - http://quantlib.org/

QuantLib is free software: you can redistribute it and/or modify it
under the terms of the QuantLib license.  You should have received a
copy of the license along with this program; if not, please email
<quantlib-dev@lists.sf.net>. The license is also available online at
<http://quantlib.org/license.shtml>.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/errors.hpp>
#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/products/multistep/multistepinversefloater.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <utility>

namespace QuantLib {

    MultiStepInverseFloater::MultiStepInverseFloater(const std::vector<Time>& rateTimes,
                                                     std::vector<Real> fixedAccruals,
                                                     const std::vector<Real>& floatingAccruals,
                                                     const std::vector<Real>& fixedStrikes,
                                                     const std::vector<Real>& fixedMultipliers,
                                                     const std::vector<Real>& floatingSpreads,
                                                     const std::vector<Time>& paymentTimes,
                                                     bool payer)
    : MultiProductMultiStep(rateTimes), fixedAccruals_(std::move(fixedAccruals)),
      floatingAccruals_(floatingAccruals), fixedStrikes_(fixedStrikes),
      fixedMultipliers_(fixedMultipliers), floatingSpreads_(floatingSpreads),
      paymentTimes_(paymentTimes), multiplier_(payer ? -1.0 : 1.0),
      lastIndex_(rateTimes.size() - 1) {
        checkIncreasingTimes(paymentTimes);
        QL_REQUIRE(fixedAccruals_.size() == lastIndex_," Incorrect number of fixedAccruals given, should be " <<  lastIndex_ << " not " << fixedAccruals_.size() );
        QL_REQUIRE(floatingAccruals.size() == lastIndex_," Incorrect number of floatingAccruals given, should be " <<  lastIndex_ << " not " << floatingAccruals.size() );
        QL_REQUIRE(fixedStrikes.size() == lastIndex_," Incorrect number of fixedStrikes given, should be " <<  lastIndex_ << " not " << fixedStrikes.size() );
        QL_REQUIRE(fixedMultipliers.size() == lastIndex_," Incorrect number of fixedMultipliers given, should be " <<  lastIndex_ << " not " << fixedMultipliers.size() );
        QL_REQUIRE(floatingSpreads.size() == lastIndex_," Incorrect number of floatingSpreads given, should be " <<  lastIndex_ << " not " << floatingSpreads.size() );
         QL_REQUIRE(paymentTimes.size() == lastIndex_," Incorrect number of paymentTimes given, should be " <<  lastIndex_ << " not " << paymentTimes.size() );
    }

    bool MultiStepInverseFloater::nextTimeStep(
        const CurveState& currentState,
        std::vector<Size>& numberCashFlowsThisStep,
        std::vector<std::vector<MarketModelMultiProduct::CashFlow> >&
        genCashFlows)
    {
         Rate liborRate = currentState.forwardRate(currentIndex_);
         Real inverseFloatingCoupon = std::max((fixedStrikes_[currentIndex_] - fixedMultipliers_[currentIndex_]*liborRate),0.0)*fixedAccruals_[currentIndex_] ;
         Real floatingCoupon = (liborRate+floatingSpreads_[currentIndex_])*floatingAccruals_[currentIndex_];

        genCashFlows[0][0].timeIndex = currentIndex_;
        genCashFlows[0][0].amount =multiplier_*(inverseFloatingCoupon - floatingCoupon);

        numberCashFlowsThisStep[0] = 1;
        ++currentIndex_;

        return (currentIndex_ == lastIndex_);
    }

    std::unique_ptr<MarketModelMultiProduct>
    MultiStepInverseFloater::clone() const 
    {
        return std::unique_ptr<MarketModelMultiProduct>(new MultiStepInverseFloater(*this));
    }

}

