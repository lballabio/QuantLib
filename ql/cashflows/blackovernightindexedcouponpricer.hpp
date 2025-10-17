/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020 Quaternion Risk Management Ltd

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.


 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
*/

/*! \file blackovernightindexedcouponpricer.hpp
    \brief black coupon pricer for capped / floored ON indexed coupons
*/

#ifndef quantlib_black_overnight_indexed_coupon_pricer_hpp
#define quantlib_black_overnight_indexed_coupon_pricer_hpp

#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/cashflows/overnightindexedcouponpricer.hpp>

#include <ql/termstructures/volatility/optionlet/optionletvolatilitystructure.hpp>

namespace QuantLib {

    //! Black compounded overnight coupon pricer
    /* The methods that are implemented here to price capped / floored compounded ON coupons are
    highly experimental and ad-hoc. As soon as a market best practice has evolved, the pricer
    should be revised. */
    class BlackOvernightIndexedCouponPricer : public CompoundingOvernightIndexedCouponPricer {
    public:
        explicit BlackOvernightIndexedCouponPricer(
                Handle<OptionletVolatilityStructure> v = Handle<OptionletVolatilityStructure>(),
                const bool effectiveVolatilityInput = false);
        //! \name FloatingRateCoupon interface
        //@{
        void initialize(const FloatingRateCoupon& coupon) override;
        Real swapletPrice() const override;
        Rate swapletRate() const override;
        Real capletPrice(Rate effectiveCap) const override;
        Rate capletRate(Rate effectiveCap) const override;
        Real floorletPrice(Rate effectiveFloor) const override;
        Rate floorletRate(Rate effectiveFloor) const override;
        //@}
        Rate capletRate(Rate effectiveCap, bool localCapFloor) const override;
        Rate floorletRate(Rate effectiveCap, bool localCapFloor) const override;
    private:
        Real optionletRateGlobal(Option::Type optionType, Real effStrike) const;
        Real optionletRateLocal(Option::Type optionType, Real effStrike) const;

        Real gearing_;
        ext::shared_ptr<IborIndex> index_;
        Real effectiveIndexFixing_, swapletRate_;
    };

    //! Black averaged overnight coupon pricer
    /* The methods that are implemented here to price capped / floored average ON coupons are
    highly experimental and ad-hoc. As soon as a market best practice has evolved, the pricer
    should be revised. */
    class BlackAverageONIndexedCouponPricer : public ArithmeticAveragedOvernightIndexedCouponPricer {
    public:
        explicit BlackAverageONIndexedCouponPricer(
                Handle<OptionletVolatilityStructure> v = Handle<OptionletVolatilityStructure>(),
                const bool effectiveVolatilityInput = false);
        //! \name FloatingRateCoupon interface
        //@{
        void initialize(const FloatingRateCoupon& coupon) override;
        Real swapletPrice() const override;
        Rate swapletRate() const override;
        Real capletPrice(Rate effectiveCap) const override;
        Rate capletRate(Rate effectiveCap) const override;
        Real floorletPrice(Rate effectiveFloor) const override;
        Rate floorletRate(Rate effectiveFloor) const override;
        //@}
        Rate capletRate(Rate effectiveCap, bool localCapFloor) const override;
        Rate floorletRate(Rate effectiveCap, bool localCapFloor) const override;
    private:
        Real optionletRateGlobal(Option::Type optionType, Real effStrike) const;
        Real optionletRateLocal(Option::Type optionType, Real effStrike) const;

        Real gearing_;
        ext::shared_ptr<IborIndex> index_;
        Real swapletRate_, forwardRate_;
    };

}

#endif