/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Marcin Rybacki
 Copyright (C) 2025 Uzair Beg
 Copyright (C) 2026 Kyrylo Protsenko

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

#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/experimental/fx/discountingmtmcrosscurrencybasisswapengine.hpp>
#include <ql/experimental/termstructures/crosscurrencyratehelpers.hpp>
#include <ql/pricingengines/swap/discountingconstnotionalcrosscurrencyswapengine.hpp>
#include <ql/utilities/null_deleter.hpp>
#include <utility>

namespace QuantLib {

    namespace {

        constexpr double sample_fixed_rate = 0.01;

        // Treat an explicitly-passed NoFrequency the same as an unset (nullopt)
        // payment frequency.  Before these parameters were migrated to
        // std::optional<Frequency>, NoFrequency was the sentinel meaning "derive
        // the schedule from the index tenor".  Normalizing it here preserves that
        // behavior and keeps the stored optional either empty or holding an
        // actual frequency, so the rest of the code can treat the two cases
        // identically.
        std::optional<Frequency> normalizedPaymentFrequency(std::optional<Frequency> frequency) {
            if (frequency && *frequency == NoFrequency)
                return std::nullopt;
            return frequency;
        }

        Schedule legSchedule(const Date& evaluationDate,
                             const Period& tenor,
                             const Period& frequency,
                             Natural fixingDays,
                             const Calendar& calendar,
                             BusinessDayConvention convention,
                             bool endOfMonth) {
            QL_REQUIRE(tenor >= frequency,
                       "XCCY instrument tenor should not be smaller than coupon frequency.");

            Date referenceDate = calendar.adjust(evaluationDate);
            Date earliestDate = calendar.advance(referenceDate, fixingDays * Days, convention);
            Date maturity = earliestDate + tenor;
            return MakeSchedule()
                .from(earliestDate)
                .to(maturity)
                .withTenor(frequency)
                .withCalendar(calendar)
                .withConvention(convention)
                .endOfMonth(endOfMonth)
                .backwards();
        }

        Schedule floatingLegSchedule(const Date& evaluationDate,
                                     const Period& tenor,
                                     Natural fixingDays,
                                     const Calendar& calendar,
                                     BusinessDayConvention convention,
                                     bool endOfMonth,
                                     const ext::shared_ptr<IborIndex>& idx,
                                     std::optional<Frequency> paymentFrequency) {
            auto overnightIndex = ext::dynamic_pointer_cast<OvernightIndex>(idx);

            Period freqPeriod;
            if (!paymentFrequency) {
                QL_REQUIRE(!overnightIndex, "Require payment frequency for overnight indices.");
                freqPeriod = idx->tenor();
            } else {
                freqPeriod = Period(*paymentFrequency);
            }

            return legSchedule(evaluationDate, tenor, freqPeriod, fixingDays, calendar,
                               convention, endOfMonth);
        }

        Leg buildFloatingLeg(const Schedule& schedule,
                             const ext::shared_ptr<IborIndex>& idx,
                             Integer paymentLag,
                             std::optional<bool> useIndexedCoupons,
                             const StubIndexConfig& stubIndexConfig = {}) {
            if (auto overnightIndex = ext::dynamic_pointer_cast<OvernightIndex>(idx)) {
                return OvernightLeg(schedule, overnightIndex)
                    .withNotionals(1.0)
                    .withPaymentLag(paymentLag);
            }
            return IborLeg(schedule, idx)
                .withNotionals(1.0)
                .withPaymentLag(paymentLag)
                .withIndexedCoupons(useIndexedCoupons)
                .withStubIndexConfig(stubIndexConfig);
        }

