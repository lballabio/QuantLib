/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
  Copyright (C) 2014, 2016 Peter Caspers

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

/*! \file lineartsrpricer.cpp
*/

#include <ql/cashflows/cmscoupon.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/lineartsrpricer.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/math/integrals/kronrodintegral.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/atmsmilesection.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/schedule.hpp>
#include <utility>

namespace QuantLib {

    class LinearTsrPricer::integrand_f {
        const LinearTsrPricer* pricer;
      public:
        explicit integrand_f(const LinearTsrPricer* pricer) : pricer(pricer) {}
        Real operator()(Real x) const {
            return pricer->integrand(x);
        }
    };

    const Real LinearTsrPricer::defaultLowerBound = 0.0001,
             LinearTsrPricer::defaultUpperBound = 2.0000;

    LinearTsrPricer::LinearTsrPricer(const Handle<SwaptionVolatilityStructure>& swaptionVol,
                                     Handle<Quote> meanReversion,
                                     Handle<YieldTermStructure> couponDiscountCurve,
                                     const Settings& settings,
                                     ext::shared_ptr<Integrator> integrator)
    : CmsCouponPricer(swaptionVol), meanReversion_(std::move(meanReversion)),
      couponDiscountCurve_(std::move(couponDiscountCurve)), settings_(settings),
      volDayCounter_(swaptionVol->dayCounter()), integrator_(std::move(integrator)) {

        if (!couponDiscountCurve_.empty())
            registerWith(couponDiscountCurve_);

        if (integrator_ == nullptr)
            integrator_ =
                ext::make_shared<GaussKronrodNonAdaptive>(1E-10, 5000, 1E-10);
    }

    Real LinearTsrPricer::GsrG(const Date &d) const {

        Real yf = volDayCounter_.yearFraction(fixingDate_, d);
        if (std::fabs(meanReversion_->value()) < 1.0E-4)
            return yf;
        else
            return (1.0 - std::exp(-meanReversion_->value() * yf)) /
                   meanReversion_->value();
    }

    Real LinearTsrPricer::singularTerms(const Option::Type type,
                                        const Real strike) const {

        Real omega = (type == Option::Call ? 1.0 : -1.0);
        Real s1 = std::max(omega * (swapRateValue_ - strike), 0.0) *
                  (a_ * swapRateValue_ + b_);
        Real s2 = (a_ * strike + b_) *
                  smileSection_->optionPrice(strike, strike < swapRateValue_
                                                         ? Option::Put
                                                         : Option::Call);
        return s1 + s2;
    }

    Real LinearTsrPricer::integrand(const Real strike) const {
        return 2.0 * a_ * smileSection_->optionPrice(
                              strike, strike < swapRateValue_ ? Option::Put
                                                              : Option::Call);
    }

    void LinearTsrPricer::initialize(const FloatingRateCoupon &coupon) {

        coupon_ = dynamic_cast<const CmsCoupon *>(&coupon);
        QL_REQUIRE(coupon_, "CMS coupon needed");
        gearing_ = coupon_->gearing();
        spread_ = coupon_->spread();

        fixingDate_ = coupon_->fixingDate();
        paymentDate_ = coupon_->date();
        swapIndex_ = coupon_->swapIndex();

        forwardCurve_ = swapIndex_->forwardingTermStructure();
        if (swapIndex_->exogenousDiscount())
            discountCurve_ = swapIndex_->discountingTermStructure();
        else
            discountCurve_ = forwardCurve_;

        // if no coupon discount curve is given just use the discounting curve
        // from the swap index. for rate calculation this curve cancels out in
        // the computation, so e.g. the discounting swap engine will produce
        // correct results, even if the couponDiscountCurve is not set here.
        // only the price member function in this class will be dependent on the
        // coupon discount curve.

        today_ = QuantLib::Settings::instance().evaluationDate();

        if (paymentDate_ > today_ && !couponDiscountCurve_.empty())
            couponDiscountRatio_ =
                couponDiscountCurve_->discount(paymentDate_) /
                discountCurve_->discount(paymentDate_);
        else
            couponDiscountRatio_ = 1.;

        spreadLegValue_ = spread_ * coupon_->accrualPeriod() *
                          discountCurve_->discount(paymentDate_) *
                          couponDiscountRatio_;

        if (fixingDate_ > today_) {

            swapTenor_ = swapIndex_->tenor();
            swap_ = swapIndex_->underlyingSwap(fixingDate_);

            swapRateValue_ = swap_->fairRate();
            annuity_ = 1.0E4 * std::fabs(swap_->fixedLegBPS());

            ext::shared_ptr<SmileSection> sectionTmp =
                swaptionVolatility()->smileSection(fixingDate_, swapTenor_);

            adjustedLowerBound_ = settings_.lowerRateBound_;
            adjustedUpperBound_ = settings_.upperRateBound_;

            if(sectionTmp->volatilityType() == Normal) {
                // adjust lower bound if it was not set explicitly
                if(settings_.defaultBounds_)
                    adjustedLowerBound_ = std::min(adjustedLowerBound_, -adjustedUpperBound_);
            } else {
                // adjust bounds by section's shift
                adjustedLowerBound_ -= sectionTmp->shift();
                adjustedUpperBound_ -= sectionTmp->shift();
            }

            // if the section does not provide an atm level, we enhance it to
            // have one, no need to exit with an exception ...

            if (sectionTmp->atmLevel() == Null<Real>())
                smileSection_ = ext::make_shared<AtmSmileSection>(
                    sectionTmp, swapRateValue_);
            else
                smileSection_ = sectionTmp;

            // compute linear model's parameters

            Real gx = 0.0, gy = 0.0;
            for (const auto& i : swap_->fixedLeg()) {
                ext::shared_ptr<Coupon> c = ext::dynamic_pointer_cast<Coupon>(i);
                Real yf = c->accrualPeriod();
                Date d = c->date();
                Real pv = yf * discountCurve_->discount(d);
                gx += pv * GsrG(d);
                gy += pv;
            }

            Real gamma = gx / gy;
            Date lastd = swap_->fixedLeg().back()->date();

            a_ = discountCurve_->discount(paymentDate_) *
                 (gamma - GsrG(paymentDate_)) /
                 (discountCurve_->discount(lastd) * GsrG(lastd) +
                  swapRateValue_ * gy * gamma);

            b_ = discountCurve_->discount(paymentDate_) / gy -
                 a_ * swapRateValue_;
        }
    }

