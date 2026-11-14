/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Thiago Rodrigues

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

/*! \file equitynotionalresetcoupon.hpp
    \brief floating-rate coupon whose notional is reset to an equity index level
*/

#ifndef quantlib_equity_notional_reset_coupon_hpp
#define quantlib_equity_notional_reset_coupon_hpp

#include <ql/cashflows/floatingratecoupon.hpp>

namespace QuantLib {

    class EquityIndex;

    //! Floating-rate coupon whose notional is reset to an equity index level
    /*! This coupon wraps an underlying floating-rate coupon and
        rescales its nominal \f$ N \f$ so that it tracks a fixed number
        of units of an equity index:
        \f[
        N_i = N \frac{I(T_{i-1})}{I(T_0)},
        \f]
        where \f$ T_0 \f$ is the initial date, on which the nominal
        equals that of the underlying coupon, and \f$ T_{i-1} \f$ is
        the reset date, usually the start of the accrual period.  The
        rate, the accrual logic and the pricer are those of the
        underlying coupon.

        It can be used for the funding leg of equity total return
        swaps with notional reset.

        \note When the reset date is in the future, the index level is
              forecast independently of the rate; that is, the
              expected amount is the product of the forecast notional
              and the forecast rate.  This is exact for deterministic
              rates; no convexity adjustment is made for the
              correlation between the equity index and the rate.
    */
    class EquityNotionalResetCoupon : public FloatingRateCoupon {
      public:
        EquityNotionalResetCoupon(const ext::shared_ptr<FloatingRateCoupon>& underlying,
                                  ext::shared_ptr<EquityIndex> equityIndex,
                                  const Date& initialDate,
                                  const Date& resetDate);
        //! \name Coupon interface
        //@{
        //! the nominal reset on the reset date
        Real nominal() const override;
        Real accruedAmount(const Date&) const override;
        //@}
        //! \name CashFlow interface
        //@{
        Real amount() const override;
        //@}
        //! \name FloatingRateCoupon interface
        //@{
        Date fixingDate() const override { return underlying_->fixingDate(); }
        Rate indexFixing() const override { return underlying_->indexFixing(); }
        //! the pricer is forwarded to the underlying coupon
        void setPricer(const ext::shared_ptr<FloatingRateCouponPricer>&) override;
        //@}
        //! \name Inspectors
        //@{
        const ext::shared_ptr<FloatingRateCoupon>& underlying() const { return underlying_; }
        const ext::shared_ptr<EquityIndex>& equityIndex() const { return equityIndex_; }
        //! the nominal on the initial date
        Real initialNominal() const { return underlying_->nominal(); }
        //! the date on which the initial nominal was set
        const Date& initialDate() const { return initialDate_; }
        //! the date on which the equity index is observed to reset the nominal
        const Date& resetDate() const { return resetDate_; }
        //@}
        //! \name LazyObject interface
        //@{
        void performCalculations() const override { rate_ = underlying_->rate(); }
        //@}
        //! \name Observer interface
        //@{
        void deepUpdate() override;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      private:
        Real nominalScale() const;

        ext::shared_ptr<FloatingRateCoupon> underlying_;
        ext::shared_ptr<EquityIndex> equityIndex_;
        Date initialDate_, resetDate_;
    };

}

#endif
