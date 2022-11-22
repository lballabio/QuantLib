/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Toyin Akin

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

#include <ql/experimental/coupons/quantocouponpricer.hpp>
#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/cashflows/digitalcoupon.hpp>
#include <ql/cashflows/digitalcmscoupon.hpp>
#include <ql/cashflows/digitaliborcoupon.hpp>
#include <ql/cashflows/rangeaccrual.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/indexes/interestrateindex.hpp>

namespace QuantLib {

    Rate BlackIborQuantoCouponPricer::adjustedFixing(Real fixing) const {

        if (fixing == Null<Rate>())
            fixing = coupon_->indexFixing();

        // Here we apply the quanto adjustment first, then delegate to
        // the parent class
        Date d1 = coupon_->fixingDate(),
             referenceDate = capletVolatility()->referenceDate();

        if (d1 > referenceDate) {
            Time t1 =
                capletVolatility()->timeFromReference(d1);
            Volatility fxsigma =
                fxRateBlackVolatility_->blackVol(d1, fixing, true);
            Volatility sigma = capletVolatility()->volatility(d1, fixing);
            Real rho = underlyingFxCorrelation_->value();

            // Apply Quanto Adjustment.
            // Hull 6th Edition, page 642, generalised to
            // shifted lognormal and normal volatilities
            if(capletVolatility()->volatilityType() == ShiftedLognormal) {
                Real dQuantoAdj = std::exp(sigma*fxsigma*rho*t1);
                Real shift = capletVolatility()->displacement();
                fixing = (fixing+shift)*dQuantoAdj-shift;
            }
            else {
                Real dQuantoAdj = sigma*fxsigma*rho*t1;
                fixing += dQuantoAdj;
            }
        }

        return BlackIborCouponPricer::adjustedFixing(fixing);
    }

}

