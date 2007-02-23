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
    \brief Floating rate coupon with additional cap/floor
*/

#ifndef quantlib_digital_coupon_hpp
#define quantlib_digital_coupon_hpp

#include <ql/CashFlows/iborcoupon.hpp>
#include <ql/CashFlows/cmscoupon.hpp>

namespace QuantLib {

    /*! \file digital.hpp

    */


    class DigitalCoupon : public FloatingRateCoupon {
      public:
        //! \name Constructors
        //@{
        //! General constructor (collar)
        DigitalCoupon(
                  const boost::shared_ptr<FloatingRateCoupon>& underlying,
                  Rate callStrike = Null<Rate>(),
                  Rate putStrike = Null<Rate>(),
                  Rate cashRate = Null<Rate>(),
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
        //! Returns the call strike 
        Rate callStrike() const;
        //! Returns the put strike 
        Rate putStrike() const;
        //!
        bool isPut() const { return (hasUpperStrike_ && !hasLowerStrike_); } 
        bool isCall() const {return (hasLowerStrike_ && !hasUpperStrike_); }
        bool isCollar() const {return (hasLowerStrike_ && hasUpperStrike_); }
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
 
        void setPricer(const boost::shared_ptr<FloatingRateCouponPricer>& pricer){
			if(pricer_)
                unregisterWith(pricer_);
            pricer_ = pricer;
            QL_REQUIRE(pricer_, "no adequate pricer given");
            registerWith(pricer_);
            update();
            underlying_->setPricer(pricer);
		}

    protected:        
        //! \name Member data
        //@{
        //!
        boost::shared_ptr<FloatingRateCoupon> underlying_;
        //! the strike rate for the the call option 
        Rate lowerStrike_;
        //! the strike rate for the the put option 
        Rate upperStrike_;
        //! the rate paid if the cas-or-nothing option is in-the-money
        Rate cashRate_;
        //! the gap between strikes in payoff replication
        Real eps_;
        //! 
        bool hasUpperStrike_, hasLowerStrike_;
        //! Digital option type: if true, cash-or-nothing, if false asset-or-nothing
        bool isCashOrNothing_;
        //@}
    };

}

#endif
