/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Peter Caspers

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

/*! \file strippedcapflooredcoupon.hpp
    \brief strips the embedded option from cap floored coupons
*/

#ifndef quantlib_stripped_capfloored_coupon_hpp
#define quantlib_stripped_capfloored_coupon_hpp

#include <ql/cashflows/capflooredcoupon.hpp>

namespace QuantLib {

    class StrippedCappedFlooredCoupon : public FloatingRateCoupon {

    public:

        explicit StrippedCappedFlooredCoupon(const ext::shared_ptr<CappedFlooredCoupon> &underlying);

        //! Coupon interface
        Rate rate() const override;
        Rate convexityAdjustment() const override;
        //! cap
        Rate cap() const;
        //! floor
        Rate floor() const;
        //! effective cap
        Rate effectiveCap() const;
        //! effective floor
        Rate effectiveFloor() const;

        //! Observer interface
        void update() override;

        //! Visitability
        void accept(AcyclicVisitor&) override;

        bool isCap() const;
        bool isFloor() const;
        bool isCollar() const;

        void setPricer(const ext::shared_ptr<FloatingRateCouponPricer>& pricer) override;

        ext::shared_ptr<CappedFlooredCoupon> underlying() { return underlying_; }

      protected:
        ext::shared_ptr<CappedFlooredCoupon> underlying_;

    };

    class StrippedCappedFlooredCouponLeg {
      public:
        explicit StrippedCappedFlooredCouponLeg(Leg underlyingLeg);
        operator Leg() const;
      private:
        Leg underlyingLeg_;
    };

}


#endif
