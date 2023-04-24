/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Jose Aparicio

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

#ifndef quantlib_interpolated_affine_hazard_rate_curve_hpp
#define quantlib_interpolated_affine_hazard_rate_curve_hpp

#include <ql/stochasticprocess.hpp>
#include <ql/experimental/credit/onefactoraffinesurvival.hpp>
#include <ql/termstructures/credit/probabilitytraits.hpp>
#include <ql/termstructures/interpolatedcurve.hpp>
#include <ql/termstructures/bootstraphelper.hpp>
#include <utility>

namespace QuantLib {

    /*! DefaultProbabilityTermStructure based on interpolation of a 
    deterministic hazard rate component plus a stochastic one factor 
    rate.
    */
    /*
    The hazard rate structure here refers to the deterministic term 
    structure added on top of the affine model intensity. It is typically
    employed to match the current market implied probabilities. The total
    probabilities keep their meaning and are those of the affine model. An
    example of this is the CIR++ model as employed in credit.

    (Although this is not usually the preferred way one can instead match the
    model to price the market.)

    Notice that here, hazardRateImpl(Time) returns the deterministic part of
    the hazard rate and not E[\lambda] This is what the bootstrapping
    requires but it might be confusing.

    \todo Redesign?:
    The Affine model type is meant to model short rates; most methods
    if not all still have sense here, though discounts mean probabilities.
    This is not satisfactory, the affine models might need more structure
    or reusing these classes should be reconsidered.
    \todo Implement forward default methods.
    \todo Implement statistics methods (expected values etc)

    */
    /*! \ingroup defaultprobabilitytermstructures */
    template <class Interpolator>
    class InterpolatedAffineHazardRateCurve
        : public OneFactorAffineSurvivalStructure,
          protected InterpolatedCurve<Interpolator> {
      public:
        InterpolatedAffineHazardRateCurve(
            const std::vector<Date>& dates,
            const std::vector<Rate>& hazardRates,
            const DayCounter& dayCounter,
            const ext::shared_ptr<OneFactorAffineModel>& model,
            const Calendar& cal = Calendar(),
            const std::vector<Handle<Quote> >& jumps = std::vector<Handle<Quote> >(),
            const std::vector<Date>& jumpDates = std::vector<Date>(),
            const Interpolator& interpolator = Interpolator());
        InterpolatedAffineHazardRateCurve(const std::vector<Date>& dates,
                                          const std::vector<Rate>& hazardRates,
                                          const DayCounter& dayCounter,
                                          const ext::shared_ptr<OneFactorAffineModel>& model,
                                          const Calendar& calendar,
                                          const Interpolator& interpolator);
        InterpolatedAffineHazardRateCurve(const std::vector<Date>& dates,
                                          const std::vector<Rate>& hazardRates,
                                          const DayCounter& dayCounter,
                                          const ext::shared_ptr<OneFactorAffineModel>& model,
                                          const Interpolator& interpolator);
        //! \name TermStructure interface
        //@{
        Date maxDate() const override;
        //@}
        //! \name other inspectors
        //@{
        const std::vector<Time>& times() const;
        const std::vector<Date>& dates() const;
        const std::vector<Real>& data() const;
        const std::vector<Rate>& hazardRates() const;
        std::vector<std::pair<Date, Real> > nodes() const;
        //@}
      protected:
        InterpolatedAffineHazardRateCurve(
            const DayCounter&,
            const ext::shared_ptr<OneFactorAffineModel>& model,
            const std::vector<Handle<Quote> >& jumps = std::vector<Handle<Quote> >(),
            const std::vector<Date>& jumpDates = std::vector<Date>(),
            const Interpolator& interpolator = Interpolator());
        InterpolatedAffineHazardRateCurve(
            const Date& referenceDate,
            const DayCounter&,
            const ext::shared_ptr<OneFactorAffineModel>& model,
            const std::vector<Handle<Quote> >& jumps = std::vector<Handle<Quote> >(),
            const std::vector<Date>& jumpDates = std::vector<Date>(),
            const Interpolator& interpolator = Interpolator());
        InterpolatedAffineHazardRateCurve(
            Natural settlementDays,
            const Calendar&,
            const DayCounter&,
            const ext::shared_ptr<OneFactorAffineModel>& model,
            const std::vector<Handle<Quote> >& jumps = std::vector<Handle<Quote> >(),
            const std::vector<Date>& jumpDates = std::vector<Date>(),
            const Interpolator& interpolator = Interpolator());
        //! \name DefaultProbabilityTermStructure implementation
        //@{
        //! Returns the deterministic hazard rate component.
        Real hazardRateImpl(Time) const override;
        Probability survivalProbabilityImpl(Time) const override;

      public:
        using DefaultProbabilityTermStructure::hazardRate;
    protected:
        /*! Probability of default conditional to the realization of a given
        value of the stochastic part of the hazard rate at a prior time (and
        thus to survival at that time).
        \f$ P_{surv}(\tau>tTarget|F_{tFwd}) \f$
        */
      Probability
      conditionalSurvivalProbabilityImpl(Time tFwd, Time tTarget, Real yVal) const override;
      //@}

      mutable std::vector<Date> dates_;

    private:
      void initialize();
    };


