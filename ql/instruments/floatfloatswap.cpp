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

#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/cmscoupon.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/experimental/coupons/cmsspreadcoupon.hpp> // internal
#include <ql/indexes/iborindex.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/instruments/floatfloatswap.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/optional.hpp>
#include <utility>

namespace QuantLib {

    FloatFloatSwap::FloatFloatSwap(const Swap::Type type,
                                   const Real nominal1,
                                   const Real nominal2,
                                   Schedule schedule1,
                                   ext::shared_ptr<InterestRateIndex> index1,
                                   DayCounter dayCount1,
                                   Schedule schedule2,
                                   ext::shared_ptr<InterestRateIndex> index2,
                                   DayCounter dayCount2,
                                   const bool intermediateCapitalExchange,
                                   const bool finalCapitalExchange,
                                   const Real gearing1,
                                   const Real spread1,
                                   const Real cappedRate1,
                                   const Real flooredRate1,
                                   const Real gearing2,
                                   const Real spread2,
                                   const Real cappedRate2,
                                   const Real flooredRate2,
                                   const ext::optional<BusinessDayConvention>& paymentConvention1,
                                   const ext::optional<BusinessDayConvention>& paymentConvention2)
    : Swap(2), type_(type), nominal1_(std::vector<Real>(schedule1.size() - 1, nominal1)),
      nominal2_(std::vector<Real>(schedule2.size() - 1, nominal2)),
      schedule1_(std::move(schedule1)), schedule2_(std::move(schedule2)),
      index1_(std::move(index1)), index2_(std::move(index2)),
      gearing1_(std::vector<Real>(schedule1_.size() - 1, gearing1)),
      gearing2_(std::vector<Real>(schedule2_.size() - 1, gearing2)),
      spread1_(std::vector<Real>(schedule1_.size() - 1, spread1)),
      spread2_(std::vector<Real>(schedule2_.size() - 1, spread2)),
      cappedRate1_(std::vector<Real>(schedule1_.size() - 1, cappedRate1)),
      flooredRate1_(std::vector<Real>(schedule1_.size() - 1, flooredRate1)),
      cappedRate2_(std::vector<Real>(schedule2_.size() - 1, cappedRate2)),
      flooredRate2_(std::vector<Real>(schedule2_.size() - 1, flooredRate2)),
      dayCount1_(std::move(dayCount1)), dayCount2_(std::move(dayCount2)),
      intermediateCapitalExchange_(intermediateCapitalExchange),
      finalCapitalExchange_(finalCapitalExchange) {

        init(paymentConvention1, paymentConvention2);
    }

    FloatFloatSwap::FloatFloatSwap(const Swap::Type type,
                                   std::vector<Real> nominal1,
                                   std::vector<Real> nominal2,
                                   Schedule schedule1,
                                   ext::shared_ptr<InterestRateIndex> index1,
                                   DayCounter dayCount1,
                                   Schedule schedule2,
                                   ext::shared_ptr<InterestRateIndex> index2,
                                   DayCounter dayCount2,
                                   const bool intermediateCapitalExchange,
                                   const bool finalCapitalExchange,
                                   std::vector<Real> gearing1,
                                   std::vector<Real> spread1,
                                   std::vector<Real> cappedRate1,
                                   std::vector<Real> flooredRate1,
                                   std::vector<Real> gearing2,
                                   std::vector<Real> spread2,
                                   std::vector<Real> cappedRate2,
                                   std::vector<Real> flooredRate2,
                                   const ext::optional<BusinessDayConvention>& paymentConvention1,
                                   const ext::optional<BusinessDayConvention>& paymentConvention2)
    : Swap(2), type_(type), nominal1_(std::move(nominal1)), nominal2_(std::move(nominal2)),
      schedule1_(std::move(schedule1)), schedule2_(std::move(schedule2)),
      index1_(std::move(index1)), index2_(std::move(index2)), gearing1_(std::move(gearing1)),
      gearing2_(std::move(gearing2)), spread1_(std::move(spread1)), spread2_(std::move(spread2)),
      cappedRate1_(std::move(cappedRate1)), flooredRate1_(std::move(flooredRate1)),
      cappedRate2_(std::move(cappedRate2)), flooredRate2_(std::move(flooredRate2)),
      dayCount1_(std::move(dayCount1)), dayCount2_(std::move(dayCount2)),
      intermediateCapitalExchange_(intermediateCapitalExchange),
      finalCapitalExchange_(finalCapitalExchange) {

        init(paymentConvention1, paymentConvention2);
    }

