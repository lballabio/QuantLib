/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include <ql/instruments/makeconstnotionalcrosscurrencyfixedvsfloatingswap.hpp>
#include <ql/pricingengines/swap/discountingconstnotionalcrosscurrencyswapengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/settings.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/schedule.hpp>
#include <utility>

namespace QuantLib {

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap(const Period& swapTenor,
                                                       Currency fixedCurrency,
                                                       ext::shared_ptr<IborIndex> floatIndex,
                                                       Rate fixedRate,
                                                       Real floatNominal,
                                                       const Period& forwardStart)
    : swapTenor_(swapTenor), fixedCurrency_(std::move(fixedCurrency)),
      floatIndex_(std::move(floatIndex)), fixedRate_(fixedRate), floatNominal_(floatNominal),
      forwardStart_(forwardStart) {}

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::
    operator ConstNotionalCrossCurrencyFixedVsFloatingSwap() const {
        ext::shared_ptr<ConstNotionalCrossCurrencyFixedVsFloatingSwap> swap = *this;
        return *swap;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::
    operator ext::shared_ptr<ConstNotionalCrossCurrencyFixedVsFloatingSwap>() const {

        QL_REQUIRE(effectiveDate_ == Date() || settlementDays_ == Null<Natural>(),
                   "cannot set both an explicit effective date and settlement days; "
                   "use one or the other");

        Currency floatCcy = floatCurrency_ != Currency() ? floatCurrency_ : floatIndex_->currency();

        Calendar sharedCalendar = calendar_.empty() ? floatIndex_->fixingCalendar() : calendar_;

        Date startDate;
        if (effectiveDate_ != Date()) {
            startDate = effectiveDate_;
        } else {
            Date refDate = Settings::instance().evaluationDate();
            refDate = sharedCalendar.adjust(refDate);
            Natural settlementDays =
                settlementDays_ == Null<Natural>() ? floatIndex_->fixingDays() : settlementDays_;
            Date spotDate = sharedCalendar.advance(refDate, settlementDays * Days);
            startDate = spotDate + forwardStart_;
            if (forwardStart_.length() < 0)
                startDate = sharedCalendar.adjust(startDate, Preceding);
            else if (forwardStart_.length() > 0)
                startDate = sharedCalendar.adjust(startDate, Following);
        }

        Date endDate = terminationDate_ != Date() ? terminationDate_ : startDate + swapTenor_;

        Calendar fixedCal = fixedPaymentCalendar_.empty() ? sharedCalendar : fixedPaymentCalendar_;
        Calendar floatCal = floatPaymentCalendar_.empty() ? sharedCalendar : floatPaymentCalendar_;

        Period fixedFreq = fixedFrequency_ ? Period(*fixedFrequency_) : Period(Annual);
        Period floatFreq = floatFrequency_ ? Period(*floatFrequency_) : floatIndex_->tenor();

        Schedule fixedSchedule = MakeSchedule()
                                     .from(startDate)
                                     .to(endDate)
                                     .withTenor(fixedFreq)
                                     .withCalendar(fixedCal)
                                     .withConvention(fixedPaymentConvention_)
                                     .withRule(rule_)
                                     .endOfMonth(endOfMonth_);

        BusinessDayConvention floatConvention =
            floatPaymentConvention_ ? *floatPaymentConvention_ : floatIndex_->businessDayConvention();

        Schedule floatSchedule = MakeSchedule()
                                     .from(startDate)
                                     .to(endDate)
                                     .withTenor(floatFreq)
                                     .withCalendar(floatCal)
                                     .withConvention(floatConvention)
                                     .withRule(rule_)
                                     .endOfMonth(endOfMonth_);

        DayCounter fixedDayCount = fixedDayCount_ != DayCounter() ? fixedDayCount_ : Actual365Fixed();
        DayCounter floatDayCount = floatDayCount_ != DayCounter() ? floatDayCount_ : floatIndex_->dayCounter();

        // the spot quote is units of the floating currency per unit of the
        // fixed currency, so the floating notional is divided by it.
        Real fixedNominal;
        if (fixedNominal_ != Null<Real>()) {
            fixedNominal = fixedNominal_;
        } else if (!fxSpot_.empty()) {
            QL_REQUIRE(fxSpot_->value() > 0.0,
                       "the FX spot quote must be positive; got " << fxSpot_->value());
            fixedNominal = floatNominal_ / fxSpot_->value();
        } else {
            QL_REQUIRE(fixedCurrency_ == floatCcy,
                       "an FX spot quote is required to derive the " << fixedCurrency_.code()
                           << " fixed notional from the " << floatCcy.code()
                           << " floating notional; use withFxSpot() or withFixedNominal()");
            fixedNominal = floatNominal_;
        }

        ext::shared_ptr<PricingEngine> engineToUse = engine_;
        if (engineToUse == nullptr &&
            (!fixedDiscountCurve_.empty() || !floatDiscountCurve_.empty())) {
            QL_REQUIRE(!fixedDiscountCurve_.empty() && !floatDiscountCurve_.empty(),
                       "both discount curves are needed to build the pricing engine, but only "
                       "the "
                           << (fixedDiscountCurve_.empty() ? floatCcy.code() :
                                                             fixedCurrency_.code())
                           << " one was given; use withFixedDiscountCurve() and "
                              "withFloatDiscountCurve()");
            QL_REQUIRE(!fxSpot_.empty() || fixedCurrency_ == floatCcy,
                       "an FX spot quote is required to build the pricing engine for a "
                           << fixedCurrency_.code() << "/" << floatCcy.code()
                           << " swap; use withFxSpot() or supply a custom engine with "
                              "withPricingEngine()");
            Handle<Quote> engineSpotFX = fxSpot_.empty() ? makeQuoteHandle(1.0) : fxSpot_;
            engineToUse = ext::make_shared<DiscountingConstNotionalCrossCurrencySwapEngine>(
                floatCcy, floatDiscountCurve_, fixedCurrency_, fixedDiscountCurve_, engineSpotFX);
        }

        Rate usedFixedRate = fixedRate_;
        if (fixedRate_ == Null<Rate>()) {
            QL_REQUIRE(engineToUse != nullptr,
                       "fixed rate is null and no pricing engine (or pair of discount curves) "
                       "was provided to compute the fair rate");
            auto temp = ext::make_shared<ConstNotionalCrossCurrencyFixedVsFloatingSwap>(
                type_, fixedNominal, fixedCurrency_, fixedSchedule, 0.0, fixedDayCount,
                fixedPaymentConvention_, fixedPaymentLag_, fixedCal, floatNominal_, floatCcy,
                floatSchedule, floatIndex_, floatSpread_, floatConvention, floatPaymentLag_,
                floatCal, telescopicValueDates_, floatCompoundSpread_, floatLookbackDays_,
                floatObservationShift_, floatLockoutDays_, floatAveragingMethod_,
                useIndexedCoupons_);
            temp->setPricingEngine(engineToUse);
            usedFixedRate = temp->fairRate();
        }

        auto swap = ext::make_shared<ConstNotionalCrossCurrencyFixedVsFloatingSwap>(
            type_, fixedNominal, fixedCurrency_, fixedSchedule, usedFixedRate, fixedDayCount,
            fixedPaymentConvention_, fixedPaymentLag_, fixedCal, floatNominal_, floatCcy,
            floatSchedule, floatIndex_, floatSpread_, floatConvention, floatPaymentLag_, floatCal,
            telescopicValueDates_, floatCompoundSpread_, floatLookbackDays_,
            floatObservationShift_, floatLockoutDays_, floatAveragingMethod_, useIndexedCoupons_);

        if (engineToUse != nullptr)
            swap->setPricingEngine(engineToUse);

        return swap;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::payFixed(bool flag) {
        type_ = flag ? Swap::Payer : Swap::Receiver;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::receiveFixed(bool flag) {
        type_ = flag ? Swap::Receiver : Swap::Payer;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withType(Swap::Type type) {
        type_ = type;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withFixedNominal(Real n) {
        fixedNominal_ = n;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withFloatNominal(Real n) {
        floatNominal_ = n;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withFxSpot(const Handle<Quote>& spotFX) {
        fxSpot_ = spotFX;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withFloatCurrency(const Currency& ccy) {
        floatCurrency_ = ccy;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withSettlementDays(
        Natural settlementDays) {
        settlementDays_ = settlementDays;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withEffectiveDate(
        const Date& effectiveDate) {
        effectiveDate_ = effectiveDate;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withTerminationDate(
        const Date& terminationDate) {
        terminationDate_ = terminationDate;
        if (terminationDate != Date())
            swapTenor_ = Period();
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withCalendar(const Calendar& cal) {
        calendar_ = cal;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withRule(DateGeneration::Rule r) {
        rule_ = r;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withEndOfMonth(bool flag) {
        endOfMonth_ = flag;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withFixedFrequency(Frequency f) {
        fixedFrequency_ = f;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withFixedDayCount(const DayCounter& dc) {
        fixedDayCount_ = dc;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withFixedPaymentConvention(
        BusinessDayConvention bdc) {
        fixedPaymentConvention_ = bdc;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withFixedPaymentLag(Natural lag) {
        fixedPaymentLag_ = lag;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withFixedPaymentCalendar(
        const Calendar& cal) {
        fixedPaymentCalendar_ = cal;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withFloatFrequency(Frequency f) {
        floatFrequency_ = f;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withFloatDayCount(const DayCounter& dc) {
        floatDayCount_ = dc;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withFloatSpread(Spread sp) {
        floatSpread_ = sp;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withFloatPaymentConvention(
        BusinessDayConvention bdc) {
        floatPaymentConvention_ = bdc;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withFloatPaymentLag(Natural lag) {
        floatPaymentLag_ = lag;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withFloatPaymentCalendar(
        const Calendar& cal) {
        floatPaymentCalendar_ = cal;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withIndexedCoupons(
        const std::optional<bool>& b) {
        useIndexedCoupons_ = b;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withFloatCompoundSpread(bool flag) {
        floatCompoundSpread_ = flag;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withFloatLookbackDays(
        Natural lookbackDays) {
        floatLookbackDays_ = lookbackDays;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withFloatLockoutDays(Natural lockoutDays) {
        floatLockoutDays_ = lockoutDays;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withFloatObservationShift(bool flag) {
        floatObservationShift_ = flag;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withFloatAveragingMethod(
        RateAveraging::Type averagingMethod) {
        floatAveragingMethod_ = averagingMethod;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withTelescopicValueDates(bool flag) {
        telescopicValueDates_ = flag;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withFixedDiscountCurve(
        const Handle<YieldTermStructure>& fixedDiscountCurve) {
        fixedDiscountCurve_ = fixedDiscountCurve;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withFloatDiscountCurve(
        const Handle<YieldTermStructure>& floatDiscountCurve) {
        floatDiscountCurve_ = floatDiscountCurve;
        return *this;
    }

    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap&
    MakeConstNotionalCrossCurrencyFixedVsFloatingSwap::withPricingEngine(
        const ext::shared_ptr<PricingEngine>& engine) {
        engine_ = engine;
        return *this;
    }

}
