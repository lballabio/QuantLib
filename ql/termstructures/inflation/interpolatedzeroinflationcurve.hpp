/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2008 Chris Kenyon
 Copyright (C) 2009 StatPro Italia srl

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

/*! \file interpolatedzeroinflationcurve.hpp
    \brief Inflation term structure based on the interpolation of zero rates.
*/

#ifndef quantlib_interpolated_zeroinflationcurve_hpp
#define quantlib_interpolated_zeroinflationcurve_hpp

#include <ql/termstructures/inflationtermstructure.hpp>
#include <ql/termstructures/interpolatedcurve.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/comparison.hpp>
#include <utility>

namespace QuantLib {

    //! Inflation term structure based on the interpolation of zero rates.
    /*! \ingroup inflationtermstructures */
    template<class Interpolator>
    class InterpolatedZeroInflationCurve
        : public ZeroInflationTermStructure,
          protected InterpolatedCurve<Interpolator> {
      public:
        InterpolatedZeroInflationCurve(const Date& referenceDate,
                                       const Calendar& calendar,
                                       const DayCounter& dayCounter,
                                       const Period& lag,
                                       Frequency frequency,
                                       bool indexIsInterpolated,
                                       std::vector<Date> dates,
                                       const std::vector<Rate>& rates,
                                       const Interpolator& interpolator = Interpolator());

        //! \name InflationTermStructure interface
        //@{
        Date baseDate() const override;
        Date maxDate() const override;
        //@}

        //! \name Inspectors
        //@{
        const std::vector<Date>& dates() const;
        const std::vector<Time>& times() const;
        const std::vector<Real>& data() const;
        const std::vector<Rate>& rates() const;
        std::vector<std::pair<Date,Rate> > nodes() const;
        //@}

      protected:
        //! \name ZeroInflationTermStructure Interface
        //@{
        Rate zeroRateImpl(Time t) const override;
        //@}
        mutable std::vector<Date> dates_;

        /*! Protected version for use when descendents don't want to
            (or can't) provide the points for interpolation on
            construction.
        */
        InterpolatedZeroInflationCurve(const Date& referenceDate,
                                       const Calendar& calendar,
                                       const DayCounter& dayCounter,
                                       const Period& lag,
                                       Frequency frequency,
                                       bool indexIsInterpolated,
                                       Rate baseZeroRate,
                                       const Interpolator &interpolator
                                                            = Interpolator());
    };

    typedef InterpolatedZeroInflationCurve<Linear> ZeroInflationCurve;



    // template definitions

    template <class Interpolator>
    InterpolatedZeroInflationCurve<Interpolator>::InterpolatedZeroInflationCurve(
        const Date& referenceDate,
        const Calendar& calendar,
        const DayCounter& dayCounter,
        const Period& lag,
        Frequency frequency,
        bool indexIsInterpolated,
        std::vector<Date> dates,
        const std::vector<Rate>& rates,
        const Interpolator& interpolator)
    : ZeroInflationTermStructure(
          referenceDate, calendar, dayCounter, rates[0], lag, frequency, indexIsInterpolated),
      InterpolatedCurve<Interpolator>(std::vector<Time>(), rates, interpolator),
      dates_(std::move(dates)) {

        QL_REQUIRE(dates_.size() > 1, "too few dates: " << dates_.size());

        // check that the data starts from the beginning,
        // i.e. referenceDate - lag, at least must be in the relevant
        // period
        std::pair<Date, Date> lim =
            inflationPeriod(referenceDate - this->observationLag(), frequency);
        QL_REQUIRE(lim.first <= dates_[0] && dates_[0] <= lim.second,
                   "first data date is not in base period, date: "
                       << dates_[0] << " not within [" << lim.first << "," << lim.second << "]");

        // by convention, if the index is not interpolated we pull all the dates
        // back to the start of their inflationPeriods
        // otherwise the time calculations will be inconsistent
        if (!indexIsInterpolated_) {
            for (auto& date : dates_) {
                date = inflationPeriod(date, frequency).first;
            }
        }

        QL_REQUIRE(this->data_.size() == dates_.size(),
                   "indices/dates count mismatch: " << this->data_.size() << " vs "
                                                    << dates_.size());

        this->times_.resize(dates_.size());
        this->times_[0] = timeFromReference(dates_[0]);
        for (Size i = 1; i < dates_.size(); i++) {
            QL_REQUIRE(dates_[i] > dates_[i - 1], "dates not sorted");

            // but must be greater than -1
            QL_REQUIRE(this->data_[i] > -1.0, "zero inflation data < -100 %");

            // this can be negative
            this->times_[i] = timeFromReference(dates_[i]);
            QL_REQUIRE(!close(this->times_[i], this->times_[i - 1]),
                       "two dates correspond to the same time "
                       "under this curve's day count convention");
        }

        this->interpolation_ = this->interpolator_.interpolate(
            this->times_.begin(), this->times_.end(), this->data_.begin());
        this->interpolation_.update();
    }

    template <class Interpolator>
    InterpolatedZeroInflationCurve<Interpolator>::
    InterpolatedZeroInflationCurve(const Date& referenceDate,
                                   const Calendar& calendar,
                                   const DayCounter& dayCounter,
                                   const Period& lag,
                                   Frequency frequency,
                                   bool indexIsInterpolated,
                                   Rate baseZeroRate,
                                   const Interpolator& interpolator)
    :  ZeroInflationTermStructure(referenceDate, calendar, dayCounter, baseZeroRate,
                                  lag, frequency, indexIsInterpolated),
       InterpolatedCurve<Interpolator>(interpolator) {
    }


    template <class T>
    Date InterpolatedZeroInflationCurve<T>::baseDate() const {
        // if indexIsInterpolated we fixed the dates in the constructor
        return dates_.front();
    }

    template <class T>
    Date InterpolatedZeroInflationCurve<T>::maxDate() const {
        Date d;
        if (indexIsInterpolated()) {
            d = dates_.back();
        } else {
            d = inflationPeriod(dates_.back(), frequency()).second;
        }
        return d;
    }


    template <class T>
    inline Rate InterpolatedZeroInflationCurve<T>::zeroRateImpl(Time t) const {
        return this->interpolation_(t, true);
    }

    template <class T>
    inline const std::vector<Time>&
    InterpolatedZeroInflationCurve<T>::times() const {
        return this->times_;
    }

    template <class T>
    inline const std::vector<Date>&
    InterpolatedZeroInflationCurve<T>::dates() const {
        return dates_;
    }

    template <class T>
    inline const std::vector<Rate>&
    InterpolatedZeroInflationCurve<T>::rates() const {
        return this->data_;
    }

    template <class T>
    inline const std::vector<Real>&
    InterpolatedZeroInflationCurve<T>::data() const {
        return this->data_;
    }

    template <class T>
    inline std::vector<std::pair<Date,Rate> >
    InterpolatedZeroInflationCurve<T>::nodes() const {
        std::vector<std::pair<Date,Rate> > results(dates_.size());
        for (Size i=0; i<dates_.size(); ++i)
            results[i] = std::make_pair(dates_[i],this->data_[i]);
        return results;
    }

}


#endif
