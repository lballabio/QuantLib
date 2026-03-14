/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013, 2016 Peter Caspers

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/cmscoupon.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/instruments/nonstandardswap.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/optional.hpp>
#include <utility>

namespace QuantLib {

    NonstandardSwap::NonstandardSwap(const FixedVsFloatingSwap &fromVanilla)
        : Swap(2), type_(fromVanilla.type()),
          fixedNominal_(std::vector<Real>(fromVanilla.fixedLeg().size(),
                                          fromVanilla.nominal())),
          floatingNominal_(std::vector<Real>(fromVanilla.floatingLeg().size(),
                                             fromVanilla.nominal())),
          fixedSchedule_(fromVanilla.fixedSchedule()),
          fixedRate_(std::vector<Real>(fromVanilla.fixedLeg().size(),
                                       fromVanilla.fixedRate())),
          fixedDayCount_(fromVanilla.fixedDayCount()),
          floatingSchedule_(fromVanilla.floatingSchedule()),
          iborIndex_(fromVanilla.iborIndex()),
          spread_(std::vector<Real>(fromVanilla.floatingLeg().size(), fromVanilla.spread())),
          gearing_(std::vector<Real>(fromVanilla.floatingLeg().size(), 1.0)),
          singleSpreadAndGearing_(true),
          floatingDayCount_(fromVanilla.floatingDayCount()),
          paymentConvention_(fromVanilla.paymentConvention()),
          intermediateCapitalExchange_(false), finalCapitalExchange_(false) {

        init();
    }

    NonstandardSwap::NonstandardSwap(const Swap::Type type,
                                     std::vector<Real> fixedNominal,
                                     const std::vector<Real>& floatingNominal,
                                     Schedule fixedSchedule,
                                     std::vector<Real> fixedRate,
                                     DayCounter fixedDayCount,
                                     Schedule floatingSchedule,
                                     ext::shared_ptr<IborIndex> iborIndex,
                                     const Real gearing,
                                     const Spread spread,
                                     DayCounter floatingDayCount,
                                     const bool intermediateCapitalExchange,
                                     const bool finalCapitalExchange,
                                     ext::optional<BusinessDayConvention> paymentConvention)
    : Swap(2), type_(type), fixedNominal_(std::move(fixedNominal)),
      floatingNominal_(floatingNominal), fixedSchedule_(std::move(fixedSchedule)),
      fixedRate_(std::move(fixedRate)), fixedDayCount_(std::move(fixedDayCount)),
      floatingSchedule_(std::move(floatingSchedule)), iborIndex_(std::move(iborIndex)),
      spread_(std::vector<Real>(floatingNominal.size(), spread)),
      gearing_(std::vector<Real>(floatingNominal.size(), gearing)), singleSpreadAndGearing_(true),
      floatingDayCount_(std::move(floatingDayCount)),
      intermediateCapitalExchange_(intermediateCapitalExchange),
      finalCapitalExchange_(finalCapitalExchange) {

        if (paymentConvention) // NOLINT(readability-implicit-bool-conversion)
            paymentConvention_ = *paymentConvention;
        else
            paymentConvention_ = floatingSchedule_.businessDayConvention();
        init();
    }

    NonstandardSwap::NonstandardSwap(const Swap::Type type,
                                     std::vector<Real> fixedNominal,
                                     std::vector<Real> floatingNominal,
                                     Schedule fixedSchedule,
                                     std::vector<Real> fixedRate,
                                     DayCounter fixedDayCount,
                                     Schedule floatingSchedule,
                                     ext::shared_ptr<IborIndex> iborIndex,
                                     std::vector<Real> gearing,
                                     std::vector<Spread> spread,
                                     DayCounter floatingDayCount,
                                     const bool intermediateCapitalExchange,
                                     const bool finalCapitalExchange,
                                     ext::optional<BusinessDayConvention> paymentConvention)
    : Swap(2), type_(type), fixedNominal_(std::move(fixedNominal)),
      floatingNominal_(std::move(floatingNominal)), fixedSchedule_(std::move(fixedSchedule)),
      fixedRate_(std::move(fixedRate)), fixedDayCount_(std::move(fixedDayCount)),
      floatingSchedule_(std::move(floatingSchedule)), iborIndex_(std::move(iborIndex)),
      spread_(std::move(spread)), gearing_(std::move(gearing)), singleSpreadAndGearing_(false),
      floatingDayCount_(std::move(floatingDayCount)),
      intermediateCapitalExchange_(intermediateCapitalExchange),
      finalCapitalExchange_(finalCapitalExchange) {

        if (paymentConvention) // NOLINT(readability-implicit-bool-conversion)
            paymentConvention_ = *paymentConvention;
        else
            paymentConvention_ = floatingSchedule_.businessDayConvention();
        init();
    }

