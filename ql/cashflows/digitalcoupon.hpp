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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file digitalcoupon.hpp
    \brief Floating-rate coupon with digital call/put option
*/

#ifndef quantlib_digital_coupon_hpp
#define quantlib_digital_coupon_hpp

#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/replication.hpp>
#include <ql/position.hpp>
#include <ql/utilities/null.hpp>

namespace QuantLib {

    //! Digital-payoff coupon
    /*! Implementation of a floating-rate coupon with digital call/put option.
        Payoffs:
        - Coupon with cash-or-nothing Digital Call
          rate + csi * payoffRate * Heaviside(rate-strike)
        - Coupon with cash-or-nothing Digital Put
          rate + csi * payoffRate * Heaviside(strike-rate)
        where csi=+1 or csi=-1.
        - Coupon with asset-or-nothing Digital Call
          rate + csi * rate * Heaviside(rate-strike)
        - Coupon with asset-or-nothing Digital Put
          rate + csi * rate * Heaviside(strike-rate)
        where csi=+1 or csi=-1.
        The evaluation of the coupon is made using the call/put spread
        replication method.
    */
    /*! \ingroup instruments

        \test
        - the correctness of the returned value in case of Asset-or-nothing
          embedded option is tested by pricing the digital option with
          Cox-Rubinstein formula.
        - the correctness of the returned value in case of deep-in-the-money
          Asset-or-nothing embedded option is tested vs the expected values of
          coupon and option.
        - the correctness of the returned value in case of deep-out-of-the-money
          Asset-or-nothing embedded option is tested vs the expected values of
          coupon and option.
        - the correctness of the returned value in case of Cash-or-nothing
          embedded option is tested by pricing the digital option with
          Reiner-Rubinstein formula.
        - the correctness of the returned value in case of deep-in-the-money
          Cash-or-nothing embedded option is tested vs the expected values of
          coupon and option.
        - the correctness of the returned value in case of deep-out-of-the-money
          Cash-or-nothing embedded option is tested vs the expected values of
          coupon and option.
        - the correctness of the returned value is tested checking the correctness
          of the call-put parity relation.
        - the correctness of the returned value is tested by the relationship
          between prices in case of different replication types.
    */
    class DigitalCoupon : public FloatingRateCoupon {
      public:
        //! \name Constructors
        //@{
        //! general constructor
        DigitalCoupon(const ext::shared_ptr<FloatingRateCoupon>& underlying,
                      Rate callStrike = Null<Rate>(),
                      Position::Type callPosition = Position::Long,
                      bool isCallITMIncluded = false,
                      Rate callDigitalPayoff = Null<Rate>(),
                      Rate putStrike = Null<Rate>(),
                      Position::Type putPosition = Position::Long,
                      bool isPutITMIncluded = false,
                      Rate putDigitalPayoff = Null<Rate>(),
                      const ext::shared_ptr<DigitalReplication>& replication =
                        ext::shared_ptr<DigitalReplication>() );

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
        Rate callDigitalPayoff() const;
        Rate putDigitalPayoff() const;
        bool hasPut() const { return hasPutStrike_; }
        bool hasCall() const {return hasCallStrike_; }
        bool hasCollar() const {return (hasCallStrike_ && hasPutStrike_); }
        bool isLongPut() const { return (putCsi_==1.); }
        bool isLongCall() const { return (callCsi_==1.); }
        ext::shared_ptr<FloatingRateCoupon> underlying() const { return underlying_; }
        /*! Returns the call option rate
           (multiplied by: nominal*accrualperiod*discount is the NPV of the option)
        */
        Rate callOptionRate() const;
        /*! Returns the put option rate
           (multiplied by: nominal*accrualperiod*discount is the NPV of the option)
        */
        Rate putOptionRate() const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);

        void setPricer(
            const ext::shared_ptr<FloatingRateCouponPricer>& pricer) {
            if (pricer_)
                unregisterWith(pricer_);
            pricer_ = pricer;
            if (pricer_)
                registerWith(pricer_);
            update();
            underlying_->setPricer(pricer);
        }

        protected:
        //! \name Data members
        //@{
        //!
        ext::shared_ptr<FloatingRateCoupon> underlying_;
        //! strike rate for the the call option
        Rate callStrike_;
        //! strike rate for the the put option
        Rate putStrike_;
        //! multiplicative factor of call payoff
        Real callCsi_;
        //! multiplicative factor of put payoff
        Real putCsi_;
        //! inclusion flag og the call payoff if the call option ends at-the-money
        bool isCallATMIncluded_;
        //! inclusion flag og the put payoff if the put option ends at-the-money
        bool isPutATMIncluded_;
        //! digital call option type: if true, cash-or-nothing, if false asset-or-nothing
        bool isCallCashOrNothing_;
        //! digital put option type: if true, cash-or-nothing, if false asset-or-nothing
        bool isPutCashOrNothing_;
        //! digital call option payoff rate, if any
        Rate callDigitalPayoff_;
        //! digital put option payoff rate, if any
        Rate putDigitalPayoff_;
        //! the left and right gaps applied in payoff replication for call
        Real callLeftEps_, callRightEps_;
        //! the left and right gaps applied in payoff replication for puf
        Real putLeftEps_, putRightEps_;
        //!
        bool hasPutStrike_, hasCallStrike_;
        //! Type of replication
        Replication::Type replicationType_;

        //@}
      private:
        Rate callPayoff() const;
        Rate putPayoff() const;

    };

}

#endif
