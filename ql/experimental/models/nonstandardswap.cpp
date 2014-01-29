/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

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

#include <ql/experimental/models/nonstandardswap.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/cmscoupon.hpp>
#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    NonstandardSwap::NonstandardSwap(const VanillaSwap &fromVanilla)
        : Swap(2), type_((VanillaSwap::Type)fromVanilla.type()),
          fixedNominal_(std::vector<Real>(fromVanilla.fixedLeg().size(),
                                          fromVanilla.nominal())),
          floatingNominal_(std::vector<Real>(fromVanilla.floatingLeg().size(),
                                             fromVanilla.nominal())),
          fixedSchedule_(fromVanilla.fixedSchedule()),
          fixedRate_(std::vector<Real>(fromVanilla.fixedLeg().size(),
                                       fromVanilla.fixedRate())),
          fixedDayCount_(fromVanilla.fixedDayCount()),
          floatingSchedule_(fromVanilla.floatingSchedule()),
          iborIndex_(fromVanilla.iborIndex()), spread_(fromVanilla.spread()),
          gearing_(1.0), floatingDayCount_(fromVanilla.floatingDayCount()),
          paymentConvention_(fromVanilla.paymentConvention()),
          intermediateCapitalExchange_(false), finalCapitalExchange_(false) {

        init();
    }

    NonstandardSwap::NonstandardSwap(
        const VanillaSwap::Type type, const std::vector<Real> &fixedNominal,
        const std::vector<Real> &floatingNominal, const Schedule &fixedSchedule,
        const std::vector<Real> &fixedRate, const DayCounter &fixedDayCount,
        const Schedule &floatingSchedule,
        const boost::shared_ptr<IborIndex> &iborIndex, const Real gearing,
        const Spread spread, const DayCounter &floatingDayCount,
        const bool intermediateCapitalExchange, const bool finalCapitalExchange,
        boost::optional<BusinessDayConvention> paymentConvention)
        : Swap(2), type_(type), fixedNominal_(fixedNominal),
          floatingNominal_(floatingNominal), fixedSchedule_(fixedSchedule),
          fixedRate_(fixedRate), fixedDayCount_(fixedDayCount),
          floatingSchedule_(floatingSchedule), iborIndex_(iborIndex),
          spread_(spread), gearing_(gearing),
          floatingDayCount_(floatingDayCount),
          intermediateCapitalExchange_(intermediateCapitalExchange),
          finalCapitalExchange_(finalCapitalExchange) {

        QL_REQUIRE(fixedNominal.size() == fixedRate.size(),
                   "Fixed nominal size ("
                       << fixedNominal.size()
                       << ") does not match fixed rate size ("
                       << fixedRate.size() << ")");

        QL_REQUIRE(fixedNominal.size() == fixedSchedule.size() - 1,
                   "Fixed nominal size (" << fixedNominal.size()
                                          << ") does not match schedule size ("
                                          << fixedSchedule.size() << ") - 1");

        QL_REQUIRE(floatingNominal.size() == floatingSchedule.size() - 1,
                   "Floating nominal size ("
                       << floatingNominal.size()
                       << ") does not match schedule size ("
                       << floatingSchedule.size() << ") - 1");

        if (paymentConvention)
            paymentConvention_ = *paymentConvention;
        else
            paymentConvention_ = floatingSchedule_.businessDayConvention();

        // if the gearing is zero then the ibor leg will be set up with fixed
        // coupons
        // which makes trouble here in this context. We therefore use a dirty
        // trick
        // and enforce the gearing to be non zero.
        if (close(gearing_, 0.0))
            gearing_ = QL_EPSILON;

        init();
    }

    void NonstandardSwap::init() {

        legs_[0] = FixedRateLeg(fixedSchedule_)
                       .withNotionals(fixedNominal_)
                       .withCouponRates(fixedRate_, fixedDayCount_)
                       .withPaymentAdjustment(paymentConvention_);

        legs_[1] = IborLeg(floatingSchedule_, iborIndex_)
                       .withNotionals(floatingNominal_)
                       .withPaymentDayCounter(floatingDayCount_)
                       .withPaymentAdjustment(paymentConvention_)
                       .withSpreads(spread_)
                       .withGearings(gearing_);

        if (intermediateCapitalExchange_) {
            for (Size i = 0; i < legs_[0].size() - 1; i++) {
                Real cap = fixedNominal_[i + 1] - fixedNominal_[i];
                if (!close(cap, 0.0)) {
                    std::vector<boost::shared_ptr<CashFlow> >::iterator it1 =
                        legs_[0].begin();
                    std::advance(it1, i + 1);
                    legs_[0].insert(
                        it1, boost::shared_ptr<CashFlow>(
                                 new Redemption(cap, legs_[0][i]->date())));
                    std::vector<Real>::iterator it2 = fixedNominal_.begin();
                    std::advance(it2, i + 1);
                    fixedNominal_.insert(it2, fixedNominal_[i]);
                    i++;
                }
            }
            for (Size i = 0; i < legs_[1].size() - 1; i++) {
                Real cap = floatingNominal_[i + 1] - floatingNominal_[i];
                if (!close(cap, 0.0)) {
                    std::vector<boost::shared_ptr<CashFlow> >::iterator it1 =
                        legs_[1].begin();
                    std::advance(it1, i + 1);
                    legs_[1].insert(
                        it1, boost::shared_ptr<CashFlow>(
                                 new Redemption(cap, legs_[1][i]->date())));
                    std::vector<Real>::iterator it2 = floatingNominal_.begin();
                    std::advance(it2, i + 1);
                    floatingNominal_.insert(it2, floatingNominal_[i]);
                    i++;
                }
            }
        }

        if (finalCapitalExchange_) {
            legs_[0].push_back(boost::shared_ptr<CashFlow>(
                new Redemption(fixedNominal_.back(), legs_[0].back()->date())));
            fixedNominal_.push_back(fixedNominal_.back());
            fixedRate_.push_back(0.0);
            legs_[1].push_back(boost::shared_ptr<CashFlow>(new Redemption(
                floatingNominal_.back(), legs_[1].back()->date())));
            floatingNominal_.push_back(floatingNominal_.back());
        }

        for (Leg::const_iterator i = legs_[1].begin(); i < legs_[1].end(); ++i)
            registerWith(*i);

        switch (type_) {
        case VanillaSwap::Payer:
            payer_[0] = -1.0;
            payer_[1] = +1.0;
            break;
        case VanillaSwap::Receiver:
            payer_[0] = +1.0;
            payer_[1] = -1.0;
            break;
        default:
            QL_FAIL("Unknown nonstandard-swap type");
        }
    }

    void NonstandardSwap::setupArguments(PricingEngine::arguments *args) const {

        Swap::setupArguments(args);

        NonstandardSwap::arguments *arguments =
            dynamic_cast<NonstandardSwap::arguments *>(args);

        if (!arguments)
            return; // swap engine ... // QL_FAIL("argument types do not
                    // match");

        arguments->type = type_;
        arguments->fixedNominal = fixedNominal_;
        arguments->floatingNominal = floatingNominal_;
        arguments->fixedRate = fixedRate_;

        const Leg &fixedCoupons = fixedLeg();

        arguments->fixedResetDates = arguments->fixedPayDates =
            std::vector<Date>(fixedCoupons.size());
        arguments->fixedCoupons = std::vector<Real>(fixedCoupons.size());
        arguments->fixedIsRedemptionFlow =
            std::vector<bool>(fixedCoupons.size(), false);

        for (Size i = 0; i < fixedCoupons.size(); ++i) {
            boost::shared_ptr<FixedRateCoupon> coupon =
                boost::dynamic_pointer_cast<FixedRateCoupon>(fixedCoupons[i]);
            if (coupon) {
                arguments->fixedPayDates[i] = coupon->date();
                arguments->fixedResetDates[i] = coupon->accrualStartDate();
                arguments->fixedCoupons[i] = coupon->amount();
            } else {
                boost::shared_ptr<CashFlow> cashflow =
                    boost::dynamic_pointer_cast<CashFlow>(fixedCoupons[i]);
                std::vector<Date>::const_iterator j =
                    std::find(arguments->fixedPayDates.begin(),
                              arguments->fixedPayDates.end(), cashflow->date());
                QL_REQUIRE(j != arguments->fixedPayDates.end(),
                           "nominal redemption on "
                               << cashflow->date()
                               << "has no corresponding coupon");
                Size jIdx = j - arguments->fixedPayDates.begin();
                arguments->fixedIsRedemptionFlow[i] = true;
                arguments->fixedCoupons[i] = cashflow->amount();
                arguments->fixedResetDates[i] =
                    arguments->fixedResetDates[jIdx];
                arguments->fixedPayDates[i] = cashflow->date();
            }
        }

        const Leg &floatingCoupons = floatingLeg();

        arguments->floatingResetDates = arguments->floatingPayDates =
            arguments->floatingFixingDates =
                std::vector<Date>(floatingCoupons.size());
        arguments->floatingAccrualTimes =
            std::vector<Time>(floatingCoupons.size());
        arguments->floatingSpreads =
            std::vector<Spread>(floatingCoupons.size());
        arguments->floatingGearings = std::vector<Real>(floatingCoupons.size());
        arguments->floatingCoupons = std::vector<Real>(floatingCoupons.size());
        arguments->floatingIsRedemptionFlow =
            std::vector<bool>(floatingCoupons.size(), false);

        for (Size i = 0; i < floatingCoupons.size(); ++i) {
            boost::shared_ptr<IborCoupon> coupon =
                boost::dynamic_pointer_cast<IborCoupon>(floatingCoupons[i]);
            if (coupon) {
                arguments->floatingResetDates[i] = coupon->accrualStartDate();
                arguments->floatingPayDates[i] = coupon->date();
                arguments->floatingFixingDates[i] = coupon->fixingDate();
                arguments->floatingAccrualTimes[i] = coupon->accrualPeriod();
                arguments->floatingSpreads[i] = coupon->spread();
                arguments->floatingGearings[i] = coupon->gearing();
                try {
                    arguments->floatingCoupons[i] = coupon->amount();
                }
                catch (Error &) {
                    arguments->floatingCoupons[i] = Null<Real>();
                }
            } else {
                boost::shared_ptr<CashFlow> cashflow =
                    boost::dynamic_pointer_cast<CashFlow>(floatingCoupons[i]);
                std::vector<Date>::const_iterator j = std::find(
                    arguments->floatingPayDates.begin(),
                    arguments->floatingPayDates.end(), cashflow->date());
                QL_REQUIRE(j != arguments->floatingPayDates.end(),
                           "nominal redemption on "
                               << cashflow->date()
                               << "has no corresponding coupon");
                Size jIdx = j - arguments->floatingPayDates.begin();
                arguments->floatingIsRedemptionFlow[i] = true;
                arguments->floatingCoupons[i] = cashflow->amount();
                arguments->floatingResetDates[i] =
                    arguments->floatingResetDates[jIdx];
                arguments->floatingFixingDates[i] =
                    arguments->floatingFixingDates[jIdx];
                arguments->floatingAccrualTimes[i] = 0.0;
                arguments->floatingSpreads[i] = 0.0;
                arguments->floatingGearings[i] = 1.0;
                arguments->floatingPayDates[i] = cashflow->date();
            }
        }

        arguments->iborIndex = iborIndex();
    }

    void NonstandardSwap::setupExpired() const { Swap::setupExpired(); }

    void NonstandardSwap::fetchResults(const PricingEngine::results *r) const {

        Swap::fetchResults(r);
    }

    void NonstandardSwap::arguments::validate() const {
        Swap::arguments::validate();
        QL_REQUIRE(fixedNominal.size() == fixedPayDates.size(),
                   "number of fixed leg nominals plus redemption flows "
                   "different from number of payment dates");
        QL_REQUIRE(fixedRate.size() == fixedPayDates.size(),
                   "number of fixed rates plus redemption flows different from "
                   "number of payment dates");
        QL_REQUIRE(floatingNominal.size() == floatingPayDates.size(),
                   "number of float leg nominals different from number of "
                   "payment dates");
        QL_REQUIRE(fixedResetDates.size() == fixedPayDates.size(),
                   "number of fixed start dates different from "
                   "number of fixed payment dates");
        QL_REQUIRE(fixedPayDates.size() == fixedCoupons.size(),
                   "number of fixed payment dates different from "
                   "number of fixed coupon amounts");
        QL_REQUIRE(floatingResetDates.size() == floatingPayDates.size(),
                   "number of floating start dates different from "
                   "number of floating payment dates");
        QL_REQUIRE(floatingFixingDates.size() == floatingPayDates.size(),
                   "number of floating fixing dates different from "
                   "number of floating payment dates");
        QL_REQUIRE(floatingAccrualTimes.size() == floatingPayDates.size(),
                   "number of floating accrual Times different from "
                   "number of floating payment dates");
        QL_REQUIRE(floatingSpreads.size() == floatingPayDates.size(),
                   "number of floating spreads different from "
                   "number of floating payment dates");
        QL_REQUIRE(floatingPayDates.size() == floatingCoupons.size(),
                   "number of floating payment dates different from "
                   "number of floating coupon amounts");
    }

    void NonstandardSwap::results::reset() { Swap::results::reset(); }
}