        std::pair<Real, Real>
        npvbpsConstNotionalLeg(const Leg& leg,
                               const Date& initialNotionalExchangeDate,
                               const Date& finalNotionalExchangeDate,
                               const Handle<YieldTermStructure>& discountCurveHandle) {
            const Spread basisPoint = 1.0e-4;
            Date refDt = discountCurveHandle->referenceDate();
            const YieldTermStructure& discountRef = **discountCurveHandle;
            bool includeSettleDtFlows = true;
            auto [npv, bps] = CashFlows::npvbps(leg, discountRef, includeSettleDtFlows, refDt, refDt);
            // Include NPV of the notional exchange at start and maturity.
            npv += (-1.0) * discountRef.discount(initialNotionalExchangeDate);
            npv += discountRef.discount(finalNotionalExchangeDate);
            bps /= basisPoint;
            return { npv, bps };
        }

    }


    CrossCurrencySwapRateHelperBase::CrossCurrencySwapRateHelperBase(
        const Handle<Quote>& quote,
        const Period& tenor,
        Natural fixingDays,
        Calendar calendar,
        BusinessDayConvention convention,
        bool endOfMonth,
        Handle<YieldTermStructure> collateralCurve,
        Integer paymentLag,
        bool paymentLagOnNotionalExchanges)
    : RelativeDateRateHelper(quote), tenor_(tenor), fixingDays_(fixingDays),
      calendar_(std::move(calendar)), convention_(convention), endOfMonth_(endOfMonth),
      paymentLag_(paymentLag), paymentLagOnNotionalExchanges_(paymentLagOnNotionalExchanges),
      collateralHandle_(std::move(collateralCurve)) {
        registerWith(collateralHandle_);
    }

    void CrossCurrencySwapRateHelperBase::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        bool observer = false;

        ext::shared_ptr<YieldTermStructure> temp(t, null_deleter());
        termStructureHandle_.linkTo(temp, observer);

