
/*
  Copyright (C) 2002, 2003 Decillion Pty(Ltd).
  Copyright (C) 2005 StatPro Italia srl

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

/*! \file discountcurve.hpp
    \brief pre-bootstrapped discount factor structure
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
        InterpolatedDiscountCurve(const DayCounter&);
        InterpolatedDiscountCurve(const Date& referenceDate,
                                  const DayCounter&);
        InterpolatedDiscountCurve(Integer settlementDays, const Calendar&,
                                  const DayCounter&);
        DiscountFactor discountImpl(Time) const;
        Size referenceNode(Time) const;
        DayCounter dayCounter_;
        mutable std::vector<Date> dates_;
        mutable std::vector<DiscountFactor> discounts_;
        mutable std::vector<Time> times_;
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
        return discounts_;
    }

    template <class T>
    inline InterpolatedDiscountCurve<T>::InterpolatedDiscountCurve(
                                                 const DayCounter& dayCounter)
    : dayCounter_(dayCounter) {}

    template <class T>
    inline InterpolatedDiscountCurve<T>::InterpolatedDiscountCurve(
                                                 const Date& referenceDate,
                                                 const DayCounter& dayCounter)
    : YieldTermStructure(referenceDate), dayCounter_(dayCounter) {}

    template <class T>
    inline InterpolatedDiscountCurve<T>::InterpolatedDiscountCurve(
                                                 Integer settlementDays,
                                                 const Calendar& calendar,
                                                 const DayCounter& dayCounter)
    : YieldTermStructure(settlementDays,calendar), dayCounter_(dayCounter) {}


    // template definitions

    template <class T>
    InterpolatedDiscountCurve<T>::InterpolatedDiscountCurve(
                                 const std::vector<Date>& dates,
                                 const std::vector<DiscountFactor>& discounts,
                                 const DayCounter& dayCounter,
                                 const T& interpolator)
    : YieldTermStructure(dates[0]),
      dayCounter_(dayCounter), dates_(dates), discounts_(discounts),
      interpolator_(interpolator) {
        QL_REQUIRE(dates_.size() > 0,
                   "no input dates given");
        QL_REQUIRE(discounts_.size() > 0,
                   "no input discount factors given");
        QL_REQUIRE(discounts_.size() == dates_.size(),
                   "dates/discount factors count mismatch");
        QL_REQUIRE(discounts_[0] == 1.0,
                   "the first discount must be == 1.0 "
                   "to flag the corrsponding date as settlement date");

        times_.resize(dates_.size());
        times_[0] = 0.0;
        for(Size i = 1; i < dates_.size(); i++) {
            QL_REQUIRE(dates_[i] > dates_[i-1],
                       "invalid date (" << dates_[i] << ", vs "
                       << dates_[i-1] << ")");
            QL_REQUIRE(discounts_[i] > 0.0, "negative discount");
            times_[i] = dayCounter.yearFraction(dates_[0], dates_[i]);
        }
        interpolation_ = interpolator_.interpolate(times_.begin(),
                                                   times_.end(),
                                                   discounts_.begin());
    }

    template <class T>
    DiscountFactor InterpolatedDiscountCurve<T>::discountImpl(Time t) const {
        if (t == 0.0) {
            return discounts_[0];
        } else {
            Size n = referenceNode(t);
            if (t == times_[n]) {
                return discounts_[n];
            } else {
                return interpolation_(t, true);
            }
        }
        QL_DUMMY_RETURN(DiscountFactor());
    }

    template <class T>
    Size InterpolatedDiscountCurve<T>::referenceNode(Time t) const {
        if (t >= times_.back())
            return times_.size()-1;
        std::vector<Time>::const_iterator i=times_.begin(),
            j=times_.end(), k;
        while (j-i > 1) {
            k = i+(j-i)/2;
            if (t <= *k)
                j = k;
            else
                i = k;
        }
        return (j-times_.begin());
    }

}


#endif
