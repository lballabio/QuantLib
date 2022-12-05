/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008 StatPro Italia srl
 Copyright (C) 2009, 2015 Ferdinando Ametrano
 Copyright (C) 2015 Paolo Mazzocchi

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

/*! \file zerocurve.hpp
    \brief interpolated zero-rates structure
*/

#ifndef quantlib_zero_curve_hpp
#define quantlib_zero_curve_hpp

#include <ql/termstructures/yield/zeroyieldstructure.hpp>
#include <ql/termstructures/interpolatedcurve.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/interestrate.hpp>
#include <ql/math/comparison.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <utility>

namespace QuantLib {

    //! YieldTermStructure based on interpolation of zero rates
    /*! \ingroup yieldtermstructures */
    template <class Interpolator>
    class InterpolatedZeroCurve : public ZeroYieldStructure,
                                  protected InterpolatedCurve<Interpolator> {
      public:
        // constructor
        InterpolatedZeroCurve(
            const std::vector<Date>& dates,
            const std::vector<Rate>& yields,
            const DayCounter& dayCounter,
            const Calendar& calendar = Calendar(),
            const std::vector<Handle<Quote> >& jumps = {},
            const std::vector<Date>& jumpDates = {},
            const Interpolator& interpolator = {},
            Compounding compounding = Continuous,
            Frequency frequency = Annual);
        InterpolatedZeroCurve(
            const std::vector<Date>& dates,
            const std::vector<Rate>& yields,
            const DayCounter& dayCounter,
            const Calendar& calendar,
            const Interpolator& interpolator,
            Compounding compounding = Continuous,
            Frequency frequency = Annual);
        InterpolatedZeroCurve(
            const std::vector<Date>& dates,
            const std::vector<Rate>& yields,
            const DayCounter& dayCounter,
            const Interpolator& interpolator,
            Compounding compounding = Continuous,
            Frequency frequency = Annual);
        //! \name TermStructure interface
        //@{
        Date maxDate() const override;
        //@}
        //! \name other inspectors
        //@{
        const std::vector<Time>& times() const;
        const std::vector<Date>& dates() const;
        const std::vector<Real>& data() const;
        const std::vector<Rate>& zeroRates() const;
        std::vector<std::pair<Date, Real> > nodes() const;
        //@}

      protected:
        explicit InterpolatedZeroCurve(
            const DayCounter&,
            const Interpolator& interpolator = {});
        InterpolatedZeroCurve(
            const Date& referenceDate,
            const DayCounter&,
            const std::vector<Handle<Quote> >& jumps = {},
            const std::vector<Date>& jumpDates = {},
            const Interpolator& interpolator = {});
        InterpolatedZeroCurve(
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
        void initialize(const Compounding& compounding, const Frequency& frequency);
    };

    //! Term structure based on linear interpolation of zero yields
    /*! \ingroup yieldtermstructures */
    typedef InterpolatedZeroCurve<Linear> ZeroCurve;


    // inline definitions

    template <class T>
    inline Date InterpolatedZeroCurve<T>::maxDate() const {
        if (this->maxDate_ != Date())
           return this->maxDate_;
        return dates_.back();
    }

    template <class T>
    inline const std::vector<Time>& InterpolatedZeroCurve<T>::times() const {
        return this->times_;
    }

    template <class T>
    inline const std::vector<Date>& InterpolatedZeroCurve<T>::dates() const {
        return dates_;
    }

    template <class T>
    inline const std::vector<Real>&
    InterpolatedZeroCurve<T>::data() const {
        return this->data_;
    }

    template <class T>
    inline const std::vector<Rate>&
    InterpolatedZeroCurve<T>::zeroRates() const {
        return this->data_;
    }

    template <class T>
    inline std::vector<std::pair<Date, Real> >
    InterpolatedZeroCurve<T>::nodes() const {
        std::vector<std::pair<Date, Real> > results(dates_.size());
        for (Size i=0; i<dates_.size(); ++i)
            results[i] = std::make_pair(dates_[i], this->data_[i]);
        return results;
    }

    #ifndef __DOXYGEN__

    // template definitions

    template <class T>
    Rate InterpolatedZeroCurve<T>::zeroYieldImpl(Time t) const {
        if (t <= this->times_.back())
            return this->interpolation_(t, true);

        // flat fwd extrapolation
        Time tMax = this->times_.back();
        Rate zMax = this->data_.back();
        Rate instFwdMax = zMax + tMax * this->interpolation_.derivative(tMax);
        return (zMax * tMax + instFwdMax * (t-tMax)) / t;
    }

    template <class T>
    InterpolatedZeroCurve<T>::InterpolatedZeroCurve(
                                    const DayCounter& dayCounter,
                                    const T& interpolator)
    : ZeroYieldStructure(dayCounter), InterpolatedCurve<T>(interpolator) {}

    template <class T>
    InterpolatedZeroCurve<T>::InterpolatedZeroCurve(
                                    const Date& referenceDate,
                                    const DayCounter& dayCounter,
                                    const std::vector<Handle<Quote> >& jumps,
                                    const std::vector<Date>& jumpDates,
                                    const T& interpolator)
    : ZeroYieldStructure(referenceDate, Calendar(), dayCounter, jumps, jumpDates),
      InterpolatedCurve<T>(interpolator) {}

    template <class T>
    InterpolatedZeroCurve<T>::InterpolatedZeroCurve(
                                    Natural settlementDays,
                                    const Calendar& calendar,
                                    const DayCounter& dayCounter,
                                    const std::vector<Handle<Quote> >& jumps,
                                    const std::vector<Date>& jumpDates,
                                    const T& interpolator)
    : ZeroYieldStructure(settlementDays, calendar, dayCounter, jumps, jumpDates),
      InterpolatedCurve<T>(interpolator) {}

    template <class T>
    InterpolatedZeroCurve<T>::InterpolatedZeroCurve(
                                    const std::vector<Date>& dates,
                                    const std::vector<Rate>& yields,
                                    const DayCounter& dayCounter,
                                    const Calendar& calendar,
                                    const std::vector<Handle<Quote> >& jumps,
                                    const std::vector<Date>& jumpDates,
                                    const T& interpolator,
                                    Compounding compounding,
                                    Frequency frequency)
    : ZeroYieldStructure(dates.at(0), calendar, dayCounter, jumps, jumpDates),
      InterpolatedCurve<T>(std::vector<Time>(), yields, interpolator),
      dates_(dates)
    {
        initialize(compounding,frequency);
    }

    template <class T>
    InterpolatedZeroCurve<T>::InterpolatedZeroCurve(
                                               const std::vector<Date>& dates,
                                               const std::vector<Rate>& yields,
                                               const DayCounter& dayCounter,
                                               const Calendar& calendar,
                                               const T& interpolator,
                                               Compounding compounding,
                                               Frequency frequency)
    : ZeroYieldStructure(dates.at(0), calendar, dayCounter),
      InterpolatedCurve<T>(std::vector<Time>(), yields, interpolator),
      dates_(dates)
    {
        initialize(compounding,frequency);
    }

    template <class T>
    InterpolatedZeroCurve<T>::InterpolatedZeroCurve(
                                               const std::vector<Date>& dates,
                                               const std::vector<Rate>& yields,
                                               const DayCounter& dayCounter,
                                               const T& interpolator,
                                               Compounding compounding,
                                               Frequency frequency)
    : ZeroYieldStructure(dates.at(0), Calendar(), dayCounter),
      InterpolatedCurve<T>(std::vector<Time>(), yields, interpolator),
      dates_(dates)
    {
        initialize(compounding,frequency);
    }

    #endif

    template <class T>
    void InterpolatedZeroCurve<T>::initialize(const Compounding& compounding, 
                                              const Frequency& frequency)
    {
        QL_REQUIRE(dates_.size() >= T::requiredPoints,
                   "not enough input dates given");
        QL_REQUIRE(this->data_.size() == dates_.size(),
                   "dates/data count mismatch");

        this->setupTimes(dates_, dates_[0], dayCounter());

        if (compounding != Continuous) {
            // adjusting zero rates to match continuous compounding

            // The first time is 0.0, so we can't use it.
            // We fall back to about one day.
            Time dt = 1.0/365;
            InterestRate r(this->data_[0], dayCounter(), compounding, frequency);
            this->data_[0] = r.equivalentRate(Continuous, NoFrequency, dt);

            for (Size i=1; i<dates_.size(); ++i) {
                InterestRate r(this->data_[i], dayCounter(), compounding, frequency);
                this->data_[i] = r.equivalentRate(Continuous, NoFrequency, this->times_[i]);
            }
        }

        this->setupInterpolation();
        this->interpolation_.update();
    }

}

#endif