    Real LinearTsrPricer::strikeFromVegaRatio(Real ratio,
                                              Option::Type optionType,
                                              Real referenceStrike) const {

        Real a, b, min, max, k;
        if (optionType == Option::Call) {
            a = swapRateValue_;
            min = referenceStrike;
            // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
            b = max = k =
                std::min(smileSection_->maxStrike(), adjustedUpperBound_);
        } else {
            // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
            a = min = k =
                std::max(smileSection_->minStrike(), adjustedLowerBound_);
            b = swapRateValue_;
            max = referenceStrike;
        }

        VegaRatioHelper h(&*smileSection_,
                          smileSection_->vega(swapRateValue_) * ratio);
        Brent solver;

        try {
            k = solver.solve(h, 1.0E-5, (a + b) / 2.0, a, b);
        }
        catch (...) {
            // use default value set above
        }

        return std::min(std::max(k, min), max);
    }

    Real LinearTsrPricer::strikeFromPrice(Real price, Option::Type optionType,
                                          Real referenceStrike) const {

        Real a, b, min, max, k;
        if (optionType == Option::Call) {
            a = swapRateValue_;
            min = referenceStrike;
            // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
            b = max = k =
                std::min(smileSection_->maxStrike(), adjustedUpperBound_);
        } else {
            // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
            a = min = k =
                std::max(smileSection_->minStrike(), adjustedLowerBound_);
            b = swapRateValue_;
            max = referenceStrike;
        }

        PriceHelper h(&*smileSection_, optionType, price);
        Brent solver;

        try {
            k = solver.solve(h, 1.0E-5, swapRateValue_, a, b);
        }
        catch (...) {
            // use default value set above
        }

        return std::min(std::max(k, min), max);
    }

