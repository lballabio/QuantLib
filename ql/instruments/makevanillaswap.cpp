/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007, 2010, 2014, 2015 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni
 Copyright (C) 2006 StatPro Italia srl
 Copyright (C) 2015 Paolo Mazzocchi
 Copyright (C) 2018 Matthias Groncki

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

#include <ql/instruments/makevanillaswap.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/time/schedule.hpp>
#include <ql/currencies/america.hpp>
#include <ql/currencies/asia.hpp>
#include <ql/currencies/europe.hpp>
#include <ql/currencies/oceania.hpp>
#include <ql/utilities/null.hpp>
#include <ql/optional.hpp>

namespace QuantLib {

    MakeVanillaSwap::MakeVanillaSwap(const Period& swapTenor,
                                     const ext::shared_ptr<IborIndex>& index,
                                     Rate fixedRate,
                                     const Period& forwardStart)
    : swapTenor_(swapTenor), iborIndex_(index), fixedRate_(fixedRate), forwardStart_(forwardStart),
      fixedCalendar_(index->fixingCalendar()), floatCalendar_(index->fixingCalendar()),
      floatTenor_(index->tenor()),
      floatConvention_(index->businessDayConvention()),
      floatTerminationDateConvention_(index->businessDayConvention()),

      floatDayCount_(index->dayCounter()) {}

    MakeVanillaSwap::operator VanillaSwap() const {
        ext::shared_ptr<VanillaSwap> swap = *this;
        return *swap;
    }

    MakeVanillaSwap::operator ext::shared_ptr<VanillaSwap>() const {

        Date startDate;
        if (effectiveDate_ != Date())
            startDate = effectiveDate_;
        else {
            Date refDate = Settings::instance().evaluationDate();
            // if the evaluation date is not a business day
            // then move to the next business day
            refDate = floatCalendar_.adjust(refDate);
            // use index valueDate interface wherever possible to estimate spot date.
            // Unless we pass an explicit settlementDays_ which does not match the index-defined number of fixing days.
            Date spotDate;
            if (settlementDays_ == Null<Natural>())
                spotDate = iborIndex_->valueDate(refDate);
            else
                spotDate = floatCalendar_.advance(refDate, settlementDays_ * Days);
            startDate = spotDate+forwardStart_;
            if (forwardStart_.length()<0)
                startDate = floatCalendar_.adjust(startDate,
                                                  Preceding);
            else if (forwardStart_.length()>0)
                startDate = floatCalendar_.adjust(startDate,
                                                  Following);
            // no explicit date adjustment needed for forwardStart_.length()==0 (already handled by spotDate arithmetic above)
        }

        Date endDate = terminationDate_;
        if (endDate == Date()) {
            endDate = startDate + swapTenor_;
            bool maturityEndOfMonth =
                maturityEndOfMonth_ ? *maturityEndOfMonth_ : floatEndOfMonth_;
            if (maturityEndOfMonth && allowsEndOfMonth(swapTenor_) &&
                floatCalendar_.isEndOfMonth(startDate))
                endDate = floatCalendar_.endOfMonth(endDate);
        }

        const Currency& curr = iborIndex_->currency();
        Period fixedTenor;
        if (fixedTenor_ != Period())
            fixedTenor = fixedTenor_;
        else {
            if ((curr == EURCurrency()) ||
                (curr == USDCurrency()) ||
                (curr == CHFCurrency()) ||
                (curr == SEKCurrency()) ||
                (curr == GBPCurrency() && swapTenor_ <= 1 * Years))
                fixedTenor = Period(1, Years);
            else if ((curr == GBPCurrency() && swapTenor_ > 1 * Years) ||
                (curr == JPYCurrency()) ||
                (curr == AUDCurrency() && swapTenor_ >= 4 * Years))
                fixedTenor = Period(6, Months);
            else if ((curr == HKDCurrency() ||
                     (curr == AUDCurrency() && swapTenor_ < 4 * Years)))
                fixedTenor = Period(3, Months);
            else
                QL_FAIL("unknown fixed leg default tenor for " << curr);
        }

        Schedule fixedSchedule(startDate, endDate,
                               fixedTenor, fixedCalendar_,
                               fixedConvention_,
                               fixedTerminationDateConvention_,
                               fixedRule_, fixedEndOfMonth_,
                               fixedFirstDate_, fixedNextToLastDate_);

        Schedule floatSchedule(startDate, endDate,
                               floatTenor_, floatCalendar_,
                               floatConvention_,
                               floatTerminationDateConvention_,
                               floatRule_, floatEndOfMonth_,
                               floatFirstDate_, floatNextToLastDate_);

        DayCounter fixedDayCount;
        if (fixedDayCount_ != DayCounter())
            fixedDayCount = fixedDayCount_;
        else {
            if (curr == USDCurrency())
                fixedDayCount = Actual360();
            else if (curr == EURCurrency() || curr == CHFCurrency() ||
                     curr == SEKCurrency())
                fixedDayCount = Thirty360(Thirty360::BondBasis);
            else if (curr == GBPCurrency() || curr == JPYCurrency() ||
                     curr == AUDCurrency() || curr == HKDCurrency() ||
                     curr == THBCurrency())
                fixedDayCount = Actual365Fixed();
            else
                QL_FAIL("unknown fixed leg day counter for " << curr);
        }

        Rate usedFixedRate = fixedRate_;
        if (fixedRate_ == Null<Rate>()) {
            VanillaSwap temp(type_, 100.00, fixedSchedule,
                             0.0, // fixed rate
                             fixedDayCount, floatSchedule, iborIndex_, floatSpread_, floatDayCount_,
                             paymentConvention_, useIndexedCoupons_);
            if (engine_ == nullptr) {
                Handle<YieldTermStructure> disc =
                                        iborIndex_->forwardingTermStructure();
                QL_REQUIRE(!disc.empty(),
                           "null term structure set to this instance of " <<
                           iborIndex_->name());
                bool includeSettlementDateFlows = false;
                ext::shared_ptr<PricingEngine> engine(new
                    DiscountingSwapEngine(disc, includeSettlementDateFlows));
                temp.setPricingEngine(engine);
            } else
                temp.setPricingEngine(engine_);

            usedFixedRate = temp.fairRate();
        }

        ext::shared_ptr<VanillaSwap> swap(new VanillaSwap(
            type_, nominal_, fixedSchedule, usedFixedRate, fixedDayCount, floatSchedule, iborIndex_,
            floatSpread_, floatDayCount_, paymentConvention_, useIndexedCoupons_));

        if (engine_ == nullptr) {
            Handle<YieldTermStructure> disc =
                                    iborIndex_->forwardingTermStructure();
            bool includeSettlementDateFlows = false;
            ext::shared_ptr<PricingEngine> engine(new
                DiscountingSwapEngine(disc, includeSettlementDateFlows));
            swap->setPricingEngine(engine);
        } else
            swap->setPricingEngine(engine_);

        return swap;
    }

