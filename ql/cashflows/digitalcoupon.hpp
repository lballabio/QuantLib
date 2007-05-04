/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
  Copyright (C) 2007 Cristina Duminuco
  Copyright (C) 2007 Giorgio Facchinetti

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file digitalcoupon.hpp
    \brief Floating-rate coupon with digital call/put option
*/

#ifndef quantlib_digital_coupon_hpp
#define quantlib_digital_coupon_hpp

#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/cmscoupon.hpp>
#include <ql/cashflows/couponpricer.hpp>

namespace QuantLib {

    //! Floating-rate coupon with digital digital call/put option
    /*! Payoffs:
        - Coupon with cash-or-nothing Digital Call
          rate + csi * cashRate * Heaviside(rate-strike)
        - Coupon with cash-or-nothing Digital Put
          rate + csi * cashRate * Heaviside(strike-rate)
        where csi=+1 or csi=-1.
        - Coupon with asset-or-nothing Digital Call
          rate + csi * rate * Heaviside(rate-strike)
        - Coupon with asset-or-nothing Digital Put
          rate + csi * rate * Heaviside(strike-rate)
        where csi=+1 or csi=-1.
        The evaluation of the coupon is made using the call/put spread
        replication method.
    */
    class DigitalCoupon : public FloatingRateCoupon {
      public:
        //! \name Constructors
        //@{
        //! general constructor (collar)
        /*! If cashRate is equal to its default value, an asset-or-nothing option
            will be constructed.
        */
        DigitalCoupon(const boost::shared_ptr<FloatingRateCoupon>& underlying,
                      Rate callStrike = Null<Rate>(),
                      Rate putStrike = Null<Rate>(),
                      Rate cashRate = Null<Rate>(),
                      bool isCallOptionAdded = true,
                      bool isPutOptionAdded = true,
                      Real eps = 1e-4);
        //@}
        //! \name Coupon interface
        //@{
        Rate rate() const;
        Rate convexityAdjustment() const;
        //@}
        //@}
        //! \name Digital inspectors
        //@{
        Rate callStrike() const;
        Rate putStrike() const;
        Rate cashRate() const;
        bool hasPut() const { return (hasPutStrike_ && !hasCallStrike_); }
        bool hasCall() const {return (hasCallStrike_ && !hasPutStrike_); }
        bool hasCollar() const {return (hasCallStrike_ && hasPutStrike_); }
        bool isPutAdded() const { return (putCsi_==1.); }
        bool isCallAdded() const { return (callCsi_==1.); }
        boost::shared_ptr<FloatingRateCoupon> underlying() const { return underlying_; }
        /*! Returns the option rate
           (multiplied by: nominal*accrualperiod*discount is the NPV of the option)
        */
        Rate optionRate() const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);

        void setPricer(
            const boost::shared_ptr<FloatingRateCouponPricer>& pricer) {
            // set underlying pricer
            if(pricer_)
                unregisterWith(pricer_);
            pricer_ = pricer;
            QL_REQUIRE(pricer_, "no adequate pricer given");
            registerWith(pricer_);
            update();
            underlying_->setPricer(pricer);
        }

        protected:
        //! \name Data members
        //@{
        //!
        boost::shared_ptr<FloatingRateCoupon> underlying_;
        //! the strike rate for the the call option
        Rate callStrike_;
        //! the strike rate for the the put option
        Rate putStrike_;
        //! the effective strike rate for the the call option
        Rate effectiveCallStrike_;
        //! the effective strike rate for the the put option
        Rate effectivePutStrike_;
        //! the rate paid if the cash-or-nothing option is in-the-money
        Rate cashRate_;
        //! the multiplicative factor of call payoff
        Real callCsi_;
        //! the multiplicative factor of put payoff
        Real putCsi_;
        //! the gap between strikes in payoff replication
        Real eps_;
        //!
        bool hasPutStrike_, hasCallStrike_;
        //! Digital option type: if true, cash-or-nothing, if false asset-or-nothing
        bool isCashOrNothing_;
        //@}
    };

    //! Ibor rate coupon with digital digital call/put option
    class DigitalIborCoupon : public DigitalCoupon {
    public:
        DigitalIborCoupon(const boost::shared_ptr<IborCoupon>& underlying,
                          Rate callStrike = Null<Rate>(),
                          Rate putStrike = Null<Rate>(),
                          Rate cashRate = Null<Rate>(),
                          bool isCallOptionAdded = true,
                          bool isPutOptionAdded = true,
                          Real eps = 1e-4) :
            DigitalCoupon(underlying, callStrike, putStrike, cashRate,
                          isCallOptionAdded, isPutOptionAdded, eps) {}
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
    };

    //! Swap rate coupon with digital digital call/put option
    class DigitalCmsCoupon : public DigitalCoupon {
    public:
        DigitalCmsCoupon(const boost::shared_ptr<CmsCoupon>& underlying,
                         Rate callStrike = Null<Rate>(),
                         Rate putStrike = Null<Rate>(),
                         Rate cashRate = Null<Rate>(),
                         bool isCallOptionAdded = true,
                         bool isPutOptionAdded = true,
                         Real eps = 1e-4) :
            DigitalCoupon(underlying, callStrike, putStrike, cashRate,
                          isCallOptionAdded, isPutOptionAdded, eps) {}
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
    };

}

#endif
