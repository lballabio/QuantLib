/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2008 Chris Kenyon

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
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/comparison.hpp>
#include <boost/noncopyable.hpp>

namespace QuantLib {

    //! Inflation term structure based on the interpolation of zero rates.
    /*! \ingroup inflationtermstructures */
    template<class Interpolator>
    class InterpolatedZeroInflationCurve : public ZeroInflationTermStructure,
                                           public boost::noncopyable {
      public:
        InterpolatedZeroInflationCurve(const Date& referenceDate,
                                       const Calendar& calendar,
                                       const DayCounter& dayCounter,
                                       const Period& lag,
                                       Frequency frequency,
                                       const Handle<YieldTermStructure>& yTS,
                                       const std::vector<Date>& dates,
                                       const std::vector<Rate>& rates,
                                       const Interpolator &interpolator
                                                            = Interpolator());

        //! \name InflationTermStructure interface
        //@{
        Date baseDate() const;
        Date maxDate() const;
        //@}

        //! \name Inspectors
        //@{
        const std::vector<Date>& dates() const;
        const std::vector<Time>& times() const;
        const std::vector<Rate>& rates() const;
        std::vector<std::pair<Date,Rate> > nodes() const;
        //@}

      protected:
        //! \name ZeroInflationTermStructure Interface
        //@{
        Rate zeroRateImpl(Time t) const;
        //@}
        mutable std::vector<Date> dates_;
        mutable std::vector<Time> times_;
        mutable std::vector<Rate> data_;
        mutable Interpolation interpolation_;
        Interpolator interpolator_;

        /*! Protected version for use when descendents don't want to
            (or can't) provide the points for interpolation on
            construction.
        */
        InterpolatedZeroInflationCurve(const Date& referenceDate,
                                       const Calendar& calendar,
                                       const DayCounter& dayCounter,
                                       const Period& lag,
                                       Frequency frequency,
                                       Rate baseZeroRate,
                                       const Handle<YieldTermStructure>& yTS,
                                       const Interpolator &interpolator
                                                            = Interpolator());
    };

    typedef InterpolatedZeroInflationCurve<Linear> ZeroInflationCurve;



    // template definitions

    template <class Interpolator>
    InterpolatedZeroInflationCurve<Interpolator>::
    InterpolatedZeroInflationCurve(const Date& referenceDate,
                                   const Calendar& calendar,
                                   const DayCounter& dayCounter,
                                   const Period& lag,
                                   Frequency frequency,
                                   const Handle<YieldTermStructure>& yTS,
                                   const std::vector<Date>& dates,
                                   const std::vector<Rate>& rates,
                                   const Interpolator& interpolator)
    : ZeroInflationTermStructure(referenceDate, calendar, dayCounter,
                                 lag, frequency, rates[0], yTS),
      dates_(dates), data_(rates), interpolator_(interpolator) {

        QL_REQUIRE(dates_.size() > 1, "too few dates: " << dates_.size());

        // check that the data starts from the beginning,
        // i.e. referenceDate - lag, at least must be in the relevant
        // period
        std::pair<Date,Date> lim =
            inflationPeriod(yTS->referenceDate() - this->lag(), frequency);
        QL_REQUIRE(lim.first <= dates_[0] && dates_[0] <= lim.second,
                   "first data date is not in base period, date: " << dates_[0]
                   << " not within [" << lim.first << "," << lim.second << "]");

        QL_REQUIRE(data_.size() == dates_.size(),
                   "indices/dates count mismatch: "
                   << data_.size() << " vs " << dates_.size());

        times_.resize(dates_.size());
        times_[0] = timeFromReference(dates_[0]);
        for (Size i = 1; i < dates_.size(); i++) {
            QL_REQUIRE(dates_[i] > dates_[i-1],
                       "dates not sorted");

            // but must be greater than -1
            QL_REQUIRE(data_[i] > -1.0, "zero inflation data < -100 %");

            // this can be negative
            times_[i] = timeFromReference(dates_[i]);
            QL_REQUIRE(!close(times_[i],times_[i-1]),
                       "two dates correspond to the same time "
                       "under this curve's day count convention");
        }

        interpolation_ = interpolator_.interpolate(times_.begin(),
                                                   times_.end(),
                                                   data_.begin());
        interpolation_.update();
    }


    template <class Interpolator>
    InterpolatedZeroInflationCurve<Interpolator>::
    InterpolatedZeroInflationCurve(const Date& referenceDate,
                                   const Calendar& calendar,
                                   const DayCounter& dayCounter,
                                   const Period& lag,
                                   Frequency frequency,
                                   Rate baseZeroRate,
                                   const Handle<YieldTermStructure>& yTS,
                                   const Interpolator& interpolator)
    :  ZeroInflationTermStructure(referenceDate, calendar, dayCounter,
                                  lag, frequency, baseZeroRate, yTS),
       interpolator_(interpolator) {}


    template <class T>
    Date InterpolatedZeroInflationCurve<T>::baseDate() const{
        return dates_.front();
    }

    template <class T>
    Date InterpolatedZeroInflationCurve<T>::maxDate() const {
        return dates_.back();
    }


    template <class T>
    inline Rate InterpolatedZeroInflationCurve<T>::zeroRateImpl(Time t) const {
        return interpolation_(t, true);
    }

    template <class T>
    inline const std::vector<Time>&
    InterpolatedZeroInflationCurve<T>::times() const {
        return times_;
    }

    template <class T>
    inline const std::vector<Date>&
    InterpolatedZeroInflationCurve<T>::dates() const {
        return dates_;
    }

    template <class T>
    inline const std::vector<Rate>&
    InterpolatedZeroInflationCurve<T>::rates() const {
        return data_;
    }

    template <class T>
    inline std::vector<std::pair<Date,Rate> >
    InterpolatedZeroInflationCurve<T>::nodes() const {
        std::vector<std::pair<Date,Rate> > results(dates_.size());
        for (Size i=0; i<dates_.size(); ++i)
            results[i] = std::make_pair(dates_[i],data_[i]);
        return results;
    }

}


#endif
