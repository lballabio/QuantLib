/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Jose Aparicio
 Copyright (C) 2014 Peter Caspers

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

#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/instruments/claim.hpp>
#include <ql/math/interpolations/forwardflatinterpolation.hpp>
#include <ql/pricingengines/credit/isdacdsengine.hpp>
#include <ql/termstructures/credit/flathazardrate.hpp>
#include <ql/termstructures/credit/piecewisedefaultcurve.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/time/calendars/weekendsonly.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <utility>

namespace QuantLib {

    IsdaCdsEngine::IsdaCdsEngine(Handle<DefaultProbabilityTermStructure> probability,
                                 Real recoveryRate,
                                 Handle<YieldTermStructure> discountCurve,
                                 const boost::optional<bool>& includeSettlementDateFlows,
                                 const NumericalFix numericalFix,
                                 const AccrualBias accrualBias,
                                 const ForwardsInCouponPeriod forwardsInCouponPeriod)
    : probability_(std::move(probability)), recoveryRate_(recoveryRate),
      discountCurve_(std::move(discountCurve)),
      includeSettlementDateFlows_(includeSettlementDateFlows), numericalFix_(numericalFix),
      accrualBias_(accrualBias), forwardsInCouponPeriod_(forwardsInCouponPeriod) {

        registerWith(probability_);
        registerWith(discountCurve_);
    }

