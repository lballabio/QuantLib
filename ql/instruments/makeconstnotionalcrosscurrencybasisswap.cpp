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

#include <ql/instruments/makeconstnotionalcrosscurrencybasisswap.hpp>
#include <ql/pricingengines/swap/discountingconstnotionalcrosscurrencyswapengine.hpp>
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

    MakeConstNotionalCrossCurrencyBasisSwap::MakeConstNotionalCrossCurrencyBasisSwap(
        const Period& swapTenor,
        ext::shared_ptr<IborIndex> payIndex,
        ext::shared_ptr<IborIndex> recIndex,
        Real payNominal,
        const Period& forwardStart)
    : swapTenor_(swapTenor), payIndex_(std::move(payIndex)), recIndex_(std::move(recIndex)),
      payNominal_(payNominal), forwardStart_(forwardStart),
      payConvention_(payIndex_->businessDayConvention()),
      recConvention_(recIndex_->businessDayConvention()) {}

    MakeConstNotionalCrossCurrencyBasisSwap::operator ConstNotionalCrossCurrencyBasisSwap() const {
        ext::shared_ptr<ConstNotionalCrossCurrencyBasisSwap> swap = *this;
        return *swap;
    }

    MakeConstNotionalCrossCurrencyBasisSwap::
    operator ext::shared_ptr<ConstNotionalCrossCurrencyBasisSwap>() const {

        QL_REQUIRE(effectiveDate_ == Date() || settlementDays_ == Null<Natural>(),
                   "cannot set both an explicit effective date and settlement days; "
                   "use one or the other");

        Currency payCcy = payCurrency_ != Currency() ? payCurrency_ : payIndex_->currency();
        Currency recCcy = recCurrency_ != Currency() ? recCurrency_ : recIndex_->currency();

        Calendar jointCalendar = calendar_.empty() ?
                                     JointCalendar(payIndex_->fixingCalendar(),
                                                   recIndex_->fixingCalendar()) :
                                     calendar_;

        Date startDate;
        if (effectiveDate_ != Date()) {
            startDate = effectiveDate_;
        } else {
            Date refDate = Settings::instance().evaluationDate();
            refDate = jointCalendar.adjust(refDate);
            Natural settlementDays = settlementDays_ == Null<Natural>() ?
                                         std::max(payIndex_->fixingDays(),
                                                  recIndex_->fixingDays()) :
                                         settlementDays_;
            Date spotDate = jointCalendar.advance(refDate, settlementDays * Days);
            startDate = spotDate + forwardStart_;
            if (forwardStart_.length() < 0)
                startDate = jointCalendar.adjust(startDate, Preceding);
            else if (forwardStart_.length() > 0)
                startDate = jointCalendar.adjust(startDate, Following);
        }

        Date endDate = terminationDate_ != Date() ? terminationDate_ : startDate + swapTenor_;

        Calendar payCal = effectiveCalendar(payCalendar_, calendar_, payIndex_);
        Calendar recCal = effectiveCalendar(recCalendar_, calendar_, recIndex_);

        Period payFreq = payFrequency_ ? Period(*payFrequency_) : payIndex_->tenor();
        Period recFreq = recFrequency_ ? Period(*recFrequency_) : recIndex_->tenor();

        Schedule paySchedule = MakeSchedule()
                                   .from(startDate)
                                   .to(endDate)
                                   .withTenor(payFreq)
                                   .withCalendar(payCal)
                                   .withConvention(payConvention_)
                                   .withRule(rule_)
                                   .endOfMonth(endOfMonth_);

        Schedule recSchedule = MakeSchedule()
                                   .from(startDate)
                                   .to(endDate)
                                   .withTenor(recFreq)
                                   .withCalendar(recCal)
                                   .withConvention(recConvention_)
                                   .withRule(rule_)
                                   .endOfMonth(endOfMonth_);

        Real recNominal;
        if (recNominal_ != Null<Real>()) {
            recNominal = recNominal_;
        } else if (!fxSpot_.empty()) {
            QL_REQUIRE(fxSpot_->value() > 0.0,
                       "the FX spot quote must be positive; got " << fxSpot_->value());
            recNominal = payNominal_ * fxSpot_->value();
        } else {
            QL_REQUIRE(payCcy == recCcy,
                       "an FX spot quote is required to derive the " << recCcy.code()
                           << " receive notional from the " << payCcy.code()
                           << " pay notional; use withFxSpot() or withRecNominal()");
            recNominal = payNominal_;
        }

        auto swap = ext::make_shared<ConstNotionalCrossCurrencyBasisSwap>(
            payNominal_, payCcy, paySchedule, payIndex_, paySpread_, payGearing_, recNominal,
            recCcy, recSchedule, recIndex_, recSpread_, recGearing_, payPaymentLag_,
            recPaymentLag_, payCompoundSpread_, payLookbackDays_, payObservationShift_,
            payLockoutDays_, payAveragingMethod_, recCompoundSpread_, recLookbackDays_,
            recObservationShift_, recLockoutDays_, recAveragingMethod_, telescopicValueDates_,
            useIndexedCoupons_);

        if (engine_ != nullptr) {
            swap->setPricingEngine(engine_);
        } else if (!payDiscountCurve_.empty() || !recDiscountCurve_.empty()) {
            QL_REQUIRE(!payDiscountCurve_.empty() && !recDiscountCurve_.empty(),
                       "both discount curves are needed to build the pricing engine, but only "
                       "the "
                           << (payDiscountCurve_.empty() ? recCcy.code() : payCcy.code())
                           << " one was given; use withPayDiscountCurve() and "
                              "withRecDiscountCurve()");
            QL_REQUIRE(!fxSpot_.empty() || payCcy == recCcy,
                       "an FX spot quote is required to build the pricing engine for a "
                           << payCcy.code() << "/" << recCcy.code()
                           << " swap; use withFxSpot() or supply a custom engine with "
                              "withPricingEngine()");
            Handle<Quote> engineSpotFX = fxSpot_.empty() ? makeQuoteHandle(1.0) : fxSpot_;
            auto builtEngine = ext::make_shared<DiscountingConstNotionalCrossCurrencySwapEngine>(
                recCcy, recDiscountCurve_, payCcy, payDiscountCurve_, engineSpotFX);
            swap->setPricingEngine(builtEngine);
        }

        return swap;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withPayNominal(Real n) {
        payNominal_ = n;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withRecNominal(Real n) {
        recNominal_ = n;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withFxSpot(const Handle<Quote>& spotFX) {
        fxSpot_ = spotFX;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withPayCurrency(const Currency& ccy) {
        payCurrency_ = ccy;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withRecCurrency(const Currency& ccy) {
        recCurrency_ = ccy;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withSettlementDays(Natural settlementDays) {
        settlementDays_ = settlementDays;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withEffectiveDate(const Date& effectiveDate) {
        effectiveDate_ = effectiveDate;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withTerminationDate(const Date& terminationDate) {
        terminationDate_ = terminationDate;
        if (terminationDate != Date())
            swapTenor_ = Period();
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withCalendar(const Calendar& cal) {
        calendar_ = cal;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withPayCalendar(const Calendar& cal) {
        payCalendar_ = cal;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withRecCalendar(const Calendar& cal) {
        recCalendar_ = cal;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withPaymentConvention(BusinessDayConvention bdc) {
        payConvention_ = bdc;
        recConvention_ = bdc;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withPayPaymentConvention(BusinessDayConvention bdc) {
        payConvention_ = bdc;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withRecPaymentConvention(BusinessDayConvention bdc) {
        recConvention_ = bdc;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withRule(DateGeneration::Rule r) {
        rule_ = r;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withEndOfMonth(bool flag) {
        endOfMonth_ = flag;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withPayFrequency(Frequency f) {
        payFrequency_ = f;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withRecFrequency(Frequency f) {
        recFrequency_ = f;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withPaySpread(Spread sp) {
        paySpread_ = sp;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withRecSpread(Spread sp) {
        recSpread_ = sp;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withPayGearing(Real g) {
        payGearing_ = g;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withRecGearing(Real g) {
        recGearing_ = g;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withPayPaymentLag(Integer lag) {
        payPaymentLag_ = lag;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withRecPaymentLag(Integer lag) {
        recPaymentLag_ = lag;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withIndexedCoupons(const std::optional<bool>& b) {
        useIndexedCoupons_ = b;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withPayCompoundSpread(bool flag) {
        payCompoundSpread_ = flag;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withRecCompoundSpread(bool flag) {
        recCompoundSpread_ = flag;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withPayLookbackDays(Natural lookbackDays) {
        payLookbackDays_ = lookbackDays;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withRecLookbackDays(Natural lookbackDays) {
        recLookbackDays_ = lookbackDays;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withPayLockoutDays(Natural lockoutDays) {
        payLockoutDays_ = lockoutDays;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withRecLockoutDays(Natural lockoutDays) {
        recLockoutDays_ = lockoutDays;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withPayObservationShift(bool flag) {
        payObservationShift_ = flag;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withRecObservationShift(bool flag) {
        recObservationShift_ = flag;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withPayAveragingMethod(
        RateAveraging::Type averagingMethod) {
        payAveragingMethod_ = averagingMethod;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withRecAveragingMethod(
        RateAveraging::Type averagingMethod) {
        recAveragingMethod_ = averagingMethod;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap&
    MakeConstNotionalCrossCurrencyBasisSwap::withTelescopicValueDates(bool flag) {
        telescopicValueDates_ = flag;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap& MakeConstNotionalCrossCurrencyBasisSwap::
    withPayDiscountCurve(const Handle<YieldTermStructure>& payDiscountCurve) {
        payDiscountCurve_ = payDiscountCurve;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap& MakeConstNotionalCrossCurrencyBasisSwap::
    withRecDiscountCurve(const Handle<YieldTermStructure>& recDiscountCurve) {
        recDiscountCurve_ = recDiscountCurve;
        return *this;
    }

    MakeConstNotionalCrossCurrencyBasisSwap& MakeConstNotionalCrossCurrencyBasisSwap::
    withPricingEngine(const ext::shared_ptr<PricingEngine>& engine) {
        engine_ = engine;
        return *this;
    }

}
