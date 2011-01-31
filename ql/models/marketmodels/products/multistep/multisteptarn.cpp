/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Mark Joshi

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
#include <ql/models/marketmodels/products/multistep/multisteptarn.hpp>

#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <cmath>

namespace QuantLib {

    MultiStepTarn::MultiStepTarn(const std::vector<Time>& rateTimes,
                         const std::vector<Real>& accruals,
                         const std::vector<Real>& accrualsFloating,                         
                         const std::vector<Time>& paymentTimes,
                         const std::vector<Time>& paymentTimesFloating,
                         Real totalCoupon,
                         const std::vector<Real>& strikes,
                         const std::vector<Real>& multipliers,
                         const std::vector<Real>& floatingSpreads)
    : MultiProductMultiStep(rateTimes),
      accruals_(accruals), accrualsFloating_(accrualsFloating), paymentTimes_(paymentTimes),paymentTimesFloating_(paymentTimesFloating),allPaymentTimes_(paymentTimes),
      totalCoupon_(totalCoupon), strikes_(strikes), multipliers_(multipliers), floatingSpreads_(floatingSpreads)
    {
        QL_REQUIRE(accruals_.size()+1 == rateTimes.size(), "missized accruals in MultiStepTARN");
        QL_REQUIRE(accrualsFloating.size()+1 == rateTimes.size(), "missized accrualsFloating in MultiStepTARN");
        QL_REQUIRE(paymentTimes.size()+1 == rateTimes.size(), "missized paymentTimes in MultiStepTARN");
        QL_REQUIRE(paymentTimesFloating.size()+1 == rateTimes.size(), "missized paymentTimesFloating in MultiStepTARN");
        QL_REQUIRE(strikes.size()+1 == rateTimes.size(), "missized strikes in MultiStepTARN");
        QL_REQUIRE(floatingSpreads.size()+1 == rateTimes.size(), "missized floatingSpreads in MultiStepTARN");

        lastIndex_ = accruals.size();

        for (Size i=0; i < paymentTimesFloating_.size(); ++i)
            allPaymentTimes_.push_back(paymentTimes[i]);


    }

    bool MultiStepTarn::nextTimeStep(
            const CurveState& currentState,
            std::vector<Size>& numberCashFlowsThisStep,
            std::vector<std::vector<MarketModelMultiProduct::CashFlow> >&
                                                                 genCashFlows)
    {
        Rate liborRate = currentState.forwardRate(currentIndex_);
        
        numberCashFlowsThisStep[0] =2;
        
        genCashFlows[0][0].amount = (liborRate+floatingSpreads_[currentIndex_])*accrualsFloating_[currentIndex_];
        genCashFlows[0][0].timeIndex = lastIndex_ + currentIndex_;

        genCashFlows[0][1].timeIndex =  currentIndex_;

        Real obviousCoupon = std::max(strikes_[currentIndex_] - multipliers_[currentIndex_]*liborRate,0.0)*accruals_[currentIndex_];

        couponPaid_+= obviousCoupon;

        ++currentIndex_;

        if (couponPaid_ < totalCoupon_ && currentIndex_ < lastIndex_ )
        {
              genCashFlows[0][1].amount = -  obviousCoupon;
              return false;
        }

        Real coupon = obviousCoupon +(totalCoupon_ -couponPaid_);
        genCashFlows[0][1].amount = - coupon;

        return true;
    }

    std::auto_ptr<MarketModelMultiProduct> MultiStepTarn::clone() const {
        return std::auto_ptr<MarketModelMultiProduct>(
                                                 new MultiStepTarn(*this));
    }



    std::vector<Time>
    MultiStepTarn::possibleCashFlowTimes() const {
        return allPaymentTimes_;
    }

   Size MultiStepTarn::numberOfProducts() const {
        return 1;
    }

    Size
    MultiStepTarn::maxNumberOfCashFlowsPerProductPerStep() const {
        return 2;
    }

    void MultiStepTarn::reset() 
    {
       currentIndex_=0;
       couponPaid_ = 0.0;
    }

}

