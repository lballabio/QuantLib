
/*
  Copyright (C) 2003, 2004, 2005 StatPro Italia srl

  This file is part of QuantLib, a free-software/open-source library
  for financial quantitative analysts and developers - http://quantlib.org/

  QuantLib is free software: you can redistribute it and/or modify it under the
  terms of the QuantLib license.  You should have received a copy of the
  license along with this program; if not, please email quantlib-dev@lists.sf.net
  The license is also available online at http://quantlib.org/html/license.html

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file zerocurve.hpp
    \brief interpolated zero-rates structure
*/

#ifndef quantlib_zero_curve_hpp
#define quantlib_zero_curve_hpp

#include <ql/TermStructures/zeroyieldstructure.hpp>
#include <ql/Math/linearinterpolation.hpp>
#include <vector>

namespace QuantLib {

    //! Term structure based on interpolation of zero yields
    /*! \ingroup yieldtermstructures */
    template <class Interpolator>
    class InterpolatedZeroCurve : public ZeroYieldStructure {
      public:
        // constructor
        InterpolatedZeroCurve(const std::vector<Date>& dates,
                              const std::vector<Rate>& yields,
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
      protected:
        InterpolatedZeroCurve(const DayCounter&,
                              const Interpolator& interpolator
                                                            = Interpolator());
        InterpolatedZeroCurve(const Date& referenceDate,
                              const DayCounter&,
                              const Interpolator& interpolator
                                                            = Interpolator());
        InterpolatedZeroCurve(Integer settlementDays, const Calendar&,
                              const DayCounter&,
                              const Interpolator& interpolator
                                                            = Interpolator());
        Rate zeroYieldImpl(Time t) const;
        DayCounter dayCounter_;
        mutable std::vector<Date> dates_;
        mutable std::vector<Time> times_;
        mutable std::vector<Rate> data_;
        mutable Interpolation interpolation_;
        Interpolator interpolator_;
    };

    //! Term structure based on linear interpolation of zero yields
    /*! \ingroup yieldtermstructures */
    typedef InterpolatedZeroCurve<Linear> ZeroCurve;


    // inline definitions

    template <class T>
    inline DayCounter InterpolatedZeroCurve<T>::dayCounter() const {
        return dayCounter_;
    }

    template <class T>
    inline Date InterpolatedZeroCurve<T>::maxDate() const {
        return dates_.back();
    }

    template <class T>
    inline Time InterpolatedZeroCurve<T>::maxTime() const {
        return times_.back();
    }

    template <class T>
    inline const std::vector<Time>& InterpolatedZeroCurve<T>::times() const {
        return times_;
    }

    template <class T>
    inline const std::vector<Date>& InterpolatedZeroCurve<T>::dates() const {
        return dates_;
    }

    template <class T>
    inline InterpolatedZeroCurve<T>::InterpolatedZeroCurve(
                                                 const DayCounter& dayCounter,
                                                 const T& interpolator)
    : dayCounter_(dayCounter), interpolator_(interpolator) {}

    template <class T>
    inline InterpolatedZeroCurve<T>::InterpolatedZeroCurve(
                                                 const Date& referenceDate,
                                                 const DayCounter& dayCounter,
                                                 const T& interpolator)
    : ZeroYieldStructure(referenceDate), dayCounter_(dayCounter),
      interpolator_(interpolator) {}

    template <class T>
    inline InterpolatedZeroCurve<T>::InterpolatedZeroCurve(
                                                 Integer settlementDays,
                                                 const Calendar& calendar,
                                                 const DayCounter& dayCounter,
                                                 const T& interpolator)
    : ZeroYieldStructure(settlementDays,calendar), dayCounter_(dayCounter),
      interpolator_(interpolator) {}

    template <class T>
    Rate InterpolatedZeroCurve<T>::zeroYieldImpl(Time t) const {
        return interpolation_(t, true);
    }

    // template definitions

    template <class T>
    InterpolatedZeroCurve<T>::InterpolatedZeroCurve(
                                              const std::vector<Date>& dates,
                                              const std::vector<Rate>& yields,
                                              const DayCounter& dayCounter,
                                              const T& interpolator)
    : ZeroYieldStructure(dates[0]), dayCounter_(dayCounter),
      dates_(dates), data_(yields), interpolator_(interpolator) {

        QL_REQUIRE(dates_.size()>1, "too few dates");
        QL_REQUIRE(data_.size()==dates_.size(),
                   "dates/yields count mismatch");

        times_.resize(dates_.size());
        times_[0]=0.0;
        for (Size i = 1; i < dates_.size(); i++) {
            QL_REQUIRE(dates_[i] > dates_[i-1],
                       "invalid date (" << dates_[i] << ", vs "
                       << dates_[i-1] << ")");
            #if !defined(QL_NEGATIVE_RATES)
            QL_REQUIRE(data_[i] >= 0.0, "negative yield");
            #endif
            times_[i] = dayCounter.yearFraction(dates_[0], dates_[i]);
        }

        interpolation_ = interpolator_.interpolate(times_.begin(),
                                                   times_.end(),
                                                   data_.begin());
    }

}


#endif
