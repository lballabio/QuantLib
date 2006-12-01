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
#include <ql/quote.hpp>
#include <ql/Math/sabrinterpolation.hpp>
#include <ql/Volatilities/smilesection.hpp> 

namespace QuantLib {

    template<class Interpolator = SABR>
    class InterpolatedSmileSection : public SmileSectionInterface,
                                     public LazyObject {
      public:
        InterpolatedSmileSection(
                           Time expiryTime,
                           const std::vector<Rate>& strikes,
                           const std::vector<Handle<Quote> >& volHandles,
                           const Interpolator& interpolator = Interpolator(),
                           const DayCounter& dc = Actual365Fixed());
        InterpolatedSmileSection(
                           Time expiryTime,
                           const std::vector<Rate>& strikes,
                           const std::vector<Volatility>& vols,
                           const Interpolator& interpolator = Interpolator(),
                           const DayCounter& dc = Actual365Fixed());

        InterpolatedSmileSection(
                           const Date& d,
                           const std::vector<Rate>& strikes,
                           const std::vector<Handle<Quote> >& volHandles,
                           const DayCounter& dc = Actual365Fixed(),
                           const Interpolator& interpolator = Interpolator(),
                           const Date& referenceDate = Date());
        InterpolatedSmileSection(
                           const Date& d,
                           const std::vector<Rate>& strikes,
                           const std::vector<Volatility>& vols,
                           const DayCounter& dc = Actual365Fixed(),
                           const Interpolator& interpolator = Interpolator(),
                           const Date& referenceDate = Date());
        void performCalculations() const;
        Real variance(Rate strike) const;
        Volatility volatility(Rate strike) const;
      private:
        std::vector<Rate> strikes_;
        std::vector<Handle<Quote> > volHandles_;
        mutable std::vector<Volatility> vols_;
        Interpolation interpolation_;
    };


    template<class Interpolator>
    InterpolatedSmileSection<Interpolator>::InterpolatedSmileSection(
                               Time timeToExpiry,
                               const std::vector<Rate>& strikes,
                               const std::vector<Handle<Quote> >& volHandles,
                               const Interpolator& interpolator,
                               const DayCounter& dc)
    : SmileSectionInterface(timeToExpiry, dc), strikes_(strikes),
      volHandles_(volHandles), vols_(volHandles.size())
    {
        for (Size i=0; i<volHandles_.size(); ++i)
            registerWith(volHandles_[i]);

        // check strikes!!!!!!!!!!!!!!!!!!!!
        interpolation_ = interpolator.interpolate(strikes_.begin(),
                                                  strikes_.end(),
                                                  vols_.begin());
    }

    template<class Interpolator>
    InterpolatedSmileSection<Interpolator>::InterpolatedSmileSection(
                                Time timeToExpiry,
                                const std::vector<Rate>& strikes,
                                const std::vector<Volatility>& vols,
                                const Interpolator& interpolator,
                                const DayCounter& dc)
    : SmileSectionInterface(timeToExpiry, dc), strikes_(strikes),
      volHandles_(vols.size()), vols_(vols)
    {
        // fill dummy handles to allow generic handle-based
        // computations later on
        for (Size i=0; i<vols.size(); ++i)
            volHandles_[i] = Handle<Quote>(boost::shared_ptr<Quote>(
                new SimpleQuote(vols_[i])));

        // check strikes!!!!!!!!!!!!!!!!!!!!
        interpolation_ = interpolator.interpolate(strikes_.begin(),
                                                  strikes_.end(),
                                                  vols_.begin());
    }

    template <class Interpolator>
    InterpolatedSmileSection<Interpolator>::InterpolatedSmileSection(
                           const Date& d,
                           const std::vector<Rate>& strikes,
                           const std::vector<Handle<Quote> >& volHandles,
                           const DayCounter& dc,
                           const Interpolator& interpolator,
                           const Date& referenceDate)
    : SmileSectionInterface(d, dc, referenceDate), strikes_(strikes),
      volHandles_(volHandles), vols_(volHandles.size())
    {
        for (Size i=0; i<volHandles_.size(); ++i)
            registerWith(volHandles_[i]);

        // check strikes!!!!!!!!!!!!!!!!!!!!
        interpolation_ = interpolator.interpolate(strikes_.begin(),
                                                  strikes_.end(),
                                                  vols_.begin());
    }

    template <class Interpolator>
    InterpolatedSmileSection<Interpolator>::InterpolatedSmileSection(
                           const Date& d,
                           const std::vector<Rate>& strikes,
                           const std::vector<Volatility>& vols,
                           const DayCounter& dc,
                           const Interpolator& interpolator,
                           const Date& referenceDate)
    : SmileSectionInterface(d, dc, referenceDate), strikes_(strikes),
      volHandles_(vols.size()), vols_(vols)
    {
        // fill dummy handles to allow generic handle-based
        // computations later on
        for (Size i=0; i<vols.size(); ++i)
            volHandles_[i] = Handle<Quote>(boost::shared_ptr<Quote>(
                new SimpleQuote(vols_[i])));

        // check strikes!!!!!!!!!!!!!!!!!!!!
        interpolation_ = interpolator.interpolate(strikes_.begin(),
                                                  strikes_.end(),
                                                  vols_.begin());
    }

    template <class Interpolator>
    inline void InterpolatedSmileSection<Interpolator>::performCalculations()
                                                                      const {
        for (Size i=0; i<volHandles_.size(); ++i)
            vols_[i] = volHandles_[i]->value();
    };

    template <class Interpolator>
    Real InterpolatedSmileSection<Interpolator>::variance(Real strike) const {
        calculate();
        Real v = interpolation_(strike, true);
        return v*v*exerciseTime_;
    }

    template <class Interpolator>
    Real InterpolatedSmileSection<Interpolator>::volatility(Real strike) const {
        calculate();
        return interpolation_(strike, true);
    }
}

#endif