        RelativeDateRateHelper::setTermStructure(t);
    }

    void CrossCurrencySwapRateHelperBase::initializeDatesFromLegs(const Leg& firstLeg,
                                                                  const Leg& secondLeg) {
        earliestDate_ = std::min(CashFlows::startDate(firstLeg),
                                 CashFlows::startDate(secondLeg));

        maturityDate_ = std::max(CashFlows::maturityDate(firstLeg),
                                 CashFlows::maturityDate(secondLeg));

        // Principal exchanges settle on the effective and maturity dates:
        // the payment lag applies only to coupons -- unless the swap's
        // convention lags the principal flows too, in which case both
        // exchanges move by the same lag as the coupons and the final
        // exchange settles together with the final coupon.  advance() with
        // zero days reduces to adjust(), preserving the default behaviour.
        Integer exchangeLag = paymentLagOnNotionalExchanges_ ? paymentLag_ : 0;
        initialNotionalExchangeDate_ = calendar_.advance(earliestDate_, exchangeLag, Days, convention_);
        finalNotionalExchangeDate_   = calendar_.advance(maturityDate_, exchangeLag, Days, convention_);

        Date lastPaymentDate =
            std::max(firstLeg.back()->date(),
                     secondLeg.back()->date());

        latestRelevantDate_ = latestDate_ = std::max(maturityDate_, lastPaymentDate);
    }



    CrossCurrencyBasisSwapRateHelperBase::CrossCurrencyBasisSwapRateHelperBase(
        const Handle<Quote>& basis,
        const Period& tenor,
        Natural fixingDays,
        Calendar calendar,
        BusinessDayConvention convention,
        bool endOfMonth,
        ext::shared_ptr<IborIndex> baseCurrencyIndex,
        ext::shared_ptr<IborIndex> quoteCurrencyIndex,
        Handle<YieldTermStructure> collateralCurve,
        bool isFxBaseCurrencyCollateralCurrency,
        bool isBasisOnFxBaseCurrencyLeg,
        std::optional<Frequency> paymentFrequency,
        Integer paymentLag,
        std::optional<Frequency> quoteCurrencyPaymentFrequency,
        std::optional<bool> useIndexedCoupons,
        bool paymentLagOnNotionalExchanges,
        StubIndexConfig baseStubIndexConfig,
        StubIndexConfig quoteStubIndexConfig)
    : CrossCurrencySwapRateHelperBase(basis, tenor, fixingDays, std::move(calendar), convention, endOfMonth,
                                      std::move(collateralCurve), paymentLag,
                                      paymentLagOnNotionalExchanges),
      baseCcyIdx_(std::move(baseCurrencyIndex)), quoteCcyIdx_(std::move(quoteCurrencyIndex)),
      isFxBaseCurrencyCollateralCurrency_(isFxBaseCurrencyCollateralCurrency),
      isBasisOnFxBaseCurrencyLeg_(isBasisOnFxBaseCurrencyLeg),
      paymentFrequency_(normalizedPaymentFrequency(paymentFrequency)),
      quoteCcyPaymentFrequency_(normalizedPaymentFrequency(quoteCurrencyPaymentFrequency)),
      useIndexedCoupons_(useIndexedCoupons),
      baseStubIndexConfig_(std::move(baseStubIndexConfig)),
      quoteStubIndexConfig_(std::move(quoteStubIndexConfig)) {
        registerWith(baseCcyIdx_);
        registerWith(quoteCcyIdx_);
        for (const auto& candidate : baseStubIndexConfig_.indices())
            registerWith(candidate);
        for (const auto& candidate : quoteStubIndexConfig_.indices())
            registerWith(candidate);

        CrossCurrencyBasisSwapRateHelperBase::initializeDates();
    }

    void CrossCurrencyBasisSwapRateHelperBase::initializeDates() {
        baseCcySchedule_ = floatingLegSchedule(evaluationDate_, tenor_, fixingDays_, calendar_,
                                               convention_, endOfMonth_, baseCcyIdx_,
                                               paymentFrequency_);
        baseCcyIborLeg_ = buildFloatingLeg(
            baseCcySchedule_, baseCcyIdx_, paymentLag_, useIndexedCoupons_,
            baseStubIndexConfig_);

        // If no quote-currency payment frequency was given, fall back to the
        // base-currency payment frequency (which may itself be unset, in which
        // case the quote-currency leg uses its own index tenor).
        std::optional<Frequency> effectiveQuoteCcyFreq =
            quoteCcyPaymentFrequency_ ? quoteCcyPaymentFrequency_ : paymentFrequency_;
        quoteCcySchedule_ = floatingLegSchedule(evaluationDate_, tenor_, fixingDays_, calendar_,
                                                convention_, endOfMonth_, quoteCcyIdx_,
                                                effectiveQuoteCcyFreq);
        quoteCcyIborLeg_ = buildFloatingLeg(
            quoteCcySchedule_, quoteCcyIdx_, paymentLag_, useIndexedCoupons_,
            quoteStubIndexConfig_);

        initializeDatesFromLegs(baseCcyIborLeg_, quoteCcyIborLeg_);
    }

    const Handle<YieldTermStructure>&
    CrossCurrencyBasisSwapRateHelperBase::baseCcyLegDiscountHandle() const {
        return isFxBaseCurrencyCollateralCurrency_ ? collateralHandle_ : termStructureHandle_;
    }

    const Handle<YieldTermStructure>&
    CrossCurrencyBasisSwapRateHelperBase::quoteCcyLegDiscountHandle() const {
        return isFxBaseCurrencyCollateralCurrency_ ? termStructureHandle_ : collateralHandle_;
    }

    ConstNotionalCrossCurrencyBasisSwapRateHelper::ConstNotionalCrossCurrencyBasisSwapRateHelper(
        const Handle<Quote>& basis,
        const Period& tenor,
        Natural fixingDays,
        const Calendar& calendar,
        BusinessDayConvention convention,
        bool endOfMonth,
        const ext::shared_ptr<IborIndex>& baseCurrencyIndex,
        const ext::shared_ptr<IborIndex>& quoteCurrencyIndex,
        const Handle<YieldTermStructure>& collateralCurve,
        bool isFxBaseCurrencyCollateralCurrency,
        bool isBasisOnFxBaseCurrencyLeg,
        std::optional<Frequency> paymentFrequency,
        Integer paymentLag,
        std::optional<Frequency> quoteCurrencyPaymentFrequency,
        std::optional<bool> useIndexedCoupons,
        bool paymentLagOnNotionalExchanges,
        StubIndexConfig baseStubIndexConfig,
        StubIndexConfig quoteStubIndexConfig)
    : CrossCurrencyBasisSwapRateHelperBase(basis,
                                           tenor,
                                           fixingDays,
                                           calendar,
                                           convention,
                                           endOfMonth,
                                           baseCurrencyIndex,
                                           quoteCurrencyIndex,
                                           collateralCurve,
                                           isFxBaseCurrencyCollateralCurrency,
                                           isBasisOnFxBaseCurrencyLeg,
                                           paymentFrequency,
                                           paymentLag,
                                           quoteCurrencyPaymentFrequency,
                                           useIndexedCoupons,
                                           paymentLagOnNotionalExchanges,
                                           std::move(baseStubIndexConfig),
                                           std::move(quoteStubIndexConfig)) {
        buildSwap();
    }

    void ConstNotionalCrossCurrencyBasisSwapRateHelper::initializeDates() {
        CrossCurrencyBasisSwapRateHelperBase::initializeDates();
        buildSwap();
    }

    void ConstNotionalCrossCurrencyBasisSwapRateHelper::buildSwap() {
        // The exposed swap mirrors the helper's par convention: unit notionals,
        // zero spreads and spot FX = 1, so that its fair spread on the basis
        // leg reproduces the helper quote.  It pays the base-currency leg.
        swap_ = ext::make_shared<ConstNotionalCrossCurrencyBasisSwap>(
            1.0, baseCcyIdx_->currency(), baseCcySchedule_, baseCcyIdx_, 0.0, 1.0,
            1.0, quoteCcyIdx_->currency(), quoteCcySchedule_, quoteCcyIdx_, 0.0, 1.0,
            paymentLag_, paymentLag_, false, Null<Natural>(), false, 0,
            RateAveraging::Compound, false, Null<Natural>(), false, 0,
            RateAveraging::Compound, false, useIndexedCoupons_,
            paymentLagOnNotionalExchanges_, baseStubIndexConfig_,
            quoteStubIndexConfig_);
        swap_->setPricingEngine(ext::make_shared<DiscountingConstNotionalCrossCurrencySwapEngine>(
            quoteCcyIdx_->currency(), quoteCcyLegDiscountHandle(),
            baseCcyIdx_->currency(), baseCcyLegDiscountHandle(),
            makeQuoteHandle(1.0), true));
    }

    Real ConstNotionalCrossCurrencyBasisSwapRateHelper::impliedQuote() const {
        QL_REQUIRE(!termStructureHandle_.empty(), "term structure not set");
        QL_REQUIRE(!collateralHandle_.empty(), "collateral term structure not set");

        auto [npvBaseCcy, bpsBaseCcy] = npvbpsConstNotionalLeg(baseCcyIborLeg_, initialNotionalExchangeDate_, finalNotionalExchangeDate_, baseCcyLegDiscountHandle());
        auto [npvQuoteCcy, bpsQuoteCcy] = npvbpsConstNotionalLeg(quoteCcyIborLeg_, initialNotionalExchangeDate_, finalNotionalExchangeDate_, quoteCcyLegDiscountHandle());

        Real bps = isBasisOnFxBaseCurrencyLeg_ ? -bpsBaseCcy : bpsQuoteCcy;

        QL_REQUIRE(std::fabs(bps) > 0.0, "null BPS");

        return -(npvQuoteCcy - npvBaseCcy) / bps;
    }

    void ConstNotionalCrossCurrencyBasisSwapRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<ConstNotionalCrossCurrencyBasisSwapRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }


    MtMCrossCurrencyBasisSwapRateHelper::MtMCrossCurrencyBasisSwapRateHelper(
        const Handle<Quote>& basis,
        const Period& tenor,
        Natural fixingDays,
        const Calendar& calendar,
        BusinessDayConvention convention,
        bool endOfMonth,
        const ext::shared_ptr<IborIndex>& baseCurrencyIndex,
        const ext::shared_ptr<IborIndex>& quoteCurrencyIndex,
        const Handle<YieldTermStructure>& collateralCurve,
        bool isFxBaseCurrencyCollateralCurrency,
        bool isBasisOnFxBaseCurrencyLeg,
        bool isFxBaseCurrencyLegResettable,
        std::optional<Frequency> paymentFrequency,
        Integer paymentLag,
        std::optional<Frequency> quoteCurrencyPaymentFrequency,
        Natural fxResetFixingDays,
        Calendar fxResetFixingCalendar,
        std::optional<bool> useIndexedCoupons,
        StubIndexConfig baseStubIndexConfig,
        StubIndexConfig quoteStubIndexConfig)
    : CrossCurrencyBasisSwapRateHelperBase(basis,
                                           tenor,
                                           fixingDays,
                                           calendar,
                                           convention,
                                           endOfMonth,
                                           baseCurrencyIndex,
                                           quoteCurrencyIndex,
                                           collateralCurve,
                                           isFxBaseCurrencyCollateralCurrency,
                                           isBasisOnFxBaseCurrencyLeg,
                                           paymentFrequency,
                                           paymentLag,
                                           quoteCurrencyPaymentFrequency,
                                           useIndexedCoupons,
                                           false,
                                           std::move(baseStubIndexConfig),
                                           std::move(quoteStubIndexConfig)),
      isFxBaseCurrencyLegResettable_(isFxBaseCurrencyLegResettable),
      fxResetFixingDays_(fxResetFixingDays), fxResetFixingCalendar_(std::move(fxResetFixingCalendar)) {
        buildSwap();
    }

    void MtMCrossCurrencyBasisSwapRateHelper::initializeDates() {
        CrossCurrencyBasisSwapRateHelperBase::initializeDates();
        buildSwap();
    }

    void MtMCrossCurrencyBasisSwapRateHelper::buildSwap() {
        // The exposed swap mirrors the helper's par convention: unit notionals,
        // zero spreads and spot FX = 1, so that its fair spread on the basis
        // leg reproduces the helper quote.  It pays the base-currency leg.
        swap_ = ext::make_shared<MtMCrossCurrencyBasisSwap>(
            MtMCrossCurrencyBasisSwap::Type::PayFxBaseCurrency,
            1.0, baseCcyIdx_->currency(), baseCcySchedule_, baseCcyIdx_, 0.0, 1.0,
            1.0, quoteCcyIdx_->currency(), quoteCcySchedule_, quoteCcyIdx_, 0.0, 1.0,
            isFxBaseCurrencyLegResettable_, fxResetFixingDays_,
            fxResetFixingCalendar_, paymentLag_, paymentLag_,
            convention_, convention_, false, Null<Natural>(), false, 0,
            RateAveraging::Compound, false, Null<Natural>(), false, 0,
            RateAveraging::Compound, false, useIndexedCoupons_,
            baseStubIndexConfig_, quoteStubIndexConfig_);
        swap_->setPricingEngine(ext::make_shared<DiscountingMtMCrossCurrencyBasisSwapEngine>(
            quoteCcyIdx_->currency(), quoteCcyLegDiscountHandle(),
            baseCcyIdx_->currency(), baseCcyLegDiscountHandle(),
            makeQuoteHandle(1.0), true));
    }

    Real MtMCrossCurrencyBasisSwapRateHelper::impliedQuote() const {
        QL_REQUIRE(!termStructureHandle_.empty(), "term structure not set");
        QL_REQUIRE(!collateralHandle_.empty(), "collateral term structure not set");

        swap_->deepUpdate();
        if (isBasisOnFxBaseCurrencyLeg_)
            return swap_->fairFxBaseSpread();
        return swap_->fairFxQuoteSpread();
    }

    void MtMCrossCurrencyBasisSwapRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<MtMCrossCurrencyBasisSwapRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }


    ConstNotionalCrossCurrencySwapRateHelper::ConstNotionalCrossCurrencySwapRateHelper(
        const Handle<Quote>& fixedRate,
        const Period& tenor,
        Natural fixingDays,
        const Calendar& calendar,
        BusinessDayConvention convention,
        bool endOfMonth,
        Frequency fixedFrequency,
        DayCounter  fixedDayCount,
        const ext::shared_ptr<IborIndex>& floatIndex,
        const Handle<YieldTermStructure>& collateralCurve,
        bool collateralOnFixedLeg,
        Integer paymentLag,
        std::optional<bool> useIndexedCoupons,
        std::optional<Frequency> floatPaymentFrequency,
        StubIndexConfig floatStubIndexConfig)
    : CrossCurrencySwapRateHelperBase(fixedRate, tenor, fixingDays, calendar, convention, endOfMonth,
                                      collateralCurve, paymentLag),
      fixedFrequency_(fixedFrequency),
      fixedDayCount_(std::move(fixedDayCount)),
      floatIndex_(floatIndex),
      collateralOnFixedLeg_(collateralOnFixedLeg),
      useIndexedCoupons_(useIndexedCoupons),
      floatPaymentFrequency_(normalizedPaymentFrequency(floatPaymentFrequency)),
      floatStubIndexConfig_(std::move(floatStubIndexConfig)) {

        QL_REQUIRE(floatIndex_, "floating index required");
        registerWith(floatIndex_);
        for (const auto& candidate : floatStubIndexConfig_.indices())
            registerWith(candidate);

        initializeDates();
    }

    void ConstNotionalCrossCurrencySwapRateHelper::initializeDates() {
        Real nominal = 1.0;
        // Both legs roll on the helper's calendar and convention.  The floating
        // index supplies its fixing calendar for fixings, not for the accrual schedule.
        // Taking the roll convention or calendar from the index would make the
        // two legs diverge at a month-end roll and move the implied pillar.
        Schedule fixedSch = legSchedule(evaluationDate_, tenor_, Period(fixedFrequency_), fixingDays_, calendar_,
                                       convention_, endOfMonth_);
        // An overnight index has no payment frequency of its own, so
        // floatingLegSchedule requires one to have been supplied.
        Schedule floatSch = floatingLegSchedule(evaluationDate_, tenor_, fixingDays_, calendar_,
                                                convention_, endOfMonth_, floatIndex_,
                                                floatPaymentFrequency_);

        xccySwap_ = ext::make_shared<ConstNotionalCrossCurrencyFixedVsFloatingSwap>(
            Swap::Payer,
            nominal,
            Currency(), 
            fixedSch,
            sample_fixed_rate,
            fixedDayCount_,
            convention_,
            paymentLag_,
            calendar_,
            nominal,
            floatIndex_->currency(),
            floatSch,
            floatIndex_,
            Spread(0.0),
            convention_,
            paymentLag_,
            calendar_,
            false, false, Null<Natural>(), false, 0,
            RateAveraging::Compound, useIndexedCoupons_,
            floatStubIndexConfig_
        );
        auto engine = ext::make_shared<DiscountingConstNotionalCrossCurrencySwapEngine>(
            floatIndex_->currency(), floatingLegDiscountHandle(),
            Currency(), fixedLegDiscountHandle(),
            makeQuoteHandle(1.0), true);
        xccySwap_->setPricingEngine(engine);

        initializeDatesFromLegs(xccySwap_->leg(0), xccySwap_->leg(1));
    }

    const Handle<YieldTermStructure>&
    ConstNotionalCrossCurrencySwapRateHelper::fixedLegDiscountHandle() const {
        return collateralOnFixedLeg_ ? collateralHandle_ : termStructureHandle_;
    }

    const Handle<YieldTermStructure>&
    ConstNotionalCrossCurrencySwapRateHelper::floatingLegDiscountHandle() const {
        return collateralOnFixedLeg_ ? termStructureHandle_ : collateralHandle_;
    }

    Real ConstNotionalCrossCurrencySwapRateHelper::impliedQuote() const {
        QL_REQUIRE(!termStructureHandle_.empty(), "term structure not set");
        QL_REQUIRE(!collateralHandle_.empty(), "collateral term structure not set");
        xccySwap_->deepUpdate();

        return xccySwap_->fairRate();
    }

    void ConstNotionalCrossCurrencySwapRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<ConstNotionalCrossCurrencySwapRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }

}
