/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Ferdinando Ametrano
 Copyright (C) 2007 Chris Kenyon
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file iterativebootstrap.hpp
    \brief universal piecewise-term-structure boostrapper.
*/

#ifndef quantlib_iterative_bootstrap_hpp
#define quantlib_iterative_bootstrap_hpp

#include <ql/termstructures/bootstraphelper.hpp>
#include <ql/termstructures/bootstraperror.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/utilities/dataformatters.hpp>

namespace QuantLib {

    //! Universal piecewise-term-structure boostrapper.
    template <class Curve>
    class IterativeBootstrap {
        typedef typename Curve::traits_type Traits;
        typedef typename Curve::interpolator_type Interpolator;
      public:
        IterativeBootstrap();
        void setup(Curve* ts);
        void calculate() const;
      private:
        mutable bool validCurve_;
        Curve* ts_;
    };


    // template definitions

    template <class Curve>
    IterativeBootstrap<Curve>::IterativeBootstrap()
    : validCurve_(false), ts_(0) {}

    template <class Curve>
    void IterativeBootstrap<Curve>::setup(Curve* ts) {

        ts_ = ts;

        Size n = ts_->instruments_.size();
        QL_REQUIRE(n+1 >= Interpolator::requiredPoints,
                   "not enough instruments: " << n << " provided, " <<
                   Interpolator::requiredPoints-1 << " required");

        for (Size i=0; i<n; ++i){
            ts_->registerWith(ts_->instruments_[i]);
        }
    }


    template <class Curve>
    void IterativeBootstrap<Curve>::calculate() const {

        Size n = ts_->instruments_.size();

        // ensure rate helpers are sorted
        std::sort(ts_->instruments_.begin(), ts_->instruments_.end(),
                  detail::BootstrapHelperSorter());

        // check that there is no instruments with the same maturity
        for (Size i=1; i<n; ++i) {
            Date m1 = ts_->instruments_[i-1]->latestDate(),
                 m2 = ts_->instruments_[i]->latestDate();
            QL_REQUIRE(m1 != m2,
                       "two instruments have the same maturity ("<< m1 <<")");
        }

        // check that there is no instruments with invalid quote
        for (Size i=0; i<n; ++i)
            QL_REQUIRE(ts_->instruments_[i]->quote()->isValid(),
                       io::ordinal(i+1) << " instrument (maturity: " <<
                       ts_->instruments_[i]->latestDate() <<
                       ") has an invalid quote");

        // setup instruments
        for (Size i=0; i<n; ++i) {
            // don't try this at home!
            // This call creates instruments, and removes "const".
            // There is a significant interaction with observability.
            ts_->instruments_[i]->setTermStructure(const_cast<Curve*>(ts_));
        }

        // calculate dates and times
        ts_->dates_ = std::vector<Date>(n+1);
        ts_->times_ = std::vector<Time>(n+1);
        ts_->dates_[0] = Traits::initialDate(ts_);
        ts_->times_[0] = ts_->timeFromReference(ts_->dates_[0]);
        for (Size i=0; i<n; ++i) {
            ts_->dates_[i+1] = ts_->instruments_[i]->latestDate();
            ts_->times_[i+1] = ts_->timeFromReference(ts_->dates_[i+1]);
        }

        // set initial guess only if the current curve cannot be used as guess
        if (validCurve_) {
            QL_ENSURE(ts_->data_.size() == n+1,
                      "dimension mismatch: expected " << n+1 <<
                      ", actual " << ts_->data_.size());
        } else {
            ts_->data_ = std::vector<Rate>(n+1);
            ts_->data_[0] = Traits::initialValue(ts_);
            for (Size i=0; i<n; ++i)
                ts_->data_[i+1] = Traits::initialGuess();
        }

        Brent solver;
        Size maxIterations = Traits::maxIterations();

        for (Size iteration=0; ; ++iteration) {
            std::vector<Rate> previousData = ts_->data_;
            // restart from the previous interpolation
            if (validCurve_) {
                ts_->interpolation_ = ts_->interpolator_.interpolate(
                                                      ts_->times_.begin(),
                                                      ts_->times_.end(),
                                                      ts_->data_.begin());
            }
            for (Size i=1; i<n+1; ++i) {

                // calculate guess before extending interpolation
                // to ensure that any extrapolation is performed
                // using the curve bootstrapped so far and no more
                boost::shared_ptr<typename Traits::helper> instrument =
                    ts_->instruments_[i-1];
                Rate guess = 0.0;
                if (validCurve_ || iteration>0) {
                    guess = ts_->data_[i];
                } else if (i==1) {
                    guess = Traits::initialGuess();
                } else {
                    // most traits extrapolate
                    guess = Traits::guess(ts_, ts_->dates_[i]);
                }

                // bracket
                Real min = Traits::minValueAfter(i, ts_->data_);
                Real max = Traits::maxValueAfter(i, ts_->data_);
                if (guess<=min || guess>=max)
                    guess = (min+max)/2.0;

                if (!validCurve_ && iteration == 0) {
                    // extend interpolation a point at a time
                    try {
                        ts_->interpolation_ = ts_->interpolator_.interpolate(
                                                      ts_->times_.begin(),
                                                      ts_->times_.begin()+i+1,
                                                      ts_->data_.begin());
                    } catch(...) {
                        if (!Interpolator::global)
                            throw; // no chance to fix it in a later iteration

                        // otherwise, if the target interpolation is
                        // not usable yet
                        ts_->interpolation_ = Linear().interpolate(
                                                      ts_->times_.begin(),
                                                      ts_->times_.begin()+i+1,
                                                      ts_->data_.begin());
                    }
                }
                // required because we just changed the data
                // is it really required?
                ts_->interpolation_.update();

                try {
                    BootstrapError<Curve> error(ts_, instrument, i);
                    Real r = solver.solve(error,ts_->accuracy_,guess,min,max);
                    // redundant assignment (as it has been already performed
                    // by BootstrapError in solve procedure), but safe
                    ts_->data_[i] = r;
                } catch (std::exception &e) {
                    validCurve_ = false;
                    QL_FAIL(io::ordinal(iteration+1) << " iteration: "
                            "failed at " << io::ordinal(i) << " instrument"
                            ", maturity " << ts_->dates_[i] <<
                            ", reference date " << ts_->dates_[0] <<
                            ": " << e.what());
                }
            }

            if (!Interpolator::global)
                break;      // no need for convergence loop
            else if (!validCurve_ && iteration == 0) {
                // ensure the target interpolation is used
                ts_->interpolation_ =
                    ts_->interpolator_.interpolate(ts_->times_.begin(),
                                                   ts_->times_.end(),
                                                   ts_->data_.begin());
                // at least one more iteration is needed to check convergence
                continue;
            }

            // exit conditions
            Real improvement = 0.0;
            for (Size i=1; i<n+1; ++i)
                improvement=std::max(improvement,
                                     std::fabs(ts_->data_[i]-previousData[i]));
            if (improvement<=ts_->accuracy_)  // convergence reached
                break;

            QL_REQUIRE(iteration+1 < maxIterations,
                       "convergence not reached after " <<
                       iteration+1 << " iterations; last improvement " <<
                       improvement << ", required accuracy " <<
                       ts_->accuracy_);
        }
        validCurve_ = true;
    }

}

#endif