    void IsdaCdsEngine::calculate() const {

        QL_REQUIRE(numericalFix_ == None || numericalFix_ == Taylor,
                   "numerical fix must be None or Taylor");
        QL_REQUIRE(accrualBias_ == HalfDayBias || accrualBias_ == NoBias,
                   "accrual bias must be HalfDayBias or NoBias");
        QL_REQUIRE(forwardsInCouponPeriod_ == Flat ||
                       forwardsInCouponPeriod_ == Piecewise,
                   "forwards in coupon period must be Flat or Piecewise");

        // it would be possible to handle the cases which are excluded below,
        // but the ISDA engine is not explicitly specified to handle them,
        // so we just forbid them too

        Actual365Fixed dc;
        Actual360 dc1;
        Actual360 dc2(true);

        Date evalDate = Settings::instance().evaluationDate();

        // check if given curves are ISDA compatible
        // (the interpolation is checked below)

        QL_REQUIRE(!discountCurve_.empty(), "no discount term structure set");
        QL_REQUIRE(!probability_.empty(), "no probability term structure set");
        QL_REQUIRE(discountCurve_->dayCounter() == dc,
                   "yield term structure day counter ("
                       << discountCurve_->dayCounter()
                       << ") should be Act/365(Fixed)");
        QL_REQUIRE(probability_->dayCounter() == dc,
                   "probability term structure day counter ("
                       << probability_->dayCounter() << ") should be "
                       << "Act/365(Fixed)");
        QL_REQUIRE(discountCurve_->referenceDate() == evalDate,
                   "yield term structure reference date ("
                       << discountCurve_->referenceDate()
                       << " should be evaluation date (" << evalDate << ")");
        QL_REQUIRE(probability_->referenceDate() == evalDate,
                   "probability term structure reference date ("
                       << probability_->referenceDate()
                       << " should be evaluation date (" << evalDate << ")");
        QL_REQUIRE(arguments_.settlesAccrual,
                   "ISDA engine not compatible with non accrual paying CDS");
        QL_REQUIRE(arguments_.paysAtDefaultTime,
                   "ISDA engine not compatible with end period payment");
        QL_REQUIRE(ext::dynamic_pointer_cast<FaceValueClaim>(arguments_.claim) != nullptr,
                   "ISDA engine not compatible with non face value claim");

        Date maturity = arguments_.maturity;
        Date effectiveProtectionStart =
            std::max<Date>(arguments_.protectionStart, evalDate + 1);

        // collect nodes from both curves and sort them
        std::vector<Date> yDates, cDates;

        // the calls to dates() below might not trigger bootstrap (because
        // they will call the InterpolatedCurve methods, not the ones from
        // PiecewiseYieldCurve or PiecewiseDefaultCurve) so we force it here
        discountCurve_->discount(0.0);
        probability_->defaultProbability(0.0);

        if(ext::shared_ptr<InterpolatedDiscountCurve<LogLinear> > castY1 =
            ext::dynamic_pointer_cast<
                InterpolatedDiscountCurve<LogLinear> >(*discountCurve_)) {
            yDates = castY1->dates();
        } else if(ext::shared_ptr<InterpolatedForwardCurve<BackwardFlat> >
        castY2 = ext::dynamic_pointer_cast<
            InterpolatedForwardCurve<BackwardFlat> >(*discountCurve_)) {
            yDates = castY2->dates();
        } else if(ext::shared_ptr<InterpolatedForwardCurve<ForwardFlat> >
        castY3 = ext::dynamic_pointer_cast<
            InterpolatedForwardCurve<ForwardFlat> >(*discountCurve_)) {
            yDates = castY3->dates();
        } else if(ext::shared_ptr<FlatForward> castY4 =
            ext::dynamic_pointer_cast<FlatForward>(*discountCurve_)) {
            // no dates to extract
        } else {
            QL_FAIL("Yield curve must be flat forward interpolated");
        }

        if(ext::shared_ptr<InterpolatedSurvivalProbabilityCurve<LogLinear> >
        castC1 = ext::dynamic_pointer_cast<
            InterpolatedSurvivalProbabilityCurve<LogLinear> >(
            *probability_)) {
            cDates = castC1->dates();
        } else if(
        ext::shared_ptr<InterpolatedHazardRateCurve<BackwardFlat> > castC2 =
            ext::dynamic_pointer_cast<
            InterpolatedHazardRateCurve<BackwardFlat> >(*probability_)) {
            cDates = castC2->dates();
        } else if(
        ext::shared_ptr<FlatHazardRate> castC3 =
            ext::dynamic_pointer_cast<FlatHazardRate>(*probability_)) {
            // no dates to extract
        } else{
            QL_FAIL("Credit curve must be flat forward interpolated");
        }

        std::vector<Date> nodes;
        std::set_union(yDates.begin(), yDates.end(), cDates.begin(), cDates.end(), std::back_inserter(nodes));


        if(nodes.empty()){
            nodes.push_back(maturity);
        }
        const Real nFix = (numericalFix_ == None ? 1E-50 : 0.0);

        // protection leg pricing (npv is always negative at this stage)
        Real protectionNpv = 0.0;

        Date d0 = effectiveProtectionStart-1;
        Real P0 = discountCurve_->discount(d0);
        Real Q0 = probability_->survivalProbability(d0);
        Date d1;
        std::vector<Date>::const_iterator it =
            std::upper_bound(nodes.begin(), nodes.end(), effectiveProtectionStart);

        for(;it != nodes.end(); ++it) {
            if(*it > maturity) {
                d1 = maturity;
                it = nodes.end() - 1; //early exit
            } else {
                d1 = *it;
            }
            Real P1 = discountCurve_->discount(d1);
            Real Q1 = probability_->survivalProbability(d1);

            Real fhat = std::log(P0) - std::log(P1);
            Real hhat = std::log(Q0) - std::log(Q1);
            Real fhphh = fhat + hhat;

            if (fhphh < 1E-4 && numericalFix_ == Taylor) {
                Real fhphhq = fhphh * fhphh;
                protectionNpv +=
                    P0 * Q0 * hhat * (1.0 - 0.5 * fhphh + 1.0 / 6.0 * fhphhq -
                                      1.0 / 24.0 * fhphhq * fhphh +
                                      1.0 / 120 * fhphhq * fhphhq);
            } else {
                protectionNpv += hhat / (fhphh + nFix) * (P0 * Q0 - P1 * Q1);
            }
            d0 = d1;
            P0 = P1;
            Q0 = Q1;
        }
        protectionNpv *= arguments_.claim->amount(
            Null<Date>(), arguments_.notional, recoveryRate_);

        results_.defaultLegNPV = protectionNpv;

        // premium leg pricing (npv is always positive at this stage)

        Real premiumNpv = 0.0, defaultAccrualNpv = 0.0;
        for (auto& i : arguments_.leg) {
            ext::shared_ptr<FixedRateCoupon> coupon = ext::dynamic_pointer_cast<FixedRateCoupon>(i);

            QL_REQUIRE(coupon->dayCounter() == dc ||
                           coupon->dayCounter() == dc1 ||
                           coupon->dayCounter() == dc2,
                       "ISDA engine requires a coupon day counter Act/365Fixed "
                           << "or Act/360 (" << coupon->dayCounter() << ")");

            // premium coupons
            if (!i->hasOccurred(effectiveProtectionStart, includeSettlementDateFlows_)) {
                premiumNpv +=
                    coupon->amount() *
                    discountCurve_->discount(coupon->date()) *
                    probability_->survivalProbability(coupon->date()-1);
            }

            // default accruals

            if (!detail::simple_event(coupon->accrualEndDate())
                     .hasOccurred(effectiveProtectionStart, false)) {
                Date start = std::max<Date>(coupon->accrualStartDate(),
                                            effectiveProtectionStart)-1;
                Date end = coupon->date()-1;
                Real tstart =
                    discountCurve_->timeFromReference(coupon->accrualStartDate()-1) -
                    (accrualBias_ == HalfDayBias ? 1.0 / 730.0 : 0.0);
                std::vector<Date> localNodes;
                localNodes.push_back(start);
                //add intermediary nodes, if any
                if (forwardsInCouponPeriod_ == Piecewise) {
                    std::vector<Date>::const_iterator it0 =
                        std::upper_bound(nodes.begin(), nodes.end(), start);
                    std::vector<Date>::const_iterator it1 =
                        std::lower_bound(nodes.begin(), nodes.end(), end);
                    localNodes.insert(localNodes.end(), it0, it1);
                }
                localNodes.push_back(end);

                Real defaultAccrThisNode = 0.;
                std::vector<Date>::const_iterator node = localNodes.begin();
                Real t0 = discountCurve_->timeFromReference(*node);
                Real P0 = discountCurve_->discount(*node);
                Real Q0 = probability_->survivalProbability(*node);

                for (++node; node != localNodes.end(); ++node) {
                    Real t1 = discountCurve_->timeFromReference(*node);
                    Real P1 = discountCurve_->discount(*node);
                    Real Q1 = probability_->survivalProbability(*node);
                    Real fhat = std::log(P0) - std::log(P1);
                    Real hhat = std::log(Q0) - std::log(Q1);
                    Real fhphh = fhat + hhat;
                    if (fhphh < 1E-4 && numericalFix_ == Taylor) {
                        // see above, terms up to (f+h)^3 seem more than enough,
                        // what exactly is implemented in the standard isda C
                        // code ?
                        Real fhphhq = fhphh * fhphh;
                        defaultAccrThisNode +=
                            hhat * P0 * Q0 *
                            ((t0 - tstart) *
                                 (1.0 - 0.5 * fhphh + 1.0 / 6.0 * fhphhq -
                                  1.0 / 24.0 * fhphhq * fhphh) +
                             (t1 - t0) *
                                 (0.5 - 1.0 / 3.0 * fhphh + 1.0 / 8.0 * fhphhq -
                                  1.0 / 30.0 * fhphhq * fhphh));
                    } else {
                        defaultAccrThisNode +=
                            (hhat / (fhphh + nFix)) *
                            ((t1 - t0) * ((P0 * Q0 - P1 * Q1) / (fhphh + nFix) -
                                          P1 * Q1) +
                             (t0 - tstart) * (P0 * Q0 - P1 * Q1));
                    }

                    t0 = t1;
                    P0 = P1;
                    Q0 = Q1;
                }
                defaultAccrualNpv += defaultAccrThisNode * arguments_.notional *
                    coupon->rate() * 365. / 360.;
			}
        }


        results_.couponLegNPV = premiumNpv + defaultAccrualNpv;

        // upfront flow npv

        Real upfPVO1 = 0.0;
        results_.upfrontNPV = 0.0;
        if (!arguments_.upfrontPayment->hasOccurred(
                evalDate, includeSettlementDateFlows_)) {
            upfPVO1 =
                discountCurve_->discount(arguments_.upfrontPayment->date());
            if(arguments_.upfrontPayment->amount() != 0.) {
                results_.upfrontNPV = upfPVO1 * arguments_.upfrontPayment->amount();
            }
        }

        results_.accrualRebateNPV = 0.;
        // NOLINTNEXTLINE(readability-implicit-bool-conversion)
        if (arguments_.accrualRebate && arguments_.accrualRebate->amount() != 0. &&
            !arguments_.accrualRebate->hasOccurred(evalDate, includeSettlementDateFlows_)) {
            results_.accrualRebateNPV =
                discountCurve_->discount(arguments_.accrualRebate->date()) *
                arguments_.accrualRebate->amount();
        }

        Real upfrontSign = Protection::Seller != 0U ? 1.0 : -1.0;

        if (arguments_.side == Protection::Seller) {
            results_.defaultLegNPV *= -1.0;
            results_.accrualRebateNPV *= -1.0;
        } else {
            results_.couponLegNPV *= -1.0;
            results_.upfrontNPV *= -1.0;
        }

        results_.value = results_.defaultLegNPV + results_.couponLegNPV +
                         results_.upfrontNPV + results_.accrualRebateNPV;

        results_.errorEstimate = Null<Real>();

        if (results_.couponLegNPV != 0.0) {
            results_.fairSpread =
                -results_.defaultLegNPV * arguments_.spread /
                (results_.couponLegNPV + results_.accrualRebateNPV);
        } else {
            results_.fairSpread = Null<Rate>();
        }

        Real upfrontSensitivity = upfPVO1 * arguments_.notional;
        if (upfrontSensitivity != 0.0) {
            results_.fairUpfront =
                -upfrontSign * (results_.defaultLegNPV + results_.couponLegNPV +
                                results_.accrualRebateNPV) /
                upfrontSensitivity;
        } else {
            results_.fairUpfront = Null<Rate>();
        }

        static const Rate basisPoint = 1.0e-4;

        if (arguments_.spread != 0.0) {
            results_.couponLegBPS =
                results_.couponLegNPV * basisPoint / arguments_.spread;
        } else {
            results_.couponLegBPS = Null<Rate>();
        }

        // NOLINTNEXTLINE(readability-implicit-bool-conversion)
        if (arguments_.upfront && *arguments_.upfront != 0.0) {
            results_.upfrontBPS =
                results_.upfrontNPV * basisPoint / (*arguments_.upfront);
        } else {
            results_.upfrontBPS = Null<Rate>();
        }
    }
}
