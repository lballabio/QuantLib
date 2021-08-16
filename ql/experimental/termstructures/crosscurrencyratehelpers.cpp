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

        bool includeSettlementDateFlows() {
            return Settings::instance().includeReferenceDateEvents();
        }

        Real npvConstNotionalLeg(const Leg& iborLeg,
                                 const Handle<YieldTermStructure>& discountCurveHandle) {
            Date refDate = discountCurveHandle->referenceDate();
            const YieldTermStructure& discountRef = **discountCurveHandle;
            return CashFlows::npv(iborLeg, discountRef, includeSettlementDateFlows(), refDate) +
                   discountRef.discount(iborLeg.back()->date());
        }

        Real bpsConstNotionalLeg(const Leg& iborLeg,
                                 const Handle<YieldTermStructure>& discountCurveHandle) {
            const Spread basisPoint = 1.0e-4;
            Date refDate = discountCurveHandle->referenceDate();
            const YieldTermStructure& discountRef = **discountCurveHandle;
            return CashFlows::bps(iborLeg, discountRef, includeSettlementDateFlows(), refDate) /
                   basisPoint;
        }

        class MtMLegHelper {
          public:
            explicit MtMLegHelper(const YieldTermStructure& baseCcyCurve,
                                  const YieldTermStructure& quoteCcyCurve,
                                  bool isForBaseCcyLeg)
            : baseCcyCurve_(baseCcyCurve), quoteCcyCurve_(quoteCcyCurve),
              isForBaseCcyLeg_(isForBaseCcyLeg) {}
            Real discount(const Date& d) const {
                if (isForBaseCcyLeg_)
                    return baseCcyCurve_.discount(d);
                return quoteCcyCurve_.discount(d);
            }
            Real adjustedNotional(const Date& d) const {
                if (isForBaseCcyLeg_)
                    return baseCcyCurve_.discount(d) / quoteCcyCurve_.discount(d);
                return quoteCcyCurve_.discount(d) / baseCcyCurve_.discount(d);
            }

          private:
            const YieldTermStructure& baseCcyCurve_;
            const YieldTermStructure& quoteCcyCurve_;
            bool isForBaseCcyLeg_;
        };

        class MtMLegNPVCalculator : public AcyclicVisitor, public Visitor<Coupon> {
          public:
            explicit MtMLegNPVCalculator(const YieldTermStructure& baseCcyCurve,
                                         const YieldTermStructure& quoteCcyCurve,
                                         bool isForBaseCcyLeg)
            : helper_(baseCcyCurve, quoteCcyCurve, isForBaseCcyLeg), npv_(0.0) {}
            void visit(Coupon& c) override {
                Date start = c.accrualStartDate();
                Date end = c.accrualEndDate();
                Time accrual = c.accrualPeriod();
                Real npv =
                    helper_.adjustedNotional(start) *
                    (-helper_.discount(start) + helper_.discount(end) * (1.0 + c.rate() * accrual));
                npv_ += npv;
            }
            Real NPV() const { return npv_; }

          private:
            MtMLegHelper helper_;
            Real npv_;
        };

        class MtMLegBPSCalculator : public AcyclicVisitor, public Visitor<Coupon> {
          public:
            explicit MtMLegBPSCalculator(const YieldTermStructure& baseCcyCurve,
                                         const YieldTermStructure& quoteCcyCurve,
                                         bool isForBaseCcyLeg)
            : helper_(baseCcyCurve, quoteCcyCurve, isForBaseCcyLeg), bps_(0.0) {}
            void visit(Coupon& c) override {
                Date start = c.accrualStartDate();
                Date end = c.accrualEndDate();
                Time accrual = c.accrualPeriod();
                Real bps = helper_.adjustedNotional(start) * helper_.discount(end) * accrual;
                bps_ += bps;
            }
            Real BPS() const { return bps_; }

          private:
            MtMLegHelper helper_;
            Real bps_;
        };

        Real npvResettingLeg(const Leg& iborLeg,
                             const Handle<YieldTermStructure>& baseCcyCurveHandle,
                             const Handle<YieldTermStructure>& quoteCcyCurveHandle,
                             bool isFxBaseCurrencyCollateralCurrency) {
            const YieldTermStructure& baseCcyCurveRef = **baseCcyCurveHandle;
            const YieldTermStructure& quoteCcyCurveRef = **quoteCcyCurveHandle;

            MtMLegNPVCalculator calc(baseCcyCurveRef, quoteCcyCurveRef,
                                     isFxBaseCurrencyCollateralCurrency);
            for (const auto& i : iborLeg) {
                CashFlow& cf = *i;
                cf.accept(calc);
            }
            return calc.NPV();
        }

        Real bpsResettingLeg(const Leg& iborLeg,
                             const Handle<YieldTermStructure>& baseCcyCurveHandle,
                             const Handle<YieldTermStructure>& quoteCcyCurveHandle,
                             bool isFxBaseCurrencyCollateralCurrency) {
            const YieldTermStructure& baseCcyCurveRef = **baseCcyCurveHandle;
            const YieldTermStructure& quoteCcyCurveRef = **quoteCcyCurveHandle;

            MtMLegBPSCalculator calc(baseCcyCurveRef, quoteCcyCurveRef,
                                     isFxBaseCurrencyCollateralCurrency);
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
    : RelativeDateRateHelper(basis), tenor_(tenor), fixingDays_(fixingDays), calendar_(calendar),
      convention_(convention), endOfMonth_(endOfMonth), baseCcyIdx_(std::move(baseCurrencyIndex)),
      quoteCcyIdx_(std::move(quoteCurrencyIndex)), collateralHandle_(std::move(collateralCurve)),
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

    Real CrossCurrencyBasisSwapRateHelper::impliedQuote() const {
        Real npvBaseCcy = -npvConstNotionalLeg(baseCcyIborLeg_, baseCcyLegDiscountHandle());
        Real npvQuoteCcy = npvConstNotionalLeg(quoteCcyIborLeg_, quoteCcyLegDiscountHandle());
        const Spread basisPoint = 1.0e-4;
        Real bps = 0.0;
        if (isBasisOnFxBaseCurrencyLeg_)
            bps = -bpsConstNotionalLeg(baseCcyIborLeg_, baseCcyLegDiscountHandle());
        else
            bps = bpsConstNotionalLeg(quoteCcyIborLeg_, quoteCcyLegDiscountHandle());
        return -(npvQuoteCcy + npvBaseCcy) / bps * basisPoint;
    }

    void CrossCurrencyBasisSwapRateHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        bool observer = false;

        ext::shared_ptr<YieldTermStructure> temp(t, null_deleter());
        termStructureHandle_.linkTo(temp, observer);

        RelativeDateRateHelper::setTermStructure(t);
    }

    void CrossCurrencyBasisSwapRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<CrossCurrencyBasisSwapRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }

    MtMCrossCurrencyBasisSwapRateHelper::MtMCrossCurrencyBasisSwapRateHelper(
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
                                               quoteCcyLegDiscountHandle(), true) :
                              -npvConstNotionalLeg(baseCcyIborLeg_, baseCcyLegDiscountHandle());
        Real npvQuoteCcy = isFxBaseCurrencyLegResettable_ ?
                               -npvConstNotionalLeg(quoteCcyIborLeg_, quoteCcyLegDiscountHandle()) :
                               -npvResettingLeg(quoteCcyIborLeg_, baseCcyLegDiscountHandle(),
                                                quoteCcyLegDiscountHandle(), false);
        Real bps = 0.0;
        if (isBasisOnFxBaseCurrencyLeg_)
            bps = isFxBaseCurrencyLegResettable_ ?
                      -bpsResettingLeg(baseCcyIborLeg_, baseCcyLegDiscountHandle(),
                                       quoteCcyLegDiscountHandle(), true) :
                      -bpsConstNotionalLeg(baseCcyIborLeg_, baseCcyLegDiscountHandle());
        else
            bps = isFxBaseCurrencyLegResettable_ ?
                      -bpsConstNotionalLeg(quoteCcyIborLeg_, quoteCcyLegDiscountHandle()) :
                      -bpsResettingLeg(quoteCcyIborLeg_, baseCcyLegDiscountHandle(),
                                       quoteCcyLegDiscountHandle(), false);
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
