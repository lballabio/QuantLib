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
#include <ql/pricingengines/swap/discountingswapengine.hpp>
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

        Leg buildConstNotionalLegProxy(const Schedule& schedule,
                                       const ext::shared_ptr<IborIndex>& idx,
                                       Real notional = 1.0) {
            Leg leg = IborLeg(schedule, idx).withNotionals(notional);
            Date lastPaymentDate = leg.back()->date();
            leg.push_back(ext::make_shared<SimpleCashFlow>(notional, lastPaymentDate));
            return leg;
        }

        bool includeSettlementDateFlows() {
            return Settings::instance().includeReferenceDateEvents();
        }

        Real npvXccyLeg(const Leg& leg, const Handle<YieldTermStructure>& discountCurve) {
            Date refDate = discountCurve->referenceDate();
            const YieldTermStructure& discount_ref = **discountCurve;
            return CashFlows::npv(leg, discount_ref, includeSettlementDateFlows(), refDate);
        }

        Real bpsXccyLeg(const Leg& leg, const Handle<YieldTermStructure>& discountCurve) {
            Date refDate = discountCurve->referenceDate();
            const YieldTermStructure& discount_ref = **discountCurve;
            return CashFlows::bps(leg, discount_ref, includeSettlementDateFlows(), refDate);
        }

        class MtMLegCalculator : public AcyclicVisitor, public Visitor<Coupon> {
          public:
            explicit MtMLegCalculator(const YieldTermStructure& quoteCcyCurve,
                                      const YieldTermStructure& baseCcyCurve,
                                      bool isFxBaseCurrencyLegResettable)
            : quoteCcyCurve_(quoteCcyCurve), baseCcyCurve_(baseCcyCurve),
              isFxBaseCurrencyLegResettable_(isFxBaseCurrencyLegResettable),
              bps_(0.0), npv_(0.0) {}
            void visit(Coupon& c) override {
                Date start = c.accrualStartDate();
                Date end = c.accrualEndDate();
                Time accrual = c.accrualPeriod();

                Real npv = adjustedNotional(start) *
                           (-discount(start) + discount(end) * (1.0 + c.rate() * accrual));
                npv_ += npv;

                Real bps = adjustedNotional(start) * discount(end) * accrual;
                bps_ += bps;
            }
            Real bps() const { return bps_; }
            Real NPV() const { return npv_; }

          private:
            Real discount(const Date& d) const {
                if (isFxBaseCurrencyLegResettable_)
                    return baseCcyCurve_.discount(d);
                return quoteCcyCurve_.discount(d);
            }
            Real adjustedNotional(const Date& d) const {
                if (isFxBaseCurrencyLegResettable_)
                    return baseCcyCurve_.discount(d) / quoteCcyCurve_.discount(d);
                return quoteCcyCurve_.discount(d) / baseCcyCurve_.discount(d);
            }

            const YieldTermStructure& quoteCcyCurve_;
            const YieldTermStructure& baseCcyCurve_;
            bool isFxBaseCurrencyLegResettable_;
            Real bps_, npv_;
        };
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
    : RelativeDateRateHelper(basis),
      baseCcyLegSchedule_(legSchedule(evaluationDate_,
                                      tenor,
                                      baseCurrencyIndex->tenor(),
                                      fixingDays,
                                      calendar,
                                      convention,
                                      endOfMonth)),
      quoteCcyLegSchedule_(legSchedule(evaluationDate_,
                                       tenor,
                                       quoteCurrencyIndex->tenor(),
                                       fixingDays,
                                       calendar,
                                       convention,
                                       endOfMonth)),
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
        baseCcyLegProxy_ = buildConstNotionalLegProxy(baseCcyLegSchedule_, baseCcyIdx_);
        quoteCcyLegProxy_ = buildConstNotionalLegProxy(quoteCcyLegSchedule_, quoteCcyIdx_);
        earliestDate_ = std::min(CashFlows::startDate(baseCcyLegProxy_),
                                 CashFlows::startDate(quoteCcyLegProxy_));
        latestDate_ = std::max(CashFlows::maturityDate(baseCcyLegProxy_),
                               CashFlows::maturityDate(quoteCcyLegProxy_));
    }

    const Handle<YieldTermStructure>&
    CrossCurrencyBasisSwapRateHelper::baseCcyLegDiscountHandle() const {
        return isFxBaseCurrencyCollateralCurrency_ ? collateralHandle_ : termStructureHandle_;
    }

    const Handle<YieldTermStructure>&
    CrossCurrencyBasisSwapRateHelper::quoteCcyLegDiscountHandle() const {
        return isFxBaseCurrencyCollateralCurrency_ ? termStructureHandle_ : collateralHandle_;
    }

    Real CrossCurrencyBasisSwapRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != nullptr, "term structure not set");
        QL_REQUIRE(!collateralHandle_.empty(), "collateral term structure not set");

        Real npvBaseCcy = -npvXccyLeg(baseCcyLegProxy_, baseCcyLegDiscountHandle());
        Real npvQuoteCcy = npvXccyLeg(quoteCcyLegProxy_, quoteCcyLegDiscountHandle());
        const Spread basisPoint = 1.0e-4;
        Real bps = 0.0;
        if (isBasisOnFxBaseCurrencyLeg_)
            bps = -bpsXccyLeg(baseCcyLegProxy_, baseCcyLegDiscountHandle());
        else
            bps = bpsXccyLeg(quoteCcyLegProxy_, quoteCcyLegDiscountHandle());
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
}
