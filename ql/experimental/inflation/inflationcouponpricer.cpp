/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon

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

#include <ql/experimental/inflation/inflationcouponpricer.hpp>
#include <ql/experimental/inflation/inflationcoupon.hpp>
#include <ql/experimental/inflation/yoyoptionletvolatilitystructures.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

namespace QuantLib {

    void YoYInflationCouponPricer::initialize(
                                         const YoYInflationCoupon& coupon) {

        // want to store a pointer not the object
        coupon_ =  dynamic_cast<const YoYInflationCoupon*>(&coupon);
        gearing_ = coupon_->gearing();
        spread_ = coupon_->spread();
        Date paymentDate = coupon_->date();
        const boost::shared_ptr<YoYInflationIndex>& index = coupon_->index();
        Handle<YieldTermStructure> rateCurve =
            index->yoyInflationTermStructure()->nominalTermStructure();

        Date today = Settings::instance().evaluationDate();

        if(paymentDate > today)
            discount_ = rateCurve->discount(paymentDate);
        else
            discount_ = 1.0;

        spreadLegValue_ = spread_ * coupon_->accrualPeriod()* discount_;
    }


    void YoYInflationCouponPricer::setCapletVolatility(
                     const Handle<YoYOptionletVolatilitySurface>& capletVol) {
        if (!capletVol_.empty())
            unregisterWith(capletVol_);
        capletVol_ = capletVol;
        QL_REQUIRE(!capletVol_.empty(), "no adequate capletVol given");
        registerWith(capletVol_);
        update();
    }

    Rate YoYInflationCouponPricer::adjustedFixing() const {

        Real adjustement = 0.0;
        Rate fixing = coupon_->indexFixing();

        // Coupons are always in arrears so no convxity adjustment is
        // required ...  However the lag of the coupon can be
        // different from the lag of the volatility surface - hence a
        // convexity adjustment is required.  We currently neglect
        // this.  I.e. this method does nothing now.

        return fixing + adjustement;
    }

    Real YoYInflationCouponPricer::swapletPrice() const {
        // past or future fixing is managed in InterestRateIndex::fixing()
        Real swapletPrice =
            adjustedFixing()* coupon_->accrualPeriod()* discount_;
        return gearing_ * swapletPrice + spreadLegValue_;
    }

    Rate YoYInflationCouponPricer::swapletRate() const {
        return swapletPrice()/
            (coupon_->accrualPeriod()*discount_);
    }

    Real YoYInflationCouponPricer::capletPrice(Rate effectiveCap) const {
        Real capletPrice = optionletPriceImp(Option::Call, effectiveCap);
        return gearing_ * capletPrice;
    }

    Rate YoYInflationCouponPricer::capletRate(Rate effectiveCap) const {
        return capletPrice(effectiveCap)/
            (coupon_->accrualPeriod()*discount_);
    }

    Real YoYInflationCouponPricer::floorletPrice(Rate effectiveFloor) const {
        Real floorletPrice = optionletPriceImp(Option::Put, effectiveFloor);
        return gearing_ * floorletPrice;
    }

    Rate YoYInflationCouponPricer::floorletRate(Rate effectiveFloor) const {
        return floorletPrice(effectiveFloor)/
            (coupon_->accrualPeriod()*discount_);
    }


    BlackYoYInflationCouponPricer::BlackYoYInflationCouponPricer(
                       const Handle<YoYOptionletVolatilitySurface>& capletVol)
    : YoYInflationCouponPricer(capletVol) {}

    UnitDisplacedBlackYoYInflationCouponPricer::
    UnitDisplacedBlackYoYInflationCouponPricer(
                       const Handle<YoYOptionletVolatilitySurface>& capletVol)
    : YoYInflationCouponPricer(capletVol) {}


    BachelierYoYInflationCouponPricer::BachelierYoYInflationCouponPricer(
                       const Handle<YoYOptionletVolatilitySurface>& capletVol)
    : YoYInflationCouponPricer(capletVol) {}




