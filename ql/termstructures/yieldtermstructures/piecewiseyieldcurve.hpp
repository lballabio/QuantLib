/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2005, 2006, 2007 StatPro Italia srl

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

/*! \file piecewiseyieldcurve.hpp
    \brief piecewise-interpolated term structure
*/

#ifndef quantlib_piecewise_yield_curve_hpp
#define quantlib_piecewise_yield_curve_hpp

#include <ql/termstructures/yieldtermstructures/ratehelper.hpp>
#include <ql/termstructures/yieldtermstructures/bootstraptraits.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/math/solvers1d/brent.hpp>

namespace QuantLib {

    //! Piecewise yield term structure
    /*! This term structure is bootstrapped on a number of interest
        rate instruments which are passed as a vector of handles to
        RateHelper instances. Their maturities mark the boundaries of
        the interpolated segments.

        Each segment is determined sequentially starting from the
        earliest period to the latest and is chosen so that the
        instrument whose maturity marks the end of such segment is
        correctly repriced on the curve.

        \warning The bootstrapping algorithm will raise an exception if
                 any two instruments have the same maturity date.

        \ingroup yieldtermstructures

        \test
        - the correctness of the returned values is tested by
          checking them against the original inputs.
        - the observability of the term structure is tested.
    */
    template <class Traits, class Interpolator>
    class PiecewiseYieldCurve
        : public Traits::template curve<Interpolator>::type,
          public LazyObject {
      private:
        typedef typename Traits::template curve<Interpolator>::type base_curve;
      public:
        //! \name Constructors
        //@{
        PiecewiseYieldCurve(
               const Date& referenceDate,
               const std::vector<boost::shared_ptr<RateHelper> >& instruments,
               const DayCounter& dayCounter,
               Real accuracy = 1.0e-12,
               const Interpolator& i = Interpolator());
        PiecewiseYieldCurve(
               Natural settlementDays,
               const Calendar& calendar,
               const std::vector<boost::shared_ptr<RateHelper> >& instruments,
               const DayCounter& dayCounter,
               Real accuracy = 1.0e-12,
               const Interpolator& i = Interpolator());
        //@}
        //! \name YieldTermStructure interface
        //@{
        const std::vector<Date>& dates() const;
        Date maxDate() const;
        const std::vector<Time>& times() const;
        //@}
        //! \name Inspectors
        //@{
        std::vector<std::pair<Date,Real> > nodes() const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
      private:
        // helper classes for bootstrapping
        class ObjectiveFunction;
        friend class ObjectiveFunction;
        // methods
        void checkInstruments();
        void performCalculations() const;
        DiscountFactor discountImpl(Time) const;
        // data members
        std::vector<boost::shared_ptr<RateHelper> > instruments_;
        Real accuracy_;
    };


    // objective function for solver

    #ifndef __DOXYGEN__

    template <class C, class I>
    class PiecewiseYieldCurve<C,I>::ObjectiveFunction {
      public:
        ObjectiveFunction(const PiecewiseYieldCurve<C,I>*,
                          const boost::shared_ptr<RateHelper>&, Size segment);
        Real operator()(DiscountFactor discountGuess) const;
      private:
        const PiecewiseYieldCurve<C,I>* curve_;
        boost::shared_ptr<RateHelper> rateHelper_;
        Size segment_;
    };

    #endif

    // inline definitions

    template <class C, class I>
    inline const std::vector<Date>& PiecewiseYieldCurve<C,I>::dates() const {
        calculate();
        return this->dates_;
    }

    template <class C, class I>
    inline Date PiecewiseYieldCurve<C,I>::maxDate() const {
        calculate();
        return this->dates_.back();
    }

    template <class C, class I>
    inline const std::vector<Time>& PiecewiseYieldCurve<C,I>::times() const {
        calculate();
        return this->times_;
    }

    template <class C, class I>
    inline std::vector<std::pair<Date,Real> >
    PiecewiseYieldCurve<C,I>::nodes() const {
        calculate();
        return base_curve::nodes();
    }

    template <class C, class I>
    inline void PiecewiseYieldCurve<C,I>::update() {
        base_curve::update();
        LazyObject::update();
    }

    template <class C, class I>
    inline DiscountFactor PiecewiseYieldCurve<C,I>::discountImpl(Time t)
                                                                       const {
        calculate();
        return base_curve::discountImpl(t);
    }


    // template definitions

    template <class C, class I>
    PiecewiseYieldCurve<C,I>::PiecewiseYieldCurve(
               const Date& referenceDate,
               const std::vector<boost::shared_ptr<RateHelper> >& instruments,
               const DayCounter& dayCounter,
               Real accuracy,
               const I& interpolator)
    : base_curve(referenceDate, dayCounter, interpolator),
      instruments_(instruments), accuracy_(accuracy) {
        checkInstruments();
    }

    template <class C, class I>
    PiecewiseYieldCurve<C,I>::PiecewiseYieldCurve(
               Natural settlementDays,
               const Calendar& calendar,
               const std::vector<boost::shared_ptr<RateHelper> >& instruments,
               const DayCounter& dayCounter,
               Real accuracy,
               const I& interpolator)
    : base_curve(settlementDays, calendar, dayCounter, interpolator),
      instruments_(instruments), accuracy_(accuracy) {
        checkInstruments();
    }

