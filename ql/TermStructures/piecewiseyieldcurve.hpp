
/*
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

/*! \file TermStructures/piecewiseyieldcurve.hpp
    \brief piecewise-interpolated term structure
*/

#ifndef quantlib_piecewise_yield_curve_hpp
#define quantlib_piecewise_yield_curve_hpp

#include <ql/TermStructures/discountcurve.hpp>
#include <ql/TermStructures/ratehelpers.hpp>
#include <ql/TermStructures/bootstraptraits.hpp>
#include <ql/Math/linearinterpolation.hpp>
#include <ql/Solvers1D/brent.hpp>

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
    class PiecewiseYieldCurve : public Traits::curve<Interpolator>::type,
                                public LazyObject {
      private:
        typedef typename Traits::curve<Interpolator>::type base_curve;
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
               Integer settlementDays, const Calendar& calendar,
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
        Time maxTime() const;
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
        // data members
        std::vector<boost::shared_ptr<RateHelper> > instruments_;
        Real accuracy_;
    };


    // objective function for solver

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

    // helper class

    namespace detail {

        class RateHelperSorter {
          public:
            bool operator()(const boost::shared_ptr<RateHelper>& h1,
                            const boost::shared_ptr<RateHelper>& h2) const {
                return (h1->latestDate() < h2->latestDate());
            }
        };

    }

    // inline definitions

    template <class C, class I>
    inline const std::vector<Date>& PiecewiseYieldCurve<C,I>::dates() const {
        calculate();
        return dates_;
    }

    template <class C, class I>
    inline Date PiecewiseYieldCurve<C,I>::maxDate() const {
        calculate();
        return dates_.back();
    }

    template <class C, class I>
    inline const std::vector<Time>& PiecewiseYieldCurve<C,I>::times() const {
        calculate();
        return times_;
    }

    template <class C, class I>
    inline Time PiecewiseYieldCurve<C,I>::maxTime() const {
        calculate();
        return times_.back();
    }

    template <class C, class I>
    inline void PiecewiseYieldCurve<C,I>::update() {
        base_curve::update();
        LazyObject::update();
    }


    // template definitions

    template <class C, class I>
    PiecewiseYieldCurve<C,I>::PiecewiseYieldCurve(
               const Date& referenceDate,
               const std::vector<boost::shared_ptr<RateHelper> >& instruments,
               const DayCounter& dayCounter, Real accuracy,
               const I& interpolator)
    : base_curve(referenceDate,dayCounter,interpolator),
      instruments_(instruments), accuracy_(accuracy) {
        checkInstruments();
    }

    template <class C, class I>
    PiecewiseYieldCurve<C,I>::PiecewiseYieldCurve(
               Integer settlementDays, const Calendar& calendar,
               const std::vector<boost::shared_ptr<RateHelper> >& instruments,
               const DayCounter& dayCounter, Real accuracy,
               const I& interpolator)
    : base_curve(settlementDays, calendar, dayCounter, interpolator),
      instruments_(instruments), accuracy_(accuracy) {
        checkInstruments();
    }

    template <class C, class I>
    void PiecewiseYieldCurve<C,I>::checkInstruments() {

        QL_REQUIRE(!instruments_.empty(), "no instrument given");

        // sort rate helpers
        Size i;
        for (i=0; i<instruments_.size(); i++)
            instruments_[i]->setTermStructure(this);
        std::sort(instruments_.begin(),instruments_.end(),
                  detail::RateHelperSorter());
        // check that there is no instruments with the same maturity
        for (i=1; i<instruments_.size(); i++) {
            Date m1 = instruments_[i-1]->latestDate(),
                 m2 = instruments_[i]->latestDate();
            QL_REQUIRE(m1 != m2,
                       "two instruments have the same maturity ("<< m1 <<")");
        }
        for (i=0; i<instruments_.size(); i++)
            registerWith(instruments_[i]);
    }

    template <class C, class I>
    void PiecewiseYieldCurve<C,I>::performCalculations() const {
        // setup vectors
        Size n = instruments_.size();
        dates_ = std::vector<Date>(n+1);
        times_ = std::vector<Time>(n+1);
        data_ = std::vector<Real>(n+1);
        dates_[0] = referenceDate();
        times_[0] = 0.0;
        data_[0] = C::initialValue();
        for (Size i=0; i<n; i++) {
            dates_[i+1] = instruments_[i]->latestDate();
            times_[i+1] = timeFromReference(dates_[i+1]);
            data_[i+1] = data_[i];
        }
        Brent solver;
        Size maxIterations = 25;
        // bootstrapping loop
        for (Size iteration = 0; ; iteration++) {
            std::vector<Real> previousData = data_;
            Size i;
            for (i=1; i<n+1; i++) {
                if (iteration == 0) {
                    // extend interpolation a point at a time
                    if (I::global && i < 2) {
                        // not enough points for splines
                        interpolation_ = Linear().interpolate(
                                           times_.begin(), times_.begin()+i+1,
                                           data_.begin());
                    } else {
                        interpolation_ = interpolator_.interpolate(
                                           times_.begin(), times_.begin()+i+1,
                                           data_.begin());
                    }
                }
                boost::shared_ptr<RateHelper> instrument = instruments_[i-1];
                // don't try this at home!
                instrument->setTermStructure(
                                 const_cast<PiecewiseYieldCurve<C,I>*>(this));
                Real guess;
                if (iteration > 0) {
                    // use perturbed value from previous loop
                    guess = 0.99*data_[i];
                } else if (i > 1) {
                    // extrapolate
                    guess = C::guess(this,dates_[i]);
                } else {
                    guess = C::initialGuess();
                }
                // bracket
                Real min = C::minValueAfter(i, data_);
                Real max = C::maxValueAfter(i, data_);
                if (guess <= min || guess >= max)
                    guess = (min+max)/2.0;
                data_[i] = solver.solve(ObjectiveFunction(this,instrument,i),
                                        accuracy_,guess,min,max);
            }
            // check exit conditions
            if (!I::global)
                break;   // no need for convergence loop

            Real improvement = 0.0;
            for (i=1; i<n+1; i++)
                improvement += std::abs(data_[i]-previousData[i]);
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
