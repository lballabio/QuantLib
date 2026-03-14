/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008 StatPro Italia srl
 Copyright (C) 2009 Ferdinando Ametrano
 Copyright (C) 2019 SoftSolutions! S.r.l.

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

/*! \file interpolatedsimplezerocurve.hpp
    \brief interpolated simply-compounded zero-rates structure
*/

#ifndef quantlib_zero_curve_simple_hpp
#define quantlib_zero_curve_simple_hpp

#include <ql/termstructures/interpolatedcurve.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <utility>

namespace QuantLib {

//! YieldTermStructure based on interpolation of zero rates
/*! \ingroup yieldtermstructures */
template <class Interpolator>
class InterpolatedSimpleZeroCurve : public YieldTermStructure, protected InterpolatedCurve<Interpolator> {
  public:
    // constructor
    InterpolatedSimpleZeroCurve(const std::vector<Date> &dates, const std::vector<Rate> &yields,
                                const DayCounter &dayCounter, const Calendar &calendar = Calendar(),
                                const std::vector<Handle<Quote> > &jumps = {},
                                const std::vector<Date> &jumpDates = {},
                                const Interpolator &interpolator = {});
    InterpolatedSimpleZeroCurve(const std::vector<Date> &dates, const std::vector<Rate> &yields,
                                const DayCounter &dayCounter, const Calendar &calendar,
                                const Interpolator &interpolator);
    InterpolatedSimpleZeroCurve(const std::vector<Date> &dates, const std::vector<Rate> &yields,
                                const DayCounter &dayCounter, const Interpolator &interpolator);
    //! \name TermStructure interface
    //@{
    Date maxDate() const override;
    //@}
    //! \name other inspectors
    //@{
    const std::vector<Time> &times() const;
    const std::vector<Date> &dates() const;
    const std::vector<Real> &data() const;
    const std::vector<Rate> &zeroRates() const;
    std::vector<std::pair<Date, Real> > nodes() const;
    //@}
  protected:
    explicit InterpolatedSimpleZeroCurve(const DayCounter &,
                                         const Interpolator &interpolator = {});
    InterpolatedSimpleZeroCurve(const Date &referenceDate, const DayCounter &,
                                const std::vector<Handle<Quote> > &jumps = {},
                                const std::vector<Date> &jumpDates = {},
                                const Interpolator &interpolator = {});
    InterpolatedSimpleZeroCurve(Natural settlementDays, const Calendar &, const DayCounter &,
                                const std::vector<Handle<Quote> > &jumps = {},
                                const std::vector<Date> &jumpDates = {},
                                const Interpolator &interpolator = {});

    //! \name YieldTermStructure implementation
    //@{
    DiscountFactor discountImpl(Time t) const override;
    //@}
    mutable std::vector<Date> dates_;

