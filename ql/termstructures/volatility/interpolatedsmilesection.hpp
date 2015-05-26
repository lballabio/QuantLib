/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 François du Vignaud

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

#include <ql/termstructure.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>

namespace QuantLib {

    template<class Interpolator>
    class InterpolatedSmileSection : public SmileSection,
                                     public LazyObject {
      public:
        InterpolatedSmileSection(
                           Time expiryTime,
                           const std::vector<Rate>& strikes,
                           const std::vector<Handle<Quote> >& stdDevHandles,
                           const Handle<Quote>& atmLevel,
                           const Interpolator& interpolator = Interpolator(),
                           const DayCounter& dc = Actual365Fixed(),
                           const Real shift = 0.0);
        InterpolatedSmileSection(
                           Time expiryTime,
                           const std::vector<Rate>& strikes,
                           const std::vector<Real>& stdDevs,
                           Real atmLevel,
                           const Interpolator& interpolator = Interpolator(),
                           const DayCounter& dc = Actual365Fixed(),
                           const Real shift = 0.0);
        InterpolatedSmileSection(
                           const Date& d,
                           const std::vector<Rate>& strikes,
                           const std::vector<Handle<Quote> >& stdDevHandles,
                           const Handle<Quote>& atmLevel,
                           const DayCounter& dc = Actual365Fixed(),
                           const Interpolator& interpolator = Interpolator(),
                           const Date& referenceDate = Date(),
                           const Real shift = 0.0);
        InterpolatedSmileSection(
                           const Date& d,
                           const std::vector<Rate>& strikes,
                           const std::vector<Real>& stdDevs,
                           Real atmLevel,
                           const DayCounter& dc = Actual365Fixed(),
                           const Interpolator& interpolator = Interpolator(),
                           const Date& referenceDate = Date(),
                           const Real shift = 0.0);
        void performCalculations() const;
        Real varianceImpl(Rate strike) const;
        Volatility volatilityImpl(Rate strike) const;
        Real minStrike () const { return strikes_.front(); }
        Real maxStrike () const { return strikes_.back(); }
        virtual Real atmLevel() const { return atmLevel_->value(); }
        void update();
      private:
        Real exerciseTimeSquareRoot_;
        std::vector<Rate> strikes_;
        std::vector<Handle<Quote> > stdDevHandles_;
        Handle<Quote> atmLevel_;
        mutable std::vector<Volatility> vols_;
        mutable Interpolation interpolation_;
    };


    template<class Interpolator>
    InterpolatedSmileSection<Interpolator>::InterpolatedSmileSection(
                               Time timeToExpiry,
                               const std::vector<Rate>& strikes,
                               const std::vector<Handle<Quote> >& stdDevHandles,
                               const Handle<Quote>& atmLevel,
                               const Interpolator& interpolator,
                               const DayCounter& dc,
                               const Real shift)
    : SmileSection(timeToExpiry, dc, ShiftedLognormal, shift),
      exerciseTimeSquareRoot_(std::sqrt(exerciseTime())), strikes_(strikes),
      stdDevHandles_(stdDevHandles), atmLevel_(atmLevel),
      vols_(stdDevHandles.size())
    {
        for (Size i=0; i<stdDevHandles_.size(); ++i)
            LazyObject::registerWith(stdDevHandles_[i]);
        LazyObject::registerWith(atmLevel_);
        // check strikes!!!!!!!!!!!!!!!!!!!!
        interpolation_ = interpolator.interpolate(strikes_.begin(),
                                                  strikes_.end(),
                                                  vols_.begin());
    }

    template<class Interpolator>
    InterpolatedSmileSection<Interpolator>::InterpolatedSmileSection(
                                Time timeToExpiry,
                                const std::vector<Rate>& strikes,
                                const std::vector<Real>& stdDevs,
                                Real atmLevel,
                                const Interpolator& interpolator,
                                const DayCounter& dc,
                                const Real shift)
    : SmileSection(timeToExpiry, dc, ShiftedLognormal, shift),
      exerciseTimeSquareRoot_(std::sqrt(exerciseTime())), strikes_(strikes),
      stdDevHandles_(stdDevs.size()), vols_(stdDevs.size())
    {
        // fill dummy handles to allow generic handle-based
        // computations later on
        for (Size i=0; i<stdDevs.size(); ++i)
            stdDevHandles_[i] = Handle<Quote>(boost::shared_ptr<Quote>(new
                SimpleQuote(stdDevs[i])));
        atmLevel_ = Handle<Quote>
           (boost::shared_ptr<Quote>(new SimpleQuote(atmLevel)));
        // check strikes!!!!!!!!!!!!!!!!!!!!
        interpolation_ = interpolator.interpolate(strikes_.begin(),
                                                  strikes_.end(),
                                                  vols_.begin());
    }

    template <class Interpolator>
    InterpolatedSmileSection<Interpolator>::InterpolatedSmileSection(
                           const Date& d,
                           const std::vector<Rate>& strikes,
                           const std::vector<Handle<Quote> >& stdDevHandles,
                           const Handle<Quote>& atmLevel,
                           const DayCounter& dc,
                           const Interpolator& interpolator,
                           const Date& referenceDate,
                           const Real shift)
    : SmileSection(d, dc, referenceDate, ShiftedLognormal, shift),
      exerciseTimeSquareRoot_(std::sqrt(exerciseTime())), strikes_(strikes),
      stdDevHandles_(stdDevHandles), atmLevel_(atmLevel), vols_(stdDevHandles.size())
    {
        for (Size i=0; i<stdDevHandles_.size(); ++i)
            LazyObject::registerWith(stdDevHandles_[i]);
        LazyObject::registerWith(atmLevel_);
        // check strikes!!!!!!!!!!!!!!!!!!!!
        interpolation_ = interpolator.interpolate(strikes_.begin(),
                                                  strikes_.end(),
                                                  vols_.begin());
    }

    template <class Interpolator>
    InterpolatedSmileSection<Interpolator>::InterpolatedSmileSection(
                           const Date& d,
                           const std::vector<Rate>& strikes,
                           const std::vector<Real>& stdDevs,
                           Real atmLevel,
                           const DayCounter& dc,
                           const Interpolator& interpolator,
                           const Date& referenceDate,
                           const Real shift)
    : SmileSection(d, dc, referenceDate, ShiftedLognormal, shift),
      exerciseTimeSquareRoot_(std::sqrt(exerciseTime())), strikes_(strikes),
      stdDevHandles_(stdDevs.size()), vols_(stdDevs.size())
    {
        //fill dummy handles to allow generic handle-based
        // computations later on
        for (Size i=0; i<stdDevs.size(); ++i)
            stdDevHandles_[i] = Handle<Quote>(boost::shared_ptr<Quote>(new
                SimpleQuote(stdDevs[i])));
        atmLevel_ = Handle<Quote>
           (boost::shared_ptr<Quote>(new SimpleQuote(atmLevel)));
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
    #endif

}

#endif
