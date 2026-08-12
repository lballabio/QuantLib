/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef quantlib_spread_discount_curve_hpp
#define quantlib_spread_discount_curve_hpp

#include <ql/termstructures/interpolatedcurve.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/yield/derivedtermstructure.hpp>
#include <ql/math/interpolations/loginterpolation.hpp>
#include <utility>

namespace QuantLib {

    //! Yield curve based on interpolation of discount factors applied as
    //! a multiplicative spread to the base YieldTermStructure
    /*! The discount factors spread at any given date is interpolated
        between the input data.

        \note This term structure will remain linked to the original
              structure, i.e., any changes in the latter will be
              reflected in this structure as well.

        \ingroup yieldtermstructures
    */

    template <class Interpolator>
    class InterpolatedSpreadDiscountCurve
        : public RelativeDerivedYieldTermStructure<>,
          protected InterpolatedCurve<Interpolator> {
      public:
        InterpolatedSpreadDiscountCurve(
            Handle<YieldTermStructure> baseCurve,
            std::vector<Date> dates,
            std::vector<DiscountFactor> dfs,
            const Interpolator& interpolator = {});
        //! \name YieldTermStructure interface
        //@{
        Date maxDate() const override;
        //@}
        //@}
        //! \name other inspectors
        //@{
        const Handle<YieldTermStructure>& baseCurve() const;
        const std::vector<Time>& times() const;
        const std::vector<Date>& dates() const;
        const std::vector<Real>& data() const;
        std::vector<std::pair<Date, Real>> nodes() const;
        //@}
      protected:
        InterpolatedSpreadDiscountCurve(
            Handle<YieldTermStructure> baseCurve,
            const Interpolator& interpolator);
        //! \name YieldTermStructure implementation
        //@{
        DiscountFactor discountImpl(Time) const override;
        //@}
        void update() override;

        mutable std::vector<Date> dates_;
      private:
        void updateInterpolation();
        DiscountFactor calcSpread(Time t) const;

        DayCounter prevDayCount_;
    };

    //! Spread yield curve based on log-linear interpolation of discount factors
    /*! Log-linear interpolation guarantees piecewise-constant spreads.

        \ingroup yieldtermstructures
    */
    typedef InterpolatedSpreadDiscountCurve<LogLinear> SpreadDiscountCurve;


    // inline definitions

    #ifndef __DOXYGEN__

    template <class T>
    inline InterpolatedSpreadDiscountCurve<T>::InterpolatedSpreadDiscountCurve(
        Handle<YieldTermStructure> baseCurve,
        std::vector<Date> dates,
        std::vector<DiscountFactor> dfs,
        const T& interpolator)
    : RelativeDerivedYieldTermStructure(std::move(baseCurve)),
      InterpolatedCurve<T>({}, std::move(dfs), interpolator),
      dates_(std::move(dates)) {
        QL_REQUIRE(dates_.size() >= T::requiredPoints,
                   "not enough input dates given");
        QL_REQUIRE(this->data_.size() == dates_.size(),
                   "dates/data count mismatch");
        QL_REQUIRE(this->data_[0] == 1.0,
                   "the first discount must be == 1.0 "
                   "to flag the corresponding date as reference date");
        for (Size i = 1; i < dates_.size(); ++i) {
            QL_REQUIRE(this->data_[i] > 0.0, "negative discount");
        }

        this->times_.resize(dates_.size());
        this->interpolation_ = detail::interpolateWithoutUpdate(
            this->interpolator_, this->times_.begin(), this->times_.end(), this->data_.begin());
        if (!originalCurve_.empty())
            updateInterpolation();
    }

    template <class T>
    inline InterpolatedSpreadDiscountCurve<T>::InterpolatedSpreadDiscountCurve(
        Handle<YieldTermStructure> baseCurve,
        const T& interpolator)
    : RelativeDerivedYieldTermStructure(std::move(baseCurve)),
      InterpolatedCurve<T>(interpolator) {}

    #endif

    template <class T>
    inline Date InterpolatedSpreadDiscountCurve<T>::maxDate() const {
        Date maxDate = this->maxDate_ != Date() ? this->maxDate_ : dates_.back();
        return std::min(originalCurve_->maxDate(), maxDate);
    }

    template <class T>
    inline const Handle<YieldTermStructure>&
    InterpolatedSpreadDiscountCurve<T>::baseCurve() const {
        return originalCurve_;
    }

    template <class T>
    inline const std::vector<Time>&
    InterpolatedSpreadDiscountCurve<T>::times() const {
        return this->times_;
    }

    template <class T>
    inline const std::vector<Date>&
    InterpolatedSpreadDiscountCurve<T>::dates() const {
        return dates_;
    }

    template <class T>
    inline const std::vector<Real>&
    InterpolatedSpreadDiscountCurve<T>::data() const {
        return this->data_;
    }

    template <class T>
    inline std::vector<std::pair<Date, Real>>
    InterpolatedSpreadDiscountCurve<T>::nodes() const {
        std::vector<std::pair<Date, Real>> results(dates_.size());
        for (Size i = 0, size = dates_.size(); i < size; ++i)
            results[i] = {dates_[i], this->data_[i]};
        return results;
    }

    template <class T>
    inline DiscountFactor
    InterpolatedSpreadDiscountCurve<T>::discountImpl(Time t) const {
        return originalCurve_->discount(t) * calcSpread(t);
    }

    template <class T>
    inline DiscountFactor
    InterpolatedSpreadDiscountCurve<T>::calcSpread(Time t) const {
        if (t <= this->times_.back())
            return this->interpolation_(t, true);

        // flat fwd extrapolation
        Time tMax = this->times_.back();
        DiscountFactor dMax = this->data_.back();
        Rate instFwdMax = - this->interpolation_.derivative(tMax) / dMax;
        return dMax * std::exp(- instFwdMax * (t-tMax));
    }

    template <class T>
    inline void InterpolatedSpreadDiscountCurve<T>::update() {
        if (!originalCurve_.empty() && !dates_.empty())
            updateInterpolation();
        RelativeDerivedYieldTermStructure::update();
    }

    template <class T>
    inline void InterpolatedSpreadDiscountCurve<T>::updateInterpolation() {
        QL_REQUIRE(dates_[0] == referenceDate(),
                   "the first date should be the same as in the original curve");
        // Since dates_ are fixed and dates_[0] must be equal to referenceDate(),
        // the only thing that can change is dayCounter().
        auto dc = dayCounter();
        if (prevDayCount_ != dc) {
            this->setupTimes(dates_, dates_[0], dc);
            this->interpolation_.update();
            prevDayCount_ = dc;
        }
    }

}

#endif
