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

/*! \file yoyoptionletstripper.hpp
    \brief yoy inflation-cap stripping
*/

#ifndef quantlib_yoy_optionlet_stripper_hpp
#define quantlib_yoy_optionlet_stripper_hpp

#include <ql/experimental/inflation/yoyinflationoptionletvolatilitystructure2.hpp>
#include <ql/pricingengines/inflation/inflationcapfloorengines.hpp>
#include <ql/experimental/inflation/yoycapfloortermpricesurface.hpp>

namespace QuantLib {

    //! Interface for inflation cap stripping, i.e. from price surfaces.
    /*! Strippers return K slices of the volatility surface at a given T.
        In initialize they actually do the stripping along each K.
    */
    class YoYOptionletStripper {
      public:
        virtual ~YoYOptionletStripper() = default;

        //! YoYOptionletStripper interface
        //@{
        virtual void initialize(const ext::shared_ptr<YoYCapFloorTermPriceSurface>&,
                                const ext::shared_ptr<YoYInflationCapFloorEngine>&,
                                Real slope) const = 0;
        virtual Rate minStrike() const = 0;
        virtual Rate maxStrike() const = 0;
        virtual std::vector<Rate> strikes() const = 0;
        virtual std::pair<std::vector<Rate>, std::vector<Volatility> >
        slice(const Date &d) const = 0;
        //@}

      protected:
        mutable ext::shared_ptr<YoYCapFloorTermPriceSurface>
                                                 YoYCapFloorTermPriceSurface_;
        mutable ext::shared_ptr<YoYInflationCapFloorEngine> p_;
        mutable Period lag_;
        mutable Frequency frequency_;
        mutable bool indexIsInterpolated_;
    };

}

#endif

