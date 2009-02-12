/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon

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

/*! \file inflationcapfloorengines.hpp
    \brief Engines for inflation cap/floors
*/

#ifndef quantlib_inflation_cap_floor_engine_hpp
#define quantlib_inflation_cap_floor_engine_hpp

#include <ql/experimental/inflation/inflationcapfloor.hpp>

namespace QuantLib {

    class YoYOptionletVolatilitySurface;

    //! base class for YoY capfloor pricing engines
    class YoYInflationCapFloorEngine : public YoYInflationCapFloor::engine {
      public:
        //! vanilla pricer root
        YoYInflationCapFloorEngine(
                               const Handle<YieldTermStructure>&,
                               const Handle<YoYOptionletVolatilitySurface>&);
        //! complex pricer root, i.e. uses model parameters
        YoYInflationCapFloorEngine(const Handle<YieldTermStructure>&);

        Handle<YieldTermStructure> termStructure();
        Handle<YoYOptionletVolatilitySurface> volatility();

      protected:
        Handle<YieldTermStructure> termStructure_;
        Handle<YoYOptionletVolatilitySurface> volatility_;
    };


    //! Black-formula inflation cap/floor engine
    /*! This class is an adaptor to CouponPricer.

        It creates a new CapFloored-coupon from each usual coupon
        with appropriate strikes, then asks it its price using the
        appropriate coupon pricer.

        Note that the spread and gearing are taken from the original
        coupon.
    */
    class BlackYoYInflationCapFloorEngine
        : public YoYInflationCapFloorEngine {
      public:
        BlackYoYInflationCapFloorEngine(
                                const Handle<YieldTermStructure>&,
                                const Handle<YoYOptionletVolatilitySurface>&);

        virtual void calculate() const;
    };


    //! Displaced-diffusion version with unit displacement.
    class UnitDisplacedBlackYoYInflationCapFloorEngine
        : public YoYInflationCapFloorEngine {
      public:
        UnitDisplacedBlackYoYInflationCapFloorEngine(
                                const Handle<YieldTermStructure>&,
                                const Handle<YoYOptionletVolatilitySurface>&);

        virtual void calculate() const;
    };


    //! Bachelier version, i.e. Normal model = Brownian motion (not geometric)
    class BachelierYoYInflationCapFloorEngine
        : public YoYInflationCapFloorEngine {
      public:
        BachelierYoYInflationCapFloorEngine(
                                const Handle<YieldTermStructure>&,
                                const Handle<YoYOptionletVolatilitySurface>&);

        virtual void calculate() const;
    };

}

#endif

