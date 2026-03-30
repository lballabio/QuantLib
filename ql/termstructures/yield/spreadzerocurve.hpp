/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 QuantLib contributors

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

/*! \file spreadzerocurve.hpp
    \brief Yield curve based on interpolation of zero-rate spreads
           over a base YieldTermStructure
*/

#ifndef quantlib_spread_zero_curve_hpp
#define quantlib_spread_zero_curve_hpp

#include <ql/termstructures/yield/zeroyieldstructure.hpp>
#include <ql/termstructures/interpolatedcurve.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <utility>

namespace QuantLib {

    //! Yield curve based on interpolation of zero-rate spreads applied
    //! as an additive spread to the base YieldTermStructure
    /*! The zero-rate spread at any given date is interpolated between
        the input data.  The resulting continuous zero rate is:
        \f[
            z(t) = z_{\text{base}}(t) + s(t)
        \f]
        where \f$ s(t) \f$ is the interpolated spread.

        \note This term structure will remain linked to the original
              structure, i.e., any changes in the latter will be
              reflected in this structure as well.

        \ingroup yieldtermstructures
    */

    template <class Interpolator>
    class InterpolatedSpreadZeroCurve
        : public ZeroYieldStructure,
          protected InterpolatedCurve<Interpolator> {
      public:
        InterpolatedSpreadZeroCurve(
            Handle<YieldTermStructure> baseCurve,
            std::vector<Date> dates,
            std::vector<Spread> spreads,
            const Interpolator& interpolator = {});
        //! \name YieldTermStructure interface
        //@{
        DayCounter dayCounter() const override;
        Natural settlementDays() const override;
        Calendar calendar() const override;
        const Date& referenceDate() const override;
        Date maxDate() const override;
        //@}
        //! \name other inspectors
        //@{
        const Handle<YieldTermStructure>& baseCurve() const;
        const std::vector<Time>& times() const;
        const std::vector<Date>& dates() const;
        const std::vector<Real>& data() const;
        const std::vector<Spread>& zeroSpreads() const;
        std::vector<std::pair<Date, Real>> nodes() const;
        //@}
      protected:
        InterpolatedSpreadZeroCurve(
            Handle<YieldTermStructure> baseCurve,
            const Interpolator& interpolator);
        //! \name ZeroYieldStructure implementation
        //@{
        Rate zeroYieldImpl(Time) const override;
        //@}
        void update() override;

        mutable std::vector<Date> dates_;
      private:
        void updateInterpolation();
        Spread calcSpread(Time t) const;

        Handle<YieldTermStructure> baseCurve_;
        DayCounter prevDayCount_;
    };

    //! Spread yield curve based on linear interpolation of zero-rate spreads
    /*! \ingroup yieldtermstructures */
    typedef InterpolatedSpreadZeroCurve<Linear> SpreadZeroCurve;


    // inline definitions

    #ifndef __DOXYGEN__

    template <class T>
    inline InterpolatedSpreadZeroCurve<T>::InterpolatedSpreadZeroCurve(
        Handle<YieldTermStructure> baseCurve,
        std::vector<Date> dates,
        std::vector<Spread> spreads,
        const T& interpolator)
    : InterpolatedCurve<T>({}, std::move(spreads), interpolator),
      dates_(std::move(dates)), baseCurve_(std::move(baseCurve)) {
        QL_REQUIRE(dates_.size() >= T::requiredPoints,
                   "not enough input dates given");
        QL_REQUIRE(this->data_.size() == dates_.size(),
                   "dates/data count mismatch");

        registerWith(baseCurve_);
        if (!baseCurve_.empty())
            updateInterpolation();
    }

    template <class T>
    inline InterpolatedSpreadZeroCurve<T>::InterpolatedSpreadZeroCurve(
        Handle<YieldTermStructure> baseCurve,
        const T& interpolator)
    : InterpolatedCurve<T>(interpolator), baseCurve_(std::move(baseCurve))
    {
        registerWith(baseCurve_);
    }

