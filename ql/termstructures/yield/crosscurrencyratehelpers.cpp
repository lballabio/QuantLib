/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021, Marcin Rybacki

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
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/termstructures/yield/crosscurrencyratehelpers.hpp>
#include <ql/utilities/null_deleter.hpp>

namespace QuantLib {

    CrossCurrencyBasisSwapRateHelper::CrossCurrencyBasisSwapRateHelper(
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
    : RelativeDateRateHelper(basis), tenor_(tenor), fixingDays_(fixingDays), calendar_(calendar),
        convention_(convention), endOfMonth_(endOfMonth), baseCcyIdx_(baseCurrencyIndex), 
        quoteCcyIdx_(quoteCurrencyIndex), collateralHandle_(collateralCurve),
        isFxBaseCurrencyCollateralCurrency_(isFxBaseCurrencyCollateralCurrency),
        isBasisOnFxBaseCurrencyLeg_(isBasisOnFxBaseCurrencyLeg) {
        registerWith(baseCcyIdx_);
        registerWith(quoteCcyIdx_);
        registerWith(collateralHandle_);

        initializeDates();
    }

    ext::shared_ptr<Swap> CrossCurrencyBasisSwapRateHelper::proxyCrossCurrencyLeg(
        const Date& evaluationDate,
        const Period& tenor,
        Natural fixingDays,
        const Calendar& calendar,
        BusinessDayConvention convention,
        bool endOfMonth,
        const ext::shared_ptr<IborIndex>& idx,
        VanillaSwap::Type type,
        Real notional,
        Spread basis) {
        bool isPayer = (type == VanillaSwap::Payer);
        Date referenceDate = calendar.adjust(evaluationDate);
        Date earliestDate = calendar.advance(referenceDate, fixingDays * Days, convention);
        Date maturity = earliestDate + tenor;

        Schedule schedule = MakeSchedule().from(earliestDate)
                                          .to(maturity)
                                          .withTenor(idx->tenor())
                                          .withCalendar(calendar)
                                          .withConvention(convention)
                                          .endOfMonth(endOfMonth)
                                          .backwards();

        Leg leg = IborLeg(schedule, idx).withNotionals(notional).withSpreads(basis);
        Date lastPaymentDate = leg.back()->date();
        leg.push_back(ext::make_shared<SimpleCashFlow>(notional, lastPaymentDate));

        return ext::make_shared<Swap>(std::vector<Leg>{leg}, std::vector<bool>{isPayer});
    }

    void CrossCurrencyBasisSwapRateHelper::initializeDates() {
        baseCcyLeg_ = CrossCurrencyBasisSwapRateHelper::proxyCrossCurrencyLeg(
            evaluationDate_, tenor_, fixingDays_, calendar_, convention_, endOfMonth_, baseCcyIdx_, VanillaSwap::Receiver);
        quoteCcyLeg_ = CrossCurrencyBasisSwapRateHelper::proxyCrossCurrencyLeg(
            evaluationDate_, tenor_, fixingDays_, calendar_, convention_, endOfMonth_, quoteCcyIdx_, VanillaSwap::Payer);

        earliestDate_ = std::min(baseCcyLeg_->startDate(), quoteCcyLeg_->startDate());
        latestDate_ = std::max(baseCcyLeg_->maturityDate(), quoteCcyLeg_->maturityDate());

        ext::shared_ptr<PricingEngine> otherCcyEngine(
            new DiscountingSwapEngine(termStructureHandle_));
        ext::shared_ptr<PricingEngine> collateralCcyEngine(
            new DiscountingSwapEngine(collateralHandle_));

        if (isFxBaseCurrencyCollateralCurrency_) {
            baseCcyLeg_->setPricingEngine(collateralCcyEngine);
            quoteCcyLeg_->setPricingEngine(otherCcyEngine);
        } else {
            baseCcyLeg_->setPricingEngine(otherCcyEngine);
            quoteCcyLeg_->setPricingEngine(collateralCcyEngine);
        }
    }

    Real CrossCurrencyBasisSwapRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        QL_REQUIRE(!collateralHandle_.empty(), "collateral term structure not set");

        baseCcyLeg_->recalculate();
        Real npvBaseCcy = baseCcyLeg_->NPV();

        quoteCcyLeg_->recalculate();
        Real npvQuoteCcy = quoteCcyLeg_->NPV();

        const Spread basisPoint = 1.0e-4;
        Real bps = isBasisOnFxBaseCurrencyLeg_ ? baseCcyLeg_->legBPS(0) : quoteCcyLeg_->legBPS(0);

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
        Visitor<CrossCurrencyBasisSwapRateHelper>* v1 = dynamic_cast<Visitor<CrossCurrencyBasisSwapRateHelper>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }
}