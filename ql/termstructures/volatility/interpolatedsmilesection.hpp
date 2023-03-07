/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 François du Vignaud
 Copyright (C) 2015 Peter Caspers

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

/*! \file interpolatedsmilesection.hpp
    \brief Interpolated smile section class
*/

#ifndef quantlib_interpolated_smile_section_hpp
#define quantlib_interpolated_smile_section_hpp

#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructure.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <utility>

namespace QuantLib {

    template<class Interpolator>
    class InterpolatedSmileSection : public SmileSection,
                                     public LazyObject {
      public:
        InterpolatedSmileSection(Time expiryTime,
                                 std::vector<Rate> strikes,
                                 const std::vector<Handle<Quote> >& stdDevHandles,
                                 Handle<Quote> atmLevel,
                                 const Interpolator& interpolator = Interpolator(),
                                 const DayCounter& dc = Actual365Fixed(),
                                 VolatilityType type = ShiftedLognormal,
                                 Real shift = 0.0);
        InterpolatedSmileSection(Time expiryTime,
                                 std::vector<Rate> strikes,
                                 const std::vector<Real>& stdDevs,
                                 Real atmLevel,
                                 const Interpolator& interpolator = Interpolator(),
                                 const DayCounter& dc = Actual365Fixed(),
                                 VolatilityType type = ShiftedLognormal,
                                 Real shift = 0.0);
        InterpolatedSmileSection(const Date& d,
                                 std::vector<Rate> strikes,
                                 const std::vector<Handle<Quote> >& stdDevHandles,
                                 Handle<Quote> atmLevel,
                                 const DayCounter& dc = Actual365Fixed(),
                                 const Interpolator& interpolator = Interpolator(),
                                 const Date& referenceDate = Date(),
                                 VolatilityType type = ShiftedLognormal,
                                 Real shift = 0.0);
        InterpolatedSmileSection(const Date& d,
                                 std::vector<Rate> strikes,
                                 const std::vector<Real>& stdDevs,
                                 Real atmLevel,
                                 const DayCounter& dc = Actual365Fixed(),
                                 const Interpolator& interpolator = Interpolator(),
                                 const Date& referenceDate = Date(),
                                 VolatilityType type = ShiftedLognormal,
                                 Real shift = 0.0);

        void performCalculations() const override;
        Real varianceImpl(Rate strike) const override;
        Volatility volatilityImpl(Rate strike) const override;
        Real minStrike() const override { return strikes_.front(); }
        Real maxStrike() const override { return strikes_.back(); }
        Real atmLevel() const override { return atmLevel_->value(); }
        void update() override;

      private:
        Real exerciseTimeSquareRoot_;
        std::vector<Rate> strikes_;
        std::vector<Handle<Quote> > stdDevHandles_;
        Handle<Quote> atmLevel_;
        mutable std::vector<Volatility> vols_;
        mutable Interpolation interpolation_;
    };


    template <class Interpolator>
    InterpolatedSmileSection<Interpolator>::InterpolatedSmileSection(
        Time timeToExpiry,
        std::vector<Rate> strikes,
        const std::vector<Handle<Quote> >& stdDevHandles,
        Handle<Quote> atmLevel,
        const Interpolator& interpolator,
        const DayCounter& dc,
        const VolatilityType type,
        const Real shift)
    : SmileSection(timeToExpiry, dc, type, shift),
      exerciseTimeSquareRoot_(std::sqrt(exerciseTime())), strikes_(std::move(strikes)),
      stdDevHandles_(stdDevHandles), atmLevel_(std::move(atmLevel)), vols_(stdDevHandles.size()) {
        for (auto& stdDevHandle : stdDevHandles_)
            LazyObject::registerWith(stdDevHandle);
        LazyObject::registerWith(atmLevel_);
        // check strikes!!!!!!!!!!!!!!!!!!!!
        interpolation_ = interpolator.interpolate(strikes_.begin(),
                                                  strikes_.end(),
                                                  vols_.begin());
    }