    void NonstandardSwap::init() {

        QL_REQUIRE(fixedNominal_.size() == fixedRate_.size(),
                   "Fixed nominal size ("
                       << fixedNominal_.size()
                       << ") does not match fixed rate size ("
                       << fixedRate_.size() << ")");

        QL_REQUIRE(fixedNominal_.size() == fixedSchedule_.size() - 1,
                   "Fixed nominal size (" << fixedNominal_.size()
                                          << ") does not match schedule size ("
                                          << fixedSchedule_.size() << ") - 1");

        QL_REQUIRE(floatingNominal_.size() == floatingSchedule_.size() - 1,
                   "Floating nominal size ("
                       << floatingNominal_.size()
                       << ") does not match schedule size ("
                       << floatingSchedule_.size() << ") - 1");

        QL_REQUIRE(floatingNominal_.size() == spread_.size(),
                   "Floating nominal size (" << floatingNominal_.size()
                                             << ") does not match spread size ("
                                             << spread_.size() << ")");

        QL_REQUIRE(floatingNominal_.size() == gearing_.size(),
                   "Floating nominal size ("
                       << floatingNominal_.size()
                       << ") does not match gearing size (" << gearing_.size()
                       << ")");

        // if the gearing is zero then the ibor leg will be set up with fixed
        // coupons which makes trouble here in this context. We therefore use
        // a dirty trick and enforce the gearing to be non zero.
        for (Real& i : gearing_) {
            if (close(i, 0.0))
                i = QL_EPSILON;
        }

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
                Real cap = fixedNominal_[i] - fixedNominal_[i + 1];
                if (!close(cap, 0.0)) {
                    auto it1 = legs_[0].begin();
                    std::advance(it1, i + 1);
                    legs_[0].insert(
                        it1, ext::shared_ptr<CashFlow>(
                                 new Redemption(cap, legs_[0][i]->date())));
                    auto it2 = fixedNominal_.begin();
                    std::advance(it2, i + 1);
                    fixedNominal_.insert(it2, fixedNominal_[i]);
                    auto it3 = fixedRate_.begin();
                    std::advance(it3, i + 1);
                    fixedRate_.insert(it3, 0.0);
                    i++;
                }
            }
            for (Size i = 0; i < legs_[1].size() - 1; i++) {
                Real cap = floatingNominal_[i] - floatingNominal_[i + 1];
                if (!close(cap, 0.0)) {
                    auto it1 = legs_[1].begin();
                    std::advance(it1, i + 1);
                    legs_[1].insert(
                        it1, ext::shared_ptr<CashFlow>(
                                 new Redemption(cap, legs_[1][i]->date())));
                    auto it2 = floatingNominal_.begin();
                    std::advance(it2, i + 1);
                    floatingNominal_.insert(it2, floatingNominal_[i]);
                    i++;
                }
            }
        }

        if (finalCapitalExchange_) {
            legs_[0].push_back(ext::shared_ptr<CashFlow>(
                new Redemption(fixedNominal_.back(), legs_[0].back()->date())));
            fixedNominal_.push_back(fixedNominal_.back());
            fixedRate_.push_back(0.0);
            legs_[1].push_back(ext::shared_ptr<CashFlow>(new Redemption(
                floatingNominal_.back(), legs_[1].back()->date())));
            floatingNominal_.push_back(floatingNominal_.back());
        }

        for (auto i = legs_[1].begin(); i < legs_[1].end(); ++i)
            registerWith(*i);

        switch (type_) {
        case Swap::Payer:
            payer_[0] = -1.0;
            payer_[1] = +1.0;
            break;
        case Swap::Receiver:
            payer_[0] = +1.0;
            payer_[1] = -1.0;
            break;
        default:
            QL_FAIL("Unknown nonstandard-swap type");
        }
    }

    void NonstandardSwap::setupArguments(PricingEngine::arguments *args) const {

        Swap::setupArguments(args);

        auto* arguments = dynamic_cast<NonstandardSwap::arguments*>(args);

        if (arguments == nullptr)
            return; // swap engine ...

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
            ext::shared_ptr<FixedRateCoupon> coupon =
                ext::dynamic_pointer_cast<FixedRateCoupon>(fixedCoupons[i]);
            if (coupon != nullptr) {
                arguments->fixedPayDates[i] = coupon->date();
                arguments->fixedResetDates[i] = coupon->accrualStartDate();
                arguments->fixedCoupons[i] = coupon->amount();
            } else {
                ext::shared_ptr<CashFlow> cashflow =
                    ext::dynamic_pointer_cast<CashFlow>(fixedCoupons[i]);
                auto j =
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
            ext::shared_ptr<IborCoupon> coupon =
                ext::dynamic_pointer_cast<IborCoupon>(floatingCoupons[i]);
            if (coupon != nullptr) {
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
                ext::shared_ptr<CashFlow> cashflow =
                    ext::dynamic_pointer_cast<CashFlow>(floatingCoupons[i]);
                auto j = std::find(
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
