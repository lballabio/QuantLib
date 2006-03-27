/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Decillion Pty(Ltd)
 Copyright (C) 2005 StatPro Italia srl

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

/*! \file discountcurve.hpp
    \brief interpolated discount factor structure
*/

#ifndef quantlib_discount_curve_hpp
#define quantlib_discount_curve_hpp

#include <ql/yieldtermstructure.hpp>
#include <ql/Math/loglinearinterpolation.hpp>
#include <vector>

namespace QuantLib {

    //! Term structure based on interpolation of discount factors
    /*! \ingroup yieldtermstructures */
    template <class Interpolator>
    class InterpolatedDiscountCurve : public YieldTermStructure {
      public:
        InterpolatedDiscountCurve(const std::vector<Date>& dates,
                                  const std::vector<DiscountFactor>& dfs,
                                  const DayCounter& dayCounter,
                                  const Interpolator& interpolator
                                                            = Interpolator());
        //! \name Inspectors
        //@{
        DayCounter dayCounter() const;
        Date maxDate() const;
        Time maxTime() const;
        const std::vector<Time>& times() const;
        const std::vector<Date>& dates() const;
        const std::vector<DiscountFactor>& discounts() const;
        //@}
      protected:
        InterpolatedDiscountCurve(const DayCounter&,
                                  const Interpolator& interpolator
                                                            = Interpolator());
        InterpolatedDiscountCurve(const Date& referenceDate,
                                  const DayCounter&,
                                  const Interpolator& interpolator
                                                            = Interpolator());
        InterpolatedDiscountCurve(Integer settlementDays, const Calendar&,
                                  const DayCounter&,
                                  const Interpolator& interpolator
                                                            = Interpolator());
        DiscountFactor discountImpl(Time) const;
        DayCounter dayCounter_;
        mutable std::vector<Date> dates_;
        mutable std::vector<Time> times_;
        mutable std::vector<DiscountFactor> data_;
        mutable Interpolation interpolation_;
        Interpolator interpolator_;
    };

    //! Term structure based on log-linear interpolation of discount factors
    /*! Log-linear interpolation guarantees piecewise-constant forward
        rates.

        \ingroup yieldtermstructures
    */
    typedef InterpolatedDiscountCurve<LogLinear> DiscountCurve;


    // inline definitions

    template <class T>
    inline DayCounter InterpolatedDiscountCurve<T>::dayCounter() const {
        return dayCounter_;
    }

    template <class T>
    inline Date InterpolatedDiscountCurve<T>::maxDate() const {
        return dates_.back();
    }

    template <class T>
    inline Time InterpolatedDiscountCurve<T>::maxTime() const {
        return times_.back();
    }

    template <class T>
    inline const std::vector<Time>&
    InterpolatedDiscountCurve<T>::times() const {
        return times_;
    }

    template <class T>
    inline const std::vector<Date>&
    InterpolatedDiscountCurve<T>::dates() const {
        return dates_;
    }

    template <class T>
    inline const std::vector<DiscountFactor>&
    InterpolatedDiscountCurve<T>::discounts() const {
        return data_;
    }

    template <class T>
    inline InterpolatedDiscountCurve<T>::InterpolatedDiscountCurve(
                                                 const DayCounter& dayCounter,
                                                 const T& interpolator)
    : dayCounter_(dayCounter), interpolator_(interpolator) {}

    template <class T>
    inline InterpolatedDiscountCurve<T>::InterpolatedDiscountCurve(
                                                 const Date& referenceDate,
                                                 const DayCounter& dayCounter,
                                                 const T& interpolator)
    : YieldTermStructure(referenceDate), dayCounter_(dayCounter),
      interpolator_(interpolator) {}

    template <class T>
    inline InterpolatedDiscountCurve<T>::InterpolatedDiscountCurve(
                                                 Integer settlementDays,
                                                 const Calendar& calendar,
                                                 const DayCounter& dayCounter,
                                                 const T& interpolator)
    : YieldTermStructure(settlementDays,calendar), dayCounter_(dayCounter),
      interpolator_(interpolator) {}


    template <class T>
    inline DiscountFactor InterpolatedDiscountCurve<T>::discountImpl(Time t)
                                                                       const {
        return interpolation_(t, true);
    }

    // template definitions

    template <class T>
    InterpolatedDiscountCurve<T>::InterpolatedDiscountCurve(
                                 const std::vector<Date>& dates,
                                 const std::vector<DiscountFactor>& discounts,
                                 const DayCounter& dayCounter,
                                 const T& interpolator)
    : YieldTermStructure(dates[0]),
      dayCounter_(dayCounter), dates_(dates), data_(discounts),
      interpolator_(interpolator) {
        QL_REQUIRE(dates_.size() > 0,
                   "no input dates given");
        QL_REQUIRE(data_.size() > 0,
                   "no input discount factors given");
        QL_REQUIRE(data_.size() == dates_.size(),
                   "dates/discount factors count mismatch");
        QL_REQUIRE(data_[0] == 1.0,
                   "the first discount must be == 1.0 "
                   "to flag the corrsponding date as settlement date");

        times_.resize(dates_.size());
        times_[0] = 0.0;
        for (Size i = 1; i < dates_.size(); i++) {
            QL_REQUIRE(dates_[i] > dates_[i-1],
                       "invalid date (" << dates_[i] << ", vs "
                       << dates_[i-1] << ")");
            QL_REQUIRE(data_[i] > 0.0, "negative discount");
            times_[i] = dayCounter.yearFraction(dates_[0], dates_[i]);
        }

        interpolation_ = interpolator_.interpolate(times_.begin(),
                                                   times_.end(),
                                                   data_.begin());
    }

}


#endif
