/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef quantlib_spread_discount_curve_hpp
#define quantlib_spread_discount_curve_hpp

#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/interpolatedcurve.hpp>
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
        : public YieldTermStructure,
          protected InterpolatedCurve<Interpolator> {
      public:
        InterpolatedSpreadDiscountCurve(
            Handle<YieldTermStructure> baseCurve,
            std::vector<Date> dates,
            std::vector<DiscountFactor> dfs,
            const Interpolator& interpolator = {});
        //! \name YieldTermStructure interface
        //@{
        DayCounter dayCounter() const override;
        Natural settlementDays() const override;
        Calendar calendar() const override;
        const Date& referenceDate() const override;
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

        Handle<YieldTermStructure> baseCurve_;
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
    : InterpolatedCurve<T>({}, std::move(dfs), interpolator),
      dates_(std::move(dates)), baseCurve_(std::move(baseCurve)) {
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

        registerWith(baseCurve_);
        if (!baseCurve_.empty())
            updateInterpolation();
    }

    template <class T>
    inline InterpolatedSpreadDiscountCurve<T>::InterpolatedSpreadDiscountCurve(
        Handle<YieldTermStructure> baseCurve,
        const T& interpolator)
    : InterpolatedCurve<T>(interpolator), baseCurve_(std::move(baseCurve))
    {
        registerWith(baseCurve_);
    }

    #endif

    template <class T>
    inline DayCounter InterpolatedSpreadDiscountCurve<T>::dayCounter() const {
        return baseCurve_->dayCounter();
    }

    template <class T>
    inline Calendar InterpolatedSpreadDiscountCurve<T>::calendar() const {
        return baseCurve_->calendar();
    }

    template <class T>
    inline Natural InterpolatedSpreadDiscountCurve<T>::settlementDays() const {
        return baseCurve_->settlementDays();
    }

    template <class T>
    inline const Date& InterpolatedSpreadDiscountCurve<T>::referenceDate() const {
        return baseCurve_->referenceDate();
    }

    template <class T>
    inline Date InterpolatedSpreadDiscountCurve<T>::maxDate() const {
        Date maxDate = this->maxDate_ != Date() ? this->maxDate_ : dates_.back();
        return std::min(baseCurve_->maxDate(), maxDate);
    }

    template <class T>
    inline const Handle<YieldTermStructure>&
    InterpolatedSpreadDiscountCurve<T>::baseCurve() const {
        return baseCurve_;
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
        return baseCurve_->discount(t) * calcSpread(t);
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
    inline void InterpolatedSpreadDiscountCurve<T>::updateInterpolation() {
        QL_REQUIRE(dates_[0] == referenceDate(),
                   "the first date should be the same as in the original curve");
        // Since dates_ are fixed and dates_[0] must be equal to referenceDate(),
        // the only thing that can change is dayCounter().
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
