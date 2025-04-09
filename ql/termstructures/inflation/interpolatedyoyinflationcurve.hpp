/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2008, 2009 Chris Kenyon
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

/*! \file interpolatedyoyinflationcurve.hpp
    \brief Inflation term structure based on the interpolation of
           year-on-year rates.
*/

#ifndef quantlib_interpolated_yoy_inflationcurve_hpp
#define quantlib_interpolated_yoy_inflationcurve_hpp

#include <ql/termstructures/inflationtermstructure.hpp>
#include <ql/termstructures/interpolatedcurve.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/comparison.hpp>
#include <utility>

namespace QuantLib {

    //! Inflation term structure based on interpolated year-on-year rates
    /*! \note The provided rates are not YY inflation-swap quotes.

        \ingroup inflationtermstructures
    */
    template<class Interpolator>
    class InterpolatedYoYInflationCurve
        : public YoYInflationTermStructure,
          protected InterpolatedCurve<Interpolator> {
      public:
        InterpolatedYoYInflationCurve(const Date& referenceDate,
                                      std::vector<Date> dates,
                                      const std::vector<Rate>& rates,
                                      Frequency frequency,
                                      const DayCounter& dayCounter,
                                      const ext::shared_ptr<Seasonality>& seasonality = {},
                                      const Interpolator& interpolator = Interpolator());

        /*! \deprecated Use the overload without indexIsInterpolated.
                        Deprecated in version 1.37.
        */
        [[deprecated("Use the overload without indexIsInterpolated")]]
        InterpolatedYoYInflationCurve(const Date& referenceDate,
                                      std::vector<Date> dates,
                                      const std::vector<Rate>& rates,
                                      Frequency frequency,
                                      bool indexIsInterpolated,
                                      const DayCounter& dayCounter,
                                      const ext::shared_ptr<Seasonality>& seasonality = {},
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
        //! \name YoYInflationTermStructure interface
        //@{
        Rate yoyRateImpl(Time t) const override;
        //@}
        mutable std::vector<Date> dates_;

        /*! Protected version for use when descendents don't want to
            (or can't) provide the points for interpolation on
            construction.
        */
        InterpolatedYoYInflationCurve(const Date& referenceDate,
                                      Date baseDate,
                                      Rate baseYoYRate,
                                      Frequency frequency,
                                      const DayCounter& dayCounter,
                                      const ext::shared_ptr<Seasonality>& seasonality = {},
                                      const Interpolator& interpolator = Interpolator());

        /*! \deprecated Use the overload without indexIsInterpolated.
                        Deprecated in version 1.37.
        */
        [[deprecated("Use the overload without indexIsInterpolated")]]
        InterpolatedYoYInflationCurve(const Date& referenceDate,
                                      Date baseDate,
                                      Rate baseYoYRate,
                                      Frequency frequency,
                                      bool indexIsInterpolated,
                                      const DayCounter& dayCounter,
                                      const ext::shared_ptr<Seasonality>& seasonality = {},
                                      const Interpolator& interpolator = Interpolator());
    };

    typedef InterpolatedYoYInflationCurve<Linear> YoYInflationCurve;



    // template definitions

    template <class Interpolator>
    InterpolatedYoYInflationCurve<Interpolator>::InterpolatedYoYInflationCurve(
        const Date& referenceDate,
        std::vector<Date> dates,
        const std::vector<Rate>& rates,
        Frequency frequency,
        const DayCounter& dayCounter,
        const ext::shared_ptr<Seasonality>& seasonality,
        const Interpolator& interpolator)
    : YoYInflationTermStructure(referenceDate, dates.at(0), rates[0],
                                frequency, dayCounter, seasonality),
      InterpolatedCurve<Interpolator>(std::vector<Time>(), rates, interpolator),
      dates_(std::move(dates)) {

        QL_REQUIRE(dates_.size()>1, "too few dates: " << dates_.size());

        QL_REQUIRE(this->data_.size() == dates_.size(),
                   "indices/dates count mismatch: "
                   << this->data_.size() << " vs " << dates_.size());

        for (Size i = 1; i < dates_.size(); i++) {
            // YoY inflation data may be positive or negative
            // but must be greater than -1
            QL_REQUIRE(this->data_[i] > -1.0,
                       "year-on-year inflation data < -100 %");
        }

        this->setupTimes(dates_, referenceDate, dayCounter);
        this->setupInterpolation();
        this->interpolation_.update();
    }

    template <class Interpolator>
    InterpolatedYoYInflationCurve<Interpolator>::InterpolatedYoYInflationCurve(
        const Date& referenceDate,
        std::vector<Date> dates,
        const std::vector<Rate>& rates,
        Frequency frequency,
        bool indexIsInterpolated,
        const DayCounter& dayCounter,
        const ext::shared_ptr<Seasonality>& seasonality,
        const Interpolator& interpolator)
    : InterpolatedYoYInflationCurve(referenceDate, dates, rates, frequency,
                                    dayCounter, seasonality, interpolator) {
        QL_DEPRECATED_DISABLE_WARNING
        indexIsInterpolated_ = indexIsInterpolated;
        QL_DEPRECATED_ENABLE_WARNING
    }

    template <class Interpolator>
    InterpolatedYoYInflationCurve<Interpolator>::
    InterpolatedYoYInflationCurve(const Date& referenceDate,
                                  Date baseDate,
                                  Rate baseYoYRate,
                                  Frequency frequency,
                                  const DayCounter& dayCounter,
                                  const ext::shared_ptr<Seasonality>& seasonality,
                                  const Interpolator& interpolator)
    : YoYInflationTermStructure(referenceDate, baseDate, baseYoYRate,
                                frequency, dayCounter, seasonality),
      InterpolatedCurve<Interpolator>(interpolator) {}

    template <class Interpolator>
    InterpolatedYoYInflationCurve<Interpolator>::
    InterpolatedYoYInflationCurve(const Date& referenceDate,
                                  Date baseDate,
                                  Rate baseYoYRate,
                                  Frequency frequency,
                                  bool indexIsInterpolated,
                                  const DayCounter& dayCounter,
                                  const ext::shared_ptr<Seasonality>& seasonality,
                                  const Interpolator& interpolator)
    : InterpolatedYoYInflationCurve(referenceDate, baseDate, baseYoYRate, frequency,
                                    dayCounter, seasonality, interpolator) {
        QL_DEPRECATED_DISABLE_WARNING
        indexIsInterpolated_ = indexIsInterpolated;
        QL_DEPRECATED_ENABLE_WARNING
    }


    template <class T>
    Date InterpolatedYoYInflationCurve<T>::baseDate() const {
        if (hasExplicitBaseDate())
            return YoYInflationTermStructure::baseDate();
        else
            return dates_.front();
    }

    template <class T>
    Date InterpolatedYoYInflationCurve<T>::maxDate() const {
        return dates_.back();
    }


    template <class T>
    inline Rate InterpolatedYoYInflationCurve<T>::yoyRateImpl(Time t) const {
        return this->interpolation_(t, true);
    }

    template <class T>
    inline const std::vector<Time>&
    InterpolatedYoYInflationCurve<T>::times() const {
        return this->times_;
    }

    template <class T>
    inline const std::vector<Date>&
    InterpolatedYoYInflationCurve<T>::dates() const {
        return dates_;
    }

    template <class T>
    inline const std::vector<Rate>&
    InterpolatedYoYInflationCurve<T>::rates() const {
        return this->data_;
    }

    template <class T>
    inline const std::vector<Real>&
    InterpolatedYoYInflationCurve<T>::data() const {
        return this->data_;
    }

    template <class T>
    inline std::vector<std::pair<Date,Rate> >
    InterpolatedYoYInflationCurve<T>::nodes() const {
        std::vector<std::pair<Date,Rate> > results(dates_.size());
        for (Size i=0; i<dates_.size(); ++i)
            results[i] = std::make_pair(dates_[i],this->data_[i]);
        return results;
    }

}


#endif