    template <class C, class I>
    void PiecewiseYieldCurve<C,I>::checkInstruments() {

        QL_REQUIRE(!instruments_.empty(), "no instrument given");

        // sort rate helpers
        for (Size i=0; i<instruments_.size(); ++i)
            instruments_[i]->setTermStructure(this);
        std::sort(instruments_.begin(),instruments_.end(),
                  detail::RateHelperSorter());
        // check that there is no instruments with the same maturity
        for (Size i=1; i<instruments_.size(); ++i) {
            Date m1 = instruments_[i-1]->latestDate(),
                 m2 = instruments_[i]->latestDate();
            QL_REQUIRE(m1 != m2,
                       "two instruments have the same maturity ("<< m1 <<")");
        }
        for (Size i=0; i<instruments_.size(); ++i)
            registerWith(instruments_[i]);
    }

    template <class C, class I>
    void PiecewiseYieldCurve<C,I>::performCalculations() const
    {
        // check that there is no instruments with invalid quote
        for (Size i=0; i<instruments_.size(); ++i)
            QL_REQUIRE(instruments_[i]->quoteIsValid(),
                       "instrument with invalid quote");

        // setup vectors
        Size n = instruments_.size();
        for (Size i=0; i<n; ++i) {
            // don't try this at home!
            instruments_[i]->setTermStructure(
                                 const_cast<PiecewiseYieldCurve<C,I>*>(this));
        }
        this->dates_ = std::vector<Date>(n+1);
        this->times_ = std::vector<Time>(n+1);
        this->data_ = std::vector<Real>(n+1);
        this->dates_[0] = this->referenceDate();
        this->times_[0] = 0.0;
        this->data_[0] = C::initialValue();
        for (Size i=0; i<n; ++i) {
            this->dates_[i+1] = instruments_[i]->latestDate();
            this->times_[i+1] = this->timeFromReference(this->dates_[i+1]);
            this->data_[i+1] = this->data_[i];
        }
        Brent solver;
        Size maxIterations = 25;
        // bootstrapping loop
        for (Size iteration = 0; ; iteration++) {
            std::vector<Real> previousData = this->data_;
            for (Size i=1; i<n+1; ++i) {
                if (iteration == 0) {
                    // extend interpolation a point at a time
                    if (I::global) {
                        // use Linear in the first iteration
                        this->interpolation_ = Linear().interpolate(
                                                    this->times_.begin(),
                                                    this->times_.begin()+i+1,
                                                    this->data_.begin());
                    } else {
                        this->interpolation_ = this->interpolator_.interpolate(
                                                    this->times_.begin(),
                                                    this->times_.begin()+i+1,
                                                    this->data_.begin());
                    }
                }
                this->interpolation_.update();
                boost::shared_ptr<RateHelper> instrument = instruments_[i-1];
                Real guess;
                if (iteration > 0) {
                    // use perturbed value from previous loop
                    guess = 0.99*this->data_[i];
                } else if (i > 1) {
                    // extrapolate
                    guess = C::guess(this,this->dates_[i]);
                } else {
                    guess = C::initialGuess();
                }
                // bracket
                Real min = C::minValueAfter(i, this->data_);
                Real max = C::maxValueAfter(i, this->data_);
                if (guess <= min || guess >= max)
                    guess = (min+max)/2.0;
                solver.setLowerBound(min);
                solver.setUpperBound(max);
                try {
                    this->data_[i] =
                        solver.solve(ObjectiveFunction(this, instrument, i),
                                     accuracy_, guess, guess*0.01);
                } catch (std::exception& e) {
                    QL_FAIL(io::ordinal(iteration) << " iteration: "
                            "could not bootstrap the " << io::ordinal(i) <<
                            " instrument, maturity " << this->dates_[i] <<
                            "\n error message: " << e.what());
                }
            }

            // check exit conditions
            if (!I::global) {
                // no need for convergence loop
                break;
            } else if (iteration == 0) {
                // at least one more iteration is needed
                // since the first one used Linear interpolation
                this->interpolation_ = this->interpolator_.interpolate(
                                            this->times_.begin(),
                                            this->times_.end(),
                                            this->data_.begin());
                continue;
            }

            Real improvement = 0.0;
            for (Size i=1; i<n+1; ++i)
                improvement += std::abs(this->data_[i]-previousData[i]);
            if (improvement <= n*accuracy_)  // convergence reached
                break;

            if (iteration > maxIterations)
                QL_FAIL("convergence not reached after "
                        << maxIterations << " iterations");
        }
    }

    #ifndef __DOXYGEN__

    template <class C, class I>
    PiecewiseYieldCurve<C,I>::ObjectiveFunction::ObjectiveFunction(
                              const PiecewiseYieldCurve<C,I>* curve,
                              const boost::shared_ptr<RateHelper>& rateHelper,
                              Size segment)
    : curve_(curve), rateHelper_(rateHelper), segment_(segment) {}

    template <class C, class I>
    Real PiecewiseYieldCurve<C,I>::ObjectiveFunction::operator()(Real guess)
                                                                       const {
        C::updateGuess(curve_->data_, guess, segment_);
        curve_->interpolation_.update();
        return rateHelper_->quoteError();
    }

    #endif

}


#endif
