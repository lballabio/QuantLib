/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Marcin Rybacki

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

#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/experimental/termstructures/crosscurrencyratehelpers.hpp>
#include <ql/utilities/null_deleter.hpp>
#include <utility>

namespace QuantLib {

    namespace {

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

        Leg buildIborLeg(const Date& evaluationDate,
                         const Period& tenor,
                         Natural fixingDays,
                         const Calendar& calendar,
                         BusinessDayConvention convention,
                         bool endOfMonth,
                         const ext::shared_ptr<IborIndex>& idx) {
            Schedule sch = legSchedule(evaluationDate, tenor, idx->tenor(), fixingDays, calendar,
                                       convention, endOfMonth);
            return IborLeg(sch, idx).withNotionals(1.0);
        }

        void npvbpsConstNotionalLeg(const Leg& iborLeg,
                                    const Handle<YieldTermStructure>& discountCurveHandle,
                                    Real& npv,
                                    Real& bps) {
            const Spread basisPoint = 1.0e-4;
            Date refDt = discountCurveHandle->referenceDate();
            const YieldTermStructure& discountRef = **discountCurveHandle;
            bool includeSettleDtFlows = true;
            CashFlows::npvbps(iborLeg, discountRef, includeSettleDtFlows, refDt, refDt, npv, bps);
            // Include NPV of the notional exchange at start and maturity.
            npv += discountRef.discount(iborLeg.back()->date()) - 1.0;
            bps /= basisPoint;
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
                                            const YieldTermStructure& foreignCurve)
            : helper_(discountCurve, foreignCurve), npv_(0.0), bps_(0.0) {}
            void visit(Coupon& c) override {
                Date start = c.accrualStartDate();
                Date end = c.accrualEndDate();
                Time accrual = c.accrualPeriod();
                Real adjustedNotional = c.nominal() * helper_.notionalAdjustment(start);
                DiscountFactor discountStart = helper_.discount(start);
                DiscountFactor discountEnd = helper_.discount(end);

                // NPV of a resetting coupon consists of a redemption of borrowed amount occuring
                // at the end of the accrual period plus the accrued interest, minus the borrowed
                // amount at the start of the period. All amounts are corrected by an adjustment
                // corresponding to the implied forward exchange rate, which is estimated by
                // the ratio of foreign and domestic curves discount factors.
                Real npvRedeemedAmount =
                    adjustedNotional * discountEnd * (1.0 + c.rate() * accrual);
                Real npvBorrowedAmount = -adjustedNotional * discountStart;

                npv_ += npvRedeemedAmount + npvBorrowedAmount;
                bps_ += adjustedNotional * discountEnd * accrual;
            }
            Real NPV() const { return npv_; }
            Real BPS() const { return bps_; }

          private:
            ResettingLegHelper helper_;
            Real npv_;
            Real bps_;
        };

