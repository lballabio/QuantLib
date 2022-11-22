/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
  Copyright (C) 2014, 2015, 2018 Peter Caspers

  This file is part of QuantLib, a free-software/open-source library
  for financial quantitative analysts and developers - http://quantlib.org/

  QuantLib is free software: you can redistribute it and/or modify it
  under the terms of the QuantLib license.  You should have received a
  copy of the license along with this program; if not, please email
  <quantlib-dev@lists.sf.net>. The license is also available online at
  <http://quantlib.org/license.shtml>.


  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
*/

/*! \file lognormalcmsspreadpricer.cpp
*/

#include <ql/experimental/coupons/cmsspreadcoupon.hpp>
#include <ql/experimental/coupons/lognormalcmsspreadpricer.hpp>
#include <ql/math/integrals/kronrodintegral.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolcube.hpp>
#include <utility>


using std::sqrt;

namespace QuantLib {

    class LognormalCmsSpreadPricer::integrand_f {
        const LognormalCmsSpreadPricer* pricer;
      public:
        explicit integrand_f(const LognormalCmsSpreadPricer* pricer)
        : pricer(pricer) {}
        Real operator()(Real x) const {
            return pricer->integrand(x);
        }
    };

    LognormalCmsSpreadPricer::LognormalCmsSpreadPricer(
        const ext::shared_ptr<CmsCouponPricer>& cmsPricer,
        const Handle<Quote>& correlation,
        Handle<YieldTermStructure> couponDiscountCurve,
        const Size integrationPoints,
        const boost::optional<VolatilityType>& volatilityType,
        const Real shift1,
        const Real shift2)
    : CmsSpreadCouponPricer(correlation), cmsPricer_(cmsPricer),
      couponDiscountCurve_(std::move(couponDiscountCurve)) {

        registerWith(correlation);
        if (!couponDiscountCurve_.empty())
            registerWith(couponDiscountCurve_);
        registerWith(cmsPricer_);

        QL_REQUIRE(integrationPoints >= 4,
                   "at least 4 integration points should be used ("
                       << integrationPoints << ")");
        integrator_ =
            ext::make_shared<GaussHermiteIntegration>(integrationPoints);

        cnd_ = ext::make_shared<CumulativeNormalDistribution>(0.0, 1.0);

        if(volatilityType == boost::none) {
            QL_REQUIRE(shift1 == Null<Real>() && shift2 == Null<Real>(),
                       "if volatility type is inherited, no shifts should be "
                       "specified");
            inheritedVolatilityType_ = true;
            volType_ = cmsPricer->swaptionVolatility()->volatilityType();
        } else {
            shift1_ = shift1 == Null<Real>() ? 0.0 : shift1;
            shift2_ = shift2 == Null<Real>() ? 0.0 : shift2;
            inheritedVolatilityType_ = false;
            volType_ = *volatilityType;
        }
    }

    Real LognormalCmsSpreadPricer::integrand(const Real x) const {

        // this is Brigo, 13.16.2 with x = v / sqrt(2)

        Real v = M_SQRT2 * x;
        Real h =
            k_ - b_ * s2_ * std::exp((m2_ - 0.5 * v2_ * v2_) * fixingTime_ +
                                     v2_ * std::sqrt(fixingTime_) * v);
        Real phi1, phi2;
        phi1 = (*cnd_)(
            phi_ * (std::log(a_ * s1_ / h) +
                    (m1_ + (0.5 - rho_ * rho_) * v1_ * v1_) * fixingTime_ +
                    rho_ * v1_ * std::sqrt(fixingTime_) * v) /
            (v1_ * std::sqrt(fixingTime_ * (1.0 - rho_ * rho_))));
        phi2 = (*cnd_)(
            phi_ * (std::log(a_ * s1_ / h) +
                    (m1_ - 0.5 * v1_ * v1_) * fixingTime_ +
                    rho_ * v1_ * std::sqrt(fixingTime_) * v) /
            (v1_ * std::sqrt(fixingTime_ * (1.0 - rho_ * rho_))));
        Real f = a_ * phi_ * s1_ *
                     std::exp(m1_ * fixingTime_ -
                              0.5 * rho_ * rho_ * v1_ * v1_ * fixingTime_ +
                              rho_ * v1_ * std::sqrt(fixingTime_) * v) *
                     phi1 -
                 phi_ * h * phi2;
        return std::exp(-x * x) * f;
    }

    Real LognormalCmsSpreadPricer::integrand_normal(const Real x) const {

        // this is http://ssrn.com/abstract=2686998, 3.20 with x = s / sqrt(2)

        Real s = M_SQRT2 * x;

        Real beta =
            phi_ *
            (gearing1_ * adjustedRate1_ + gearing2_ * adjustedRate2_ - k_ +
             std::sqrt(fixingTime_) *
                 (rho_ * gearing1_ * vol1_ + gearing2_ * vol2_) * s);
        Real f =
            close_enough(alpha_, 0.0)
                ? Real(std::max(beta, 0.0))
                : psi_ * alpha_ / (M_SQRTPI * M_SQRT2) *
                          std::exp(-beta * beta / (2.0 * alpha_ * alpha_)) +
                      beta * (1.0 - (*cnd_)(-psi_ * beta / alpha_));
        return std::exp(-x * x) * f;
    }

