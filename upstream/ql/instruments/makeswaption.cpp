/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2008, 2014 Ferdinando Ametrano
 Copyright (C) 2007 Giorgio Facchinetti

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

#include <ql/cashflows/cashflows.hpp>
#include <ql/exercise.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/instruments/makeois.hpp>
#include <ql/instruments/makeswaption.hpp>
#include <ql/instruments/makevanillaswap.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/optional.hpp>
#include <ql/settings.hpp>
#include <utility>

namespace QuantLib {

    MakeSwaption::MakeSwaption(ext::shared_ptr<SwapIndex> swapIndex,
                               const Period& optionTenor,
                               Rate strike)
    : swapIndex_(std::move(swapIndex)), delivery_(Settlement::Physical),
      settlementMethod_(Settlement::PhysicalOTC), optionTenor_(optionTenor),
      optionConvention_(ModifiedFollowing), strike_(strike),
      underlyingType_(Swap::Payer), nominal_(1.0) {}

    MakeSwaption::MakeSwaption(ext::shared_ptr<SwapIndex> swapIndex,
                               const Date& fixingDate,
                               Rate strike)
    : swapIndex_(std::move(swapIndex)), delivery_(Settlement::Physical),
      settlementMethod_(Settlement::PhysicalOTC), optionConvention_(ModifiedFollowing),
      fixingDate_(fixingDate), strike_(strike), underlyingType_(Swap::Payer) {}

    MakeSwaption::operator Swaption() const {
        ext::shared_ptr<Swaption> swaption = *this;
        return *swaption;
    }

    MakeSwaption::operator ext::shared_ptr<Swaption>() const {

        const Calendar& fixingCalendar = swapIndex_->fixingCalendar();
        Date refDate = Settings::instance().evaluationDate();
        // if the evaluation date is not a business day
        // then move to the next business day
        refDate = fixingCalendar.adjust(refDate);
        if (fixingDate_ == Date())
            fixingDate_ = fixingCalendar.advance(refDate, optionTenor_,
                                                 optionConvention_);
        if (exerciseDate_ == Date()) {
            exercise_ = ext::shared_ptr<Exercise>(new
                EuropeanExercise(fixingDate_));
        } else {
            QL_REQUIRE(exerciseDate_ <= fixingDate_,
                       "exercise date (" << exerciseDate_ << ") must be less "
                       "than or equal to fixing date (" << fixingDate_ << ")");
            exercise_ = ext::shared_ptr<Exercise>(new
                EuropeanExercise(exerciseDate_));
        }

        Rate usedStrike;
        ext::shared_ptr<OvernightIndexedSwapIndex> OIswap_index = ext::dynamic_pointer_cast<OvernightIndexedSwapIndex>(swapIndex_);
        if (strike_ == Null<Rate>()) {
            // ATM on curve(s) attached to index
            QL_REQUIRE(!swapIndex_->forwardingTermStructure().empty(),
                       "null term structure set to this instance of " <<
                       swapIndex_->name());
            if (OIswap_index) {
                auto temp = OIswap_index->underlyingSwap(fixingDate_);
                temp->setPricingEngine(
                    ext::make_shared<DiscountingSwapEngine>(
                        swapIndex_->exogenousDiscount()
                        ? swapIndex_->discountingTermStructure()
                        : swapIndex_->forwardingTermStructure(),
                        false
                    )
                );
                usedStrike = temp->fairRate();
            } else {
                auto temp = swapIndex_->underlyingSwap(fixingDate_);
                temp->setPricingEngine(
                    ext::make_shared<DiscountingSwapEngine>(
                        swapIndex_->exogenousDiscount()
                        ? swapIndex_->discountingTermStructure()
                        : swapIndex_->forwardingTermStructure(),
                        false
                    )
                );
                usedStrike = temp->fairRate();
            }
        } else {
            usedStrike = strike_;
        }

        BusinessDayConvention bdc = swapIndex_->fixedLegConvention();
        if (OIswap_index) {
            underlyingSwap_ =
                (ext::shared_ptr<OvernightIndexedSwap>)(
                    MakeOIS(swapIndex_->tenor(),
                            OIswap_index->overnightIndex(), usedStrike)
                    .withEffectiveDate(swapIndex_->valueDate(fixingDate_))
                    .withPaymentCalendar(swapIndex_->fixingCalendar())
                    .withFixedLegDayCount(swapIndex_->dayCounter())
                    .withPaymentAdjustment(bdc)
                    .withFixedLegConvention(bdc)
                    .withFixedLegTerminationDateConvention(bdc)
                    .withType(underlyingType_)
                    .withNominal(nominal_)
                    );
        } else {
            underlyingSwap_ =
                (ext::shared_ptr<VanillaSwap>)(
                    MakeVanillaSwap(swapIndex_->tenor(),
                                    swapIndex_->iborIndex(), usedStrike)
                    .withEffectiveDate(swapIndex_->valueDate(fixingDate_))
                    .withFixedLegCalendar(swapIndex_->fixingCalendar())
                    .withFixedLegDayCount(swapIndex_->dayCounter())
                    .withFixedLegTenor(swapIndex_->fixedLegTenor())
                    .withFixedLegConvention(bdc)
                    .withFixedLegTerminationDateConvention(bdc)
                    .withType(underlyingType_)
                    .withNominal(nominal_)
                    .withIndexedCoupons(useIndexedCoupons_)
                    );
        }
        ext::shared_ptr<Swaption> swaption = ext::make_shared<Swaption>(
            underlyingSwap_, exercise_, delivery_, settlementMethod_);
        swaption->setPricingEngine(engine_);
        return swaption;
    }

    MakeSwaption& MakeSwaption::withSettlementType(Settlement::Type delivery) {
        delivery_ = delivery;
        return *this;
    }

    MakeSwaption& MakeSwaption::withSettlementMethod(
        Settlement::Method settlementMethod) {
        settlementMethod_ = settlementMethod;
        return *this;
    }

    MakeSwaption&
    MakeSwaption::withOptionConvention(BusinessDayConvention bdc) {
        optionConvention_ = bdc;
        return *this;
    }

    MakeSwaption& MakeSwaption::withExerciseDate(const Date& date) {
        exerciseDate_ = date;
        return *this;
    }

    MakeSwaption& MakeSwaption::withUnderlyingType(const Swap::Type type) {
        underlyingType_ = type;
        return *this;
    }

    MakeSwaption& MakeSwaption::withPricingEngine(
                             const ext::shared_ptr<PricingEngine>& engine) {
        engine_ = engine;
        return *this;
    }

    MakeSwaption& MakeSwaption::withNominal(Real n) {
        nominal_ = n;
        return *this;
    }

    MakeSwaption& MakeSwaption::withIndexedCoupons(const ext::optional<bool>& b) {
        useIndexedCoupons_ = b;
        return *this;
    }

    MakeSwaption& MakeSwaption::withAtParCoupons(bool b) {
        useIndexedCoupons_ = !b;
        return *this;
    }

}
