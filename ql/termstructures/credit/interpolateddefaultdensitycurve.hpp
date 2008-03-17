/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Chris Kenyon
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008 StatPro Italia srl

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

/*! \file interpolateddefaultdensitycurve.hpp
    \brief interpolated default-density term structure
*/

#ifndef quantlib_interpolated_default_density_curve_hpp
#define quantlib_interpolated_default_density_curve_hpp

#include <ql/termstructures/credit/defaultdensitystructure.hpp>
#include <ql/math/interpolation.hpp>
#include <boost/noncopyable.hpp>

namespace QuantLib {

    //! interpolated default-density curve
    template <class Interpolator>
    class InterpolatedDefaultDensityCurve : public DefaultDensityStructure,
                                            public boost::noncopyable {
      public:
        InterpolatedDefaultDensityCurve(const std::vector<Date>& dates,
                                        const std::vector<Real>& densities,
                                        const DayCounter& dayCounter,
                                        const Calendar& calendar = Calendar(),
                                        const Interpolator& interpolator
                                                            = Interpolator());
        //! \name TermStructure interface
        //@{
        Date maxDate() const;
        //@}
        //! \name other inspectors
        //@{
        const std::vector<Time>& times() const;
        const std::vector<Date>& dates() const;
        const std::vector<Real>& defaultDensities() const;
        std::vector<std::pair<Date,Real> > nodes() const;
        //@}
      protected:
        InterpolatedDefaultDensityCurve(const DayCounter&,
                                        const Interpolator& interpolator
                                                            = Interpolator());
        InterpolatedDefaultDensityCurve(const Date& referenceDate,
                                        const DayCounter&,
                                        const Interpolator& interpolator
                                                            = Interpolator());
        InterpolatedDefaultDensityCurve(Natural settlementDays,
                                        const Calendar&,
                                        const DayCounter&,
                                        const Interpolator& interpolator
                                                            = Interpolator());
        Real defaultDensityImpl(Time) const;
        Probability survivalProbabilityImpl(Time) const;
        mutable std::vector<Date> dates_;
        mutable std::vector<Time> times_;
        mutable std::vector<Real> data_;
        mutable Interpolation interpolation_;
        Interpolator interpolator_;
    };


    // template definitions

    template <class T>
    InterpolatedDefaultDensityCurve<T>::InterpolatedDefaultDensityCurve(
                                         const std::vector<Date>& dates,
                                         const std::vector<Real>& densities,
                                         const DayCounter& dayCounter,
                                         const Calendar& calendar,
                                         const T& interpolator)
    : DefaultDensityStructure(dates.front(), calendar, dayCounter),
      dates_(dates), data_(densities), interpolator_(interpolator) {
        QL_REQUIRE(data_.size() == dates_.size(),
                   "dates/densities count mismatch");
        QL_REQUIRE(dates_.size() >= T::requiredPoints,
                   "not enough input dates given");

        times_.resize(dates_.size());
        times_[0] = 0.0;
        for (Size i = 1; i < dates_.size(); i++) {
            QL_REQUIRE(dates_[i] > dates_[i-1],
                       "invalid date (" << dates_[i] << ", vs "
                       << dates_[i-1] << ")");
            QL_REQUIRE(data_[i] > 0.0, "negative hazard rate");
            times_[i] = dayCounter.yearFraction(dates_[0], dates_[i]);
            QL_REQUIRE(!close(times_[i],times_[i-1]),
                       "two dates correspond to the same time "
                       "under this curve's day count convention");
        }

        interpolation_ = interpolator_.interpolate(times_.begin(),
                                                   times_.end(),
                                                   data_.begin());
        interpolation_.update();
    }


    template <class T>
    InterpolatedDefaultDensityCurve<T>::InterpolatedDefaultDensityCurve(
                                                 const DayCounter& dayCounter,
                                                 const T& interpolator)
    : DefaultDensityStructure(dayCounter), interpolator_(interpolator) {}

    template <class T>
    InterpolatedDefaultDensityCurve<T>::InterpolatedDefaultDensityCurve(
                                                 const Date& referenceDate,
                                                 const DayCounter& dayCounter,
                                                 const T& interpolator)
    : DefaultDensityStructure(referenceDate, Calendar(), dayCounter),
      interpolator_(interpolator) {}

    template <class T>
    InterpolatedDefaultDensityCurve<T>::InterpolatedDefaultDensityCurve(
                                                 Natural settlementDays,
                                                 const Calendar& calendar,
                                                 const DayCounter& dayCounter,
                                                 const T& interpolator)
    : DefaultDensityStructure(settlementDays, calendar, dayCounter),
      interpolator_(interpolator) {}


    template <class T>
    Date InterpolatedDefaultDensityCurve<T>::maxDate() const {
        return dates_.back();
    }

    template <class T>
    const std::vector<Time>& InterpolatedDefaultDensityCurve<T>::times() const {
        return times_;
    }

    template <class T>
    const std::vector<Date>& InterpolatedDefaultDensityCurve<T>::dates() const {
        return dates_;
    }

    template <class T>
    const std::vector<Real>&
    InterpolatedDefaultDensityCurve<T>::defaultDensities() const {
        return data_;
    }

    template <class T>
    std::vector<std::pair<Date,Real> >
    InterpolatedDefaultDensityCurve<T>::nodes() const {
        std::vector<std::pair<Date,Real> > results(dates_.size());
        for (Size i=0; i<dates_.size(); ++i)
            results[i] = std::make_pair(dates_[i],data_[i]);
        return results;
    }

    template <class T>
    Real InterpolatedDefaultDensityCurve<T>::defaultDensityImpl(Time t) const {
        if (t <= times_.back()) {
            return interpolation_(t, true);
        } else {
            // flat extrapolation
            return data_.back();
        }
    }

    template <class T>
    Probability InterpolatedDefaultDensityCurve<T>::survivalProbabilityImpl(
                                                               Time t) const {
        Real integral = 0.0;
        if (t <= times_.back()) {
            integral = interpolation_.primitive(t, true);
        } else {
            // flat extrapolation
            integral = interpolation_.primitive(times_.back(), true)
                     + data_.back()*(t - times_.back());
        }
        Probability P = 1.0 - integral;
        QL_ENSURE(P >= 0.0, "negative survival probability");
        return P;
    }

}


#endif