    void
    LognormalCmsSpreadPricer::initialize(const FloatingRateCoupon &coupon) {

        coupon_ = dynamic_cast<const CmsSpreadCoupon *>(&coupon);
        QL_REQUIRE(coupon_, "CMS spread coupon needed");
        index_ = coupon_->swapSpreadIndex();
        gearing_ = coupon_->gearing();
        spread_ = coupon_->spread();

        fixingDate_ = coupon_->fixingDate();
        paymentDate_ = coupon_->date();

        // if no coupon discount curve is given just use the discounting curve
        // from the _first_ swap index.
        // for rate calculation this curve cancels out in the computation, so
        // e.g. the discounting
        // swap engine will produce correct results, even if the
        // couponDiscountCurve is not set here.
        // only the price member function in this class will be dependent on the
        // coupon discount curve.

        today_ = QuantLib::Settings::instance().evaluationDate();

        if (couponDiscountCurve_.empty())
            couponDiscountCurve_ =
                index_->swapIndex1()->exogenousDiscount()
                    ? index_->swapIndex1()->discountingTermStructure()
                    : index_->swapIndex1()->forwardingTermStructure();

        discount_ = paymentDate_ > couponDiscountCurve_->referenceDate()
                        ? couponDiscountCurve_->discount(paymentDate_)
                        : 1.0;

        spreadLegValue_ = spread_ * coupon_->accrualPeriod() * discount_;

        gearing1_ = index_->gearing1();
        gearing2_ = index_->gearing2();

        QL_REQUIRE(gearing1_ > 0.0 && gearing2_ < 0.0,
                   "gearing1 (" << gearing1_
                                << ") should be positive while gearing2 ("
                                << gearing2_ << ") should be negative");

        c1_ = ext::shared_ptr<CmsCoupon>(new CmsCoupon(
            coupon_->date(), coupon_->nominal(), coupon_->accrualStartDate(),
            coupon_->accrualEndDate(), coupon_->fixingDays(),
            index_->swapIndex1(), 1.0, 0.0, coupon_->referencePeriodStart(),
            coupon_->referencePeriodEnd(), coupon_->dayCounter(),
            coupon_->isInArrears()));

        c2_ = ext::shared_ptr<CmsCoupon>(new CmsCoupon(
            coupon_->date(), coupon_->nominal(), coupon_->accrualStartDate(),
            coupon_->accrualEndDate(), coupon_->fixingDays(),
            index_->swapIndex2(), 1.0, 0.0, coupon_->referencePeriodStart(),
            coupon_->referencePeriodEnd(), coupon_->dayCounter(),
            coupon_->isInArrears()));

        c1_->setPricer(cmsPricer_);
        c2_->setPricer(cmsPricer_);

        if (fixingDate_ > today_) {

            fixingTime_ = cmsPricer_->swaptionVolatility()->timeFromReference(
                fixingDate_);

            swapRate1_ = c1_->indexFixing();
            swapRate2_ = c2_->indexFixing();

            adjustedRate1_ = c1_->adjustedFixing();
            adjustedRate2_ = c2_->adjustedFixing();

            ext::shared_ptr<SwaptionVolatilityStructure> swvol =
                *cmsPricer_->swaptionVolatility();
            ext::shared_ptr<SwaptionVolatilityCube> swcub =
                ext::dynamic_pointer_cast<SwaptionVolatilityCube>(swvol);

            if(inheritedVolatilityType_ && volType_ == ShiftedLognormal) {
                shift1_ =
                    swvol->shift(fixingDate_, index_->swapIndex1()->tenor());
                shift2_ =
                    swvol->shift(fixingDate_, index_->swapIndex2()->tenor());
            }

            if (swcub == nullptr) {
                // not a cube, just an atm surface given, so we can
                // not easily convert volatilities and just forbid it
                QL_REQUIRE(inheritedVolatilityType_,
                           "if only an atm surface is given, the volatility "
                           "type must be inherited");
                vol1_ = swvol->volatility(
                    fixingDate_, index_->swapIndex1()->tenor(), swapRate1_);
                vol2_ = swvol->volatility(
                    fixingDate_, index_->swapIndex2()->tenor(), swapRate2_);
            } else {
                vol1_ = swcub->smileSection(fixingDate_,
                                            index_->swapIndex1()->tenor())
                            ->volatility(swapRate1_, volType_, shift1_);
                vol2_ = swcub->smileSection(fixingDate_,
                                            index_->swapIndex2()->tenor())
                            ->volatility(swapRate2_, volType_, shift2_);
            }

            if(volType_ == ShiftedLognormal) {
                mu1_ = 1.0 / fixingTime_ * std::log((adjustedRate1_ + shift1_) /
                                                    (swapRate1_ + shift1_));
                mu2_ = 1.0 / fixingTime_ * std::log((adjustedRate2_ + shift2_) /
                                                    (swapRate2_ + shift2_));
            }
            // for the normal volatility case we do not need the drifts
            // but rather use adjusted rates directly in the integrand

            rho_ = std::max(std::min(correlation()->value(), 0.9999),
                            -0.9999); // avoid division by zero in integrand
        } else {
            // fixing is in the past or today
            adjustedRate1_ = c1_->indexFixing();
            adjustedRate2_ = c2_->indexFixing();
        }
    }