    template <class Interpolator>
    InterpolatedSmileSection<Interpolator>::InterpolatedSmileSection(
        Time timeToExpiry,
        std::vector<Rate> strikes,
        const std::vector<Real>& stdDevs,
        Real atmLevel,
        const Interpolator& interpolator,
        const DayCounter& dc,
        const VolatilityType type,
        const Real shift)
    : SmileSection(timeToExpiry, dc, type, shift),
      exerciseTimeSquareRoot_(std::sqrt(exerciseTime())), strikes_(std::move(strikes)),
      stdDevHandles_(stdDevs.size()), vols_(stdDevs.size()) {
        // fill dummy handles to allow generic handle-based
        // computations later on
        for (Size i=0; i<stdDevs.size(); ++i)
            stdDevHandles_[i] = Handle<Quote>(ext::shared_ptr<Quote>(new
                SimpleQuote(stdDevs[i])));
        atmLevel_ = Handle<Quote>
           (ext::shared_ptr<Quote>(new SimpleQuote(atmLevel)));
        // check strikes!!!!!!!!!!!!!!!!!!!!
        interpolation_ = interpolator.interpolate(strikes_.begin(),
                                                  strikes_.end(),
                                                  vols_.begin());
    }

    template <class Interpolator>
    InterpolatedSmileSection<Interpolator>::InterpolatedSmileSection(
        const Date& d,
        std::vector<Rate> strikes,
        const std::vector<Handle<Quote> >& stdDevHandles,
        Handle<Quote> atmLevel,
        const DayCounter& dc,
        const Interpolator& interpolator,
        const Date& referenceDate,
        const VolatilityType type,
        const Real shift)
    : SmileSection(d, dc, referenceDate, type, shift),
      exerciseTimeSquareRoot_(std::sqrt(exerciseTime())), strikes_(std::move(strikes)),
      stdDevHandles_(stdDevHandles), atmLevel_(std::move(atmLevel)), vols_(stdDevHandles.size()) {
        for (auto& stdDevHandle : stdDevHandles_)
            LazyObject::registerWith(stdDevHandle);
        LazyObject::registerWith(atmLevel_);
        // check strikes!!!!!!!!!!!!!!!!!!!!
        interpolation_ = interpolator.interpolate(strikes_.begin(),
                                                  strikes_.end(),
                                                  vols_.begin());
    }

    template <class Interpolator>
    InterpolatedSmileSection<Interpolator>::InterpolatedSmileSection(
        const Date& d,
        std::vector<Rate> strikes,
        const std::vector<Real>& stdDevs,
        Real atmLevel,
        const DayCounter& dc,
        const Interpolator& interpolator,
        const Date& referenceDate,
        const VolatilityType type,
        const Real shift)
    : SmileSection(d, dc, referenceDate, type, shift),
      exerciseTimeSquareRoot_(std::sqrt(exerciseTime())), strikes_(std::move(strikes)),
      stdDevHandles_(stdDevs.size()), vols_(stdDevs.size()) {
        //fill dummy handles to allow generic handle-based
        // computations later on
        for (Size i=0; i<stdDevs.size(); ++i)
            stdDevHandles_[i] = Handle<Quote>(ext::shared_ptr<Quote>(new
                SimpleQuote(stdDevs[i])));
        atmLevel_ = Handle<Quote>
           (ext::shared_ptr<Quote>(new SimpleQuote(atmLevel)));
        // check strikes!!!!!!!!!!!!!!!!!!!!
        interpolation_ = interpolator.interpolate(strikes_.begin(),
                                                  strikes_.end(),
                                                  vols_.begin());
    }


    template <class Interpolator>
    inline void InterpolatedSmileSection<Interpolator>::performCalculations()
                                                                      const {
        for (Size i=0; i<stdDevHandles_.size(); ++i)
            vols_[i] = stdDevHandles_[i]->value()/exerciseTimeSquareRoot_;
        interpolation_.update();
    }

    #ifndef __DOXYGEN__
    template <class Interpolator>
    Real InterpolatedSmileSection<Interpolator>::varianceImpl(Real strike) const {
        calculate();
        Real v = interpolation_(strike, true);
        return v*v*exerciseTime();
    }

    template <class Interpolator>
    Real InterpolatedSmileSection<Interpolator>::volatilityImpl(Real strike) const {
        calculate();
        return interpolation_(strike, true);
    }

    template <class Interpolator>
    void InterpolatedSmileSection<Interpolator>::update() {
        LazyObject::update();
        SmileSection::update();
    }

}

#endif
