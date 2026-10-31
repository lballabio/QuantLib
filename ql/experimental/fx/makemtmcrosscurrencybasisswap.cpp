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

#include <ql/experimental/fx/discountingmtmcrosscurrencybasisswapengine.hpp>
#include <ql/experimental/fx/makemtmcrosscurrencybasisswap.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/settings.hpp>
#include <ql/time/calendars/jointcalendar.hpp>
#include <ql/time/schedule.hpp>
#include <utility>

namespace QuantLib {

    namespace {

        Calendar effectiveCalendar(const Calendar& legCalendar,
                                   const Calendar& sharedCalendar,
                                   const ext::shared_ptr<IborIndex>& index) {
            if (!legCalendar.empty())
                return legCalendar;
            if (!sharedCalendar.empty())
                return sharedCalendar;
            return index->fixingCalendar();
        }

    }

    MakeMtMCrossCurrencyBasisSwap::MakeMtMCrossCurrencyBasisSwap(
        const Period& swapTenor,
        ext::shared_ptr<IborIndex> fxBaseIndex,
        ext::shared_ptr<IborIndex> fxQuoteIndex,
        bool isFxBaseCurrencyLegResettable,
        Real fxBaseNominal,
        const Period& forwardStart)
    : swapTenor_(swapTenor), fxBaseIndex_(std::move(fxBaseIndex)),
      fxQuoteIndex_(std::move(fxQuoteIndex)),
      isFxBaseCurrencyLegResettable_(isFxBaseCurrencyLegResettable),
      fxBaseNominal_(fxBaseNominal), forwardStart_(forwardStart),
      fxBasePaymentConvention_(fxBaseIndex_->businessDayConvention()),
      fxQuotePaymentConvention_(fxQuoteIndex_->businessDayConvention()) {}

    MakeMtMCrossCurrencyBasisSwap::operator MtMCrossCurrencyBasisSwap() const {
        ext::shared_ptr<MtMCrossCurrencyBasisSwap> swap = *this;
        return *swap;
    }

