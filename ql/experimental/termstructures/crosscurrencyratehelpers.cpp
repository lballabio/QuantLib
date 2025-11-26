/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Marcin Rybacki
 Copyright (C) 2025 Uzair Beg

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
#include <ql/experimental/termstructures/crosscurrencyratehelpers.hpp>
#include <ql/utilities/null_deleter.hpp>
#include <utility>

namespace QuantLib {

    namespace {

        constexpr double sample_fixed_rate = 0.01;

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

        Leg buildFloatingLeg(const Date& evaluationDate,
                         const Period& tenor,
                         Natural fixingDays,
                         const Calendar& calendar,
                         BusinessDayConvention convention,
                         bool endOfMonth,
                         const ext::shared_ptr<IborIndex>& idx,
                         Frequency paymentFrequency,
                         Integer paymentLag) {
            auto overnightIndex = ext::dynamic_pointer_cast<OvernightIndex>(idx);

            Period freqPeriod;
            if (paymentFrequency == NoFrequency) {
                QL_REQUIRE(!overnightIndex, "Require payment frequency for overnight indices.");
                freqPeriod = idx->tenor();
            } else {
                freqPeriod = Period(paymentFrequency);
            }

            Schedule sch = legSchedule(evaluationDate, tenor, freqPeriod, fixingDays, calendar,
                                       convention, endOfMonth);
            if (overnightIndex != nullptr) {
                return OvernightLeg(sch, overnightIndex)
                    .withNotionals(1.0)
                    .withPaymentLag(paymentLag);
            }
            return IborLeg(sch, idx).withNotionals(1.0).withPaymentLag(paymentLag);
        }

