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
#include <ql/time/calendar.hpp>
#include <optional>

namespace QuantLib {

class Quote;
class YieldTermStructure;

//! Contractual observation used to reset a cross-currency notional
/*! The fixing date is the date on which the FX rate is observed.  The value
    date is the settlement date of that FX observation and is normally the
    corresponding accrual-period boundary.
*/
class FxReset {
  public:
    FxReset(Date fixingDate, Date valueDate);

    const Date& fixingDate() const { return fixingDate_; }
    const Date& valueDate() const { return valueDate_; }

  private:
    Date fixingDate_;
    Date valueDate_;
};


//! Convention for creating FX-reset observations from accrual dates
/*! A non-empty calendar is required when the convention is used directly
    with a non-zero fixing lag.  Instruments may resolve an empty calendar to
    an appropriate schedule calendar.
*/
class FxResetConvention {
  public:
    explicit FxResetConvention(Natural fixingDays = 0,
                               Calendar fixingCalendar = Calendar());

    FxReset reset(const Date& valueDate) const;

    Natural fixingDays() const { return fixingDays_; }
    const Calendar& fixingCalendar() const { return fixingCalendar_; }

  private:
    Natural fixingDays_;
    Calendar fixingCalendar_;
};


//! Interface for obtaining the rate of an FX-reset observation
class FxResetPricer : public virtual Observer, public virtual Observable {
  public:
    virtual ~FxResetPricer() = default;
    virtual Real fxRate(const FxReset& reset) const = 0;
    void update() override { notifyObservers(); }
};


//! FX-reset pricer based on spot FX and two discount curves
/*! The returned FX rate is expressed in units of resettable-leg currency per
    unit of constant-leg currency.  Historical observations are obtained from
    ExchangeRateManager on the reset fixing date; future observations are
    projected to the reset value date.
*/
class DiscountingFxResetPricer : public FxResetPricer {
  public:
    DiscountingFxResetPricer(Currency constantLegCurrency,
                             Currency resettableLegCurrency,
                             Handle<YieldTermStructure> constantLegCurve,
                             Handle<YieldTermStructure> resettableLegCurve,
                             Handle<Quote> spotFx,
                             bool spotIsResettablePerConstant,
                             Date spotFxSettleDate = Date());

    Real fxRate(const FxReset& reset) const override;

    const Currency& constantLegCurrency() const { return constantLegCurrency_; }
    const Currency& resettableLegCurrency() const { return resettableLegCurrency_; }

  private:
    Real historicalRate(const Date& fixingDate) const;

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
                  Real constantLegNotional,
                  FxReset fxReset);

    Real nominal() const override;
    Rate rate() const override;
    // the underlying coupon's amounts, rescaled to the FX-reset notional;
    // delegating preserves the underlying's accrual logic, e.g. the
    // partial-period compounding of an overnight-indexed coupon
    Real amount() const override;
    Real accruedAmount(const Date& d) const override;
    Date fixingDate() const override { return underlying_->fixingDate(); }
    Rate indexFixing() const override { return underlying_->indexFixing(); }
    // forwarded to the underlying coupon, whose pricer drives the paid rate
    void setPricer(const ext::shared_ptr<FloatingRateCouponPricer>& pricer) override {
        underlying_->setPricer(pricer);
        FloatingRateCoupon::setPricer(pricer);
    }
    const ext::shared_ptr<FloatingRateCoupon>& underlying() const { return underlying_; }
    Real constantLegNotional() const { return constantLegNotional_; }
    const FxReset& fxReset() const { return fxReset_; }
    Date fxResetDate() const { return fxReset_.fixingDate(); }
    Date fxResetValueDate() const { return fxReset_.valueDate(); }
    const ext::shared_ptr<FxResetPricer>& fxResetPricer() const { return fxResetPricer_; }
    void setFxResetPricer(const ext::shared_ptr<FxResetPricer>& pricer);
    void deepUpdate() override;
    void performCalculations() const override { rate_ = underlying_->rate(); }
    void accept(AcyclicVisitor&) override;

  private:
    Real nominalScale() const { return nominal() / underlying_->nominal(); }

    ext::shared_ptr<FloatingRateCoupon> underlying_;
    Real constantLegNotional_;
    FxReset fxReset_;
    ext::shared_ptr<FxResetPricer> fxResetPricer_;
};


// Netted notional exchange of an FX-resetting leg
class FxResetNotionalExchange : public CashFlow {
  public:
    FxResetNotionalExchange(const Date& paymentDate,
                            Real constantLegNotional,
                            std::optional<FxReset> previousReset,
                            std::optional<FxReset> currentReset);
    Date date() const override { return paymentDate_; }
    Real amount() const override;
    Real constantLegNotional() const { return constantLegNotional_; }
    const std::optional<FxReset>& previousReset() const { return previousReset_; }
    const std::optional<FxReset>& currentReset() const { return currentReset_; }
    const ext::shared_ptr<FxResetPricer>& fxResetPricer() const { return fxResetPricer_; }
    void setFxResetPricer(const ext::shared_ptr<FxResetPricer>& pricer);
    void accept(AcyclicVisitor&) override;

  private:
    Date paymentDate_;
    Real constantLegNotional_;
    std::optional<FxReset> previousReset_;
    std::optional<FxReset> currentReset_;
    ext::shared_ptr<FxResetPricer> fxResetPricer_;
};

//! Assign an FX-reset pricer to all FX-reset cash flows in a leg
void setFxResetPricer(const Leg& leg, const ext::shared_ptr<FxResetPricer>& pricer);

}

#endif