  private:
    void initialize();
};


// inline definitions

template <class T> inline Date InterpolatedSimpleZeroCurve<T>::maxDate() const { return dates_.back(); }

template <class T> inline const std::vector<Time> &InterpolatedSimpleZeroCurve<T>::times() const {
    return this->times_;
}

template <class T> inline const std::vector<Date> &InterpolatedSimpleZeroCurve<T>::dates() const { return dates_; }

template <class T> inline const std::vector<Real> &InterpolatedSimpleZeroCurve<T>::data() const { return this->data_; }

template <class T> inline const std::vector<Rate> &InterpolatedSimpleZeroCurve<T>::zeroRates() const {
    return this->data_;
}

template <class T> inline std::vector<std::pair<Date, Real> > InterpolatedSimpleZeroCurve<T>::nodes() const {
    std::vector<std::pair<Date, Real> > results(dates_.size());
    for (Size i = 0; i < dates_.size(); ++i)
        results[i] = std::make_pair(dates_[i], this->data_[i]);
    return results;
}

#ifndef __DOXYGEN__

// template definitions

template <class T> DiscountFactor InterpolatedSimpleZeroCurve<T>::discountImpl(Time t) const {
    Rate R;
    if (t <= this->times_.back()) {
        R = this->interpolation_(t, true);
    } else {
        // flat fwd extrapolation after last pillar,
        // Notice that bbg uses flat extrapolation of non-annualized zero instead
        Time tMax = this->times_.back();
        Rate zMax = this->data_.back();
        Rate instFwdMax = zMax + tMax * this->interpolation_.derivative(tMax);
        R = (zMax * tMax + instFwdMax * (t - tMax)) / t;
    }

	return DiscountFactor(1.0 / (1.0 + R * t));    
}

template <class T>
InterpolatedSimpleZeroCurve<T>::InterpolatedSimpleZeroCurve(const DayCounter &dayCounter, const T &interpolator)
    : YieldTermStructure(dayCounter), InterpolatedCurve<T>(interpolator) {}

template <class T>
InterpolatedSimpleZeroCurve<T>::InterpolatedSimpleZeroCurve(const Date &referenceDate, const DayCounter &dayCounter,
                                                            const std::vector<Handle<Quote> > &jumps,
                                                            const std::vector<Date> &jumpDates, const T &interpolator)
    : YieldTermStructure(referenceDate, Calendar(), dayCounter, jumps, jumpDates), InterpolatedCurve<T>(interpolator) {}

template <class T>
InterpolatedSimpleZeroCurve<T>::InterpolatedSimpleZeroCurve(Natural settlementDays, const Calendar &calendar,
                                                            const DayCounter &dayCounter,
                                                            const std::vector<Handle<Quote> > &jumps,
                                                            const std::vector<Date> &jumpDates, const T &interpolator)
    : YieldTermStructure(settlementDays, calendar, dayCounter, jumps, jumpDates), InterpolatedCurve<T>(interpolator) {}

template <class T>
InterpolatedSimpleZeroCurve<T>::InterpolatedSimpleZeroCurve(const std::vector<Date> &dates,
                                                            const std::vector<Rate> &yields,
                                                            const DayCounter &dayCounter, const Calendar &calendar,
                                                            const std::vector<Handle<Quote> > &jumps,
                                                            const std::vector<Date> &jumpDates, const T &interpolator)
    : YieldTermStructure(dates.at(0), calendar, dayCounter, jumps, jumpDates),
      InterpolatedCurve<T>(std::vector<Time>(), yields, interpolator), dates_(dates) {
    initialize();
}

template <class T>
InterpolatedSimpleZeroCurve<T>::InterpolatedSimpleZeroCurve(const std::vector<Date> &dates,
                                                            const std::vector<Rate> &yields,
                                                            const DayCounter &dayCounter, const Calendar &calendar,
                                                            const T &interpolator)
    : YieldTermStructure(dates.at(0), calendar, dayCounter), InterpolatedCurve<T>(std::vector<Time>(), yields,
                                                                                  interpolator),
      dates_(dates) {
    initialize();
}

template <class T>
InterpolatedSimpleZeroCurve<T>::InterpolatedSimpleZeroCurve(const std::vector<Date> &dates,
                                                            const std::vector<Rate> &yields,
                                                            const DayCounter &dayCounter, const T &interpolator)
    : YieldTermStructure(dates.at(0), Calendar(), dayCounter), InterpolatedCurve<T>(std::vector<Time>(), yields,
                                                                                    interpolator),
      dates_(dates) {
    initialize();
}

#endif

template <class T> void InterpolatedSimpleZeroCurve<T>::initialize() {
    QL_REQUIRE(dates_.size() >= T::requiredPoints,
               "not enough input dates given");
    QL_REQUIRE(this->data_.size() == dates_.size(),
               "dates/data count mismatch");

    this->setupTimes(dates_, dates_[0], dayCounter());
    this->setupInterpolation();
    this->interpolation_.update();
}

} // namespace QuantLib

#endif