        void npvbpsResettingLeg(const Leg& iborLeg,
                                const Handle<YieldTermStructure>& discountCurveHandle,
                                const Handle<YieldTermStructure>& foreignCurveHandle,
                                Real& npv,
                                Real& bps) {
            const YieldTermStructure& discountCurveRef = **discountCurveHandle;
            const YieldTermStructure& foreignCurveRef = **foreignCurveHandle;

            ResettingLegCalculator calc(discountCurveRef, foreignCurveRef);
            for (const auto& i : iborLeg) {
                CashFlow& cf = *i;
                cf.accept(calc);
            }
            npv = calc.NPV();
            bps = calc.BPS();
        }
    }

    CrossCurrencyBasisSwapRateHelper::CrossCurrencyBasisSwapRateHelper(
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
        bool isBasisOnFxBaseCurrencyLeg)
    : RelativeDateRateHelper(basis), tenor_(tenor), fixingDays_(fixingDays),
      calendar_(std::move(calendar)), convention_(convention), endOfMonth_(endOfMonth),
      baseCcyIdx_(std::move(baseCurrencyIndex)), quoteCcyIdx_(std::move(quoteCurrencyIndex)),
      collateralHandle_(std::move(collateralCurve)),
      isFxBaseCurrencyCollateralCurrency_(isFxBaseCurrencyCollateralCurrency),
      isBasisOnFxBaseCurrencyLeg_(isBasisOnFxBaseCurrencyLeg) {
        registerWith(baseCcyIdx_);
        registerWith(quoteCcyIdx_);
        registerWith(collateralHandle_);

        initializeDates();
    }

    void CrossCurrencyBasisSwapRateHelper::initializeDates() {
        baseCcyIborLeg_ = buildIborLeg(evaluationDate_, tenor_, fixingDays_, calendar_, convention_,
                                       endOfMonth_, baseCcyIdx_);
        quoteCcyIborLeg_ = buildIborLeg(evaluationDate_, tenor_, fixingDays_, calendar_,
                                        convention_, endOfMonth_, quoteCcyIdx_);
        earliestDate_ =
            std::min(CashFlows::startDate(baseCcyIborLeg_), CashFlows::startDate(quoteCcyIborLeg_));
        latestDate_ = std::max(CashFlows::maturityDate(baseCcyIborLeg_),
                               CashFlows::maturityDate(quoteCcyIborLeg_));
    }

    const Handle<YieldTermStructure>&
    CrossCurrencyBasisSwapRateHelper::baseCcyLegDiscountHandle() const {
        QL_REQUIRE(!termStructureHandle_.empty(), "term structure not set");
        QL_REQUIRE(!collateralHandle_.empty(), "collateral term structure not set");
        return isFxBaseCurrencyCollateralCurrency_ ? collateralHandle_ : termStructureHandle_;
    }

    const Handle<YieldTermStructure>&
    CrossCurrencyBasisSwapRateHelper::quoteCcyLegDiscountHandle() const {
        QL_REQUIRE(!termStructureHandle_.empty(), "term structure not set");
        QL_REQUIRE(!collateralHandle_.empty(), "collateral term structure not set");
        return isFxBaseCurrencyCollateralCurrency_ ? termStructureHandle_ : collateralHandle_;
    }

    void CrossCurrencyBasisSwapRateHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        bool observer = false;

        ext::shared_ptr<YieldTermStructure> temp(t, null_deleter());
        termStructureHandle_.linkTo(temp, observer);

        RelativeDateRateHelper::setTermStructure(t);
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
        bool isBasisOnFxBaseCurrencyLeg)
    : CrossCurrencyBasisSwapRateHelper(basis,
                                       tenor,
                                       fixingDays,
                                       calendar,
                                       convention,
                                       endOfMonth,
                                       baseCurrencyIndex,
                                       quoteCurrencyIndex,
                                       collateralCurve,
                                       isFxBaseCurrencyCollateralCurrency,
                                       isBasisOnFxBaseCurrencyLeg) {}

    Real ConstNotionalCrossCurrencyBasisSwapRateHelper::impliedQuote() const {
        Real npvBaseCcy = 0.0, bpsBaseCcy = 0.0;
        npvbpsConstNotionalLeg(baseCcyIborLeg_, baseCcyLegDiscountHandle(), npvBaseCcy, bpsBaseCcy);
        Real npvQuoteCcy = 0.0, bpsQuoteCcy = 0.0;
        npvbpsConstNotionalLeg(quoteCcyIborLeg_, quoteCcyLegDiscountHandle(), npvQuoteCcy,
                               bpsQuoteCcy);
        Real bps = isBasisOnFxBaseCurrencyLeg_ ? -bpsBaseCcy : bpsQuoteCcy;
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
        bool isFxBaseCurrencyLegResettable)
    : CrossCurrencyBasisSwapRateHelper(basis,
                                       tenor,
                                       fixingDays,
                                       calendar,
                                       convention,
                                       endOfMonth,
                                       baseCurrencyIndex,
                                       quoteCurrencyIndex,
                                       collateralCurve,
                                       isFxBaseCurrencyCollateralCurrency,
                                       isBasisOnFxBaseCurrencyLeg),
      isFxBaseCurrencyLegResettable_(isFxBaseCurrencyLegResettable) {}

    Real MtMCrossCurrencyBasisSwapRateHelper::impliedQuote() const {
        Real npvBaseCcy = 0.0, bpsBaseCcy = 0.0;
        Real npvQuoteCcy = 0.0, bpsQuoteCcy = 0.0;
        if (isFxBaseCurrencyLegResettable_) {
            npvbpsResettingLeg(baseCcyIborLeg_, baseCcyLegDiscountHandle(),
                               quoteCcyLegDiscountHandle(), npvBaseCcy, bpsBaseCcy);
            npvbpsConstNotionalLeg(quoteCcyIborLeg_, quoteCcyLegDiscountHandle(), npvQuoteCcy,
                                   bpsQuoteCcy);
        } else {
            npvbpsConstNotionalLeg(baseCcyIborLeg_, baseCcyLegDiscountHandle(), npvBaseCcy,
                                   bpsBaseCcy);
            npvbpsResettingLeg(quoteCcyIborLeg_, quoteCcyLegDiscountHandle(),
                               baseCcyLegDiscountHandle(), npvQuoteCcy, bpsQuoteCcy);
        }

        Real bps = isBasisOnFxBaseCurrencyLeg_ ? -bpsBaseCcy : bpsQuoteCcy;

        return -(npvQuoteCcy - npvBaseCcy) / bps;
    }

    void MtMCrossCurrencyBasisSwapRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<MtMCrossCurrencyBasisSwapRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }
}
