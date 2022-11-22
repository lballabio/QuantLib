/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Chris Kenyon
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008, 2009 StatPro Italia srl

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
#include <ql/termstructures/interpolatedcurve.hpp>
#include <utility>

namespace QuantLib {

    //! DefaultProbabilityTermStructure based on interpolation of default densities
    /*! \ingroup defaultprobabilitytermstructures */
    template <class Interpolator>
    class InterpolatedDefaultDensityCurve
        : public DefaultDensityStructure,
          protected InterpolatedCurve<Interpolator> {
      public:
        InterpolatedDefaultDensityCurve(
            const std::vector<Date>& dates,
            const std::vector<Real>& densities,
            const DayCounter& dayCounter,
            const Calendar& calendar = Calendar(),
            const std::vector<Handle<Quote> >& jumps = {},
            const std::vector<Date>& jumpDates = {},
            const Interpolator& interpolator = {});
        InterpolatedDefaultDensityCurve(
            const std::vector<Date>& dates,
            const std::vector<Real>& densities,
            const DayCounter& dayCounter,
            const Calendar& calendar,
            const Interpolator& interpolator);
        InterpolatedDefaultDensityCurve(
            const std::vector<Date>& dates,
            const std::vector<Real>& densities,
            const DayCounter& dayCounter,
            const Interpolator& interpolator);
        //! \name TermStructure interface
        //@{
        Date maxDate() const override;
        //@}
        //! \name other inspectors
        //@{
        const std::vector<Time>& times() const;
        const std::vector<Date>& dates() const;
        const std::vector<Real>& data() const;
        const std::vector<Real>& defaultDensities() const;
        std::vector<std::pair<Date, Real> > nodes() const;
        //@}
      protected:
        InterpolatedDefaultDensityCurve(
            const DayCounter&,
            const std::vector<Handle<Quote> >& jumps = {},
            const std::vector<Date>& jumpDates = {},
            const Interpolator& interpolator = {});
        InterpolatedDefaultDensityCurve(
            const Date& referenceDate,
            const DayCounter&,
            const std::vector<Handle<Quote> >& jumps = {},
            const std::vector<Date>& jumpDates = {},
            const Interpolator& interpolator = {});
        InterpolatedDefaultDensityCurve(
            Natural settlementDays,
            const Calendar&,
            const DayCounter&,
            const std::vector<Handle<Quote> >& jumps = {},
            const std::vector<Date>& jumpDates = {},
            const Interpolator& interpolator = {});
        //! \name DefaultDensityStructure implementation
        //@{
        Real defaultDensityImpl(Time) const override;
        Probability survivalProbabilityImpl(Time) const override;
        //@}
        mutable std::vector<Date> dates_;
      private:
        void initialize(const std::vector<Date>& dates,
                        const std::vector<Real>& densities,
                        const DayCounter& dayCounter);
    };

    // inline definitions

    template <class T>
    inline Date InterpolatedDefaultDensityCurve<T>::maxDate() const {
        return dates_.back();
    }

    template <class T>
    inline const std::vector<Time>&
    InterpolatedDefaultDensityCurve<T>::times() const {
        return this->times_;
    }

    template <class T>
    inline const std::vector<Date>&
    InterpolatedDefaultDensityCurve<T>::dates() const {
        return dates_;
    }

    template <class T>
    inline const std::vector<Real>&
    InterpolatedDefaultDensityCurve<T>::data() const {
        return this->data_;
    }

    template <class T>
    inline const std::vector<Real>&
    InterpolatedDefaultDensityCurve<T>::defaultDensities() const {
        return this->data_;
    }

    template <class T>
    inline std::vector<std::pair<Date, Real> >
    InterpolatedDefaultDensityCurve<T>::nodes() const {
        std::vector<std::pair<Date, Real> > results(dates_.size());
        for (Size i=0; i<dates_.size(); ++i)
            results[i] = std::make_pair(dates_[i], this->data_[i]);
        return results;
    }

    #ifndef __DOXYGEN__

    // template definitions

    template <class T>
    Real InterpolatedDefaultDensityCurve<T>::defaultDensityImpl(Time t) const {
        if (t <= this->times_.back())
            return this->interpolation_(t, true);

        // flat default density extrapolation
        return this->data_.back();
    }

    template <class T>
    Probability
    InterpolatedDefaultDensityCurve<T>::survivalProbabilityImpl(Time t) const {
        if (t == 0.0)
            return 1.0;

        Real integral = 0.0;
        if (t <= this->times_.back()) {
            integral = this->interpolation_.primitive(t, true);
        } else {
            // flat default density extrapolation
            integral = this->interpolation_.primitive(this->times_.back(), true)
                     + this->data_.back()*(t - this->times_.back());
        }
        Probability P = 1.0 - integral;
        // QL_ENSURE(P >= 0.0, "negative survival probability");
        return std::max<Real>(P, 0.0);
    }