    namespace detail {
        // hazard rate compensation TS for affine models
        const Real minHazardRateComp = -1.0;
    }

    /*! Piecewise (deterministic) plus affine (stochastic) terms composed
        hazard rate
    */
    struct AffineHazardRate {
        // interpolated curve type
        template <class Interpolator>
        struct curve {
            typedef InterpolatedAffineHazardRateCurve<Interpolator> type;
        };
        // helper class
        typedef BootstrapHelper<DefaultProbabilityTermStructure> helper;

        // start of curve data
        static Date initialDate(const DefaultProbabilityTermStructure* c) {
            return c->referenceDate();
        }
        // dummy value at reference date
        static Real initialValue(const DefaultProbabilityTermStructure*) {
            return detail::avgHazardRate;
        }

        // guesses
        template <class C>
        static Real guess(Size i,
                          const C* c,
                          bool validData,
                          Size) // firstAliveHelper
        {
            if (validData) // previous iteration value
                return c->data()[i];

            if (i==1) // first pillar
                return 0.0001;
               // return detail::avgHazardRate;

            // extrapolate
            Date d = c->dates()[i];
            /* Uneasy about the naming: Here we are bootstrapping only the
             deterministic part of the intensity it might be a better idea to
             have a different naming when having these two components.
             What is meant here is the deterministic part of a ++model type
            */
            return c->hazardRate(d, true);
        }

        // constraints
        template <class C>
        static Real minValueAfter(Size i,
                                  const C* c,
                                  bool validData,
                                  Size) // firstAliveHelper
        {
            if (validData) {
                Real r = *(std::min_element(c->data().begin(),
                                            c->data().end()));
                return r/2.0;
            }
            return detail::minHazardRateComp;
            ///return QL_EPSILON;
        }
        template <class C>
        static Real maxValueAfter(Size i,
                                  const C* c,
                                  bool validData,
                                  Size) // firstAliveHelper
        {
            if (validData) {
                Real r = *(std::max_element(c->data().begin(),
                                            c->data().end()));
                return r*2.0;
            }
            // no constraints.
            // We choose as max a value very unlikely to be exceeded.
            return detail::maxHazardRate;
        }
        // update with new guess
        static void updateGuess(std::vector<Real>& data,
                                Real rate,
                                Size i) {
            data[i] = rate;
            if (i==1)
                data[0] = rate; // first point is updated as well
        }
        // upper bound for convergence loop
        static Size maxIterations() { return 30; }
    };


    // inline definitions

    template <class T>
    inline Date InterpolatedAffineHazardRateCurve<T>::maxDate() const {
        return dates_.back();
    }