    MakeVanillaSwap& MakeVanillaSwap::receiveFixed(bool flag) {
        type_ = flag ? Swap::Receiver : Swap::Payer ;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withType(Swap::Type type) {
        type_ = type;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withNominal(Real n) {
        nominal_ = n;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withSettlementDays(Natural settlementDays) {
        settlementDays_ = settlementDays;
        effectiveDate_ = Date();
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withEffectiveDate(const Date& effectiveDate) {
        effectiveDate_ = effectiveDate;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withTerminationDate(const Date& terminationDate) {
        terminationDate_ = terminationDate;
        if (terminationDate != Date())
            swapTenor_ = Period();
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withRule(DateGeneration::Rule r) {
        fixedRule_ = r;
        floatRule_ = r;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withPaymentConvention(BusinessDayConvention bdc) {
        paymentConvention_ = bdc;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withDiscountingTermStructure(
                                        const Handle<YieldTermStructure>& d) {
        bool includeSettlementDateFlows = false;
        engine_ = ext::shared_ptr<PricingEngine>(new
            DiscountingSwapEngine(d, includeSettlementDateFlows));
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withPricingEngine(
                             const ext::shared_ptr<PricingEngine>& engine) {
        engine_ = engine;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withFixedLegTenor(const Period& t) {
        fixedTenor_ = t;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFixedLegCalendar(const Calendar& cal) {
        fixedCalendar_ = cal;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFixedLegConvention(BusinessDayConvention bdc) {
        fixedConvention_ = bdc;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFixedLegTerminationDateConvention(BusinessDayConvention bdc) {
        fixedTerminationDateConvention_ = bdc;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withFixedLegRule(DateGeneration::Rule r) {
        fixedRule_ = r;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withFixedLegEndOfMonth(bool flag) {
        fixedEndOfMonth_ = flag;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withFixedLegFirstDate(const Date& d) {
        fixedFirstDate_ = d;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFixedLegNextToLastDate(const Date& d) {
        fixedNextToLastDate_ = d;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFixedLegDayCount(const DayCounter& dc) {
        fixedDayCount_ = dc;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withFloatingLegTenor(const Period& t) {
        floatTenor_ = t;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegCalendar(const Calendar& cal) {
        floatCalendar_ = cal;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegConvention(BusinessDayConvention bdc) {
        floatConvention_ = bdc;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegTerminationDateConvention(BusinessDayConvention bdc) {
        floatTerminationDateConvention_ = bdc;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withFloatingLegRule(DateGeneration::Rule r) {
        floatRule_ = r;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withFloatingLegEndOfMonth(bool flag) {
        floatEndOfMonth_ = flag;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withMaturityEndOfMonth(bool flag) {
        maturityEndOfMonth_ = flag;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegFirstDate(const Date& d) {
        floatFirstDate_ = d;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegNextToLastDate(const Date& d) {
        floatNextToLastDate_ = d;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegDayCount(const DayCounter& dc) {
        floatDayCount_ = dc;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withFloatingLegSpread(Spread sp) {
        floatSpread_ = sp;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withIndexedCoupons(const ext::optional<bool>& b) {
        useIndexedCoupons_ = b;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withAtParCoupons(bool b) {
        useIndexedCoupons_ = !b;
        return *this;
    }

}