    Real LinearTsrPricer::optionletPrice(Option::Type optionType,
                                         Real strike) const {

        if (optionType == Option::Call && strike >= adjustedUpperBound_)
            return 0.0;
        if (optionType == Option::Put && strike <= adjustedLowerBound_)
            return 0.0;

        // determine lower or upper integration bound (depending on option type)

        Real lower = strike, upper = strike;

        switch (settings_.strategy_) {

        case Settings::RateBound: {
            if (optionType == Option::Call)
                upper = adjustedUpperBound_;
            else
                lower = adjustedLowerBound_;
            break;
        }

        case Settings::VegaRatio: {
            // strikeFromVegaRatio ensures that returned strike is on the
            // expected side of strike
            Real bound =
                strikeFromVegaRatio(settings_.vegaRatio_, optionType, strike);
            if (optionType == Option::Call)
                upper = std::min(bound, adjustedUpperBound_);
            else
                lower = std::max(bound, adjustedLowerBound_);
            break;
        }

        case Settings::PriceThreshold: {
            // strikeFromPrice ensures that returned strike is on the expected
            // side of strike
            Real bound =
                strikeFromPrice(settings_.vegaRatio_, optionType, strike);
            if (optionType == Option::Call)
                upper = std::min(bound, adjustedUpperBound_);
            else
                lower = std::max(bound, adjustedLowerBound_);
            break;
        }

        case Settings::BSStdDevs : {
            Real atm = smileSection_->atmLevel();
            Real atmVol = smileSection_->volatility(atm);
            Real shift = smileSection_->shift();
            Real lowerTmp, upperTmp;
            if (smileSection_->volatilityType() == ShiftedLognormal) {
                upperTmp = (atm + shift) *
                               std::exp(settings_.stdDevs_ * atmVol -
                                        0.5 * atmVol * atmVol *
                                            smileSection_->exerciseTime()) -
                           shift;
                lowerTmp = (atm + shift) *
                               std::exp(-settings_.stdDevs_ * atmVol -
                                        0.5 * atmVol * atmVol *
                                            smileSection_->exerciseTime()) -
                           shift;
            } else {
                Real tmp = settings_.stdDevs_ * atmVol *
                           std::sqrt(smileSection_->exerciseTime());
                upperTmp = atm + tmp;
                lowerTmp = atm - tmp;
            }
            upper = std::min(upperTmp - shift, adjustedUpperBound_);
            lower = std::max(lowerTmp - shift, adjustedLowerBound_);
            break;
        }

        default:
            QL_FAIL("Unknown strategy (" << settings_.strategy_ << ")");
        }

        // compute the relevant integral

        Real result = 0.0;
        Real tmpBound;
        if (upper > lower) {
            tmpBound = std::min(upper, swapRateValue_);
            if (tmpBound > lower) {
                result += (*integrator_)(integrand_f(this),
                                         lower, tmpBound);
            }
            tmpBound = std::max(lower, swapRateValue_);
            if (upper > tmpBound) {
                result += (*integrator_)(integrand_f(this),
                                         tmpBound, upper);
            }
            result *= (optionType == Option::Call ? 1.0 : -1.0);
        }

        result += singularTerms(optionType, strike);

        return annuity_ * result * couponDiscountRatio_ *
               coupon_->accrualPeriod();
    }

    Real LinearTsrPricer::meanReversion() const { return meanReversion_->value(); }

    Rate LinearTsrPricer::swapletRate() const {
        return swapletPrice() /
               (coupon_->accrualPeriod() *
                discountCurve_->discount(paymentDate_) * couponDiscountRatio_);
    }

    Real LinearTsrPricer::capletPrice(Rate effectiveCap) const {
        // caplet is equivalent to call option on fixing
        if (fixingDate_ <= today_) {
            // the fixing is determined
            const Rate Rs = std::max(
                coupon_->swapIndex()->fixing(fixingDate_) - effectiveCap, 0.);
            Rate price =
                (gearing_ * Rs) *
                (coupon_->accrualPeriod() *
                 discountCurve_->discount(paymentDate_) * couponDiscountRatio_);
            return price;
        } else {
            Real capletPrice = optionletPrice(Option::Call, effectiveCap);
            return gearing_ * capletPrice;
        }
    }

    Rate LinearTsrPricer::capletRate(Rate effectiveCap) const {
        return capletPrice(effectiveCap) /
               (coupon_->accrualPeriod() *
                discountCurve_->discount(paymentDate_) * couponDiscountRatio_);
    }

    Real LinearTsrPricer::floorletPrice(Rate effectiveFloor) const {
        // floorlet is equivalent to put option on fixing
        if (fixingDate_ <= today_) {
            // the fixing is determined
            const Rate Rs = std::max(
                effectiveFloor - coupon_->swapIndex()->fixing(fixingDate_), 0.);
            Rate price =
                (gearing_ * Rs) *
                (coupon_->accrualPeriod() *
                 discountCurve_->discount(paymentDate_) * couponDiscountRatio_);
            return price;
        } else {
            Real floorletPrice = optionletPrice(Option::Put, effectiveFloor);
            return gearing_ * floorletPrice;
        }
    }

    Rate LinearTsrPricer::floorletRate(Rate effectiveFloor) const {
        return floorletPrice(effectiveFloor) /
               (coupon_->accrualPeriod() *
                discountCurve_->discount(paymentDate_) * couponDiscountRatio_);
    }

    Real LinearTsrPricer::swapletPrice() const {
        if (fixingDate_ <= today_) {
            // the fixing is determined
            const Rate Rs = coupon_->swapIndex()->fixing(fixingDate_);
            Rate price =
                (gearing_ * Rs + spread_) *
                (coupon_->accrualPeriod() *
                 discountCurve_->discount(paymentDate_) * couponDiscountRatio_);
            return price;
        } else {
            Real atmCapletPrice = optionletPrice(Option::Call, swapRateValue_);
            Real atmFloorletPrice = optionletPrice(Option::Put, swapRateValue_);
            return gearing_ * (coupon_->accrualPeriod() *
                                   discountCurve_->discount(paymentDate_) *
                                   swapRateValue_ * couponDiscountRatio_ +
                               atmCapletPrice - atmFloorletPrice) +
                   spreadLegValue_;
        }
    }
}
