/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009, 2011 Chris Kenyon

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

namespace QuantLib {

    CPICouponPricer::CPICouponPricer() {}

    CPICouponPricer::CPICouponPricer(
                       const Handle<CPIVolatilitySurface>& capletVol)
    : capletVol_(capletVol) {
        registerWith(capletVol_);
    }

    CPICouponPricer::CPICouponPricer(
                       const Handle<CPIVolatilitySurface>& capletVol,
                       const Handle<YieldTermStructure>& nominalTermStructure)
    : capletVol_(capletVol), nominalTermStructure_(nominalTermStructure) {
        registerWith(capletVol_);
        registerWith(nominalTermStructure_);
    }


    void CPICouponPricer::setCapletVolatility(
       const Handle<CPIVolatilitySurface>& capletVol) {
        QL_REQUIRE(!capletVol.empty(),"empty capletVol handle")
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
            return optionletPriceImp(optionType,
                                     effStrike,
                                     adjustedFixing(),
                                     stdDev);
        }
    }


    Rate CPICouponPricer::adjustedFixing(Rate fixing) const {

        if (fixing == Null<Rate>())
            fixing = coupon_->indexFixing() / coupon_->baseCPI();
        //std::cout << " adjustedFixing " << fixing << std::endl;
        // no adjustment
        return fixing;
    }


    void CPICouponPricer::initialize(const InflationCoupon& coupon) {
        coupon_ = dynamic_cast<const CPICoupon*>(&coupon);
        gearing_ = coupon_->fixedRate();
        spread_ = coupon_->spread();
        paymentDate_ = coupon_->date();
        rateCurve_ =
            !nominalTermStructure_.empty() ?
            nominalTermStructure_ :
            ext::dynamic_pointer_cast<ZeroInflationIndex>(coupon.index())
            ->zeroInflationTermStructure()
            ->nominalTermStructure();

        // past or future fixing is managed in YoYInflationIndex::fixing()
        // use yield curve from index (which sets discount)

        discount_ = 1.0;
        if (paymentDate_ > rateCurve_->referenceDate()) {
            if (rateCurve_.empty()) {
                // allow to extract rates, but mark the discount as invalid for prices
                discount_ = Null<Real>();
            } else {
                discount_ = rateCurve_->discount(paymentDate_);
            }
        }
    }


    Real CPICouponPricer::swapletPrice() const {
        QL_REQUIRE(discount_ != Null<Real>(), "no nominal term structure provided");
        return swapletRate() * coupon_->accrualPeriod() * discount_;
    }


    Rate CPICouponPricer::swapletRate() const {
        // This way we do not require the index to have
        // a yield curve, i.e. we do not get the problem
        // that a discounting-instrument-pricer is used
        // with a different yield curve
        return gearing_ * adjustedFixing() + spread_;
    }


    //=========================================================================
    // vol-dependent pricers, note that these do not discount
    //=========================================================================

/*
    NOT IMPLEMENTED
*/

}