    template <class T>
    inline const std::vector<Time>&
    InterpolatedAffineHazardRateCurve<T>::times() const {
        return this->times_;
    }

    template <class T>
    inline const std::vector<Date>&
    InterpolatedAffineHazardRateCurve<T>::dates() const {
        return dates_;
    }

    template <class T>
    inline const std::vector<Real>&
    InterpolatedAffineHazardRateCurve<T>::data() const {
        return this->data_;
    }

    template <class T>
    inline const std::vector<Rate>&
    InterpolatedAffineHazardRateCurve<T>::hazardRates() const {
        return this->data_;
    }

    template <class T>
    inline std::vector<std::pair<Date, Real> >
    InterpolatedAffineHazardRateCurve<T>::nodes() const {
        std::vector<std::pair<Date, Real> > results(dates_.size());
        for (Size i=0; i<dates_.size(); ++i)
            results[i] = std::make_pair(dates_[i], this->data_[i]);
        return results;
    }

    #ifndef __DOXYGEN__

    // template definitions

    template <class T>
    Real InterpolatedAffineHazardRateCurve<T>::hazardRateImpl(Time t) const {
        if (t <= this->times_.back())
            return this->interpolation_(t, true);

        // deterministic flat hazard rate extrapolation
        return this->data_.back();
    }

    // notice it is rewritten and no call is made to hazardRateImpl
    template <class T>
    Probability
    InterpolatedAffineHazardRateCurve<T>::survivalProbabilityImpl(
        Time t) const 
    {
        // the way x0 is defined:
        Real initValHR = std::pow(model_->dynamics()->process()->x0(), 2);

        if (t == 0.0)
            return model_->discountBond(0., t, initValHR);

        Real integral;
        if (t <= this->times_.back()) {
            integral = this->interpolation_.primitive(t, true);
        } else {
            // flat hazard rate extrapolation
            integral = 
                this->interpolation_.primitive(this->times_.back(), true)
                     + this->data_.back()*(t - this->times_.back());
        }
        return std::exp(-integral) * model_->discountBond(0., t, initValHR);
    }

    template <class T>
    Probability
    InterpolatedAffineHazardRateCurve<T>::conditionalSurvivalProbabilityImpl(
        Time tFwd, Time tTarget, Real yVal) const 
    {
        QL_REQUIRE(tFwd <= tTarget, "Probability time in the past.");
        // Still leaves the possibility of sending tFwd=0 and an yVal different
        //   to the initial conditions. In an abstract sense thats all right as
        //   long as it is seen as a zero probability scenario.
        #if defined(QL_EXTRA_SAFETY_CHECKS)
            QL_REQUIRE(tFwd > 0. || yVal == 
                model_->dynamics()->process()->x0(), 
                "Initial value different to process'.");
        #endif
        if (tFwd == 0.) return survivalProbabilityImpl(tTarget);
        if (tFwd - tTarget == 0.0)
            return 1.;

        Real integralTFwd, integralTP;
        if (tFwd <= this->times_.back()) {
            integralTFwd = this->interpolation_.primitive(tFwd, true);
        } else {
            // flat hazard rate extrapolation
            integralTFwd = 
                this->interpolation_.primitive(this->times_.back(), true)
                     + this->data_.back()*(tFwd - this->times_.back());
        }
        if (tTarget <= this->times_.back()) {
            integralTP = this->interpolation_.primitive(tTarget, true);
        } else {
            // flat hazard rate extrapolation
            integralTP = 
                this->interpolation_.primitive(this->times_.back(), true)
                     + this->data_.back()*(tTarget - this->times_.back());
        }

        return std::exp(-(integralTP-integralTFwd)) * 
            model_->discountBond(tFwd, tTarget, yVal );
    }

