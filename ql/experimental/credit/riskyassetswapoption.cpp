/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters

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

#include <ql/event.hpp>
#include <ql/experimental/credit/riskyassetswapoption.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <utility>

namespace QuantLib {

    RiskyAssetSwapOption::RiskyAssetSwapOption(ext::shared_ptr<RiskyAssetSwap> asw,
                                               const Date& expiry,
                                               Rate marketSpread,
                                               Volatility spreadVolatility)
    : asw_(std::move(asw)), expiry_(expiry), marketSpread_(marketSpread),
      spreadVolatility_(spreadVolatility) {}

    bool RiskyAssetSwapOption::isExpired() const {
        return detail::simple_event(expiry_).hasOccurred();
    }


    void RiskyAssetSwapOption::performCalculations() const {
        Real w;
        if (asw_->fixedPayer()) // strike receiver = asw call = spread put
            w = -1.0;
        else
            w = 1.0;

        Date today = Settings::instance().evaluationDate();
        Time expiryTime = Actual365Fixed().yearFraction(today, expiry_);
        Real stdDev = spreadVolatility_ * std::sqrt(expiryTime);
        Real d = (asw_->spread() - marketSpread_) / stdDev;
        Real A0 = asw_->nominal() * asw_->floatAnnuity();

        NPV_ = A0 * stdDev * (w*d * CumulativeNormalDistribution()(w*d)
                              + NormalDistribution()(d));
    }

}
