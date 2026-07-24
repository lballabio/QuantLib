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

/*! \file fxresetcashflows.hpp
    \brief Cash flows of an FX-resetting (mark-to-market) swap leg
*/

#ifndef quantlib_fx_reset_cashflows_hpp
#define quantlib_fx_reset_cashflows_hpp

#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/currency.hpp>
#include <ql/handle.hpp>

namespace QuantLib {

class Quote;
class YieldTermStructure;

// The FX rate in units of resettable-leg currency per unit of constant-leg currency
class FxResetSource {
  public:
    FxResetSource(Currency constantLegCurrency,
                  Currency resettableLegCurrency,
                  Handle<YieldTermStructure> constantLegCurve,
                  Handle<YieldTermStructure> resettableLegCurve,
                  Handle<Quote> spotFx,
                  bool spotIsResettablePerConstant,
                  Date spotFxSettleDate = Date());

    Real fxRate(const Date& resetDate) const;

    const Currency& constantLegCurrency() const { return constantLegCurrency_; }
    const Currency& resettableLegCurrency() const { return resettableLegCurrency_; }

  private:
    Currency constantLegCurrency_;
    Currency resettableLegCurrency_;
    Handle<YieldTermStructure> constantLegCurve_;
    Handle<YieldTermStructure> resettableLegCurve_;
    Handle<Quote> spotFx_;
    bool spotIsResettablePerConstant_;
    Date spotFxSettleDate_;
};


// Floating-rate coupon of an FX-resetting leg
class FxResetCoupon : public FloatingRateCoupon {
  public:
    FxResetCoupon(const ext::shared_ptr<FloatingRateCoupon>& underlying,
                  Real constantLegNotional);

    Real nominal() const override;
    Rate rate() const override { return underlying_->rate(); }
    // the underlying coupon's amounts, rescaled to the FX-reset notional;
    // delegating preserves the underlying's accrual logic, e.g. the
    // partial-period compounding of an overnight-indexed coupon
    Real amount() const override { return underlying_->amount() * nominalScale(); }
    Real accruedAmount(const Date& d) const override {
        return underlying_->accruedAmount(d) * nominalScale();
    }
    Date fixingDate() const override { return underlying_->fixingDate(); }
    Rate indexFixing() const override { return underlying_->indexFixing(); }
    // forwarded to the underlying coupon, whose pricer drives the paid rate
    void setPricer(const ext::shared_ptr<FloatingRateCouponPricer>& pricer) override {
        underlying_->setPricer(pricer);
        FloatingRateCoupon::setPricer(pricer);
    }
    const ext::shared_ptr<FloatingRateCoupon>& underlying() const { return underlying_; }
    Real constantLegNotional() const { return constantLegNotional_; }
    //FX observation date for the notional reset: start of the accrual period
    Date fxResetDate() const { return accrualStartDate(); }
    const ext::shared_ptr<FxResetSource>& fxResetSource() const { return fxResetSource_; }
    void setFxResetSource(const ext::shared_ptr<FxResetSource>& source) {
        fxResetSource_ = source;
    }
    void performCalculations() const override { rate_ = underlying_->rate(); }
    void accept(AcyclicVisitor&) override;

  private:
    Real nominalScale() const { return nominal() / underlying_->nominal(); }

    ext::shared_ptr<FloatingRateCoupon> underlying_;
    Real constantLegNotional_;
    ext::shared_ptr<FxResetSource> fxResetSource_;
};


// Netted notional exchange of an FX-resetting leg
class FxResetNotionalExchange : public CashFlow {
  public:
    FxResetNotionalExchange(const Date& paymentDate,
                            Real constantLegNotional,
                            const Date& previousResetDate,
                            const Date& currentResetDate);
    Date date() const override { return paymentDate_; }
    Real amount() const override;
    Real constantLegNotional() const { return constantLegNotional_; }
    const Date& previousResetDate() const { return previousResetDate_; }
    const Date& currentResetDate() const { return currentResetDate_; }
    const ext::shared_ptr<FxResetSource>& fxResetSource() const { return fxResetSource_; }
    void setFxResetSource(const ext::shared_ptr<FxResetSource>& source) {
        fxResetSource_ = source;
    }
    void accept(AcyclicVisitor&) override;

  private:
    Date paymentDate_;
    Real constantLegNotional_;
    Date previousResetDate_;
    Date currentResetDate_;
    ext::shared_ptr<FxResetSource> fxResetSource_;
};

}

#endif