    Real BlackYoYInflationCouponPricer::optionletPriceImp(
                              Option::Type optionType, Real effStrike) const {

        Date fixingDate = coupon_->fixingDate();
        if (fixingDate <= Settings::instance().evaluationDate()) {
            // the amount is already known as the fixing is known
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
            QL_REQUIRE(!capletVolatility().empty(),
                       "missing optionlet volatility");
            // not yet known, use Black model here, note that this
            // already has t scaled out
            // N.B. integrated variance is called "total" rather than
            // "black" because the volatility surface does not know
            // what sort it is ... the user is responsible for
            // ensuring that it has the correct contents
            Rate fixing = blackFormula(
                       optionType,
                       effStrike,
                       adjustedFixing(),
                       std::sqrt(capletVolatility()->totalVariance(fixingDate,
                                                                   effStrike)));
            return fixing* coupon_->accrualPeriod()*discount_;
        }
    }


    Real UnitDisplacedBlackYoYInflationCouponPricer::optionletPriceImp(
                              Option::Type optionType, Real effStrike) const {

        Date fixingDate = coupon_->fixingDate();
        if (fixingDate <= Settings::instance().evaluationDate()) {
            // the amount is already known as the fixing is known
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
            QL_REQUIRE(!capletVolatility().empty(),
                       "missing optionlet volatility");
            // not yet known, use DDBlack model here, note that this
            // already has t scaled out
            // N.B. integrated variance is called "total" rather than
            // "black" because the volatility surface does not know
            // what sort it is ... the user is responsible for
            // ensuring that it has the correct contents
            Rate fixing = blackFormula(
                       optionType,
                       effStrike + 1.0,
                       adjustedFixing() + 1.0,
                       std::sqrt(capletVolatility()->totalVariance(fixingDate,
                                                                   effStrike)));
            return fixing* coupon_->accrualPeriod()*discount_;
        }
    }


    namespace {

        // replacement for bachelierBlackFormula
        // Different formula from bachelierBlackFormula because this
        // is for Normally distributed forwards in their terminal
        // measures that are not interest rates.  Basically different
        // interpretation of input data.
        // N.B. can have -h or +h in Phi.derivative because Standard
        // Normal is symmetric about mean=0.
        Real bachelierFormula(Option::Type optionType,
                              Real strike,
                              Real forward,
                              Real sigma) {

            QL_REQUIRE(sigma>=0.0,
                       "stdDev (" << sigma << ") must be non-negative");
            Real d = (forward-strike)*optionType;
            Real h = d/sigma;
            if (sigma==0.0) return std::max(d, 0.0);

            CumulativeNormalDistribution Phi;

            Real result = d*Phi(h) + sigma*Phi.derivative(-h);

            QL_ENSURE(result>=0.0,
                      "[bachelierFormula] negative value (" <<
                      result << ") for " <<
                      sigma << " sigma, " <<
                      optionType << " option, " <<
                      strike << " strike , " <<
                      forward << " forward");

            return result;
        }

    }


    Real BachelierYoYInflationCouponPricer::optionletPriceImp(
                              Option::Type optionType, Real effStrike) const {

        Date fixingDate = coupon_->fixingDate();
        if (fixingDate <= Settings::instance().evaluationDate()) {
            // the amount is already known as the fixing is known
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
            QL_REQUIRE(!capletVolatility().empty(),
                       "missing optionlet volatility");
            // not yet known, use Bachelier model here, note that this
            // already has t scaled out
            // N.B. integrated variance is called "total" rather than
            // "black" because the volatility surface does not know
            // what sort it is ... the user is responsible for
            // ensuring that it has the correct contents
            Rate fixing = bachelierFormula(
                       optionType,
                       effStrike,
                       adjustedFixing(),
                       std::sqrt(capletVolatility()->totalVariance(fixingDate,
                                                                   effStrike)));
            return fixing* coupon_->accrualPeriod()*discount_;
        }
    }

}