    #endif

    template <class T>
    inline DayCounter InterpolatedSpreadZeroCurve<T>::dayCounter() const {
        return baseCurve_->dayCounter();
    }

    template <class T>
    inline Calendar InterpolatedSpreadZeroCurve<T>::calendar() const {
        return baseCurve_->calendar();
    }

    template <class T>
    inline Natural InterpolatedSpreadZeroCurve<T>::settlementDays() const {
        return baseCurve_->settlementDays();
    }

    template <class T>
    inline const Date& InterpolatedSpreadZeroCurve<T>::referenceDate() const {
        return baseCurve_->referenceDate();
    }

    template <class T>
    inline Date InterpolatedSpreadZeroCurve<T>::maxDate() const {
        Date maxDate = this->maxDate_ != Date() ? this->maxDate_ : dates_.back();
        return std::min(baseCurve_->maxDate(), maxDate);
    }

    template <class T>
    inline const Handle<YieldTermStructure>&
    InterpolatedSpreadZeroCurve<T>::baseCurve() const {
        return baseCurve_;
    }

    template <class T>
    inline const std::vector<Time>&
    InterpolatedSpreadZeroCurve<T>::times() const {
        return this->times_;
    }

    template <class T>
    inline const std::vector<Date>&
    InterpolatedSpreadZeroCurve<T>::dates() const {
        return dates_;
    }

    template <class T>
    inline const std::vector<Real>&
    InterpolatedSpreadZeroCurve<T>::data() const {
        return this->data_;
    }

    template <class T>
    inline const std::vector<Spread>&
    InterpolatedSpreadZeroCurve<T>::zeroSpreads() const {
        return this->data_;
    }

    template <class T>
    inline std::vector<std::pair<Date, Real>>
    InterpolatedSpreadZeroCurve<T>::nodes() const {
        std::vector<std::pair<Date, Real>> results(dates_.size());
        for (Size i = 0, size = dates_.size(); i < size; ++i)
            results[i] = {dates_[i], this->data_[i]};
        return results;
    }

    template <class T>
    inline Rate
    InterpolatedSpreadZeroCurve<T>::zeroYieldImpl(Time t) const {
        Rate baseZero = baseCurve_->zeroRate(t, Continuous, NoFrequency, true);
        return baseZero + calcSpread(t);
    }

    template <class T>
    inline Spread
    InterpolatedSpreadZeroCurve<T>::calcSpread(Time t) const {
        if (t <= this->times_.back())
            return this->interpolation_(t, true);

        // flat fwd extrapolation on the spread
        Time tMax = this->times_.back();
        Spread sMax = this->data_.back();
        Spread instFwdSpreadMax = sMax + tMax * this->interpolation_.derivative(tMax);
        return (sMax * tMax + instFwdSpreadMax * (t - tMax)) / t;
    }

    template <class T>
    inline void InterpolatedSpreadZeroCurve<T>::update() {
        if (!baseCurve_.empty()) {
            if (!dates_.empty())
                updateInterpolation();
            YieldTermStructure::update();
        } else {
            /* The implementation inherited from YieldTermStructure
               asks for our reference date, which we don't have since
               the original curve is still not set. Therefore, we skip
               over that and just call the base-class behavior. */
            // NOLINTNEXTLINE(bugprone-parent-virtual-call)
            TermStructure::update();
        }
    }

    template <class T>
    inline void InterpolatedSpreadZeroCurve<T>::updateInterpolation() {
        QL_REQUIRE(dates_[0] == referenceDate(),
                   "the first date should be the same as in the original curve");
        auto dc = dayCounter();
        if (prevDayCount_ != dc) {
            this->setupTimes(dates_, dates_[0], dc);
            this->setupInterpolation();
            this->interpolation_.update();
            prevDayCount_ = dc;
        }
    }

}

#endif