    void FloatFloatSwap::init(
        ext::optional<BusinessDayConvention> paymentConvention1,
        ext::optional<BusinessDayConvention> paymentConvention2) {

        QL_REQUIRE(nominal1_.size() == schedule1_.size() - 1,
                   "nominal1 size (" << nominal1_.size()
                                     << ") does not match schedule1 size ("
                                     << schedule1_.size() << ")");
        QL_REQUIRE(nominal2_.size() == schedule2_.size() - 1,
                   "nominal2 size (" << nominal2_.size()
                                     << ") does not match schedule2 size ("
                                     << nominal2_.size() << ")");
        QL_REQUIRE(gearing1_.empty() || gearing1_.size() == nominal1_.size(),
                   "nominal1 size (" << nominal1_.size() << ") does not match gearing1 size ("
                                     << gearing1_.size() << ")");
        QL_REQUIRE(gearing2_.empty() || gearing2_.size() == nominal2_.size(),
                   "nominal2 size (" << nominal2_.size() << ") does not match gearing2 size ("
                                     << gearing2_.size() << ")");
        QL_REQUIRE(cappedRate1_.empty() || cappedRate1_.size() == nominal1_.size(),
                   "nominal1 size (" << nominal1_.size() << ") does not match cappedRate1 size ("
                                     << cappedRate1_.size() << ")");
        QL_REQUIRE(cappedRate2_.empty() || cappedRate2_.size() == nominal2_.size(),
                   "nominal2 size (" << nominal2_.size() << ") does not match cappedRate2 size ("
                                     << cappedRate2_.size() << ")");
        QL_REQUIRE(flooredRate1_.empty() || flooredRate1_.size() == nominal1_.size(),
                   "nominal1 size (" << nominal1_.size() << ") does not match flooredRate1 size ("
                                     << flooredRate1_.size() << ")");
        QL_REQUIRE(flooredRate2_.empty() || flooredRate2_.size() == nominal2_.size(),
                   "nominal2 size (" << nominal2_.size() << ") does not match flooredRate2 size ("
                                     << flooredRate2_.size() << ")");

        if (paymentConvention1) // NOLINT(readability-implicit-bool-conversion)
            paymentConvention1_ = *paymentConvention1;
        else
            paymentConvention1_ = schedule1_.businessDayConvention();

        if (paymentConvention2) // NOLINT(readability-implicit-bool-conversion)
            paymentConvention2_ = *paymentConvention2;
        else
            paymentConvention2_ = schedule2_.businessDayConvention();

        if (gearing1_.empty())
            gearing1_ = std::vector<Real>(nominal1_.size(), 1.0);
        if (gearing2_.empty())
            gearing2_ = std::vector<Real>(nominal2_.size(), 1.0);
        if (spread1_.empty())
            spread1_ = std::vector<Real>(nominal1_.size(), 0.0);
        if (spread2_.empty())
            spread2_ = std::vector<Real>(nominal2_.size(), 0.0);
        if (cappedRate1_.empty())
            cappedRate1_ = std::vector<Real>(nominal1_.size(), Null<Real>());
        if (cappedRate2_.empty())
            cappedRate2_ = std::vector<Real>(nominal2_.size(), Null<Real>());
        if (flooredRate1_.empty())
            flooredRate1_ = std::vector<Real>(nominal1_.size(), Null<Real>());
        if (flooredRate2_.empty())
            flooredRate2_ = std::vector<Real>(nominal2_.size(), Null<Real>());

        bool isNull;
        isNull = cappedRate1_[0] == Null<Real>();
        for (Size i = 0; i < cappedRate1_.size(); i++) {
            if (isNull)
                QL_REQUIRE(cappedRate1_[i] == Null<Real>(),
                           "cappedRate1 must be null for all or none entry ("
                               << (i + 1) << "th is " << cappedRate1_[i]
                               << ")");
            else
                QL_REQUIRE(cappedRate1_[i] != Null<Real>(),
                           "cappedRate 1 must be null for all or none entry ("
                               << "1st is " << cappedRate1_[0] << ")");
        }
        isNull = cappedRate2_[0] == Null<Real>();
        for (Size i = 0; i < cappedRate2_.size(); i++) {
            if (isNull)
                QL_REQUIRE(cappedRate2_[i] == Null<Real>(),
                           "cappedRate2 must be null for all or none entry ("
                               << (i + 1) << "th is " << cappedRate2_[i]
                               << ")");
            else
                QL_REQUIRE(cappedRate2_[i] != Null<Real>(),
                           "cappedRate2 must be null for all or none entry ("
                               << "1st is " << cappedRate2_[0] << ")");
        }
        isNull = flooredRate1_[0] == Null<Real>();
        for (Size i = 0; i < flooredRate1_.size(); i++) {
            if (isNull)
                QL_REQUIRE(flooredRate1_[i] == Null<Real>(),
                           "flooredRate1 must be null for all or none entry ("
                               << (i + 1) << "th is " << flooredRate1_[i]
                               << ")");
            else
                QL_REQUIRE(flooredRate1_[i] != Null<Real>(),
                           "flooredRate 1 must be null for all or none entry ("
                               << "1st is " << flooredRate1_[0] << ")");
        }
        isNull = flooredRate2_[0] == Null<Real>();
        for (Size i = 0; i < flooredRate2_.size(); i++) {
            if (isNull)
                QL_REQUIRE(flooredRate2_[i] == Null<Real>(),
                           "flooredRate2 must be null for all or none entry ("
                               << (i + 1) << "th is " << flooredRate2_[i]
                               << ")");
            else
                QL_REQUIRE(flooredRate2_[i] != Null<Real>(),
                           "flooredRate2 must be null for all or none entry ("
                               << "1st is " << flooredRate2_[0] << ")");
        }

        // if the gearing is zero then the ibor / cms leg will be set up with
        // fixed coupons which makes trouble here in this context. We therefore
        // use a dirty trick and enforce the gearing to be non zero.
        for (Real& i : gearing1_)
            if (close(i, 0.0))
                i = QL_EPSILON;
        for (Real& i : gearing2_)
            if (close(i, 0.0))
                i = QL_EPSILON;

        ext::shared_ptr<IborIndex> ibor1 =
            ext::dynamic_pointer_cast<IborIndex>(index1_);
        ext::shared_ptr<IborIndex> ibor2 =
            ext::dynamic_pointer_cast<IborIndex>(index2_);
        ext::shared_ptr<SwapIndex> cms1 =
            ext::dynamic_pointer_cast<SwapIndex>(index1_);
        ext::shared_ptr<SwapIndex> cms2 =
            ext::dynamic_pointer_cast<SwapIndex>(index2_);
        ext::shared_ptr<SwapSpreadIndex> cmsspread1 =
            ext::dynamic_pointer_cast<SwapSpreadIndex>(index1_);
        ext::shared_ptr<SwapSpreadIndex> cmsspread2 =
            ext::dynamic_pointer_cast<SwapSpreadIndex>(index2_);

        QL_REQUIRE(ibor1 != nullptr || cms1 != nullptr || cmsspread1 != nullptr,
                   "index1 must be ibor or cms or cms spread");
        QL_REQUIRE(ibor2 != nullptr || cms2 != nullptr || cmsspread2 != nullptr,
                   "index2 must be ibor or cms");

        if (ibor1 != nullptr) {
            IborLeg leg(schedule1_, ibor1);
            leg = leg.withNotionals(nominal1_)
                      .withPaymentDayCounter(dayCount1_)
                      .withPaymentAdjustment(paymentConvention1_)
                      .withSpreads(spread1_)
                      .withGearings(gearing1_);
            if (cappedRate1_[0] != Null<Real>())
                leg = leg.withCaps(cappedRate1_);
            if (flooredRate1_[0] != Null<Real>())
                leg = leg.withFloors(flooredRate1_);
            legs_[0] = leg;
        }

        if (ibor2 != nullptr) {
            IborLeg leg(schedule2_, ibor2);
            leg = leg.withNotionals(nominal2_)
                      .withPaymentDayCounter(dayCount2_)
                      .withPaymentAdjustment(paymentConvention2_)
                      .withSpreads(spread2_)
                      .withGearings(gearing2_);
            if (cappedRate2_[0] != Null<Real>())
                leg = leg.withCaps(cappedRate2_);
            if (flooredRate2_[0] != Null<Real>())
                leg = leg.withFloors(flooredRate2_);
            legs_[1] = leg;
        }

        if (cms1 != nullptr) {
            CmsLeg leg(schedule1_, cms1);
            leg = leg.withNotionals(nominal1_)
                      .withPaymentDayCounter(dayCount1_)
                      .withPaymentAdjustment(paymentConvention1_)
                      .withSpreads(spread1_)
                      .withGearings(gearing1_);
            if (cappedRate1_[0] != Null<Real>())
                leg = leg.withCaps(cappedRate1_);
            if (flooredRate1_[0] != Null<Real>())
                leg = leg.withFloors(flooredRate1_);
            legs_[0] = leg;
        }

        if (cms2 != nullptr) {
            CmsLeg leg(schedule2_, cms2);
            leg = leg.withNotionals(nominal2_)
                      .withPaymentDayCounter(dayCount2_)
                      .withPaymentAdjustment(paymentConvention2_)
                      .withSpreads(spread2_)
                      .withGearings(gearing2_);
            if (cappedRate2_[0] != Null<Real>())
                leg = leg.withCaps(cappedRate2_);
            if (flooredRate2_[0] != Null<Real>())
                leg = leg.withFloors(flooredRate2_);
            legs_[1] = leg;
        }

        if (cmsspread1 != nullptr) {
            CmsSpreadLeg leg(schedule1_, cmsspread1);
            leg = leg.withNotionals(nominal1_)
                      .withPaymentDayCounter(dayCount1_)
                      .withPaymentAdjustment(paymentConvention1_)
                      .withSpreads(spread1_)
                      .withGearings(gearing1_);
            if (cappedRate1_[0] != Null<Real>())
                leg = leg.withCaps(cappedRate1_);
            if (flooredRate1_[0] != Null<Real>())
                leg = leg.withFloors(flooredRate1_);
            legs_[0] = leg;
        }

        if (cmsspread2 != nullptr) {
            CmsSpreadLeg leg(schedule2_, cmsspread2);
            leg = leg.withNotionals(nominal2_)
                      .withPaymentDayCounter(dayCount2_)
                      .withPaymentAdjustment(paymentConvention2_)
                      .withSpreads(spread2_)
                      .withGearings(gearing2_);
            if (cappedRate2_[0] != Null<Real>())
                leg = leg.withCaps(cappedRate2_);
            if (flooredRate2_[0] != Null<Real>())
                leg = leg.withFloors(flooredRate2_);
            legs_[1] = leg;
        }

        if (intermediateCapitalExchange_) {
            for (Size i = 0; i < legs_[0].size() - 1; i++) {
                Real cap = nominal1_[i] - nominal1_[i + 1];
                if (!close(cap, 0.0)) {
                    auto it1 = legs_[0].begin();
                    std::advance(it1, i + 1);
                    legs_[0].insert(
                        it1, ext::shared_ptr<CashFlow>(
                                 new Redemption(cap, legs_[0][i]->date())));
                    auto it2 = nominal1_.begin();
                    std::advance(it2, i + 1);
                    nominal1_.insert(it2, nominal1_[i]);
                    i++;
                }
            }
            for (Size i = 0; i < legs_[1].size() - 1; i++) {
                Real cap = nominal2_[i] - nominal2_[i + 1];
                if (!close(cap, 0.0)) {
                    auto it1 = legs_[1].begin();
                    std::advance(it1, i + 1);
                    legs_[1].insert(
                        it1, ext::shared_ptr<CashFlow>(
                                 new Redemption(cap, legs_[1][i]->date())));
                    auto it2 = nominal2_.begin();
                    std::advance(it2, i + 1);
                    nominal2_.insert(it2, nominal2_[i]);
                    i++;
                }
            }
        }

        if (finalCapitalExchange_) {
            legs_[0].push_back(ext::shared_ptr<CashFlow>(
                new Redemption(nominal1_.back(), legs_[0].back()->date())));
            nominal1_.push_back(nominal1_.back());
            legs_[1].push_back(ext::shared_ptr<CashFlow>(
                new Redemption(nominal2_.back(), legs_[1].back()->date())));
            nominal2_.push_back(nominal2_.back());
        }

        for (Leg::const_iterator i = legs_[0].begin(); i < legs_[0].end(); ++i)
            registerWith(*i);

        for (Leg::const_iterator i = legs_[1].begin(); i < legs_[1].end(); ++i)
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
            QL_FAIL("Unknown float float - swap type");
        }
    }

    void FloatFloatSwap::setupArguments(PricingEngine::arguments *args) const {

        Swap::setupArguments(args);

        auto* arguments = dynamic_cast<FloatFloatSwap::arguments*>(args);

        if (arguments == nullptr)
            return; // swap engine ... // QL_REQUIRE(arguments != 0, "argument type does not match");

        arguments->type = type_;
        arguments->nominal1 = nominal1_;
        arguments->nominal2 = nominal2_;
        arguments->index1 = index1_;
        arguments->index2 = index2_;

        const Leg &leg1Coupons = leg1();
        const Leg &leg2Coupons = leg2();

        arguments->leg1ResetDates = arguments->leg1PayDates =
            arguments->leg1FixingDates = std::vector<Date>(leg1Coupons.size());
        arguments->leg2ResetDates = arguments->leg2PayDates =
            arguments->leg2FixingDates = std::vector<Date>(leg2Coupons.size());

        arguments->leg1Spreads = arguments->leg1AccrualTimes =
            arguments->leg1Gearings = std::vector<Real>(leg1Coupons.size());
        arguments->leg2Spreads = arguments->leg2AccrualTimes =
            arguments->leg2Gearings = std::vector<Real>(leg2Coupons.size());

        arguments->leg1Coupons =
            std::vector<Real>(leg1Coupons.size(), Null<Real>());
        arguments->leg2Coupons =
            std::vector<Real>(leg2Coupons.size(), Null<Real>());

        arguments->leg1IsRedemptionFlow =
            std::vector<bool>(leg1Coupons.size(), false);
        arguments->leg2IsRedemptionFlow =
            std::vector<bool>(leg2Coupons.size(), false);

        arguments->leg1CappedRates = arguments->leg1FlooredRates =
            std::vector<Real>(leg1Coupons.size(), Null<Real>());
        arguments->leg2CappedRates = arguments->leg2FlooredRates =
            std::vector<Real>(leg2Coupons.size(), Null<Real>());

        for (Size i = 0; i < leg1Coupons.size(); ++i) {
            ext::shared_ptr<FloatingRateCoupon> coupon =
                ext::dynamic_pointer_cast<FloatingRateCoupon>(leg1Coupons[i]);
            if (coupon != nullptr) {
                arguments->leg1AccrualTimes[i] = coupon->accrualPeriod();
                arguments->leg1PayDates[i] = coupon->date();
                arguments->leg1ResetDates[i] = coupon->accrualStartDate();
                arguments->leg1FixingDates[i] = coupon->fixingDate();
                arguments->leg1Spreads[i] = coupon->spread();
                arguments->leg1Gearings[i] = coupon->gearing();
                try {
                    arguments->leg1Coupons[i] = coupon->amount();
                }
                catch (Error &) {
                    arguments->leg1Coupons[i] = Null<Real>();
                }
                ext::shared_ptr<CappedFlooredCoupon> cfcoupon =
                    ext::dynamic_pointer_cast<CappedFlooredCoupon>(
                        leg1Coupons[i]);
                if (cfcoupon != nullptr) {
                    arguments->leg1CappedRates[i] = cfcoupon->cap();
                    arguments->leg1FlooredRates[i] = cfcoupon->floor();
                }
            } else {
                ext::shared_ptr<CashFlow> cashflow =
                    ext::dynamic_pointer_cast<CashFlow>(leg1Coupons[i]);
                std::vector<Date>::const_iterator j =
                    std::find(arguments->leg1PayDates.begin(),
                              arguments->leg1PayDates.end(), cashflow->date());
                QL_REQUIRE(j != arguments->leg1PayDates.end(),
                           "nominal redemption on "
                               << cashflow->date()
                               << "has no corresponding coupon");
                Size jIdx = j - arguments->leg1PayDates.begin();
                arguments->leg1IsRedemptionFlow[i] = true;
                arguments->leg1Coupons[i] = cashflow->amount();
                arguments->leg1ResetDates[i] = arguments->leg1ResetDates[jIdx];
                arguments->leg1FixingDates[i] =
                    arguments->leg1FixingDates[jIdx];
                arguments->leg1AccrualTimes[i] = 0.0;
                arguments->leg1Spreads[i] = 0.0;
                arguments->leg1Gearings[i] = 1.0;
                arguments->leg1PayDates[i] = cashflow->date();
            }
        }

        for (Size i = 0; i < leg2Coupons.size(); ++i) {
            ext::shared_ptr<FloatingRateCoupon> coupon =
                ext::dynamic_pointer_cast<FloatingRateCoupon>(leg2Coupons[i]);
            if (coupon != nullptr) {
                arguments->leg2AccrualTimes[i] = coupon->accrualPeriod();
                arguments->leg2PayDates[i] = coupon->date();
                arguments->leg2ResetDates[i] = coupon->accrualStartDate();
                arguments->leg2FixingDates[i] = coupon->fixingDate();
                arguments->leg2Spreads[i] = coupon->spread();
                arguments->leg2Gearings[i] = coupon->gearing();
                try {
                    arguments->leg2Coupons[i] = coupon->amount();
                }
                catch (Error &) {
                    arguments->leg2Coupons[i] = Null<Real>();
                }
                ext::shared_ptr<CappedFlooredCoupon> cfcoupon =
                    ext::dynamic_pointer_cast<CappedFlooredCoupon>(
                        leg2Coupons[i]);
                if (cfcoupon != nullptr) {
                    arguments->leg2CappedRates[i] = cfcoupon->cap();
                    arguments->leg2FlooredRates[i] = cfcoupon->floor();
                }
            } else {
                ext::shared_ptr<CashFlow> cashflow =
                    ext::dynamic_pointer_cast<CashFlow>(leg2Coupons[i]);
                std::vector<Date>::const_iterator j =
                    std::find(arguments->leg2PayDates.begin(),
                              arguments->leg2PayDates.end(), cashflow->date());
                QL_REQUIRE(j != arguments->leg2PayDates.end(),
                           "nominal redemption on "
                               << cashflow->date()
                               << "has no corresponding coupon");
                Size jIdx = j - arguments->leg2PayDates.begin();
                arguments->leg2IsRedemptionFlow[i] = true;
                arguments->leg2Coupons[i] = cashflow->amount();
                arguments->leg2ResetDates[i] = arguments->leg2ResetDates[jIdx];
                arguments->leg2FixingDates[i] =
                    arguments->leg2FixingDates[jIdx];
                arguments->leg2AccrualTimes[i] = 0.0;
                arguments->leg2Spreads[i] = 0.0;
                arguments->leg2Gearings[i] = 1.0;
                arguments->leg2PayDates[i] = cashflow->date();
            }
        }
    }

    void FloatFloatSwap::setupExpired() const { Swap::setupExpired(); }

    void FloatFloatSwap::fetchResults(const PricingEngine::results *r) const {
        Swap::fetchResults(r);
    }

    void FloatFloatSwap::arguments::validate() const {

        Swap::arguments::validate();

        QL_REQUIRE(nominal1.size() == leg1ResetDates.size(),
                   "nominal1 size is different from resetDates1 size");
        QL_REQUIRE(nominal1.size() == leg1FixingDates.size(),
                   "nominal1 size is different from fixingDates1 size");
        QL_REQUIRE(nominal1.size() == leg1PayDates.size(),
                   "nominal1 size is different from payDates1 size");
        QL_REQUIRE(nominal1.size() == leg1Spreads.size(),
                   "nominal1 size is different from spreads1 size");
        QL_REQUIRE(nominal1.size() == leg1Gearings.size(),
                   "nominal1 size is different from gearings1 size");
        QL_REQUIRE(nominal1.size() == leg1CappedRates.size(),
                   "nominal1 size is different from cappedRates1 size");
        QL_REQUIRE(nominal1.size() == leg1FlooredRates.size(),
                   "nominal1 size is different from flooredRates1 size");
        QL_REQUIRE(nominal1.size() == leg1Coupons.size(),
                   "nominal1 size is different from coupons1 size");
        QL_REQUIRE(nominal1.size() == leg1AccrualTimes.size(),
                   "nominal1 size is different from accrualTimes1 size");
        QL_REQUIRE(nominal1.size() == leg1IsRedemptionFlow.size(),
                   "nominal1 size is different from redemption1 size");

        QL_REQUIRE(nominal2.size() == leg2ResetDates.size(),
                   "nominal2 size is different from resetDates2 size");
        QL_REQUIRE(nominal2.size() == leg2FixingDates.size(),
                   "nominal2 size is different from fixingDates2 size");
        QL_REQUIRE(nominal2.size() == leg2PayDates.size(),
                   "nominal2 size is different from payDates2 size");
        QL_REQUIRE(nominal2.size() == leg2Spreads.size(),
                   "nominal2 size is different from spreads2 size");
        QL_REQUIRE(nominal2.size() == leg2Gearings.size(),
                   "nominal2 size is different from gearings2 size");
        QL_REQUIRE(nominal2.size() == leg2CappedRates.size(),
                   "nominal2 size is different from cappedRates2 size");
        QL_REQUIRE(nominal2.size() == leg2FlooredRates.size(),
                   "nominal2 size is different from flooredRates2 size");
        QL_REQUIRE(nominal2.size() == leg2Coupons.size(),
                   "nominal2 size is different from coupons2 size");
        QL_REQUIRE(nominal2.size() == leg2AccrualTimes.size(),
                   "nominal2 size is different from accrualTimes2 size");
        QL_REQUIRE(nominal2.size() == leg2IsRedemptionFlow.size(),
                   "nominal2 size is different from redemption2 size");

        QL_REQUIRE(index1 != nullptr, "index1 is null");
        QL_REQUIRE(index2 != nullptr, "index2 is null");
    }

    void FloatFloatSwap::results::reset() { Swap::results::reset(); }
}
