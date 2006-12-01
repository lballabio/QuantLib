/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 François du Vignaud

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

/*! \file smilesection.hpp
    \brief Swaption volatility structure
*/

#ifndef quantlib_interpolated_smile_section_hpp
#define quantlib_interpolated_smile_section_hpp

#include <ql/termstructure.hpp>
#include <ql/Math/linearinterpolation.hpp>
#include <ql/Volatilities/smilesection.hpp> 
namespace QuantLib {

    template <class Interpolator>
    class GenericInterpolatedSmileSection : public SmileSectionInterface,
                                            public LazyObject {
      public:
        GenericInterpolatedSmileSection(Time expiryTime,
                 const std::vector<Rate>& strikes,
                 const std::vector<Handle<Quote> >& volatilities,
                 const Interpolator& interpolator = Interpolator());

        GenericInterpolatedSmileSection(const Date&,
                 const DayCounter&,
                 const std::vector<Rate>& strikes,
                 const std::vector<Handle<Quote> >& volatilities,
                 const Interpolator& interpolator = Interpolator());
        void performCalculations() const;
        Real variance(Rate strike) const;
        Volatility volatility(Rate strike) const;
      private:
        std::vector<Rate> strikes_;
        mutable std::vector<Volatility> volatilities_;
        Interpolation interpolation_;
        std::vector<Handle<Quote> > volatilitiesHandles_;
    };


    template <class Interpolator>
    GenericInterpolatedSmileSection<Interpolator>::GenericInterpolatedSmileSection(
        Time timeToExpiry,
        const std::vector<Rate>& strikes,
        const std::vector<Handle<Quote> >& volatilitiesHandles,
        const Interpolator& interpolator = Interpolator())
    : SmileSectionInterface(timeToExpiry), strikes_(strikes),
        volatilitiesHandles_(volatilitiesHandles) {
    for(Size i=0; i<volatilitiesHandles_.size(); ++i)
        registerWith(volatilitiesHandles_[i]);
    // check strikes!!!!!!!!!!!!!!!!!!!!
    interpolation_ = interpolator.interpolate(strikes_.begin(),
                                              strikes_.end(),
                                              volatilities_.begin());
    }

    template <class Interpolator>
    GenericInterpolatedSmileSection<Interpolator>::GenericInterpolatedSmileSection(
        const Date& d,
        const DayCounter& dc,
        const std::vector<Rate>& strikes,
        const std::vector<Handle<Quote> >& volatilitiesHandles,
        const Interpolator& interpolator = Interpolator())
    : SmileSectionInterface(d, dc), strikes_(strikes),
        volatilitiesHandles_(volatilitiesHandles) {
    for(Size i=0; i<volatilitiesHandles_.size(); ++i)
        registerWith(volatilitiesHandles_[i]);
       // check strikes!!!!!!!!!!!!!!!!!!!!
    interpolation_ = interpolator.interpolate(strikes_.begin(),
                                               strikes_.end(),
                                               volatilities_.begin());
    }

    template <class Interpolator>
    inline void GenericInterpolatedSmileSection<Interpolator>::
        performCalculations() const {
        for (Size i=0; i<volatilitiesHandles_.size(); ++i)
            volatilities_[i] = volatilitiesHandles_[i]->value();
    };

    template <class Interpolator>
    Real GenericInterpolatedSmileSection<Interpolator>::variance(Real strike) const {
        calculate();
        Real v = interpolation_(strike, true);
        return v*v*exerciseTime_;
    }

    template <class Interpolator>
    Real GenericInterpolatedSmileSection<Interpolator>::volatility(Real strike) const {
        calculate();
        return interpolation_(strike, true);
    }
}

#endif

