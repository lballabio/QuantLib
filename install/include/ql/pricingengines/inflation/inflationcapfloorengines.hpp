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
    \brief Inflation cap/floor engines
 */

#ifndef quantlib_pricers_inflation_capfloor_hpp
#define quantlib_pricers_inflation_capfloor_hpp

#include <ql/instruments/inflationcapfloor.hpp>
#include <ql/termstructures/volatility/inflation/yoyinflationoptionletvolatilitystructure.hpp>
#include <ql/option.hpp>

namespace QuantLib {

    class Quote;
    class YoYOptionletVolatilitySurface;
    class YoYInflationIndex;

    //! Base YoY inflation cap/floor engine
    /*! This class doesn't know yet what sort of vol it is.  The
        inflation index must be linked to a yoy inflation term
        structure.

        \ingroup inflationcapfloorengines
    */
    class YoYInflationCapFloorEngine : public YoYInflationCapFloor::engine {
      public:
        YoYInflationCapFloorEngine(ext::shared_ptr<YoYInflationIndex>,
                                   Handle<YoYOptionletVolatilitySurface> vol,
                                   Handle<YieldTermStructure> nominalTermStructure);

        ext::shared_ptr<YoYInflationIndex> index() const { return index_;}
        Handle<YoYOptionletVolatilitySurface> volatility() const { return volatility_; }
        Handle<YieldTermStructure> nominalTermStructure() const { return nominalTermStructure_; }

        void setVolatility(const Handle<YoYOptionletVolatilitySurface>& vol);

        void calculate() const override;

      protected:
        //! descendents only need to implement this
        virtual Real optionletImpl(Option::Type type, Rate strike,
                                   Rate forward, Real stdDev,
                                   Real d) const = 0;

        ext::shared_ptr<YoYInflationIndex> index_;
        Handle<YoYOptionletVolatilitySurface> volatility_;
        Handle<YieldTermStructure> nominalTermStructure_;
    };



    //! Black-formula inflation cap/floor engine (standalone, i.e. no coupon pricer)
    class YoYInflationBlackCapFloorEngine
    : public YoYInflationCapFloorEngine {
      public:
        YoYInflationBlackCapFloorEngine(const ext::shared_ptr<YoYInflationIndex>&,
                                        const Handle<YoYOptionletVolatilitySurface>& vol,
                                        const Handle<YieldTermStructure>& nominalTermStructure);
      protected:
        Real
        optionletImpl(Option::Type, Real strike, Real forward, Real stdDev, Real d) const override;
    };


    //! Unit Displaced Black-formula inflation cap/floor engine (standalone, i.e. no coupon pricer)
    class YoYInflationUnitDisplacedBlackCapFloorEngine
    : public YoYInflationCapFloorEngine {
      public:
        YoYInflationUnitDisplacedBlackCapFloorEngine(
                    const ext::shared_ptr<YoYInflationIndex>&,
                    const Handle<YoYOptionletVolatilitySurface>& vol,
                    const Handle<YieldTermStructure>& nominalTermStructure);
      protected:
        Real
        optionletImpl(Option::Type, Real strike, Real forward, Real stdDev, Real d) const override;
    };


    //! Unit Displaced Black-formula inflation cap/floor engine (standalone, i.e. no coupon pricer)
    class YoYInflationBachelierCapFloorEngine
    : public YoYInflationCapFloorEngine {
      public:
        YoYInflationBachelierCapFloorEngine(
                    const ext::shared_ptr<YoYInflationIndex>&,
                    const Handle<YoYOptionletVolatilitySurface>& vol,
                    const Handle<YieldTermStructure>& nominalTermStructure);
      protected:
        Real
        optionletImpl(Option::Type, Real strike, Real forward, Real stdDev, Real d) const override;
    };

}

#endif