    template <class T>
    InterpolatedAffineHazardRateCurve<T>::InterpolatedAffineHazardRateCurve(
        const DayCounter& dayCounter,
        const ext::shared_ptr<OneFactorAffineModel>& model,
        const std::vector<Handle<Quote> >& jumps,
        const std::vector<Date>& jumpDates,
        const T& interpolator)
    : OneFactorAffineSurvivalStructure(model, dayCounter, jumps, jumpDates), InterpolatedCurve<T>(
                                                                                 interpolator) {}

    template <class T>
    InterpolatedAffineHazardRateCurve<T>::InterpolatedAffineHazardRateCurve(
        const Date& referenceDate,
        const DayCounter& dayCounter,
        const ext::shared_ptr<OneFactorAffineModel>& model,
        const std::vector<Handle<Quote> >& jumps,
        const std::vector<Date>& jumpDates,
        const T& interpolator)
    : OneFactorAffineSurvivalStructure(
          model, referenceDate, Calendar(), dayCounter, jumps, jumpDates),
      InterpolatedCurve<T>(interpolator) {}

    template <class T>
    InterpolatedAffineHazardRateCurve<T>::InterpolatedAffineHazardRateCurve(
        Natural settlementDays,
        const Calendar& calendar,
        const DayCounter& dayCounter,
        const ext::shared_ptr<OneFactorAffineModel>& model,
        const std::vector<Handle<Quote> >& jumps,
        const std::vector<Date>& jumpDates,
        const T& interpolator)
    : OneFactorAffineSurvivalStructure(
          model, settlementDays, calendar, dayCounter, jumps, jumpDates),
      InterpolatedCurve<T>(interpolator) {}

    template <class T>
    InterpolatedAffineHazardRateCurve<T>::InterpolatedAffineHazardRateCurve(
        const std::vector<Date>& dates,
        const std::vector<Rate>& hazardRates,
        const DayCounter& dayCounter,
        const ext::shared_ptr<OneFactorAffineModel>& model,
        const Calendar& calendar,
        const std::vector<Handle<Quote> >& jumps,
        const std::vector<Date>& jumpDates,
        const T& interpolator)
    : OneFactorAffineSurvivalStructure(model, dates.at(0), calendar, dayCounter, jumps, jumpDates),
      InterpolatedCurve<T>(std::vector<Time>(), hazardRates, interpolator), dates_(dates) {
        initialize();
    }

    template <class T>
    InterpolatedAffineHazardRateCurve<T>::InterpolatedAffineHazardRateCurve(
        const std::vector<Date>& dates,
        const std::vector<Rate>& hazardRates,
        const DayCounter& dayCounter,
        const ext::shared_ptr<OneFactorAffineModel>& model,
        const Calendar& calendar,
        const T& interpolator)
    : OneFactorAffineSurvivalStructure(model, dates.at(0), calendar, dayCounter),
      InterpolatedCurve<T>(std::vector<Time>(), hazardRates, interpolator), dates_(dates) {
        initialize();
    }

    template <class T>
    InterpolatedAffineHazardRateCurve<T>::InterpolatedAffineHazardRateCurve(
        const std::vector<Date>& dates,
        const std::vector<Rate>& hazardRates,
        const DayCounter& dayCounter,
        const ext::shared_ptr<OneFactorAffineModel>& model,
        const T& interpolator)
    : OneFactorAffineSurvivalStructure(model, dates.at(0), Calendar(), dayCounter),
      InterpolatedCurve<T>(std::vector<Time>(), hazardRates, interpolator), dates_(dates) {
        initialize();
    }

    template <class T>
    void InterpolatedAffineHazardRateCurve<T>::initialize()
    {
        QL_REQUIRE(dates_.size() >= T::requiredPoints,
                   "not enough input dates given");
        QL_REQUIRE(this->data_.size() == dates_.size(),
                   "dates/data count mismatch");

        this->setupTimes(dates_, dates_[0], dayCounter());
        this->setupInterpolation();
        this->interpolation_.update();
    }

    #endif

}

#endif
