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
        void initialize() const;
        mutable bool validCurve_;
        Curve* ts_;
        mutable Size firstInstrument_;
    };


    // template definitions

    template <class Curve>
    IterativeBootstrap<Curve>::IterativeBootstrap()
    : validCurve_(false), ts_(0) {}

    template <class Curve>
    void IterativeBootstrap<Curve>::setup(Curve* ts) {

        ts_ = ts;

        Size n = ts_->instruments_.size();
        for (Size i=0; i<n; ++i)
            ts_->registerWith(ts_->instruments_[i]);
    }

    template <class Curve>
    void IterativeBootstrap<Curve>::initialize() const {
        // ensure rate helpers are sorted
        std::sort(ts_->instruments_.begin(), ts_->instruments_.end(),
                  detail::BootstrapHelperSorter());

        // skip expired instruments
        Date firstDate = Traits::initialDate(ts_);
        Size n = ts_->instruments_.size();
        QL_REQUIRE(ts_->instruments_[n-1]->latestDate()>firstDate,
                   "all instruments expired");
        firstInstrument_ = 0;
        while (ts_->instruments_[firstInstrument_]->latestDate() <= firstDate)
            ++firstInstrument_;
        Size alive = n-firstInstrument_;
        QL_REQUIRE(alive >= Interpolator::requiredPoints-1,
                   "not enough alive instruments: " << alive <<
                   " provided, " << Interpolator::requiredPoints-1 <<
                   " required");

        // calculate dates and times
        ts_->dates_.resize(alive+1);
        ts_->times_.resize(alive+1);
        ts_->dates_[0] = firstDate;
        ts_->times_[0] = ts_->timeFromReference(firstDate);
        Size j=1; // pillar counter
        for (Size i=firstInstrument_; i<n; ++i) {
            // check for duplicated maturity
            QL_REQUIRE(ts_->dates_[j-1] != ts_->instruments_[i]->latestDate(),
                       "two instruments have the same maturity (" <<
                       ts_->dates_[j-1] << ")");
            ts_->dates_[j] = ts_->instruments_[i]->latestDate();
            ts_->times_[j] = ts_->timeFromReference(ts_->dates_[j]);
            ++j;
        }

        // set initial guess only if the current curve cannot be used as guess
        if (!validCurve_ || ts_->data_.size()!=alive+1) {
            ts_->data_.resize(alive+1);
            ts_->data_[0] = Traits::initialValue(ts_);
            for (Size j=1; j<alive+1; ++j)
                ts_->data_[j] = Traits::initialGuess();
        }

    }

    template <class Curve>
    void IterativeBootstrap<Curve>::calculate() const {

        initialize();

        Size n = ts_->instruments_.size();
        Size alive = n-firstInstrument_;

        // setup instruments
        for (Size i=firstInstrument_; i<n; ++i) {
            // check for valid quote
            QL_REQUIRE(ts_->instruments_[i]->quote()->isValid(),
                       io::ordinal(i+1) << " instrument (maturity: " <<
                       ts_->instruments_[i]->latestDate() <<
                       ") has an invalid quote");
            // don't try this at home!
            // This call creates instruments, and removes "const".
            // There is a significant interaction with observability.
            ts_->instruments_[i]->setTermStructure(const_cast<Curve*>(ts_));
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
            for (Size i=1; i<alive+1; ++i) {

                // calculate guess before extending interpolation
                // to ensure that any extrapolation is performed
                // using the curve bootstrapped so far and no more
                boost::shared_ptr<typename Traits::helper> instrument =
                    ts_->instruments_[i-1+firstInstrument_];
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
                    } catch (...) {
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
                            "failed at " << io::ordinal(i) <<
                            " alive instrument, maturity " <<
                            instrument->latestDate() << ", reference date " <<
                            ts_->dates_[0] << ": " << e.what());
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
            for (Size i=1; i<alive+1; ++i)
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