    Real LognormalCmsSpreadPricer::optionletPrice(Option::Type optionType,
                                                  Real strike) const {
        // this method is only called for future fixings
        optionType_ = optionType;
        phi_ = optionType == Option::Call ? 1.0 : -1.0;
        Real res = 0.0;
        if (volType_ == ShiftedLognormal) {
            // (shifted) lognormal volatility
            if (strike >= 0.0) {
                a_ = gearing1_;
                b_ = gearing2_;
                s1_ = swapRate1_ + shift1_;
                s2_ = swapRate2_ + shift2_;
                m1_ = mu1_;
                m2_ = mu2_;
                v1_ = vol1_;
                v2_ = vol2_;
                k_ = strike + gearing1_ * shift1_ + gearing2_ * shift2_;
            } else {
                a_ = -gearing2_;
                b_ = -gearing1_;
                s1_ = swapRate2_ + shift1_;
                s2_ = swapRate1_ + shift2_;
                m1_ = mu2_;
                m2_ = mu1_;
                v1_ = vol2_;
                v2_ = vol1_;
                k_ = -strike - gearing1_ * shift1_ - gearing2_ * shift2_;
                res += phi_ * (gearing1_ * adjustedRate1_ +
                               gearing2_ * adjustedRate2_ - strike);
            }
            res +=
                1.0 / M_SQRTPI * (*integrator_)(integrand_f(this));
        } else {
            // normal volatility
            Real forward = gearing1_ * adjustedRate1_ +
                gearing2_ * adjustedRate2_;
            Real stddev =
                std::sqrt(fixingTime_ *
                          (gearing1_ * gearing1_ * vol1_ * vol1_ +
                           gearing2_ * gearing2_ * vol2_ * vol2_ +
                           2.0 * gearing1_ * gearing2_ * rho_ * vol1_ * vol2_));
            res =
                bachelierBlackFormula(optionType_, strike, forward, stddev, 1.0);
        }
        return res * discount_ * coupon_->accrualPeriod();
    }

    Rate LognormalCmsSpreadPricer::swapletRate() const {
        return swapletPrice() / (coupon_->accrualPeriod() * discount_);
    }

    Real LognormalCmsSpreadPricer::capletPrice(Rate effectiveCap) const {
        // caplet is equivalent to call option on fixing
        if (fixingDate_ <= today_) {
            // the fixing is determined
            const Rate Rs = std::max(
                coupon_->index()->fixing(fixingDate_) - effectiveCap, 0.);
            Rate price = gearing_ * Rs * coupon_->accrualPeriod() * discount_;
            return price;
        } else {
            Real capletPrice = optionletPrice(Option::Call, effectiveCap);
            return gearing_ * capletPrice;
        }
    }

    Rate LognormalCmsSpreadPricer::capletRate(Rate effectiveCap) const {
        return capletPrice(effectiveCap) /
               (coupon_->accrualPeriod() * discount_);
    }

    Real LognormalCmsSpreadPricer::floorletPrice(Rate effectiveFloor) const {
        // floorlet is equivalent to put option on fixing
        if (fixingDate_ <= today_) {
            // the fixing is determined
            const Rate Rs = std::max(
                effectiveFloor - coupon_->index()->fixing(fixingDate_), 0.);
            Rate price = gearing_ * Rs * coupon_->accrualPeriod() * discount_;
            return price;
        } else {
            Real floorletPrice = optionletPrice(Option::Put, effectiveFloor);
            return gearing_ * floorletPrice;
        }
    }

    Rate LognormalCmsSpreadPricer::floorletRate(Rate effectiveFloor) const {
        return floorletPrice(effectiveFloor) /
               (coupon_->accrualPeriod() * discount_);
    }

    Real LognormalCmsSpreadPricer::swapletPrice() const {
        return gearing_ * coupon_->accrualPeriod() * discount_ *
                   (gearing1_ * adjustedRate1_ + gearing2_ * adjustedRate2_) +
               spreadLegValue_;
    }
}
