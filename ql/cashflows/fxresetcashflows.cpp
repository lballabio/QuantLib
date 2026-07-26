/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include <ql/cashflows/fxresetcashflows.hpp>
#include <ql/currencies/exchangeratemanager.hpp>
#include <ql/money.hpp>
#include <ql/quote.hpp>
#include <ql/settings.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>

namespace QuantLib {

FxReset::FxReset(Date fixingDate, Date valueDate)
: fixingDate_(fixingDate), valueDate_(valueDate) {
    QL_REQUIRE(fixingDate_ != Date(), "null FX reset fixing date");
    QL_REQUIRE(valueDate_ != Date(), "null FX reset value date");
    QL_REQUIRE(fixingDate_ <= valueDate_,
               "FX reset fixing date (" << fixingDate_
                                         << ") cannot be after its value date (" << valueDate_
                                         << ")");
}

FxResetConvention::FxResetConvention(Natural fixingDays, Calendar fixingCalendar)
: fixingDays_(fixingDays), fixingCalendar_(std::move(fixingCalendar)) {}

FxReset FxResetConvention::reset(const Date& valueDate) const {
    QL_REQUIRE(valueDate != Date(), "null FX reset value date");
    QL_REQUIRE(fixingDays_ == 0 || !fixingCalendar_.empty(),
               "an FX reset fixing calendar is required when fixing days are non-zero");
    Date fixingDate = fixingDays_ == 0 ? valueDate :
        fixingCalendar_.advance(valueDate, -static_cast<Integer>(fixingDays_), Days);
    return FxReset(fixingDate, valueDate);
}

Date FxResetConvention::valueDate(const Date& fixingDate) const {
    QL_REQUIRE(fixingDate != Date(), "null FX fixing date");
    QL_REQUIRE(fixingDays_ == 0 || !fixingCalendar_.empty(),
               "an FX reset fixing calendar is required when fixing days are non-zero");
    return fixingDays_ == 0 ? fixingDate :
        fixingCalendar_.advance(fixingDate, static_cast<Integer>(fixingDays_), Days);
}

DiscountingFxResetPricer::DiscountingFxResetPricer(
    Currency constantLegCurrency,
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
    registerWith(constantLegCurve_);
    registerWith(resettableLegCurve_);
    registerWith(spotFx_);
}

Real DiscountingFxResetPricer::fxRate(const FxReset& reset) const {
    Date referenceDate = resettableLegCurve_->referenceDate();
    Date today = Settings::instance().evaluationDate();

    if (reset.fixingDate() < today ||
        (reset.fixingDate() == today &&
         Settings::instance().enforcesTodaysHistoricFixings())) {
        return historicalRate(reset.fixingDate());
    }

    if (reset.fixingDate() == today) {
        try {
            return historicalRate(reset.fixingDate());
        } catch (Error&) {
            // A today's fixing may not have been published yet; forecast it.
        }
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

    DiscountFactor resettableResetDiscount = resettableLegCurve_->discount(reset.valueDate());
    QL_REQUIRE(resettableResetDiscount != 0.0,
               "Discount factor for currency " << resettableLegCurrency_ << " at "
                                               << reset.valueDate() << " cannot be zero");
    return referenceDateFx * constantLegCurve_->discount(reset.valueDate()) /
           resettableResetDiscount;
}

Real DiscountingFxResetPricer::historicalRate(const Date& fixingDate) const {
    ExchangeRate exchangeRate = ExchangeRateManager::instance().lookup(
        constantLegCurrency_, resettableLegCurrency_, fixingDate);
    Real fx = exchangeRate.exchange(Money(1.0, constantLegCurrency_)).value();
    QL_REQUIRE(fx > 0.0, "FX fixing from " << constantLegCurrency_ << " to "
                                           << resettableLegCurrency_ << " on " << fixingDate
                                           << " must be positive");
    return fx;
}


FxResetCoupon::FxResetCoupon(const ext::shared_ptr<FloatingRateCoupon>& underlying,
                             Real constantLegNotional,
                             FxReset fxReset)
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
  underlying_(underlying), constantLegNotional_(constantLegNotional),
  fxReset_(std::move(fxReset)) {
    QL_REQUIRE(underlying_->nominal() != 0.0,
               "underlying coupon nominal cannot be zero");
    registerWith(underlying_);
}

Real FxResetCoupon::nominal() const {
    QL_REQUIRE(fxResetPricer_,
               "no FX reset pricer set; the coupon nominal is available only "
               "after assigning an FX reset pricer");
    return constantLegNotional_ * fxResetPricer_->fxRate(fxReset_);
}

Rate FxResetCoupon::rate() const {
    calculate();
    return rate_;
}

Real FxResetCoupon::amount() const {
    calculate();
    return underlying_->amount() * nominalScale();
}

Real FxResetCoupon::accruedAmount(const Date& d) const {
    calculate();
    return underlying_->accruedAmount(d) * nominalScale();
}

void FxResetCoupon::setFxResetPricer(const ext::shared_ptr<FxResetPricer>& pricer) {
    if (fxResetPricer_)
        unregisterWith(fxResetPricer_);
    fxResetPricer_ = pricer;
    if (fxResetPricer_)
        registerWith(fxResetPricer_);
    update();
}

void FxResetCoupon::deepUpdate() {
    update();
    underlying_->deepUpdate();
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
                                                 std::optional<FxReset> previousReset,
                                                 std::optional<FxReset> currentReset)
: paymentDate_(paymentDate), constantLegNotional_(constantLegNotional),
  previousReset_(std::move(previousReset)), currentReset_(std::move(currentReset)) {
    QL_REQUIRE(paymentDate_ != Date(), "null payment date");
    QL_REQUIRE(previousReset_ || currentReset_,
               "at least one of the reset dates must be given");
}

Real FxResetNotionalExchange::amount() const {
    QL_REQUIRE(fxResetPricer_,
               "no FX reset pricer set; the exchange amount is available only "
               "after assigning an FX reset pricer");
    Real amount = 0.0;
    if (previousReset_)
        amount += constantLegNotional_ * fxResetPricer_->fxRate(*previousReset_);
    if (currentReset_)
        amount -= constantLegNotional_ * fxResetPricer_->fxRate(*currentReset_);
    return amount;
}

void FxResetNotionalExchange::setFxResetPricer(
    const ext::shared_ptr<FxResetPricer>& pricer) {
    if (fxResetPricer_)
        unregisterWith(fxResetPricer_);
    fxResetPricer_ = pricer;
    if (fxResetPricer_)
        registerWith(fxResetPricer_);
    update();
}

void FxResetNotionalExchange::accept(AcyclicVisitor& v) {
    auto* v1 = dynamic_cast<Visitor<FxResetNotionalExchange>*>(&v);
    if (v1 != nullptr)
        v1->visit(*this);
    else
        CashFlow::accept(v);
}

void setFxResetPricer(const Leg& leg, const ext::shared_ptr<FxResetPricer>& pricer) {
    for (const auto& cf : leg) {
        if (auto coupon = ext::dynamic_pointer_cast<FxResetCoupon>(cf))
            coupon->setFxResetPricer(pricer);
        else if (auto exchange = ext::dynamic_pointer_cast<FxResetNotionalExchange>(cf))
            exchange->setFxResetPricer(pricer);
    }
}

} // namespace QuantLib
