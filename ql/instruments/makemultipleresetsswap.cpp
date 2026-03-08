/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Zain Mughal

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

#include <ql/indexes/iborindex.hpp>
#include <ql/instruments/makemultipleresetsswap.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/settings.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    MakeMultipleResetsSwap::MakeMultipleResetsSwap(
        const Period& tenor,
        const ext::shared_ptr<IborIndex>& iborIndex,
        Size resetsPerCoupon,
        Rate fixedRate,
        const Period& fwdStart)
    : tenor_(tenor), iborIndex_(iborIndex), resetsPerCoupon_(resetsPerCoupon),
      fixedRate_(fixedRate), forwardStart_(fwdStart),
      fixedDayCount_(iborIndex->dayCounter()) {}

    MakeMultipleResetsSwap::operator MultipleResetsSwap() const {
        ext::shared_ptr<MultipleResetsSwap> swap = *this;
        return *swap;
    }

    MakeMultipleResetsSwap::operator ext::shared_ptr<MultipleResetsSwap>() const {
        Calendar cal = iborIndex_->fixingCalendar();
        BusinessDayConvention bdc = iborIndex_->businessDayConvention();

        Date startDate;
        if (effectiveDate_ != Date()) {
            startDate = effectiveDate_;
        } else {
            Natural settlDays = settlementDays_ != Null<Natural>() ?
                                settlementDays_ : iborIndex_->fixingDays();
            Date refDate = Settings::instance().evaluationDate();
            startDate = cal.advance(cal.adjust(refDate), settlDays * Days);
            startDate = cal.advance(startDate, forwardStart_,
                                   forwardStart_.length() < 0 ? Preceding : Following);
        }

        Date endDate = terminationDate_ != Date() ?
                       terminationDate_ :
                       cal.advance(startDate, tenor_, bdc);

        Period resetTenor = iborIndex_->tenor();
        // Fixed coupon period: resetsPerCoupon consecutive reset periods.
        // If not overridden, derive the fixed frequency from the coupon period.
        Frequency fixedFreq = fixedFrequency_;
        if (fixedFreq == NoFrequency) {
            Period couponTenor(resetsPerCoupon_ * resetTenor.length(), resetTenor.units());
            fixedFreq = couponTenor.frequency();
        }

        Schedule fixedSchedule(startDate, endDate, Period(fixedFreq),
                               cal, fixedConvention_, fixedConvention_,
                               DateGeneration::Backward, false);

        Schedule fullResetSchedule(startDate, endDate, resetTenor,
                                   cal, bdc, bdc,
                                   DateGeneration::Backward, false);

        Rate usedFixedRate = fixedRate_;
        if (fixedRate_ == Null<Rate>()) {
            MultipleResetsSwap temp(type_, nominal_,
                                    fixedSchedule, 0.0, fixedDayCount_,
                                    fullResetSchedule, iborIndex_, resetsPerCoupon_,
                                    spread_, averagingMethod_);
            if (engine_ == nullptr) {
                Handle<YieldTermStructure> disc =
                    iborIndex_->forwardingTermStructure();
                QL_REQUIRE(!disc.empty(),
                           "null term structure set to this instance of "
                           << iborIndex_->name());
                temp.setPricingEngine(ext::make_shared<DiscountingSwapEngine>(disc, false));
            } else {
                temp.setPricingEngine(engine_);
            }
            usedFixedRate = temp.fairRate();
        }

        auto swap = ext::make_shared<MultipleResetsSwap>(
            type_, nominal_,
            fixedSchedule, usedFixedRate, fixedDayCount_,
            fullResetSchedule, iborIndex_, resetsPerCoupon_,
            spread_, averagingMethod_);

        if (engine_ == nullptr) {
            Handle<YieldTermStructure> disc = iborIndex_->forwardingTermStructure();
            if (!disc.empty())
                swap->setPricingEngine(ext::make_shared<DiscountingSwapEngine>(disc, false));
        } else {
            swap->setPricingEngine(engine_);
        }

        return swap;
    }

    MakeMultipleResetsSwap& MakeMultipleResetsSwap::receiveFixed(bool flag) {
        type_ = flag ? Swap::Receiver : Swap::Payer;
        return *this;
    }

    MakeMultipleResetsSwap& MakeMultipleResetsSwap::withType(Swap::Type type) {
        type_ = type;
        return *this;
    }

    MakeMultipleResetsSwap& MakeMultipleResetsSwap::withNominal(Real n) {
        nominal_ = n;
        return *this;
    }

    MakeMultipleResetsSwap& MakeMultipleResetsSwap::withSettlementDays(Natural settlementDays) {
        settlementDays_ = settlementDays;
        effectiveDate_ = Date();
        return *this;
    }

    MakeMultipleResetsSwap& MakeMultipleResetsSwap::withEffectiveDate(const Date& d) {
        effectiveDate_ = d;
        return *this;
    }

    MakeMultipleResetsSwap& MakeMultipleResetsSwap::withTerminationDate(const Date& d) {
        terminationDate_ = d;
        if (d != Date())
            tenor_ = Period();
        return *this;
    }

    MakeMultipleResetsSwap& MakeMultipleResetsSwap::withFixedLegFrequency(Frequency f) {
        fixedFrequency_ = f;
        return *this;
    }

    MakeMultipleResetsSwap& MakeMultipleResetsSwap::withFixedLegDayCount(const DayCounter& dc) {
        fixedDayCount_ = dc;
        return *this;
    }

    MakeMultipleResetsSwap& MakeMultipleResetsSwap::withFixedLegConvention(BusinessDayConvention bdc) {
        fixedConvention_ = bdc;
        return *this;
    }

    MakeMultipleResetsSwap& MakeMultipleResetsSwap::withFloatingLegSpread(Spread sp) {
        spread_ = sp;
        return *this;
    }

    MakeMultipleResetsSwap& MakeMultipleResetsSwap::withAveragingMethod(RateAveraging::Type m) {
        averagingMethod_ = m;
        return *this;
    }

    MakeMultipleResetsSwap& MakeMultipleResetsSwap::withDiscountingTermStructure(
                                const Handle<YieldTermStructure>& d) {
        engine_ = ext::make_shared<DiscountingSwapEngine>(d, false);
        return *this;
    }

    MakeMultipleResetsSwap& MakeMultipleResetsSwap::withPricingEngine(
                                const ext::shared_ptr<PricingEngine>& engine) {
        engine_ = engine;
        return *this;
    }

}
