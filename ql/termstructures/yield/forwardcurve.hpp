/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006, 2007, 2008, 2009 StatPro Italia srl
 Copyright (C) 2009, 2015 Ferdinando Ametrano
 Copyright (C) 2015 Paolo Mazzocchi

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file forwardcurve.hpp
    \brief interpolated forward-rate structure
*/

#ifndef quantlib_forward_curve_hpp
#define quantlib_forward_curve_hpp

#include <ql/termstructures/yield/zeroyieldstructure.hpp>
#include <ql/termstructures/interpolatedcurve.hpp>
#include <ql/math/interpolations/backwardflatinterpolation.hpp>
#include <utility>

namespace QuantLib {

    //! YieldTermStructure based on interpolation of forward rates
    /*! \ingroup yieldtermstructures */
    template <class Interpolator>
    class InterpolatedForwardCurve : public ZeroYieldStructure,
                                     protected InterpolatedCurve<Interpolator> {
      public:
        // constructor
        InterpolatedForwardCurve(
            const std::vector<Date>& dates,
            const std::vector<Rate>& forwards,
            const DayCounter& dayCounter,
            const Calendar& cal = Calendar(),
            const std::vector<Handle<Quote> >& jumps = {},
            const std::vector<Date>& jumpDates = {},
            const Interpolator& interpolator = {});
        InterpolatedForwardCurve(
            const std::vector<Date>& dates,
            const std::vector<Rate>& forwards,
            const DayCounter& dayCounter,
            const Calendar& calendar,
            const Interpolator& interpolator);
        InterpolatedForwardCurve(
            const std::vector<Date>& dates,
            const std::vector<Rate>& forwards,
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
        const std::vector<Rate>& forwards() const;
        std::vector<std::pair<Date, Real> > nodes() const;
        //@}

      protected:
        explicit InterpolatedForwardCurve(
            const DayCounter&,
            const Interpolator& interpolator = {});
        InterpolatedForwardCurve(
            const Date& referenceDate,
            const DayCounter&,
            const std::vector<Handle<Quote> >& jumps = {},
            const std::vector<Date>& jumpDates = {},
            const Interpolator& interpolator = {});
        InterpolatedForwardCurve(
            Natural settlementDays,
            const Calendar&,
            const DayCounter&,
            const std::vector<Handle<Quote> >& jumps = {},
            const std::vector<Date>& jumpDates = {},
            const Interpolator& interpolator = {});

        //! \name ZeroYieldStructure implementation
        //@{
        Rate zeroYieldImpl(Time t) const override;
        //@}
        mutable std::vector<Date> dates_;
      private:
        void initialize();
    };

    //! Term structure based on flat interpolation of forward rates
    /*! \ingroup yieldtermstructures */

    typedef InterpolatedForwardCurve<BackwardFlat> ForwardCurve;


    // inline definitions

    template <class T>
    inline Date InterpolatedForwardCurve<T>::maxDate() const {
        if (this->maxDate_ != Date())
           return this->maxDate_;
        return dates_.back();
    }

    template <class T>
    inline const std::vector<Time>&
    InterpolatedForwardCurve<T>::times() const {
        return this->times_;
    }

    template <class T>
    inline const std::vector<Date>&
    InterpolatedForwardCurve<T>::dates() const {
        return dates_;
    }

    template <class T>
    inline const std::vector<Real>&
    InterpolatedForwardCurve<T>::data() const {
        return this->data_;
    }

    template <class T>
    inline const std::vector<Rate>&
    InterpolatedForwardCurve<T>::forwards() const {
        return this->data_;
    }

    template <class T>
    inline std::vector<std::pair<Date, Real> >
    InterpolatedForwardCurve<T>::nodes() const {
        std::vector<std::pair<Date, Real> > results(dates_.size());
        for (Size i=0; i<dates_.size(); ++i)
            results[i] = std::make_pair(dates_[i], this->data_[i]);
        return results;
    }

    #ifndef __DOXYGEN__

    // template definitions

    template <class T>
    Rate InterpolatedForwardCurve<T>::zeroYieldImpl(Time t) const {
        if (t == 0.0)
            return this->interpolation_(t, true);

        const Time maxTime = this->times_.back();
        Real integral;
        if (t <= maxTime) {
            integral = this->interpolation_.primitive(t, true);
        } else {
            // flat fwd extrapolation
            integral = this->interpolation_.primitive(maxTime, true)
                     + this->data_.back()*(t - maxTime);
        }
        return integral/t;
    }

    template <class T>
    InterpolatedForwardCurve<T>::InterpolatedForwardCurve(
                                    const DayCounter& dayCounter,
                                    const T& interpolator)
    : ZeroYieldStructure(dayCounter), InterpolatedCurve<T>(interpolator) {}

    template <class T>
    InterpolatedForwardCurve<T>::InterpolatedForwardCurve(
                                    const Date& referenceDate,
                                    const DayCounter& dayCounter,
                                    const std::vector<Handle<Quote> >& jumps,
                                    const std::vector<Date>& jumpDates,
                                    const T& interpolator)
    : ZeroYieldStructure(referenceDate, Calendar(), dayCounter, jumps, jumpDates),
      InterpolatedCurve<T>(interpolator) {}

    template <class T>
    InterpolatedForwardCurve<T>::InterpolatedForwardCurve(
                                    Natural settlementDays,
                                    const Calendar& calendar,
                                    const DayCounter& dayCounter,
                                    const std::vector<Handle<Quote> >& jumps,
                                    const std::vector<Date>& jumpDates,
                                    const T& interpolator)
    : ZeroYieldStructure(settlementDays, calendar, dayCounter, jumps, jumpDates),
      InterpolatedCurve<T>(interpolator) {}

    template <class T>
    InterpolatedForwardCurve<T>::InterpolatedForwardCurve(
                                    const std::vector<Date>& dates,
                                    const std::vector<Rate>& forwards,
                                    const DayCounter& dayCounter,
                                    const Calendar& calendar,
                                    const std::vector<Handle<Quote> >& jumps,
                                    const std::vector<Date>& jumpDates,
                                    const T& interpolator)
    : ZeroYieldStructure(dates.at(0), calendar, dayCounter, jumps, jumpDates),
      InterpolatedCurve<T>(std::vector<Time>(), forwards, interpolator),
      dates_(dates)
    {
        initialize();
    }

    template <class T>
    InterpolatedForwardCurve<T>::InterpolatedForwardCurve(
            const std::vector<Date>& dates,
            const std::vector<Rate>& forwards,
            const DayCounter& dayCounter,
            const Calendar& calendar,
            const T& interpolator)
    : ZeroYieldStructure(dates.at(0), calendar, dayCounter),
      InterpolatedCurve<T>(std::vector<Time>(), forwards, interpolator),
      dates_(dates)
    {
        initialize();
    }

    template <class T>
    InterpolatedForwardCurve<T>::InterpolatedForwardCurve(
            const std::vector<Date>& dates,
            const std::vector<Rate>& forwards,
            const DayCounter& dayCounter,
            const T& interpolator)
    : ZeroYieldStructure(dates.at(0), Calendar(), dayCounter),
      InterpolatedCurve<T>(std::vector<Time>(), forwards, interpolator),
      dates_(dates)
    {
        initialize();
    }

    #endif

    template <class T>
    void InterpolatedForwardCurve<T>::initialize()
    {
        QL_REQUIRE(dates_.size() >= T::requiredPoints,
                   "not enough input dates given");
        QL_REQUIRE(this->data_.size() == dates_.size(),
                   "dates/data count mismatch");

        this->setupTimes(dates_, dates_[0], dayCounter());
        this->setupInterpolation();
        this->interpolation_.update();
    }

}

#endif
