
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

        \warning Although working, this class is still being developed:
                 don't use it yet.

        \warning The bootstrapping algorithm will raise an exception if
                 any two instruments have the same maturity date.

        \warning This class doesn't yet work with interpolations (such
                 as cubic splines) which need a boostrapping cycle.

        \ingroup yieldtermstructures

        \test
        - the correctness of the returned values is tested by
          checking them against the original inputs.
        - the observability of the term structure is tested.
    */
    template <class Interpolator>
    class PiecewiseYieldCurve : public InterpolatedDiscountCurve<Interpolator>,
                                public LazyObject {
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

    template <class I>
    class PiecewiseYieldCurve<I>::ObjectiveFunction {
      public:
        ObjectiveFunction(const PiecewiseYieldCurve<I>*,
                          const boost::shared_ptr<RateHelper>&, Size segment);
        Real operator()(DiscountFactor discountGuess) const;
      private:
        const PiecewiseYieldCurve<I>* curve_;
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

    template <class I>
    inline const std::vector<Date>& PiecewiseYieldCurve<I>::dates() const {
        calculate();
        return dates_;
    }

    template <class I>
    inline Date PiecewiseYieldCurve<I>::maxDate() const {
        calculate();
        return dates_.back();
    }

    template <class I>
    inline const std::vector<Time>& PiecewiseYieldCurve<I>::times() const {
        calculate();
        return times_;
    }

    template <class I>
    inline Time PiecewiseYieldCurve<I>::maxTime() const {
        calculate();
        return times_.back();
    }

    template <class I>
    inline void PiecewiseYieldCurve<I>::update() {
        InterpolatedDiscountCurve<I>::update();
        LazyObject::update();
    }


    // template definitions

    template <class I>
    PiecewiseYieldCurve<I>::PiecewiseYieldCurve(
               const Date& referenceDate,
               const std::vector<boost::shared_ptr<RateHelper> >& instruments,
               const DayCounter& dayCounter, Real accuracy,
               const I& interpolator)
    : InterpolatedDiscountCurve<I>(referenceDate,dayCounter,interpolator),
      instruments_(instruments), accuracy_(accuracy) {
        checkInstruments();
    }

    template <class I>
    PiecewiseYieldCurve<I>::PiecewiseYieldCurve(
               Integer settlementDays, const Calendar& calendar,
               const std::vector<boost::shared_ptr<RateHelper> >& instruments,
               const DayCounter& dayCounter, Real accuracy,
               const I& interpolator)
    : InterpolatedDiscountCurve<I>(settlementDays, calendar, dayCounter,
                                   interpolator),
      instruments_(instruments), accuracy_(accuracy) {
        checkInstruments();
    }

    template <class I>
    void PiecewiseYieldCurve<I>::checkInstruments() {

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
                       "two instruments have the same maturity (" +
                       DateFormatter::toString(m1) + ")");
        }
        for (i=0; i<instruments_.size(); i++)
            registerWith(instruments_[i]);
    }

    template <class I>
    void PiecewiseYieldCurve<I>::performCalculations() const {
        // values at reference date
        dates_ = std::vector<Date>(1, referenceDate());
        times_ = std::vector<Time>(1, 0.0);
        discounts_ = std::vector<DiscountFactor>(1, 1.0);

        Brent solver;
        // bootstrapping loop
        for (Size i=1; i<instruments_.size()+1; i++) {
            boost::shared_ptr<RateHelper> instrument = instruments_[i-1];
            // don't try this at home!
            instrument->setTermStructure(
                                   const_cast<PiecewiseYieldCurve<I>*>(this));
            DiscountFactor guess = instrument->discountGuess();
            if (guess == Null<DiscountFactor>()) {
                if (i > 1) {    // we can extrapolate
                    guess = this->discount(instrument->latestDate(),true);
                } else {        // any guess will do
                    guess = 0.9;
                }
            }
            // bracket
            DiscountFactor min = accuracy_*1.0e-3;
            #if defined(QL_NEGATIVE_RATES)
            // discount are not required to be decreasing--all bets are off.
            // We choose as max a value very unlikely to be exceeded.
            DiscountFactor max = 3.0;
            #else
            DiscountFactor max = discounts_[i-1];
            #endif
            solver.solve(ObjectiveFunction(this,instrument,i),
                         accuracy_,guess,min,max);
        }
    }

    template <class I>
    PiecewiseYieldCurve<I>::ObjectiveFunction::ObjectiveFunction(
                              const PiecewiseYieldCurve<I>* curve,
                              const boost::shared_ptr<RateHelper>& rateHelper,
                              Size segment)
    : curve_(curve), rateHelper_(rateHelper), segment_(segment) {
        // extend curve to next point
        curve_->dates_.push_back(rateHelper_->latestDate());
        curve_->times_.push_back(
                            curve_->timeFromReference(curve_->dates_.back()));
        // add dummy value for next point - will be reset by operator()
        curve_->discounts_.push_back(1.0);
        curve_->interpolation_ =
            curve_->interpolator_.interpolate(curve_->times_.begin(),
                                              curve_->times_.end(),
                                              curve_->discounts_.begin());
    }

    template <class I>
    Real PiecewiseYieldCurve<I>::ObjectiveFunction::operator()(
                                              DiscountFactor discount) const {
        curve_->discounts_[segment_] = discount;
        curve_->interpolation_.update();
        return rateHelper_->quoteError();
    }

}


#endif