        Leg buildFixedLeg(const Date& evaluationDate,
                          const Period& tenor,
                          Natural fixingDays,
                          const Calendar& calendar,
                          BusinessDayConvention convention,
                          bool endOfMonth,
                          Frequency paymentFrequency,
                          const DayCounter& dayCount,
                          Integer paymentLag) {

            Period freqPeriod = Period(paymentFrequency);

            Schedule sch = legSchedule(evaluationDate, tenor, freqPeriod, fixingDays, calendar,
                                       convention, endOfMonth);
            return FixedRateLeg(sch)
                .withNotionals(1.0)
                .withCouponRates(sample_fixed_rate, dayCount)
                .withPaymentLag(paymentLag);
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

        class ResettingLegHelper {
          public:
            explicit ResettingLegHelper(const YieldTermStructure& discountCurve,
                                        const YieldTermStructure& foreignCurve)
            : discountCurve_(discountCurve), foreignCurve_(foreignCurve) {}
            DiscountFactor discount(const Date& d) const {
                return discountCurve_.discount(d);
            }
            Real notionalAdjustment(const Date& d) const {
                return foreignCurve_.discount(d) / discountCurve_.discount(d);
            }

          private:
            const YieldTermStructure& discountCurve_;
            const YieldTermStructure& foreignCurve_;
        };

        class ResettingLegCalculator : public AcyclicVisitor, public Visitor<Coupon> {
          public:
            explicit ResettingLegCalculator(const YieldTermStructure& discountCurve,
                                            const YieldTermStructure& foreignCurve,
                                            Integer paymentLag,
                                            Calendar paymentCalendar,
                                            BusinessDayConvention convention)
            : helper_(discountCurve, foreignCurve), paymentLag_(paymentLag),
              paymentCalendar_(std::move(paymentCalendar)), convention_(convention) {}

            void visit(Coupon& c) override {
                Date start = c.accrualStartDate();
                Date end = c.accrualEndDate();
                Time accrual = c.accrualPeriod();
                Real adjustedNotional = c.nominal() * helper_.notionalAdjustment(start);

                DiscountFactor discountStart, discountEnd;

                if (paymentLag_ == 0) {
                    discountStart = helper_.discount(start);
                    discountEnd = helper_.discount(end);
                } else {
                    Date paymentStart =
                        paymentCalendar_.advance(start, paymentLag_, Days, convention_);
                    Date paymentEnd = paymentCalendar_.advance(end, paymentLag_, Days, convention_);
                    discountStart = helper_.discount(paymentStart);
                    discountEnd = helper_.discount(paymentEnd);
                }

                // NPV of a resetting coupon consists of a redemption of borrowed amount occurring
                // at the end of the accrual period plus the accrued interest, minus the borrowed
                // amount at the start of the period. All amounts are corrected by an adjustment
                // corresponding to the implied forward exchange rate, which is estimated by
                // the ratio of foreign and domestic curves discount factors.
                Real npvRedeemedAmount =
                    adjustedNotional * discountEnd * (1.0 + c.rate() * accrual);
                Real npvBorrowedAmount = -adjustedNotional * discountStart;

                npv_ += (npvRedeemedAmount + npvBorrowedAmount);
                bps_ += adjustedNotional * discountEnd * accrual;
            }
            Real NPV() const { return npv_; }
            Real BPS() const { return bps_; }

          private:
            ResettingLegHelper helper_;
            Real npv_ = 0.0;
            Real bps_ = 0.0;
            Integer paymentLag_;
            Calendar paymentCalendar_;
            BusinessDayConvention convention_;
        };

        std::pair<Real, Real> npvbpsResettingLeg(const Leg& iborLeg,
                                                 Integer paymentLag,
                                                 const Calendar& paymentCalendar,
                                                 BusinessDayConvention convention,
                                                 const Handle<YieldTermStructure>& discountCurveHandle,
                                                 const Handle<YieldTermStructure>& foreignCurveHandle) {
            const YieldTermStructure& discountCurveRef = **discountCurveHandle;
            const YieldTermStructure& foreignCurveRef = **foreignCurveHandle;

            ResettingLegCalculator calc(discountCurveRef, foreignCurveRef, paymentLag,
                                        paymentCalendar, convention);
            for (const auto& i : iborLeg) {
                CashFlow& cf = *i;
                cf.accept(calc);
            }
            return { calc.NPV(), calc.BPS() };
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
        Integer paymentLag)
    : RelativeDateRateHelper(quote), tenor_(tenor), fixingDays_(fixingDays),
      calendar_(std::move(calendar)), convention_(convention), endOfMonth_(endOfMonth),
      paymentLag_(paymentLag), collateralHandle_(std::move(collateralCurve)) {
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

        if (paymentLag_ == 0) {
            initialNotionalExchangeDate_ = earliestDate_;
            finalNotionalExchangeDate_   = maturityDate_;
        } else {
            initialNotionalExchangeDate_ = calendar_.advance(earliestDate_, paymentLag_, Days, convention_);
            finalNotionalExchangeDate_   = calendar_.advance(maturityDate_, paymentLag_, Days, convention_);
        }

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
        Frequency paymentFrequency,
        Integer paymentLag)
    : CrossCurrencySwapRateHelperBase(basis, tenor, fixingDays, calendar, convention, endOfMonth,
                                      std::move(collateralCurve), paymentLag),
      baseCcyIdx_(std::move(baseCurrencyIndex)), quoteCcyIdx_(std::move(quoteCurrencyIndex)),
      isFxBaseCurrencyCollateralCurrency_(isFxBaseCurrencyCollateralCurrency),
      isBasisOnFxBaseCurrencyLeg_(isBasisOnFxBaseCurrencyLeg),
      paymentFrequency_(paymentFrequency) {
        registerWith(baseCcyIdx_);
        registerWith(quoteCcyIdx_);

        CrossCurrencyBasisSwapRateHelperBase::initializeDates();
    }

    void CrossCurrencyBasisSwapRateHelperBase::initializeDates() {
        baseCcyIborLeg_ = buildFloatingLeg(evaluationDate_, tenor_, fixingDays_, calendar_, convention_,
                                           endOfMonth_, baseCcyIdx_, paymentFrequency_, paymentLag_);

        quoteCcyIborLeg_ = buildFloatingLeg(evaluationDate_, tenor_, fixingDays_, calendar_,
                                            convention_, endOfMonth_, quoteCcyIdx_, paymentFrequency_, paymentLag_);

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
        Frequency paymentFrequency,
        Integer paymentLag)
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
                                           paymentLag) {}

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
        Frequency paymentFrequency,
        Integer paymentLag)
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
                                           paymentLag),
      isFxBaseCurrencyLegResettable_(isFxBaseCurrencyLegResettable) {}

    Real MtMCrossCurrencyBasisSwapRateHelper::impliedQuote() const {
        QL_REQUIRE(!termStructureHandle_.empty(), "term structure not set");
        QL_REQUIRE(!collateralHandle_.empty(), "collateral term structure not set");

        auto [npvBaseCcy, bpsBaseCcy] =
            isFxBaseCurrencyLegResettable_ ?
                npvbpsResettingLeg(baseCcyIborLeg_, paymentLag_, calendar_, convention_,
                                   baseCcyLegDiscountHandle(), quoteCcyLegDiscountHandle()) :
                npvbpsConstNotionalLeg(baseCcyIborLeg_, initialNotionalExchangeDate_,
                                       finalNotionalExchangeDate_, baseCcyLegDiscountHandle());

        auto [npvQuoteCcy, bpsQuoteCcy] =
            isFxBaseCurrencyLegResettable_ ?
                npvbpsConstNotionalLeg(quoteCcyIborLeg_, initialNotionalExchangeDate_,
                                       finalNotionalExchangeDate_, quoteCcyLegDiscountHandle()) :
                npvbpsResettingLeg(quoteCcyIborLeg_, paymentLag_, calendar_, convention_,
                                   quoteCcyLegDiscountHandle(), baseCcyLegDiscountHandle());

        Real bps = isBasisOnFxBaseCurrencyLeg_ ? -bpsBaseCcy : bpsQuoteCcy;

        QL_REQUIRE(std::fabs(bps) > 0.0, "null BPS");

        return -(npvQuoteCcy - npvBaseCcy) / bps;
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
        const DayCounter& fixedDayCount,
        const ext::shared_ptr<IborIndex>& floatIndex,
        const Handle<YieldTermStructure>& collateralCurve,
        bool collateralOnFixedLeg,
        Integer paymentLag)
    : CrossCurrencySwapRateHelperBase(fixedRate, tenor, fixingDays, calendar, convention, endOfMonth,
                                      std::move(collateralCurve), paymentLag),
      fixedFrequency_(fixedFrequency),
      fixedDayCount_(fixedDayCount),
      floatIndex_(floatIndex),
      collateralOnFixedLeg_(collateralOnFixedLeg) {

        QL_REQUIRE(floatIndex_, "floating index required");
        registerWith(floatIndex_);

        initializeDates();
    }

    void ConstNotionalCrossCurrencySwapRateHelper::initializeDates() {
        fixedLeg_ = buildFixedLeg(evaluationDate_, tenor_, fixingDays_, calendar_, convention_,
                                  endOfMonth_, fixedFrequency_, fixedDayCount_, paymentLag_);
        floatLeg_ = buildFloatingLeg(evaluationDate_, tenor_, fixingDays_, floatIndex_->fixingCalendar(),
                                     floatIndex_->businessDayConvention(), endOfMonth_,
                                     floatIndex_, floatIndex_->tenor().frequency(), paymentLag_);

        initializeDatesFromLegs(fixedLeg_, floatLeg_);
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

        auto [fixedNpv, fixedBps] = npvbpsConstNotionalLeg(
            fixedLeg_, initialNotionalExchangeDate_, finalNotionalExchangeDate_, fixedLegDiscountHandle());

        auto [floatNpv, floatBps] = npvbpsConstNotionalLeg(
            floatLeg_, initialNotionalExchangeDate_, finalNotionalExchangeDate_, floatingLegDiscountHandle());

        QL_REQUIRE(std::fabs(fixedBps) > 0.0, "null fixed-leg BPS");

        return sample_fixed_rate + (floatNpv - fixedNpv) / fixedBps;
    }

    void ConstNotionalCrossCurrencySwapRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<ConstNotionalCrossCurrencySwapRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }

}