    template <class T>
    InterpolatedDefaultDensityCurve<T>::InterpolatedDefaultDensityCurve(
                                    const DayCounter& dayCounter,
                                    const std::vector<Handle<Quote> >& jumps,
                                    const std::vector<Date>& jumpDates,
                                    const T& interpolator)
    : DefaultDensityStructure(dayCounter, jumps, jumpDates),
      InterpolatedCurve<T>(interpolator) {}

    template <class T>
    InterpolatedDefaultDensityCurve<T>::InterpolatedDefaultDensityCurve(
                                    const Date& referenceDate,
                                    const DayCounter& dayCounter,
                                    const std::vector<Handle<Quote> >& jumps,
                                    const std::vector<Date>& jumpDates,
                                    const T& interpolator)
    : DefaultDensityStructure(referenceDate, Calendar(), dayCounter, jumps, jumpDates),
      InterpolatedCurve<T>(interpolator) {}

    template <class T>
    InterpolatedDefaultDensityCurve<T>::InterpolatedDefaultDensityCurve(
                                    Natural settlementDays,
                                    const Calendar& calendar,
                                    const DayCounter& dayCounter,
                                    const std::vector<Handle<Quote> >& jumps,
                                    const std::vector<Date>& jumpDates,
                                    const T& interpolator)
    : DefaultDensityStructure(settlementDays, calendar, dayCounter, jumps, jumpDates),
      InterpolatedCurve<T>(interpolator) {}

    template <class T>
    InterpolatedDefaultDensityCurve<T>::InterpolatedDefaultDensityCurve(
                                    const std::vector<Date>& dates,
                                    const std::vector<Real>& densities,
                                    const DayCounter& dayCounter,
                                    const Calendar& calendar,
                                    const std::vector<Handle<Quote> >& jumps,
                                    const std::vector<Date>& jumpDates,
                                    const T& interpolator)
    : DefaultDensityStructure(dates.at(0), calendar, dayCounter, jumps, jumpDates),
      InterpolatedCurve<T>(std::vector<Time>(), densities, interpolator),
      dates_(dates)
    {
        initialize(dates, densities, dayCounter);
    }

    template <class T>
    InterpolatedDefaultDensityCurve<T>::InterpolatedDefaultDensityCurve(
            const std::vector<Date>& dates,
            const std::vector<Real>& densities,
            const DayCounter& dayCounter,
            const Calendar& calendar,
            const T& interpolator)
    : DefaultDensityStructure(dates.at(0), calendar, dayCounter),
      InterpolatedCurve<T>(std::vector<Time>(), densities, interpolator),
      dates_(dates)
    {
        initialize(dates, densities, dayCounter);
    }

    template <class T>
    InterpolatedDefaultDensityCurve<T>::InterpolatedDefaultDensityCurve(
            const std::vector<Date>& dates,
            const std::vector<Real>& densities,
            const DayCounter& dayCounter,
            const T& interpolator)
    : DefaultDensityStructure(dates.at(0), Calendar(), dayCounter),
      InterpolatedCurve<T>(std::vector<Time>(), densities, interpolator),
      dates_(dates)
    {
        initialize(dates, densities, dayCounter);
    }


    #endif


    template <class T>
    void InterpolatedDefaultDensityCurve<T>::initialize(
                                    const std::vector<Date>& dates,
                                    const std::vector<Real>& densities,
                                    const DayCounter& dayCounter)
    {
        QL_REQUIRE(dates_.size() >= T::requiredPoints,
                   "not enough input dates given");
        QL_REQUIRE(this->data_.size() == dates_.size(),
                   "dates/data count mismatch");

        this->times_.resize(dates_.size());
        this->times_[0] = 0.0;
        for (Size i=1; i<dates_.size(); ++i) {
            QL_REQUIRE(dates_[i] > dates_[i-1],
                       "invalid date (" << dates_[i] << ", vs "
                       << dates_[i-1] << ")");
            this->times_[i] = dayCounter.yearFraction(dates_[0], dates_[i]);
            QL_REQUIRE(!close(this->times_[i],this->times_[i-1]),
                       "two dates correspond to the same time "
                       "under this curve's day count convention");
            QL_REQUIRE(this->data_[i] >= 0.0, "negative default density");
        }

        this->interpolation_ =
            this->interpolator_.interpolate(this->times_.begin(),
                                            this->times_.end(),
                                            this->data_.begin());
        this->interpolation_.update();
    }

}

#endif
