/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008, 2011 Ferdinando Ametrano
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
#include <ql/math/solvers1d/finitedifferencenewtonsafe.hpp>
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
        void initialize() const;
        mutable bool initialized_, validCurve_;
        Curve* ts_;
        Size n_;
        mutable Size firstInstrument_, alive_;
        mutable std::vector<Real> previousData_;
    };


    // template definitions

    template <class Curve>
    IterativeBootstrap<Curve>::IterativeBootstrap()
    : initialized_(false), validCurve_(false), ts_(0) {}

    template <class Curve>
    void IterativeBootstrap<Curve>::setup(Curve* ts) {

        ts_ = ts;
        n_ = ts_->instruments_.size();
        for (Size j=0; j<n_; ++j)
            ts_->registerWith(ts_->instruments_[j]);

        // do not initialize yet: instruments could be invalid here
        // but valid later when bootstrapping is actually required
    }

    template <class Curve>
    void IterativeBootstrap<Curve>::initialize() const {
        // ensure rate helpers are sorted
        std::sort(ts_->instruments_.begin(), ts_->instruments_.end(),
                  detail::BootstrapHelperSorter());

        // skip expired instruments
        Date firstDate = Traits::initialDate(ts_);
        QL_REQUIRE(ts_->instruments_[n_-1]->latestDate()>firstDate,
                   "all instruments expired");
        firstInstrument_ = 0;
        while (ts_->instruments_[firstInstrument_]->latestDate() <= firstDate)
            ++firstInstrument_;
        alive_ = n_-firstInstrument_;
        QL_REQUIRE(alive_>=Interpolator::requiredPoints-1,
                   "not enough alive instruments: " << alive_ <<
                   " provided, " << Interpolator::requiredPoints-1 <<
                   " required");

        // calculate dates and times
        ts_->dates_.resize(alive_+1);
        ts_->times_.resize(alive_+1);
        ts_->dates_[0] = firstDate;
        ts_->times_[0] = ts_->timeFromReference(firstDate);
        //     pillar counter: i
        // instrument counter: j
        for (Size i=1, j=firstInstrument_; j<n_; ++i, ++j) {
            // check for duplicated maturity
            QL_REQUIRE(ts_->dates_[i-1] != ts_->instruments_[j]->latestDate(),
                       "two instruments have the same maturity (" <<
                       ts_->dates_[i-1] << ")");
            ts_->dates_[i] = ts_->instruments_[j]->latestDate();
            ts_->times_[i] = ts_->timeFromReference(ts_->dates_[i]);
        }

        // set initial guess only if the current curve cannot be used as guess
        if (!validCurve_ || ts_->data_.size()!=alive_+1) {
            ts_->data_.resize(alive_+1);
            previousData_.resize(alive_+1);
            ts_->data_[0] = Traits::initialValue(ts_);
            // reasonable numbers needed for the starting interpolation
            for (Size i=1; i<alive_+1; ++i)
                ts_->data_[i] = Traits::initialGuess();
        }
        initialized_ = true;
    }

    template <class Curve>
    void IterativeBootstrap<Curve>::calculate() const {

        if (!initialized_ || ts_->moving_)
            initialize();

        // setup instruments
        for (Size j=firstInstrument_; j<n_; ++j) {
            // check for valid quote
            QL_REQUIRE(ts_->instruments_[j]->quote()->isValid(),
                       io::ordinal(j+1) << " instrument (maturity: " <<
                       ts_->instruments_[j]->latestDate() <<
                       ") has an invalid quote");
            // don't try this at home!
            // This call creates instruments, and removes "const".
            // There is a significant interaction with observability.
            ts_->instruments_[j]->setTermStructure(const_cast<Curve*>(ts_));
        }

        Brent firstSolver;
        FiniteDifferenceNewtonSafe solver;
        Size maxIterations = Traits::maxIterations();

        for (Size iteration=0; ; ++iteration) {
            previousData_ = ts_->data_;

            for (Size i=1; i<alive_+1; ++i) { // pillar loop

                // bracket root
                Real min = Traits::minValueAfter(i, ts_->data_);
                Real max = Traits::maxValueAfter(i, ts_->data_);

                // calculate guess and extend interpolation if needed
                Real guess = 0.0;
                if (validCurve_ || iteration>0) {
                    guess = ts_->data_[i]; // previous iteration value
                } else {
                    if (i==1) // special first pillar case
                        guess = Traits::initialGuess();
                    else // most traits extrapolate (using only
                         // the curve bootstrapped so far)
                        guess = Traits::guess(ts_, ts_->dates_[i]);

                    try { // extend interpolation a point at a time
                          // including the pillar to be boostrapped
                        ts_->interpolation_ = ts_->interpolator_.interpolate(
                            ts_->times_.begin(), ts_->times_.begin()+i+1,
                            ts_->data_.begin());
                        ts_->interpolation_.update();
                    } catch (...) {
                        if (!Interpolator::global)
                            throw; // no chance to fix it in a later iteration

                        // otherwise use Linear while the target
                        // interpolation is not usable yet
                        ts_->interpolation_ = Linear().interpolate(
                            ts_->times_.begin(), ts_->times_.begin()+i+1,
                            ts_->data_.begin());
                        ts_->interpolation_.update();
                    }
                }
                // adjust guess if needed
                if (guess>=max)
                    guess = max - (max-min)/5.0;
                else if (guess<=min)
                    guess = min + (max-min)/5.0;

                // the actual instrument used for the current pillar
                boost::shared_ptr<typename Traits::helper> instrument =
                    ts_->instruments_[i-1+firstInstrument_];
                try {
                    BootstrapError<Curve> error(ts_, instrument, i);
                    if (validCurve_ || iteration>0)
                        solver.solve(error, ts_->accuracy_, guess, min, max);
                    else
                        firstSolver.solve(error,ts_->accuracy_,guess,min,max);
                } catch (std::exception &e) {
                    validCurve_ = false;
                    QL_FAIL(io::ordinal(iteration+1) << " iteration: "
                            "failed at " << io::ordinal(i) <<
                            " alive instrument, maturity " <<
                            instrument->latestDate() << ", reference date " <<
                            ts_->dates_[0] << ": " << e.what());
                }
            }

            if (!Interpolator::global)
                break;     // no need for convergence loop
            else if (iteration == 0)
                continue; // at least one more iteration to convergence check

            // exit condition
            Real change = 0.0;
            for (Size i=1; i<alive_+1; ++i)
                change = std::max(change,
                                  std::fabs(ts_->data_[i]-previousData_[i]));
            if (change<=ts_->accuracy_)  // convergence reached
                break;

            QL_REQUIRE(iteration+1<maxIterations,
                       "convergence not reached after " <<
                       iteration+1 << " iterations; last improvement " <<
                       change << ", required accuracy " <<
                       ts_->accuracy_);
        }
        validCurve_ = true;
    }

}

#endif
