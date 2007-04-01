/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2007 Cristina Duminuco


 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/pricingengines/blackformula.hpp>

namespace QuantLib {

//===========================================================================//
//                              BlackIborCouponPricer                        //
//===========================================================================//

    void BlackIborCouponPricer::initialize(const FloatingRateCoupon& coupon){
        coupon_ =  dynamic_cast<const IborCoupon*>(&coupon);
        gearing_ = coupon_->gearing();
        spread_ = coupon_->spread();
        Date paymentDate = coupon_->date();
        const boost::shared_ptr<InterestRateIndex>& index = coupon_->index();
        Handle<YieldTermStructure> rateCurve = index->termStructure();

        Date today = Settings::instance().evaluationDate();

        if(paymentDate > today)
            discount_ = rateCurve->discount(paymentDate);
        else
            discount_ = 1.0;

        spreadLegValue_ = spread_ * coupon_->accrualPeriod()* discount_;
    }

    Real BlackIborCouponPricer::swapletPrice() const {
        // past or future fixing is managed in InterestRateIndex::fixing()

        Real swapletPrice =
           adjustedFixing()* coupon_->accrualPeriod()* discount_;
        return gearing_ * swapletPrice + spreadLegValue_;
    }

    Rate BlackIborCouponPricer::swapletRate() const {
        return swapletPrice()/(coupon_->accrualPeriod()*discount_);
    }

    Real BlackIborCouponPricer::capletPrice(Rate effectiveCap) const {
        Real capletPrice = optionletPrice(Option::Call, effectiveCap);
        return gearing_ * capletPrice;
    }

    Rate BlackIborCouponPricer::capletRate(Rate effectiveCap) const {
        return capletPrice(effectiveCap)/(coupon_->accrualPeriod()*discount_);
    }

    Real BlackIborCouponPricer::floorletPrice(Rate effectiveFloor) const {
        Real floorletPrice = optionletPrice(Option::Put, effectiveFloor);
        return gearing_ * floorletPrice;
    }

    Rate BlackIborCouponPricer::floorletRate(Rate effectiveFloor) const {
        return floorletPrice(effectiveFloor)/(coupon_->accrualPeriod()*discount_);
    }

    Real BlackIborCouponPricer::optionletPrice(Option::Type optionType, Real effStrike) const {
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
            return std::max(a - b, 0.0)* coupon_->accrualPeriod()*discount_;
        } else {
            QL_REQUIRE(!capletVolatility().empty(),"missing caplet volatility");
            // not yet determined, use Black model
            Rate fixing =
                 blackFormula(optionType,
                              effStrike,
                              adjustedFixing(),
                              std::sqrt(capletVolatility()->blackVariance(fixingDate,effStrike)));
            #if defined(QL_PATCH_MSVC6)
            return std::max(fixing,0.0)* coupon_->accrualPeriod()*discount_;
            #else
            return fixing* coupon_->accrualPeriod()*discount_; ;
            #endif
        }
    }

    Rate BlackIborCouponPricer::adjustedFixing() const {

        Real adjustement;

        Rate fixing = coupon_->indexFixing();

        if (!coupon_->isInArrears()) {
            adjustement = 0.0;
        } else {
            // see Hull, 4th ed., page 550
            QL_REQUIRE(!capletVolatility().empty(),"missing caplet volatility");
            Date d1 = coupon_->fixingDate(), referenceDate = capletVolatility()->referenceDate();
            if (d1 <= referenceDate) {
                adjustement = 0.0;
            } else {
                Date d2 = coupon_->index()->maturityDate(d1);
                Time tau = coupon_->index()->dayCounter().yearFraction(d1, d2);
                Real variance = capletVolatility()->blackVariance(d1, fixing);
                adjustement = fixing*fixing*variance*tau/(1.0+fixing*tau);
            }
        }
        return fixing + adjustement;
    }

//===========================================================================//
//                         CouponSelectorToSetPricer                         //
//===========================================================================//

    void CouponSelectorToSetPricer::visit(CashFlow&) {
        // nothing to do
    }

    void CouponSelectorToSetPricer::visit(Coupon&) {
        // nothing to do
    }

    void CouponSelectorToSetPricer::visit(IborCoupon& c) {
        const boost::shared_ptr<IborCouponPricer> iborCouponPricer =
            boost::dynamic_pointer_cast<IborCouponPricer>(pricer_);
        if (iborCouponPricer)
            c.setPricer(iborCouponPricer);
        else
            QL_FAIL("unexpected error when casting to IborCouponPricer");
    }

    void CouponSelectorToSetPricer::visit(CappedFlooredIborCoupon& c) {
        const boost::shared_ptr<IborCouponPricer> iborCouponPricer =
            boost::dynamic_pointer_cast<IborCouponPricer>(pricer_);
        if (iborCouponPricer)
            c.setPricer(iborCouponPricer);
        else
            QL_FAIL("unexpected error when casting to IborCouponPricer");
    }

    void CouponSelectorToSetPricer::visit(CmsCoupon& c) {
        const boost::shared_ptr<CmsCouponPricer> cmsCouponPricer =
            boost::dynamic_pointer_cast<CmsCouponPricer>(pricer_);
        if (cmsCouponPricer)
            c.setPricer(cmsCouponPricer);
        else
            QL_FAIL("unexpected error when casting to CmsCouponPricer");
    }

    void CouponSelectorToSetPricer::visit(CappedFlooredCmsCoupon& c) {
        const boost::shared_ptr<CmsCouponPricer> cmsCouponPricer =
            boost::dynamic_pointer_cast<CmsCouponPricer>(pricer_);
        if (cmsCouponPricer)
            c.setPricer(cmsCouponPricer);
        else
            QL_FAIL("unexpected error when casting to CmsCouponPricer");
    }

}
