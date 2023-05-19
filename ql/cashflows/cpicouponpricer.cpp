/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009, 2011 Chris Kenyon
 Copyright (C) 2022 Quaternion Risk Management Ltd

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

#include <ql/cashflows/cpicouponpricer.hpp>
#include <utility>

namespace QuantLib {

    QL_DEPRECATED_DISABLE_WARNING

    CPICouponPricer::CPICouponPricer(Handle<YieldTermStructure> nominalTermStructure)
    : nominalTermStructure_(std::move(nominalTermStructure)) {
        registerWith(nominalTermStructure_);
    }

    CPICouponPricer::CPICouponPricer(Handle<CPIVolatilitySurface> capletVol,
                                     Handle<YieldTermStructure> nominalTermStructure)
    : capletVol_(std::move(capletVol)), nominalTermStructure_(std::move(nominalTermStructure)) {
        registerWith(capletVol_);
        registerWith(nominalTermStructure_);
    }

    QL_DEPRECATED_ENABLE_WARNING

    void CPICouponPricer::setCapletVolatility(
       const Handle<CPIVolatilitySurface>& capletVol) {
        QL_REQUIRE(!capletVol.empty(),"empty capletVol handle");
        capletVol_ = capletVol;
        registerWith(capletVol_);
    }


    Real CPICouponPricer::floorletPrice(Rate effectiveFloor) const{
        Real floorletPrice = optionletPrice(Option::Put, effectiveFloor);
        return gearing_ * floorletPrice;
    }

    Real CPICouponPricer::capletPrice(Rate effectiveCap) const{
        Real capletPrice = optionletPrice(Option::Call, effectiveCap);
        return gearing_ * capletPrice;
    }


    Rate CPICouponPricer::floorletRate(Rate effectiveFloor) const {
        return gearing_ * optionletRate(Option::Put, effectiveFloor);
    }

    Rate CPICouponPricer::capletRate(Rate effectiveCap) const{
        return gearing_ * optionletRate(Option::Call, effectiveCap);
    }


    Real CPICouponPricer::optionletPriceImp(Option::Type,
                                            Real,
                                            Real,
                                            Real) const {
        QL_FAIL("you must implement this to get a vol-dependent price");
    }


    Real CPICouponPricer::optionletPrice(Option::Type optionType,
                                         Real effStrike) const {
        QL_REQUIRE(discount_ != Null<Real>(), "no nominal term structure provided");
        return optionletRate(optionType, effStrike) * coupon_->accrualPeriod() * discount_;
    }


    Real CPICouponPricer::optionletRate(Option::Type optionType,
                                        Real effStrike) const {
        Date fixingDate = coupon_->fixingDate();
        if (fixingDate <= Settings::instance().evaluationDate()) {
            // the amount is determined
            Real a, b;
            if (optionType==Option::Call) {
                a = coupon_->indexFixing();
                b = effStrike;
            } else {
                a = effStrike;
                b = coupon_->indexFixing();
            }
            return std::max(a - b, 0.0);
        } else {
            // not yet determined, use Black/DD1/Bachelier/whatever from Impl
            QL_REQUIRE(!capletVolatility().empty(),
                       "missing optionlet volatility");
            Real stdDev =
            std::sqrt(capletVolatility()->totalVariance(fixingDate,
                                                        effStrike));
            QL_DEPRECATED_DISABLE_WARNING
            return optionletPriceImp(optionType,
                                     effStrike,
                                     adjustedFixing(),
                                     stdDev);
            QL_DEPRECATED_ENABLE_WARNING
        }
    }


    Rate CPICouponPricer::adjustedFixing(Rate fixing) const {
        if (fixing != Null<Rate>())
            return fixing;

        return coupon_->indexRatio(coupon_->accrualEndDate());
    }


    void CPICouponPricer::initialize(const InflationCoupon& coupon) {
        coupon_ = dynamic_cast<const CPICoupon*>(&coupon);
        gearing_ = coupon_->fixedRate();
        QL_DEPRECATED_DISABLE_WARNING
        spread_ = coupon_->spread();
        QL_DEPRECATED_ENABLE_WARNING
        paymentDate_ = coupon_->date();

        // past or future fixing is managed in YoYInflationIndex::fixing()
        // use yield curve from index (which sets discount)

        discount_ = 1.0;
        if (nominalTermStructure_.empty()) {
            // allow to extract rates, but mark the discount as invalid for prices
            discount_ = Null<Real>();
        } else {
            if (paymentDate_ > nominalTermStructure_->referenceDate())
                discount_ = nominalTermStructure_->discount(paymentDate_);
        }
    }


    Real CPICouponPricer::swapletPrice() const {
        QL_REQUIRE(discount_ != Null<Real>(), "no nominal term structure provided");
        return swapletRate() * coupon_->accrualPeriod() * discount_;
    }


    Rate CPICouponPricer::swapletRate() const {
        QL_DEPRECATED_DISABLE_WARNING
        return gearing_ * adjustedFixing() + spread_;
        QL_DEPRECATED_ENABLE_WARNING
        // after deprecating and removing adjustedFixing:
        // return accruedRate(coupon_->accrualEndDate());
    }


    Rate CPICouponPricer::accruedRate(Date settlementDate) const {
        QL_DEPRECATED_DISABLE_WARNING
        return gearing_ * coupon_->indexRatio(settlementDate) + spread_;
        QL_DEPRECATED_ENABLE_WARNING
    }

}
