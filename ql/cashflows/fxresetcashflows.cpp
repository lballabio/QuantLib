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

#include <ql/cashflows/fxresetcashflows.hpp>
#include <ql/currencies/exchangeratemanager.hpp>
#include <ql/money.hpp>
#include <ql/quote.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>

namespace QuantLib {

FxResetSource::FxResetSource(Currency constantLegCurrency,
                             Currency resettableLegCurrency,
                             Handle<YieldTermStructure> constantLegCurve,
                             Handle<YieldTermStructure> resettableLegCurve,
                             Handle<Quote> spotFx,
                             bool spotIsResettablePerConstant,
                             Date spotFxSettleDate)
: constantLegCurrency_(std::move(constantLegCurrency)),
  resettableLegCurrency_(std::move(resettableLegCurrency)),
  constantLegCurve_(std::move(constantLegCurve)),
  resettableLegCurve_(std::move(resettableLegCurve)), spotFx_(std::move(spotFx)),
  spotIsResettablePerConstant_(spotIsResettablePerConstant),
  spotFxSettleDate_(spotFxSettleDate) {
    QL_REQUIRE(!constantLegCurve_.empty() && !resettableLegCurve_.empty(),
               "Discounting term structure handle is empty.");
    QL_REQUIRE(!spotFx_.empty(), "FX spot quote handle is empty.");
}

Real FxResetSource::fxRate(const Date& resetDate) const {
    Date referenceDate = resettableLegCurve_->referenceDate();

    if (resetDate < referenceDate) {
        ExchangeRate exchangeRate = ExchangeRateManager::instance().lookup(
            constantLegCurrency_, resettableLegCurrency_, resetDate);
        Real fx = exchangeRate.exchange(Money(1.0, constantLegCurrency_)).value();
        QL_REQUIRE(fx > 0.0, "FX fixing from " << constantLegCurrency_ << " to "
                                               << resettableLegCurrency_ << " on " << resetDate
                                               << " must be positive");
        return fx;
    }

    Real spot = spotFx_->value();
    if (!spotIsResettablePerConstant_) {
        QL_REQUIRE(spot != 0.0, "FX spot quote cannot be zero");
        spot = 1.0 / spot;
    }

    Date settleDate = spotFxSettleDate_ == Date() ? referenceDate : spotFxSettleDate_;
    DiscountFactor constantSettleDiscount = constantLegCurve_->discount(settleDate);
    QL_REQUIRE(constantSettleDiscount != 0.0,
               "Discount factor for currency " << constantLegCurrency_ << " at " << settleDate
                                               << " cannot be zero");
    Real referenceDateFx =
        spot * resettableLegCurve_->discount(settleDate) / constantSettleDiscount;

    DiscountFactor resettableResetDiscount = resettableLegCurve_->discount(resetDate);
    QL_REQUIRE(resettableResetDiscount != 0.0,
               "Discount factor for currency " << resettableLegCurrency_ << " at " << resetDate
                                               << " cannot be zero");
    return referenceDateFx * constantLegCurve_->discount(resetDate) / resettableResetDiscount;
}


FxResetCoupon::FxResetCoupon(const ext::shared_ptr<FloatingRateCoupon>& underlying,
                             Real constantLegNotional)
: FloatingRateCoupon(underlying->date(),
                     underlying->nominal(),
                     underlying->accrualStartDate(),
                     underlying->accrualEndDate(),
                     underlying->fixingDays(),
                     underlying->index(),
                     underlying->gearing(),
                     underlying->spread(),
                     underlying->referencePeriodStart(),
                     underlying->referencePeriodEnd(),
                     underlying->dayCounter(),
                     underlying->isInArrears(),
                     underlying->exCouponDate(),
                     underlying->fixingConvention()),
  underlying_(underlying), constantLegNotional_(constantLegNotional) {
    QL_REQUIRE(underlying_->nominal() != 0.0,
               "underlying coupon nominal cannot be zero");
    registerWith(underlying_);
}

Real FxResetCoupon::nominal() const {
    QL_REQUIRE(fxResetSource_,
               "no FX reset source set; the coupon nominal is available only "
               "after pricing the swap with an engine that handles FX-resetting legs");
    return constantLegNotional_ * fxResetSource_->fxRate(fxResetDate());
}

void FxResetCoupon::accept(AcyclicVisitor& v) {
    auto* v1 = dynamic_cast<Visitor<FxResetCoupon>*>(&v);
    if (v1 != nullptr)
        v1->visit(*this);
    else
        FloatingRateCoupon::accept(v);
}


FxResetNotionalExchange::FxResetNotionalExchange(const Date& paymentDate,
                                                 Real constantLegNotional,
                                                 const Date& previousResetDate,
                                                 const Date& currentResetDate)
: paymentDate_(paymentDate), constantLegNotional_(constantLegNotional),
  previousResetDate_(previousResetDate), currentResetDate_(currentResetDate) {
    QL_REQUIRE(paymentDate_ != Date(), "null payment date");
    QL_REQUIRE(previousResetDate_ != Date() || currentResetDate_ != Date(),
               "at least one of the reset dates must be given");
}

Real FxResetNotionalExchange::amount() const {
    QL_REQUIRE(fxResetSource_,
               "no FX reset source set; the exchange amount is available only "
               "after pricing the swap with an engine that handles FX-resetting legs");
    Real amount = 0.0;
    if (previousResetDate_ != Date())
        amount += constantLegNotional_ * fxResetSource_->fxRate(previousResetDate_);
    if (currentResetDate_ != Date())
        amount -= constantLegNotional_ * fxResetSource_->fxRate(currentResetDate_);
    return amount;
}

void FxResetNotionalExchange::accept(AcyclicVisitor& v) {
    auto* v1 = dynamic_cast<Visitor<FxResetNotionalExchange>*>(&v);
    if (v1 != nullptr)
        v1->visit(*this);
    else
        CashFlow::accept(v);
}

} // namespace QuantLib
