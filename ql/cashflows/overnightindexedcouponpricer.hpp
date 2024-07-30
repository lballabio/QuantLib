/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters
 Copyright (C) 2009 Ferdinando Ametrano
 Copyright (C) 2014 Peter Caspers
 Copyright (C) 2017 Joseph Jeisman
 Copyright (C) 2017 Fabrice Lecuyer

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

/*! \file overnightindexedcouponpricer.hpp
    \brief contains the pricer for an OvernightIndexedCoupon
*/

#ifndef quantlib_overnight_indexed_coupon_pricer_hpp
#define quantlib_overnight_indexed_coupon_pricer_hpp

#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/cashflows/overnightindexedcoupon.hpp>

namespace QuantLib {

    //! CompoudAveragedOvernightIndexedCouponPricer pricer
    class CompoundingOvernightIndexedCouponPricer : public FloatingRateCouponPricer {
      public:
        //! \name FloatingRateCoupon interface
        //@{
        void initialize(const FloatingRateCoupon& coupon) override;
        Rate swapletRate() const override;
        Real swapletPrice() const override { QL_FAIL("swapletPrice not available"); }
        Real capletPrice(Rate) const override { QL_FAIL("capletPrice not available"); }
        Rate capletRate(Rate) const override { QL_FAIL("capletRate not available"); }
        Real floorletPrice(Rate) const override { QL_FAIL("floorletPrice not available"); }
        Rate floorletRate(Rate) const override { QL_FAIL("floorletRate not available"); }
        //@}
        Rate averageRate(const Date& date) const;

      protected:
        const OvernightIndexedCoupon* coupon_ = nullptr;
    };
}

#endif
