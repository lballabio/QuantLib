/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Simon Ibbotson

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

/*! \file localbootstrap.hpp
    \brief localised-term-structure bootstrapper for most curve types.
*/

#ifndef quantlib_local_bootstrap_hpp
#define quantlib_local_bootstrap_hpp

#include <ql/termstructures/bootstraphelper.hpp>
#include <ql/math/optimization/costfunction.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <ql/math/optimization/armijo.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/optimization/problem.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/shared_ptr.hpp>

namespace QuantLib {

    //! Localised-term-structure bootstrapper for most curve types.
    /*! This algorithm enables a localised fitting for non-local
        interpolation methods.

        As in the similar class (IterativeBootstrap) the input term
        structure is solved on a number of market instruments which
        are passed as a vector of handles to BootstrapHelper
        instances. Their maturities mark the boundaries of the
        interpolated segments.

        Unlike the IterativeBootstrap class, the solution for each
        interpolated segment is derived using a local
        approximation. This restricts the risk profile s.t.  the risk
        is localised. Therefore, we obtain a local IR risk profile
        whilst using a smoother interpolation method. Particularly
        good for the convex-monotone spline method.
    */
    template <class Curve>
    class LocalBootstrap {
        typedef typename Curve::traits_type Traits;
        typedef typename Curve::interpolator_type Interpolator;
      public:
        LocalBootstrap(Size localisation = 2,
                       bool forcePositive = true,
                       Real accuracy = Null<Real>());
        void setup(Curve* ts);
        void calculate() const;

      private:
        mutable bool validCurve_ = false;
        Curve* ts_;
        Size localisation_;
        bool forcePositive_;
        Real accuracy_;
    };



    // template definitions

    template <class Curve>
    LocalBootstrap<Curve>::LocalBootstrap(Size localisation, bool forcePositive, Real accuracy)
    : ts_(nullptr), localisation_(localisation), forcePositive_(forcePositive),
      accuracy_(accuracy) {}

    template <class Curve>
    void LocalBootstrap<Curve>::setup(Curve* ts) {

        ts_ = ts;

        Size n = ts_->instruments_.size();
        QL_REQUIRE(n >= Interpolator::requiredPoints,
                   "not enough instruments: " << n << " provided, " <<
                   Interpolator::requiredPoints << " required");

        QL_REQUIRE(n > localisation_,
                   "not enough instruments: " << n << " provided, " <<
                   localisation_ << " required.");

        for (Size i=0; i<n; ++i){
            ts_->registerWithObservables(ts_->instruments_[i]);
        }
    }

    template <class Curve>
    void LocalBootstrap<Curve>::calculate() const {

        validCurve_ = false;
        Size nInsts = ts_->instruments_.size();

        // ensure rate helpers are sorted
        std::sort(ts_->instruments_.begin(), ts_->instruments_.end(),
                  detail::BootstrapHelperSorter());

        // check that there is no instruments with the same maturity
        for (Size i=1; i<nInsts; ++i) {
            Date m1 = ts_->instruments_[i-1]->pillarDate(),
                 m2 = ts_->instruments_[i]->pillarDate();
            QL_REQUIRE(m1 != m2,
                       "two instruments have the same pillar date ("<<m1<<")");
        }

        // check that there is no instruments with invalid quote
        for (Size i=0; i<nInsts; ++i)
            QL_REQUIRE(ts_->instruments_[i]->quote()->isValid(),
                       io::ordinal(i+1) << " instrument (maturity: " <<
                       ts_->instruments_[i]->maturityDate() << ", pillar: " <<
                       ts_->instruments_[i]->pillarDate() <<
                       ") has an invalid quote");

        // setup instruments
        for (Size i=0; i<nInsts; ++i) {
            // don't try this at home!
            // This call creates instruments, and removes "const".
            // There is a significant interaction with observability.
            ts_->instruments_[i]->setTermStructure(const_cast<Curve*>(ts_));
        }
        // set initial guess only if the current curve cannot be used as guess
        if (validCurve_)
            QL_ENSURE(ts_->data_.size() == nInsts+1,
                      "dimension mismatch: expected " << nInsts+1 <<
                      ", actual " << ts_->data_.size());
        else {
            ts_->data_ = std::vector<Rate>(nInsts+1);
            ts_->data_[0] = Traits::initialValue(ts_);
        }

        // calculate dates and times
        ts_->dates_ = std::vector<Date>(nInsts+1);
        ts_->times_ = std::vector<Time>(nInsts+1);
        ts_->dates_[0] = Traits::initialDate(ts_);
        ts_->times_[0] = ts_->timeFromReference(ts_->dates_[0]);
        for (Size i=0; i<nInsts; ++i) {
            ts_->dates_[i+1] = ts_->instruments_[i]->pillarDate();
            ts_->times_[i+1] = ts_->timeFromReference(ts_->dates_[i+1]);
            if (!validCurve_)
                ts_->data_[i+1] = ts_->data_[i];
        }

        Real accuracy = accuracy_ != Null<Real>() ? accuracy_ : ts_->accuracy_;

        LevenbergMarquardt solver(accuracy,
                                  accuracy,
                                  accuracy);
        EndCriteria endCriteria(100, 10, 0.00, accuracy, 0.00);
        PositiveConstraint posConstraint;
        NoConstraint noConstraint;
        Constraint& solverConstraint = forcePositive_ ?
            static_cast<Constraint&>(posConstraint) :
            static_cast<Constraint&>(noConstraint);

        // now start the bootstrapping.
        Size iInst = localisation_-1;

        Size dataAdjust = Curve::interpolator_type::dataSizeAdjustment;

        do {
            Size initialDataPt = iInst+1-localisation_+dataAdjust;
            Array startArray(localisation_+1-dataAdjust);
            for (Size j = 0; j < startArray.size()-1; ++j)
                startArray[j] = ts_->data_[initialDataPt+j];

            // here we are extending the interpolation a point at a
            // time... but the local interpolator can make an
            // approximation for the final localisation period.
            // e.g. if the localisation is 2, then the first section
            // of the curve will be solved using the first 2
            // instruments... with the local interpolator making
            // suitable boundary conditions.
            ts_->interpolation_ =
                ts_->interpolator_.localInterpolate(
                                              ts_->times_.begin(),
                                              ts_->times_.begin()+(iInst + 2),
                                              ts_->data_.begin(),
                                              localisation_,
                                              ts_->interpolation_,
                                              nInsts+1);

            if (iInst >= localisation_) {
                startArray[localisation_-dataAdjust] =
                    Traits::guess(iInst, ts_, false, 0); // ?
            } else {
                startArray[localisation_-dataAdjust] = ts_->data_[0];
            }

            SimpleCostFunction currentCost([&](const Array& x) {
                for (Size i = 0; i < x.size(); ++i) {
                    Traits::updateGuess(ts_->data_, x[i], initialDataPt + i);
                }
                ts_->interpolation_.update();

                Array penalties(localisation_);
                auto helpersEnd = ts_->instruments_.begin() + (iInst + 1);
                std::transform(helpersEnd - localisation_, helpersEnd,
                               penalties.begin(),
                               [](const auto& helper) { return helper->quoteError(); });
                return penalties;
            });

            Problem toSolve(currentCost, solverConstraint, startArray);

            EndCriteria::Type endType = solver.minimize(toSolve, endCriteria);

            // check the end criteria
            QL_REQUIRE(EndCriteria::succeeded(endType),
                       "Unable to strip yieldcurve to required accuracy: " << endType);
            ++iInst;
        } while ( iInst < nInsts );
        validCurve_ = true;
    }

}

#endif
