/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008, 2011, 2015 Ferdinando Ametrano
 Copyright (C) 2007 Chris Kenyon
 Copyright (C) 2007 StatPro Italia srl
 Copyright (C) 2015 Paolo Mazzocchi

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

namespace detail {

    /*! If \c dontThrow is \c true in IterativeBootstrap and on a given pillar the bootstrap fails when
        searching for a helper root between \c xMin and \c xMax, we use this function to return the value that
        gives the minimum absolute helper error in the interval between \c xMin and \c xMax inclusive.
    */
    template <class Curve>
    Real dontThrowFallback(const BootstrapError<Curve>& error,
        Real xMin, Real xMax, Size steps) {

        QL_REQUIRE(xMin < xMax, "Expected xMin to be less than xMax");

        // Set the initial value of the result to xMin and store the absolute bootstrap error at xMin
        Real result = xMin;
        Real absError = std::abs(error(xMin));
        Real minError = absError;

        // Step out to xMax
        Real stepSize = (xMax - xMin) / steps;
        for (Size i = 0; i < steps; i++) {

            // Get absolute bootstrap error at updated x value
            xMin += stepSize;
            absError = std::abs(error(xMin));

            // If this absolute bootstrap error is less than the minimum, update result and minError
            if (absError < minError) {
                result = xMin;
                minError = absError;
            }
        }

        return result;
    }

}

    //! Universal piecewise-term-structure boostrapper.
    template <class Curve>
    class IterativeBootstrap {
        typedef typename Curve::traits_type Traits;
        typedef typename Curve::interpolator_type Interpolator;
      public:
        /*! Constructor
            \param accuracy       Accuracy for the bootstrap stopping criterion. If it is set to
                                  \c Null<Real>(), its value is taken from the termstructure's accuracy.
            \param minValue       Allow to override the initial minimum value coming from traits.
            \param maxValue       Allow to override the initial maximum value coming from traits.
            \param maxAttempts    Number of attempts on each iteration. A number greater than 1 implies retries.
            \param maxFactor      Factor for max value retry on each iteration if there is a failure.
            \param minFactor      Factor for min value retry on each iteration if there is a failure.
            \param dontThrow      If set to \c true, the bootstrap doesn't throw and returns a <em>fall back</em>
                                  result.
            \param dontThrowSteps If \p dontThrow is \c true, this gives the number of steps to use when searching
                                  for a fallback curve pillar value that gives the minimum bootstrap helper error.
        */
        IterativeBootstrap(Real accuracy = Null<Real>(),
                           Real minValue = Null<Real>(),
                           Real maxValue = Null<Real>(),
                           Size maxAttempts = 1,
                           Real maxFactor = 2.0,
                           Real minFactor = 2.0,
                           bool dontThrow = false,
                           Size dontThrowSteps = 10);
        void setup(Curve* ts);
        void calculate() const;
      private:
        void initialize() const;
        Real accuracy_;
        Real minValue_, maxValue_;
        Size maxAttempts_;
        Real maxFactor_;
        Real minFactor_;
        bool dontThrow_;
        Size dontThrowSteps_;
        Curve* ts_;
        Size n_;
        Brent firstSolver_;
        FiniteDifferenceNewtonSafe solver_;
        mutable bool initialized_ = false, validCurve_ = false, loopRequired_;
        mutable Size firstAliveHelper_, alive_;
        mutable std::vector<Real> previousData_;
        mutable std::vector<ext::shared_ptr<BootstrapError<Curve> > > errors_;
    };


    // template definitions

    template <class Curve>
    IterativeBootstrap<Curve>::IterativeBootstrap(Real accuracy,
                                                  Real minValue,
                                                  Real maxValue,
                                                  Size maxAttempts,
                                                  Real maxFactor,
                                                  Real minFactor,
                                                  bool dontThrow,
                                                  Size dontThrowSteps)
    : accuracy_(accuracy), minValue_(minValue), maxValue_(maxValue), maxAttempts_(maxAttempts),
      maxFactor_(maxFactor), minFactor_(minFactor), dontThrow_(dontThrow),
      dontThrowSteps_(dontThrowSteps), ts_(nullptr), loopRequired_(Interpolator::global) {
        QL_REQUIRE(maxFactor_ >= 1.0, "Expected that maxFactor would be at least 1.0 but got " << maxFactor_);
        QL_REQUIRE(minFactor_ >= 1.0, "Expected that minFactor would be at least 1.0 but got " << minFactor_);
    }

    template <class Curve>
    void IterativeBootstrap<Curve>::setup(Curve* ts) {
        ts_ = ts;
        n_ = ts_->instruments_.size();
        QL_REQUIRE(n_ > 0, "no bootstrap helpers given");
        for (Size j=0; j<n_; ++j)
            ts_->registerWith(ts_->instruments_[j]);

        // do not initialize yet: instruments could be invalid here
        // but valid later when bootstrapping is actually required
    }

    template <class Curve>
    void IterativeBootstrap<Curve>::initialize() const {
        // ensure helpers are sorted
        std::sort(ts_->instruments_.begin(), ts_->instruments_.end(),
                  detail::BootstrapHelperSorter());
        // skip expired helpers
        Date firstDate = Traits::initialDate(ts_);
        QL_REQUIRE(ts_->instruments_[n_-1]->pillarDate()>firstDate,
                   "all instruments expired");
        firstAliveHelper_ = 0;
        while (ts_->instruments_[firstAliveHelper_]->pillarDate() <= firstDate)
            ++firstAliveHelper_;
        alive_ = n_-firstAliveHelper_;
        Size nodes = alive_+1;
        QL_REQUIRE(nodes >= Interpolator::requiredPoints,
                   "not enough alive instruments: " << alive_ <<
                   " provided, " << Interpolator::requiredPoints-1 <<
                   " required");

        // calculate dates and times, create errors_
        std::vector<Date>& dates = ts_->dates_;
        std::vector<Time>& times = ts_->times_;
        dates.resize(alive_+1);
        times.resize(alive_+1);
        errors_.resize(alive_+1);
        dates[0] = firstDate;
        times[0] = ts_->timeFromReference(dates[0]);

        Date latestRelevantDate, maxDate = firstDate;
        // pillar counter: i
        // helper counter: j
        for (Size i=1, j=firstAliveHelper_; j<n_; ++i, ++j) {
            const ext::shared_ptr<typename Traits::helper>& helper =
                                                        ts_->instruments_[j];
            dates[i] = helper->pillarDate();
            times[i] = ts_->timeFromReference(dates[i]);
            // check for duplicated pillars
            QL_REQUIRE(dates[i-1]!=dates[i],
                       "more than one instrument with pillar " << dates[i]);

            latestRelevantDate = helper->latestRelevantDate();
            // check that the helper is really extending the curve, i.e. that
            // pillar-sorted helpers are also sorted by latestRelevantDate
            QL_REQUIRE(latestRelevantDate > maxDate,
                       io::ordinal(j+1) << " instrument (pillar: " <<
                       dates[i] << ") has latestRelevantDate (" <<
                       latestRelevantDate << ") before or equal to "
                       "previous instrument's latestRelevantDate (" <<
                       maxDate << ")");
            maxDate = latestRelevantDate;

            // when a pillar date is different from the last relevant date the
            // convergence loop is required even if the Interpolator is local
            if (dates[i] != latestRelevantDate)
                loopRequired_ = true;

            errors_[i] = ext::shared_ptr<BootstrapError<Curve> >(new
                BootstrapError<Curve>(ts_, helper, i));
        }
        ts_->maxDate_ = maxDate;

        // set initial guess only if the current curve cannot be used as guess
        if (!validCurve_ || ts_->data_.size()!=alive_+1) {
            // ts_->data_[0] is the only relevant item,
            // but reasonable numbers might be needed for the whole data vector
            // because, e.g., of interpolation's early checks
            ts_->data_ = std::vector<Real>(alive_+1, Traits::initialValue(ts_));
            previousData_.resize(alive_+1);
            validCurve_ = false;
        }
        initialized_ = true;
    }

    template <class Curve>
    void IterativeBootstrap<Curve>::calculate() const {

        // we might have to call initialize even if the curve is initialized
        // and not moving, just because helpers might be date relative and change
        // with evaluation date change.
        // anyway it makes little sense to use date relative helpers with a
        // non-moving curve if the evaluation date changes
        if (!initialized_ || ts_->moving_)
            initialize();

        // setup helpers
        for (Size j=firstAliveHelper_; j<n_; ++j) {
            const ext::shared_ptr<typename Traits::helper>& helper =
                                                        ts_->instruments_[j];
            // check for valid quote
            QL_REQUIRE(helper->quote()->isValid(),
                       io::ordinal(j + 1) << " instrument (maturity: " <<
                       helper->maturityDate() << ", pillar: " <<
                       helper->pillarDate() << ") has an invalid quote");
            // don't try this at home!
            // This call creates helpers, and removes "const".
            // There is a significant interaction with observability.
            helper->setTermStructure(const_cast<Curve*>(ts_));
        }

        const std::vector<Time>& times = ts_->times_;
        const std::vector<Real>& data = ts_->data_;
        Real accuracy = accuracy_ != Null<Real>() ? accuracy_ : ts_->accuracy_;

        Size maxIterations = Traits::maxIterations()-1;

        // there might be a valid curve state to use as guess
        bool validData = validCurve_;

        for (Size iteration=0; ; ++iteration) {
            previousData_ = ts_->data_;

            // Store min value and max value at each pillar so that we can expand search if necessary.
            std::vector<Real> minValues(alive_+1, Null<Real>());
            std::vector<Real> maxValues(alive_+1, Null<Real>());
            std::vector<Size> attempts(alive_+1, 1);

            for (Size i=1; i<=alive_; ++i) { // pillar loop

                // shorter aliases for readability and to avoid duplication
                Real& min = minValues[i];
                Real& max = maxValues[i];

                // bracket root and calculate guess
                if (min == Null<Real>()) {
                    // First attempt; we take min and max either from
                    // explicit constructor parameter or from traits
                    min = (minValue_ != Null<Real>() ? minValue_ :
                           Traits::minValueAfter(i, ts_, validData, firstAliveHelper_));
                    max = (maxValue_ != Null<Real>() ? maxValue_ :
                           Traits::maxValueAfter(i, ts_, validData, firstAliveHelper_));
                } else {
                    // Extending a previous attempt.  A negative min
                    // is enlarged; a positive one is shrunk towards 0.
                    min = (min < 0.0 ? min * minFactor_ : min / minFactor_);
                    // The opposite holds for the max.
                    max = (max > 0.0 ? max * maxFactor_ : max / maxFactor_);
                }
                Real guess = Traits::guess(i, ts_, validData, firstAliveHelper_);

                // adjust guess if needed
                if (guess >= max)
                    guess = max - (max - min) / 5.0;
                else if (guess <= min)
                    guess = min + (max - min) / 5.0;

                // extend interpolation if needed
                if (!validData) {
                    try { // extend interpolation a point at a time
                          // including the pillar to be boostrapped
                        ts_->interpolation_ = ts_->interpolator_.interpolate(
                            times.begin(), times.begin()+i+1, data.begin());
                    } catch (...) {
                        if (!Interpolator::global)
                            throw; // no chance to fix it in a later iteration

                        // otherwise use Linear while the target
                        // interpolation is not usable yet
                        ts_->interpolation_ = Linear().interpolate(
                            times.begin(), times.begin()+i+1, data.begin());
                    }
                    ts_->interpolation_.update();
                }

                try {
                    if (validData)
                        solver_.solve(*errors_[i], accuracy, guess, min, max);
                    else
                        firstSolver_.solve(*errors_[i], accuracy, guess, min, max);
                } catch (std::exception &e) {
                    if (validCurve_) {
                        // the previous curve state might have been a
                        // bad guess, so we retry without using it.
                        // This would be tricky to do here (we're
                        // inside multiple nested for loops, we need
                        // to re-initialize...), so we invalidate the
                        // curve, make a recursive call and then exit.
                        validCurve_ = initialized_ = false;
                        calculate();
                        return;
                    }

                    // If we have more attempts left on this iteration, try again. Note that the max and min
                    // bounds will be widened on the retry.
                    if (attempts[i] < maxAttempts_) {
                        attempts[i]++;
                        i--;
                        continue;
                    }

                    if (dontThrow_) {
                        // Use the fallback value
                        ts_->data_[i] = detail::dontThrowFallback(*errors_[i], min, max, dontThrowSteps_);

                        // Remember to update the interpolation. If we don't and we are on the last "i", we will still
                        // have the last attempted value in the solver being used in ts_->interpolation_.
                        ts_->interpolation_.update();
                    } else {
                        QL_FAIL(io::ordinal(iteration + 1) << " iteration: failed "
                                "at " << io::ordinal(i) << " alive instrument, "
                                "pillar " << errors_[i]->helper()->pillarDate() <<
                                ", maturity " << errors_[i]->helper()->maturityDate() <<
                                ", reference date " << ts_->dates_[0] <<
                                ": " << e.what());
                    }
                }
            }

            if (!loopRequired_)
                 break;

            // exit condition
            Real change = std::fabs(data[1]-previousData_[1]);
            for (Size i=2; i<=alive_; ++i)
                change = std::max(change, std::fabs(data[i]-previousData_[i]));
            if (change<=accuracy)  // convergence reached
                break;

            // If we hit the max number of iterations and dontThrow is true, just use what we have
            if (iteration == maxIterations) {
                if (dontThrow_) {
                    break;
                } else {
                    QL_FAIL("convergence not reached after " << iteration <<
                            " iterations; last improvement " << change <<
                            ", required accuracy " << accuracy);
                }
            }

            validData = true;
        }
        validCurve_ = true;
    }

}

#endif
