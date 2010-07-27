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

#include <ql/models/marketmodels/products/multistep/multistepratchet.hpp>
#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <cmath>

namespace QuantLib {

    MultiStepRatchet::MultiStepRatchet(const std::vector<Time>& rateTimes,
                                 const std::vector<Real>& accruals,
                                 const std::vector<Time>& paymentTimes,
                                 Real gearingOfFloor,
                                 Real gearingOfFixing,
                                 Rate spreadOfFloor,
                                 Rate spreadOfFixing,
                                 Real initialFloor,
                                 bool payer)
    : MultiProductMultiStep(rateTimes),
      accruals_(accruals), paymentTimes_(paymentTimes),
      gearingOfFloor_(gearingOfFloor), gearingOfFixing_(gearingOfFixing),
      spreadOfFloor_(spreadOfFloor), spreadOfFixing_(spreadOfFixing),
      payer_(payer), multiplier_(payer ? 1.0 : -1.0),
      lastIndex_(rateTimes.size()-1),
      initialFloor_(initialFloor) {
        checkIncreasingTimes(paymentTimes);
    }

    bool MultiStepRatchet::nextTimeStep(
            const CurveState& currentState,
            std::vector<Size>& numberCashFlowsThisStep,
            std::vector<std::vector<MarketModelMultiProduct::CashFlow> >&
                                                                 genCashFlows)
    {
        Rate liborRate = currentState.forwardRate(currentIndex_);
        Real currentCoupon = std::max(gearingOfFloor_* floor_ + spreadOfFloor_,
                                      gearingOfFixing_* liborRate + spreadOfFixing_);

        genCashFlows[0][0].timeIndex = currentIndex_;
        genCashFlows[0][0].amount =
            multiplier_* accruals_[currentIndex_]*currentCoupon;

        //floor_ = liborRate;                           //StepRatchet
        floor_ = currentCoupon;                         //FullRatchet
        numberCashFlowsThisStep[0] = 1;

        ++currentIndex_;

        return (currentIndex_ == lastIndex_);
    }

    std::auto_ptr<MarketModelMultiProduct> MultiStepRatchet::clone() const {
        return std::auto_ptr<MarketModelMultiProduct>(
                                                 new MultiStepRatchet(*this));
    }

}

