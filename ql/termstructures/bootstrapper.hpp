/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file bootstrapper.hpp
    \brief universal piecewise-term-structure boostrapper.
*/

#ifndef quantlib_bootstrapper_hpp
#define quantlib_bootstrapper_hpp

#include <ql/termstructures/bootstraphelper.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/utilities/dataformatters.hpp>

namespace QuantLib {

    //! Universal piecewise-term-structure boostrapper.
    template <class Curve, class Traits, class Interpolator>
    class IterativeBootstrap {
      public:
        IterativeBootstrap();
        void setup(Curve* ts);
        void calculate() const;
      private:
        Curve* ts_;
    };

    template <class Curve, class Traits, class Interpolator>
    class BootstrapError {
      public:
        BootstrapError(
                 const Curve* curve,
                 const boost::shared_ptr<typename Traits::helper>& instrument,
                 Size segment);
        Real operator()(Rate guess) const;
      private:
        const Curve* curve_;
        const boost::shared_ptr<typename Traits::helper> helper_;
        const Size segment_;
    };


    template <class Curve, class Traits, class Interpolator>
    IterativeBootstrap<Curve, Traits, Interpolator>::IterativeBootstrap()
    : ts_(0) {}

    template <class Curve, class Traits, class Interpolator>
    void IterativeBootstrap<Curve, Traits, Interpolator>::setup(Curve* ts) {

        ts_ = ts;

        Size n = ts_->instruments_.size();
        QL_REQUIRE(n+1 > Interpolator::requiredPoints,
                   "not enough instruments given: " << n);

        for (Size i=0; i<n; ++i){
            ts_->registerWith(ts_->instruments_[i]);
        }
    }


    template <class Curve, class Traits, class Interpolator>
    void IterativeBootstrap<Curve, Traits, Interpolator>::calculate() const {

        Size n = ts_->instruments_.size();

        // sort rate helpers
        std::sort(ts_->instruments_.begin(),ts_->instruments_.end(),
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
            QL_REQUIRE(ts_->instruments_[i]->quoteIsValid(),
                       "instrument with invalid quote");

        // setup instruments
        for (Size i=0; i<n; ++i) {
            // don't try this at home!
            // This call creates instruments, and removes "const".
            // There is a significant interaction with observability.
            ts_->instruments_[i]->setTermStructure(const_cast<Curve*>(ts_));
        }

        ts_->dates_ = std::vector<Date>(n+1);
        ts_->times_ = std::vector<Time>(n+1);
        ts_->data_ = std::vector<Rate>(n+1);

        ts_->dates_[0] = Traits::initialDate(ts_);
        ts_->times_[0] = ts_->timeFromReference(ts_->dates_[0]);
        ts_->data_[0] = Traits::initialValue(ts_);

        for (Size i=0; i<n; ++i) {
            ts_->dates_[i+1] = ts_->instruments_[i]->latestDate();
            ts_->times_[i+1] = ts_->timeFromReference(ts_->dates_[i+1]);
            ts_->data_[i+1] = Traits::initialGuess();
        }

        Brent solver;
        Size maxIterations = Traits::maxIterations();

        for (Size iteration = 0; ; ++iteration) {
            std::vector<Rate> previousData = ts_->data_;

            for (Size i=1; i<n+1; ++i) {
                if (iteration == 0)   {
                    // extend interpolation a point at a time
                    if (Interpolator::global) {
                        // use Linear in the first iteration
                        ts_->interpolation_ =  Linear().interpolate(
                                                      ts_->times_.begin(),
                                                      ts_->times_.begin()+i+1,
                                                      ts_->data_.begin());
                    } else {
                        ts_->interpolation_ = ts_->interpolator_.interpolate(
                                                      ts_->times_.begin(),
                                                      ts_->times_.begin()+i+1,
                                                      ts_->data_.begin());
                    }
                }
                // required because we just changed the data
                ts_->interpolation_.update();

                boost::shared_ptr<typename Traits::helper> instrument =
                    ts_->instruments_[i-1];
                Rate guess;
                if (iteration > 0) {
                    // use perturbed value from previous loop
                    guess = 0.99 * ts_->data_[i];
                } else if (i == 1) {
                    guess = Traits::initialGuess();
                } else {
                    // most traits extrapolate
                    guess = Traits::guess(ts_, ts_->dates_[i]);
                }

                // bracket
                Real min = Traits::minValueAfter(i, ts_->data_);
                Real max = Traits::maxValueAfter(i, ts_->data_);
                if (guess <= min || guess >= max)
                    guess = (min+max)/2.0;

                try {
                    BootstrapError<Curve,Traits,Interpolator> error(
                                                         ts_, instrument, i);
                    ts_->data_[i] =
                        solver.solve(error, ts_->accuracy_, guess, min, max);
                    if (i==1 && Traits::dummyInitialValue())
                        ts_->data_[0] = ts_->data_[1];
                } catch (std::exception &e) {
                    QL_FAIL(io::ordinal(iteration+1) << " iteration: "
                            "could not bootstrap the " << io::ordinal(i) <<
                            " instrument, maturity " << ts_->dates_[i] <<
                            ": " << e.what());
                }
            }

            if (!Interpolator::global) {
                break;      // no need for convergence loop
            } else if (iteration == 0) {
                // at least one more iteration is needed
                // since the first one used Linear interpolation
                ts_->interpolation_ = ts_->interpolator_.interpolate(
                                                          ts_->times_.begin(),
                                                          ts_->times_.end(),
                                                          ts_->data_.begin());
                continue;
            }

            // exit conditions
            Real improvement = 0.0;
            for (Size i=1; i<n+1; ++i)
                improvement += std::fabs(ts_->data_[i]-previousData[i]);
            improvement /= n;
            if (improvement <= ts_->accuracy_)  // convergence reached
                break;

            if (iteration+1 >= maxIterations)
                QL_FAIL("convergence not reached after "
                        << iteration+1 << " iterations");
        }
    }




    template <class Curve, class Traits, class Interpolator>
    BootstrapError<Curve, Traits, Interpolator>::BootstrapError(
                     const Curve* curve,
                     const boost::shared_ptr<typename Traits::helper>& helper,
                     Size segment)
    : curve_(curve), helper_(helper), segment_(segment) {}


    template <class Curve, class Traits, class Interpolator>
    Real BootstrapError<Curve, Traits, Interpolator>::operator()(Real guess)
                                                                      const {
        Traits::updateGuess(curve_->data_, guess, segment_);
        curve_->interpolation_.update();
        return helper_->quoteError();
    }

}


#endif
