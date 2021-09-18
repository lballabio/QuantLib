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

        Real npvConstNotionalLeg(const Leg& iborLeg,
                                 const Handle<YieldTermStructure>& discountCurveHandle) {
            Date refDate = discountCurveHandle->referenceDate();
            const YieldTermStructure& discountRef = **discountCurveHandle;
            bool includeSettleDateFlows = true;
            return CashFlows::npv(iborLeg, discountRef, includeSettleDateFlows, refDate) +
                   discountRef.discount(iborLeg.back()->date()) - 1.0;
        }

        Real bpsConstNotionalLeg(const Leg& iborLeg,
                                 const Handle<YieldTermStructure>& discountCurveHandle) {
            const Spread basisPoint = 1.0e-4;
            Date refDate = discountCurveHandle->referenceDate();
            const YieldTermStructure& discountRef = **discountCurveHandle;
            bool includeSettleDateFlows = true;
            return CashFlows::bps(iborLeg, discountRef, includeSettleDateFlows, refDate) /
                   basisPoint;
        }

        class ResettingLegHelper {
          public:
            explicit ResettingLegHelper(const YieldTermStructure& discountCurve,
                                        const YieldTermStructure& foreignCurve)
            : discountCurve_(discountCurve), foreignCurve_(foreignCurve) {}
            Real discount(const Date& d) const {
                return discountCurve_.discount(d);
            }
            Real notionalAdjustment(const Date& d) const {
                return foreignCurve_.discount(d) / discountCurve_.discount(d);
            }

          private:
            const YieldTermStructure& discountCurve_;
            const YieldTermStructure& foreignCurve_;
        };

        class ResettingLegNPVCalculator : public AcyclicVisitor, public Visitor<Coupon> {
          public:
            explicit ResettingLegNPVCalculator(const YieldTermStructure& discountCurve,
                                               const YieldTermStructure& foreignCurve)
            : helper_(discountCurve, foreignCurve), npv_(0.0) {}
            void visit(Coupon& c) override {
                Date start = c.accrualStartDate();
                Date end = c.accrualEndDate();
                Real adjustedNotional = c.nominal() * helper_.notionalAdjustment(start);

                // NPV of a resetting coupon consists of a redemption of borrowed amount occuring
                // at the end of the accrual period plus the accrued interest, minus the borrowed
                // amount at the start of the period. All amounts are corrected by an adjustment
                // corresponding to the implied forward exchange rate, which is estimated by
                // the ratio of foreign and domestic curves discount factors.
                Real npvRedeemedAmount =
                    adjustedNotional * helper_.discount(end) * (1.0 + c.rate() * c.accrualPeriod());
                Real npvBorrowedAmount = -adjustedNotional * helper_.discount(start);
                npv_ += npvRedeemedAmount + npvBorrowedAmount;
            }
            Real NPV() const { return npv_; }

          private:
            ResettingLegHelper helper_;
            Real npv_;
        };

        class ResettingLegBPSCalculator : public AcyclicVisitor, public Visitor<Coupon> {
          public:
            explicit ResettingLegBPSCalculator(const YieldTermStructure& discountCurve,
                                               const YieldTermStructure& foreignCurve)
            : helper_(discountCurve, foreignCurve), bps_(0.0) {}
            void visit(Coupon& c) override {
                Date start = c.accrualStartDate();
                Date end = c.accrualEndDate();
                Time accrual = c.accrualPeriod();
                Real adjustedNotional = c.nominal() * helper_.notionalAdjustment(start);
                bps_ += adjustedNotional * helper_.discount(end) * accrual;
            }
            Real BPS() const { return bps_; }

          private:
            ResettingLegHelper helper_;
            Real bps_;
        };

        Real npvResettingLeg(const Leg& iborLeg,
                             const Handle<YieldTermStructure>& discountCurveHandle,
                             const Handle<YieldTermStructure>& foreignCurveHandle) {
            const YieldTermStructure& discountCurveRef = **discountCurveHandle;
            const YieldTermStructure& foreignCurveRef = **foreignCurveHandle;

            ResettingLegNPVCalculator calc(discountCurveRef, foreignCurveRef);
            for (const auto& i : iborLeg) {
                CashFlow& cf = *i;
                cf.accept(calc);
            }
            return calc.NPV();
        }

        Real bpsResettingLeg(const Leg& iborLeg,
                             const Handle<YieldTermStructure>& discountCurveHandle,
                             const Handle<YieldTermStructure>& foreignCurveHandle) {
            const YieldTermStructure& discountCurveRef = **discountCurveHandle;
            const YieldTermStructure& foreignCurveRef = **foreignCurveHandle;

            ResettingLegBPSCalculator calc(discountCurveRef, foreignCurveRef);
            for (const auto& i : iborLeg) {
                CashFlow& cf = *i;
                cf.accept(calc);
            }
            return calc.BPS();
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
        Real npvBaseCcy = -npvConstNotionalLeg(baseCcyIborLeg_, baseCcyLegDiscountHandle());
        Real npvQuoteCcy = npvConstNotionalLeg(quoteCcyIborLeg_, quoteCcyLegDiscountHandle());
        Real bps = 0.0;
        if (isBasisOnFxBaseCurrencyLeg_)
            bps = -bpsConstNotionalLeg(baseCcyIborLeg_, baseCcyLegDiscountHandle());
        else
            bps = bpsConstNotionalLeg(quoteCcyIborLeg_, quoteCcyLegDiscountHandle());
        return -(npvQuoteCcy + npvBaseCcy) / bps;
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
        Real npvBaseCcy = isFxBaseCurrencyLegResettable_ ?
                              -npvResettingLeg(baseCcyIborLeg_, baseCcyLegDiscountHandle(),
                                               quoteCcyLegDiscountHandle()) :
                              -npvConstNotionalLeg(baseCcyIborLeg_, baseCcyLegDiscountHandle());
        Real npvQuoteCcy = isFxBaseCurrencyLegResettable_ ?
                               npvConstNotionalLeg(quoteCcyIborLeg_, quoteCcyLegDiscountHandle()) :
                               npvResettingLeg(quoteCcyIborLeg_, quoteCcyLegDiscountHandle(),
                                               baseCcyLegDiscountHandle());
        Real bps = 0.0;
        if (isBasisOnFxBaseCurrencyLeg_)
            bps = isFxBaseCurrencyLegResettable_ ?
                      -bpsResettingLeg(baseCcyIborLeg_, baseCcyLegDiscountHandle(),
                                       quoteCcyLegDiscountHandle()) :
                      -bpsConstNotionalLeg(baseCcyIborLeg_, baseCcyLegDiscountHandle());
        else
            bps = isFxBaseCurrencyLegResettable_ ?
                      bpsConstNotionalLeg(quoteCcyIborLeg_, quoteCcyLegDiscountHandle()) :
                      bpsResettingLeg(quoteCcyIborLeg_, quoteCcyLegDiscountHandle(),
                                      baseCcyLegDiscountHandle());
        return -(npvQuoteCcy + npvBaseCcy) / bps;
    }

    void MtMCrossCurrencyBasisSwapRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<MtMCrossCurrencyBasisSwapRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }
}