    MakeMtMCrossCurrencyBasisSwap::operator ext::shared_ptr<MtMCrossCurrencyBasisSwap>() const {

        QL_REQUIRE(effectiveDate_ == Date() || settlementDays_ == Null<Natural>(),
                   "cannot set both an explicit effective date and settlement days; "
                   "use one or the other");

        Currency fxBaseCcy =
            fxBaseCurrency_ != Currency() ? fxBaseCurrency_ : fxBaseIndex_->currency();
        Currency fxQuoteCcy =
            fxQuoteCurrency_ != Currency() ? fxQuoteCurrency_ : fxQuoteIndex_->currency();

        Calendar jointCalendar = calendar_.empty() ?
                                     JointCalendar(fxBaseIndex_->fixingCalendar(),
                                                   fxQuoteIndex_->fixingCalendar()) :
                                     calendar_;

        Date startDate;
        if (effectiveDate_ != Date()) {
            startDate = effectiveDate_;
        } else {
            Date refDate = Settings::instance().evaluationDate();
            refDate = jointCalendar.adjust(refDate);
            Natural settlementDays = settlementDays_ == Null<Natural>() ?
                                         std::max(fxBaseIndex_->fixingDays(),
                                                  fxQuoteIndex_->fixingDays()) :
                                         settlementDays_;
            Date spotDate = jointCalendar.advance(refDate, settlementDays * Days);
            startDate = spotDate + forwardStart_;
            if (forwardStart_.length() < 0)
                startDate = jointCalendar.adjust(startDate, Preceding);
            else if (forwardStart_.length() > 0)
                startDate = jointCalendar.adjust(startDate, Following);
        }

        Date endDate = terminationDate_ != Date() ? terminationDate_ : startDate + swapTenor_;

        Calendar fxBaseCal = effectiveCalendar(fxBaseCalendar_, calendar_, fxBaseIndex_);
        Calendar fxQuoteCal = effectiveCalendar(fxQuoteCalendar_, calendar_, fxQuoteIndex_);

        Period fxBaseFreq = fxBaseFrequency_ ? Period(*fxBaseFrequency_) : fxBaseIndex_->tenor();
        Period fxQuoteFreq =
            fxQuoteFrequency_ ? Period(*fxQuoteFrequency_) : fxQuoteIndex_->tenor();

        Schedule fxBaseSchedule = MakeSchedule()
                                      .from(startDate)
                                      .to(endDate)
                                      .withTenor(fxBaseFreq)
                                      .withCalendar(fxBaseCal)
                                      .withConvention(fxBasePaymentConvention_)
                                      .withRule(rule_)
                                      .endOfMonth(endOfMonth_);

        Schedule fxQuoteSchedule = MakeSchedule()
                                       .from(startDate)
                                       .to(endDate)
                                       .withTenor(fxQuoteFreq)
                                       .withCalendar(fxQuoteCal)
                                       .withConvention(fxQuotePaymentConvention_)
                                       .withRule(rule_)
                                       .endOfMonth(endOfMonth_);

        Real fxQuoteNominal;
        if (fxQuoteNominal_ != Null<Real>()) {
            fxQuoteNominal = fxQuoteNominal_;
        } else if (!fxSpot_.empty()) {
            QL_REQUIRE(fxSpot_->value() > 0.0,
                       "the FX spot quote must be positive; got " << fxSpot_->value());
            fxQuoteNominal = fxBaseNominal_ * fxSpot_->value();
        } else {
            QL_REQUIRE(fxBaseCcy == fxQuoteCcy,
                       "an FX spot quote is required to derive the " << fxQuoteCcy.code()
                           << " notional from the " << fxBaseCcy.code()
                           << " notional; use withFxSpot() or withFxQuoteNominal()");
            fxQuoteNominal = fxBaseNominal_;
        }

        auto swap = ext::make_shared<MtMCrossCurrencyBasisSwap>(
            type_, fxBaseNominal_, fxBaseCcy, fxBaseSchedule, fxBaseIndex_, fxBaseSpread_,
            fxBaseGearing_, fxQuoteNominal, fxQuoteCcy, fxQuoteSchedule, fxQuoteIndex_,
            fxQuoteSpread_, fxQuoteGearing_, isFxBaseCurrencyLegResettable_, fxResetFixingDays_,
            fxResetFixingCalendar_, fxBasePaymentLag_, fxQuotePaymentLag_,
            fxBasePaymentConvention_, fxQuotePaymentConvention_, fxBaseCompoundSpread_,
            fxBaseLookbackDays_, fxBaseObservationShift_, fxBaseLockoutDays_,
            fxBaseAveragingMethod_, fxQuoteCompoundSpread_, fxQuoteLookbackDays_,
            fxQuoteObservationShift_, fxQuoteLockoutDays_, fxQuoteAveragingMethod_,
            telescopicValueDates_, useIndexedCoupons_);

        if (engine_ != nullptr) {
            swap->setPricingEngine(engine_);
        } else if (!fxBaseDiscountCurve_.empty() || !fxQuoteDiscountCurve_.empty()) {
            QL_REQUIRE(!fxBaseDiscountCurve_.empty() && !fxQuoteDiscountCurve_.empty(),
                       "both discount curves are needed to build the pricing engine, but only "
                       "the "
                           << (fxBaseDiscountCurve_.empty() ? fxQuoteCcy.code() :
                                                              fxBaseCcy.code())
                           << " one was given; use withFxBaseDiscountCurve() and "
                              "withFxQuoteDiscountCurve()");
            QL_REQUIRE(!fxSpot_.empty() || fxBaseCcy == fxQuoteCcy,
                       "an FX spot quote is required to build the pricing engine for a "
                           << fxBaseCcy.code() << "/" << fxQuoteCcy.code()
                           << " swap; use withFxSpot() or supply a custom engine with "
                              "withPricingEngine()");
            Handle<Quote> engineSpotFX = fxSpot_.empty() ? makeQuoteHandle(1.0) : fxSpot_;
            auto builtEngine = ext::make_shared<DiscountingMtMCrossCurrencyBasisSwapEngine>(
                fxQuoteCcy, fxQuoteDiscountCurve_, fxBaseCcy, fxBaseDiscountCurve_, engineSpotFX);
            swap->setPricingEngine(builtEngine);
        }

        return swap;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::payFxBaseCurrency(bool flag) {
        type_ = flag ? MtMCrossCurrencyBasisSwap::Type::PayFxBaseCurrency :
                       MtMCrossCurrencyBasisSwap::Type::ReceiveFxBaseCurrency;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap&
    MakeMtMCrossCurrencyBasisSwap::receiveFxBaseCurrency(bool flag) {
        type_ = flag ? MtMCrossCurrencyBasisSwap::Type::ReceiveFxBaseCurrency :
                       MtMCrossCurrencyBasisSwap::Type::PayFxBaseCurrency;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap&
    MakeMtMCrossCurrencyBasisSwap::withType(MtMCrossCurrencyBasisSwap::Type type) {
        type_ = type;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withFxBaseNominal(Real n) {
        fxBaseNominal_ = n;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withFxQuoteNominal(Real n) {
        fxQuoteNominal_ = n;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap&
    MakeMtMCrossCurrencyBasisSwap::withFxSpot(const Handle<Quote>& spotFX) {
        fxSpot_ = spotFX;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap&
    MakeMtMCrossCurrencyBasisSwap::withFxBaseCurrency(const Currency& ccy) {
        fxBaseCurrency_ = ccy;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap&
    MakeMtMCrossCurrencyBasisSwap::withFxQuoteCurrency(const Currency& ccy) {
        fxQuoteCurrency_ = ccy;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap&
    MakeMtMCrossCurrencyBasisSwap::withSettlementDays(Natural settlementDays) {
        settlementDays_ = settlementDays;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap&
    MakeMtMCrossCurrencyBasisSwap::withEffectiveDate(const Date& effectiveDate) {
        effectiveDate_ = effectiveDate;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap&
    MakeMtMCrossCurrencyBasisSwap::withTerminationDate(const Date& terminationDate) {
        terminationDate_ = terminationDate;
        if (terminationDate != Date())
            swapTenor_ = Period();
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withCalendar(
        const Calendar& cal) {
        calendar_ = cal;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap&
    MakeMtMCrossCurrencyBasisSwap::withFxBaseCalendar(const Calendar& cal) {
        fxBaseCalendar_ = cal;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap&
    MakeMtMCrossCurrencyBasisSwap::withFxQuoteCalendar(const Calendar& cal) {
        fxQuoteCalendar_ = cal;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap&
    MakeMtMCrossCurrencyBasisSwap::withPaymentConvention(BusinessDayConvention bdc) {
        fxBasePaymentConvention_ = bdc;
        fxQuotePaymentConvention_ = bdc;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap&
    MakeMtMCrossCurrencyBasisSwap::withFxBasePaymentConvention(BusinessDayConvention bdc) {
        fxBasePaymentConvention_ = bdc;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap&
    MakeMtMCrossCurrencyBasisSwap::withFxQuotePaymentConvention(BusinessDayConvention bdc) {
        fxQuotePaymentConvention_ = bdc;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withRule(
        DateGeneration::Rule r) {
        rule_ = r;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withEndOfMonth(bool flag) {
        endOfMonth_ = flag;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withFxBaseFrequency(
        Frequency f) {
        fxBaseFrequency_ = f;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withFxQuoteFrequency(
        Frequency f) {
        fxQuoteFrequency_ = f;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withFxBaseSpread(Spread sp) {
        fxBaseSpread_ = sp;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withFxQuoteSpread(Spread sp) {
        fxQuoteSpread_ = sp;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withFxBaseGearing(Real g) {
        fxBaseGearing_ = g;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withFxQuoteGearing(Real g) {
        fxQuoteGearing_ = g;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withFxBasePaymentLag(
        Integer lag) {
        fxBasePaymentLag_ = lag;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withFxQuotePaymentLag(
        Integer lag) {
        fxQuotePaymentLag_ = lag;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withIndexedCoupons(
        const std::optional<bool>& b) {
        useIndexedCoupons_ = b;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap&
    MakeMtMCrossCurrencyBasisSwap::withFxResetFixingDays(Natural fixingDays) {
        fxResetFixingDays_ = fixingDays;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap&
    MakeMtMCrossCurrencyBasisSwap::withFxResetFixingCalendar(const Calendar& cal) {
        fxResetFixingCalendar_ = cal;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withFxBaseCompoundSpread(
        bool flag) {
        fxBaseCompoundSpread_ = flag;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withFxQuoteCompoundSpread(
        bool flag) {
        fxQuoteCompoundSpread_ = flag;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withFxBaseLookbackDays(
        Natural lookbackDays) {
        fxBaseLookbackDays_ = lookbackDays;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withFxQuoteLookbackDays(
        Natural lookbackDays) {
        fxQuoteLookbackDays_ = lookbackDays;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withFxBaseLockoutDays(
        Natural lockoutDays) {
        fxBaseLockoutDays_ = lockoutDays;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withFxQuoteLockoutDays(
        Natural lockoutDays) {
        fxQuoteLockoutDays_ = lockoutDays;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withFxBaseObservationShift(
        bool flag) {
        fxBaseObservationShift_ = flag;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withFxQuoteObservationShift(
        bool flag) {
        fxQuoteObservationShift_ = flag;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withFxBaseAveragingMethod(
        RateAveraging::Type averagingMethod) {
        fxBaseAveragingMethod_ = averagingMethod;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withFxQuoteAveragingMethod(
        RateAveraging::Type averagingMethod) {
        fxQuoteAveragingMethod_ = averagingMethod;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withTelescopicValueDates(
        bool flag) {
        telescopicValueDates_ = flag;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withFxBaseDiscountCurve(
        const Handle<YieldTermStructure>& fxBaseDiscountCurve) {
        fxBaseDiscountCurve_ = fxBaseDiscountCurve;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withFxQuoteDiscountCurve(
        const Handle<YieldTermStructure>& fxQuoteDiscountCurve) {
        fxQuoteDiscountCurve_ = fxQuoteDiscountCurve;
        return *this;
    }

    MakeMtMCrossCurrencyBasisSwap& MakeMtMCrossCurrencyBasisSwap::withPricingEngine(
        const ext::shared_ptr<PricingEngine>& engine) {
        engine_ = engine;
        return *this;
    }

}
