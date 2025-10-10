/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters
 Copyright (C) 2009 Ferdinando Ametrano
 Copyright (C) 2014 Peter Caspers
 Copyright (C) 2016 Stefano Fondi
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

    class OptionletVolatilityStructure;

    //! Pricer for overnight-indexed floating coupons
    /*!
        This is the base pricer class for coupons indexed to an overnight rate.  
        It defines the common pricing interface and provides the foundation for 
        more specialized overnight coupon pricers (e.g., compounded, averaged, 
        capped/floored variants).

        Derived classes should implement the specific logic for computing the 
        rate and optional adjustments, depending on the compounding or 
        averaging convention used.
    */
    class OvernightIndexedCouponPricer : public FloatingRateCouponPricer {
      using FloatingRateCouponPricer::capletRate;
      using FloatingRateCouponPricer::floorletRate;
      public:

        explicit OvernightIndexedCouponPricer(
          Handle<OptionletVolatilityStructure> v = Handle<OptionletVolatilityStructure>(),
          const bool effectiveVolatilityInput = false);

        void initialize(const FloatingRateCoupon& coupon) override;

        void setCapletVolatility(
                            const Handle<OptionletVolatilityStructure>& v =
                                    Handle<OptionletVolatilityStructure>()) {
            unregisterWith(capletVol_);
            capletVol_ = v;
            registerWith(capletVol_);
            update();
        }

        /*! \brief Returns the handle to the optionlet volatility structure used for caplets/floorlets */
        Handle<OptionletVolatilityStructure> capletVolatility() const {
            return capletVol_;
        }
        
        void setEffectiveVolatilityInput(const bool effectiveVolatilityInput) {
            effectiveVolatilityInput_ = effectiveVolatilityInput;
        }

        /*! \brief Returns true if the volatility input is interpreted as effective volatility */
        bool effectiveVolatilityInput() const;
        /*! \brief Returns the effective caplet volatility used in the last capletRate() calculation.
            \note Only available after capletRate() was called.
        */
        virtual Real effectiveCapletVolatility() const;
        /*! \brief Returns the effective floorlet volatility used in the last floorletRate() calculation.
            \note Only available after floorletRate() was called.
        */
        virtual Real effectiveFloorletVolatility() const;

        virtual Rate capletRate(Rate effectiveCap, bool localCapFloor) const;
        virtual Rate floorletRate(Rate effectiveCap, bool localCapFloor) const;

      protected:
        const OvernightIndexedCoupon* coupon_ = nullptr;
        Handle<OptionletVolatilityStructure> capletVol_;
        bool effectiveVolatilityInput_ = false;
        mutable Real effectiveCapletVolatility_ = Null<Real>();
        mutable Real effectiveFloorletVolatility_ = Null<Real>();
    };

    //! CompoudAveragedOvernightIndexedCouponPricer pricer
    class CompoundingOvernightIndexedCouponPricer : public OvernightIndexedCouponPricer {
      public:
        explicit CompoundingOvernightIndexedCouponPricer(
          Handle<OptionletVolatilityStructure> v = Handle<OptionletVolatilityStructure>(),
          const bool effectiveVolatilityInput = false);
        //! \name FloatingRateCoupon interface
        //@{
        //void initialize(const FloatingRateCoupon& coupon) override;
        Rate swapletRate() const override;
        Real swapletPrice() const override { QL_FAIL("swapletPrice not available"); }
        Real capletPrice(Rate) const override { QL_FAIL("capletPrice not available"); }
        Rate capletRate(Rate) const override { QL_FAIL("capletRate not available"); }
        Real floorletPrice(Rate) const override { QL_FAIL("floorletPrice not available"); }
        Rate floorletRate(Rate) const override { QL_FAIL("floorletRate not available"); }
        //@}
        Rate averageRate(const Date& date) const;
        Rate effectiveSpread() const;
        Rate effectiveIndexFixing() const;

      protected:
        std::tuple<Rate, Spread, Rate> compute(const Date& date) const;
        mutable Real swapletRate_, effectiveSpread_, effectiveIndexFixing_;
    };

    /*! pricer for arithmetically averaged overnight indexed coupons
    Reference: Katsumi Takada 2011, Valuation of Arithmetically Average of
    Fed Funds Rates and Construction of the US Dollar Swap Yield Curve
    */
    class ArithmeticAveragedOvernightIndexedCouponPricer : public OvernightIndexedCouponPricer {
      public:
        explicit ArithmeticAveragedOvernightIndexedCouponPricer(
            Real meanReversion = 0.03,
            Real volatility = 0.00, // NO convexity adjustment by default
            bool byApprox = false, // TRUE to use Katsumi Takada approximation
            Handle<OptionletVolatilityStructure> v = Handle<OptionletVolatilityStructure>(),
            const bool effectiveVolatilityInput = false)
        : OvernightIndexedCouponPricer(v, effectiveVolatilityInput),
         byApprox_(byApprox), mrs_(meanReversion), vol_(volatility) {}

        explicit ArithmeticAveragedOvernightIndexedCouponPricer(
            bool byApprox) // Simplified constructor assuming no convexity correction
        : ArithmeticAveragedOvernightIndexedCouponPricer(0.03, 0.0, byApprox) {}

        //void initialize(const FloatingRateCoupon& coupon) override;
        Rate swapletRate() const override;
        Real swapletPrice() const override { QL_FAIL("swapletPrice not available"); }
        Real capletPrice(Rate) const override { QL_FAIL("capletPrice not available"); }
        Rate capletRate(Rate) const override { QL_FAIL("capletRate not available"); }
        Real floorletPrice(Rate) const override { QL_FAIL("floorletPrice not available"); }
        Rate floorletRate(Rate) const override { QL_FAIL("floorletRate not available"); }

      protected:
        Real convAdj1(Time ts, Time te) const;
        Real convAdj2(Time ts, Time te) const;
        bool byApprox_;
        Real mrs_;
        Real vol_;
    };
}

#endif