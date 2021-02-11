/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2019 SoftSolutions! S.r.l.

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

/*! \file globalbootstrap.hpp
    \brief global bootstrap, with additional restrictions
    \ingroup termstructures
*/

#ifndef quantlib_global_bootstrap_hpp
#define quantlib_global_bootstrap_hpp

#include <ql/functional.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/termstructures/bootstraperror.hpp>
#include <ql/termstructures/bootstraphelper.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <utility>

namespace QuantLib {

//! Global boostrapper, with additional restrictions
template <class Curve> class GlobalBootstrap {
    typedef typename Curve::traits_type Traits;             // ZeroYield, Discount, ForwardRate
    typedef typename Curve::interpolator_type Interpolator; // Linear, LogLinear, ...

  public:
    GlobalBootstrap(Real accuracy = Null<Real>());
    /*! The set of (alive) additional dates is added to the interpolation grid. The set of additional dates must only
      depend on the current global evaluation date.  The additionalErrors functor must yield at least as many values
      such that

      number of (usual, alive) rate helpers + number of (alive) additional values >= number of data points - 1

      (note that the data points contain t=0). These values are treated as additional error terms in the optimization,
      the usual rate helpers return marketQuote - impliedQuote here. All error terms are equally weighted in the
      optimisation.

      The additional helpers are treated like the usual rate helpers, but no standard pillar dates are added for them.

      WARNING: This class is known to work with Traits Discount, ZeroYield, Forward (i.e. the usual traits for IR curves
      in QL), it might fail for other traits - check the usage of Traits::updateGuess(), Traits::guess(),
      Traits::minValueAfter(), Traits::maxValueAfter() in this class against them.
    */
    GlobalBootstrap(std::vector<ext::shared_ptr<typename Traits::helper> > additionalHelpers,
                    ext::function<std::vector<Date>()> additionalDates,
                    ext::function<Array()> additionalErrors,
                    Real accuracy = Null<Real>());
    void setup(Curve *ts);
    void calculate() const;

  private:
    void initialize() const;
    Curve *ts_;
    Real accuracy_;
    mutable std::vector<ext::shared_ptr<typename Traits::helper> > additionalHelpers_;
    ext::function<std::vector<Date>()> additionalDates_;
    ext::function<Array()> additionalErrors_;
    mutable bool initialized_ = false, validCurve_ = false;
    mutable Size firstHelper_, numberHelpers_;
    mutable Size firstAdditionalHelper_, numberAdditionalHelpers_;
    mutable Size firstAdditionalDate_, numberAdditionalDates_;
    mutable std::vector<Real> lowerBounds_, upperBounds_;
};

// template definitions

template <class Curve>
GlobalBootstrap<Curve>::GlobalBootstrap(Real accuracy) : ts_(0), accuracy_(accuracy) {}

template <class Curve>
GlobalBootstrap<Curve>::GlobalBootstrap(
    std::vector<ext::shared_ptr<typename Traits::helper> > additionalHelpers,
    ext::function<std::vector<Date>()> additionalDates,
    ext::function<Array()> additionalErrors,
    Real accuracy)
: ts_(nullptr), accuracy_(accuracy), additionalHelpers_(std::move(additionalHelpers)),
  additionalDates_(std::move(additionalDates)), additionalErrors_(std::move(additionalErrors)) {}

template <class Curve> void GlobalBootstrap<Curve>::setup(Curve *ts) {
    ts_ = ts;
    for (Size j = 0; j < ts_->instruments_.size(); ++j)
        ts_->registerWith(ts_->instruments_[j]);
    for (Size j = 0; j < additionalHelpers_.size(); ++j)
        ts_->registerWith(additionalHelpers_[j]);

    // do not initialize yet: instruments could be invalid here
    // but valid later when bootstrapping is actually required
}

template <class Curve> void GlobalBootstrap<Curve>::initialize() const {

    // ensure helpers are sorted
    std::sort(ts_->instruments_.begin(), ts_->instruments_.end(), detail::BootstrapHelperSorter());
    std::sort(additionalHelpers_.begin(), additionalHelpers_.end(), detail::BootstrapHelperSorter());

    // skip expired helpers
    Date firstDate = Traits::initialDate(ts_);

    firstHelper_ = 0;
    if (!ts_->instruments_.empty()) {
        while (firstHelper_ < ts_->instruments_.size() && ts_->instruments_[firstHelper_]->pillarDate() <= firstDate)
            ++firstHelper_;
    }
    numberHelpers_ = ts_->instruments_.size() - firstHelper_;

    // skip expired additional helpers
    firstAdditionalHelper_ = 0;
    if (!additionalHelpers_.empty()) {
        while (firstAdditionalHelper_ < additionalHelpers_.size() &&
               additionalHelpers_[firstAdditionalHelper_]->pillarDate() <= firstDate)
            ++firstAdditionalHelper_;
    }
    numberAdditionalHelpers_ = additionalHelpers_.size() - firstAdditionalHelper_;

    // skip expired additional dates
    std::vector<Date> additionalDates;
    if (!(additionalDates_ == QL_NULL_FUNCTION))
        additionalDates = additionalDates_();
    firstAdditionalDate_ = 0;
    if (!additionalDates.empty()) {
        while (firstAdditionalDate_ < additionalDates.size() && additionalDates[firstAdditionalDate_] <= firstDate)
            ++firstAdditionalDate_;
    }
    numberAdditionalDates_ = additionalDates.size() - firstAdditionalDate_;

    QL_REQUIRE(numberHelpers_ + numberAdditionalDates_ >= Interpolator::requiredPoints - 1,
               "not enough alive instruments (" << numberHelpers_ << ") + additional dates (" << numberAdditionalDates_
                                                << ") = " << numberHelpers_ + numberAdditionalDates_ << " provided, "
                                                << Interpolator::requiredPoints - 1 << " required");

    // calculate dates and times
    std::vector<Date> &dates = ts_->dates_;
    std::vector<Time> &times = ts_->times_;

    // first populate the dates vector and make sure they are sorted and there are no duplicates
    dates.clear();
    dates.push_back(firstDate);
    for (Size j = 0; j < numberHelpers_; ++j)
        dates.push_back(ts_->instruments_[firstHelper_ + j]->pillarDate());
    for (Size j = firstAdditionalDate_; j < numberAdditionalDates_; ++j)
        dates.push_back(additionalDates[firstAdditionalDate_ + j]);
    std::sort(dates.begin(), dates.end());
    auto it = std::unique(dates.begin(), dates.end());
    QL_REQUIRE(it == dates.end(), "duplicate dates among alive instruments and additional dates");

    // build times vector
    times.clear();
    for (auto& date : dates)
        times.push_back(ts_->timeFromReference(date));

    // determine maxDate
    Date maxDate = firstDate;
    for (Size j = 0; j < numberHelpers_; ++j) {
        maxDate = std::max(ts_->instruments_[firstHelper_ + j]->latestRelevantDate(), maxDate);
    }
    for (Size j = 0; j < numberAdditionalDates_; ++j) {
        maxDate = std::max(additionalDates[firstAdditionalDate_ + j], maxDate);
    }
    ts_->maxDate_ = maxDate;

    // set initial guess only if the current curve cannot be used as guess
    if (!validCurve_ || ts_->data_.size() != dates.size()) {
        // ts_->data_[0] is the only relevant item,
        // but reasonable numbers might be needed for the whole data vector
        // because, e.g., of interpolation's early checks
        ts_->data_ = std::vector<Real>(dates.size(), Traits::initialValue(ts_));
    }
    initialized_ = true;
}

template <class Curve> void GlobalBootstrap<Curve>::calculate() const {

    // we might have to call initialize even if the curve is initialized
    // and not moving, just because helpers might be date relative and change
    // with evaluation date change.
    // anyway it makes little sense to use date relative helpers with a
    // non-moving curve if the evaluation date changes
    if (!initialized_ || ts_->moving_)
        initialize();

    // setup helpers
    for (Size j = 0; j < numberHelpers_; ++j) {
        const ext::shared_ptr<typename Traits::helper> &helper = ts_->instruments_[firstHelper_ + j];
        // check for valid quote
        QL_REQUIRE(helper->quote()->isValid(), io::ordinal(j + 1)
                                                   << " instrument (maturity: " << helper->maturityDate()
                                                   << ", pillar: " << helper->pillarDate() << ") has an invalid quote");
        // don't try this at home!
        // This call creates helpers, and removes "const".
        // There is a significant interaction with observability.
        helper->setTermStructure(const_cast<Curve *>(ts_));
    }

    // setup additional helpers
    for (Size j = 0; j < numberAdditionalHelpers_; ++j) {
        const ext::shared_ptr<typename Traits::helper> &helper = additionalHelpers_[firstAdditionalHelper_ + j];
        QL_REQUIRE(helper->quote()->isValid(), io::ordinal(j + 1)
                                                   << " additional instrument (maturity: " << helper->maturityDate()
                                                   << ") has an invalid quote");
        helper->setTermStructure(const_cast<Curve *>(ts_));
    }

    Real accuracy = accuracy_ != Null<Real>() ? accuracy_ : ts_->accuracy_;

    // setup optimizer and EndCriteria
    Real optEps = accuracy;
    LevenbergMarquardt optimizer(optEps, optEps, optEps); // FIXME hardcoded tolerances
    EndCriteria ec(1000, 10, optEps, optEps, optEps);      // FIXME hardcoded values here as well

    // setup interpolation
    if (!validCurve_) {
        ts_->interpolation_ =
            ts_->interpolator_.interpolate(ts_->times_.begin(), ts_->times_.end(), ts_->data_.begin());
    }

    // determine bounds, we use an unconstrained optimisation transforming the free variables to [lowerBound,upperBound]
    std::vector<Real> lowerBounds(numberHelpers_ + numberAdditionalDates_),
        upperBounds(numberHelpers_ + numberAdditionalDates_);
    for (Size i = 0; i < numberHelpers_ + numberAdditionalDates_; ++i) {
        // just pass zero as the first alive helper, it's not used in the standard QL traits anyway
        lowerBounds[i] = Traits::minValueAfter(i + 1, ts_, validCurve_, 0);
        upperBounds[i] = Traits::maxValueAfter(i + 1, ts_, validCurve_, 0);
    }

    // setup cost function
    class TargetFunction : public CostFunction {
      public:
        TargetFunction(const Size firstHelper,
                       const Size numberHelpers,
                       ext::function<Array()> additionalErrors,
                       Curve* ts,
                       std::vector<Real> lowerBounds,
                       std::vector<Real> upperBounds)
        : firstHelper_(firstHelper), numberHelpers_(numberHelpers),
          additionalErrors_(std::move(additionalErrors)), ts_(ts),
          lowerBounds_(std::move(lowerBounds)), upperBounds_(std::move(upperBounds)) {}

        Real transformDirect(const Real x, const Size i) const {
            return (std::atan(x) + M_PI_2) / M_PI * (upperBounds_[i] - lowerBounds_[i]) + lowerBounds_[i];
        }

        Real transformInverse(const Real y, const Size i) const {
            return std::tan((y - lowerBounds_[i]) * M_PI / (upperBounds_[i] - lowerBounds_[i]) - M_PI_2);
        }

        Real value(const Array& x) const override {
            Array v = values(x);
            std::transform(v.begin(), v.end(), v.begin(), square<Real>());
            return std::sqrt(std::accumulate(v.begin(), v.end(), 0.0) / static_cast<Real>(v.size()));
        }

        Disposable<Array> values(const Array& x) const override {
            for (Size i = 0; i < x.size(); ++i) {
                Traits::updateGuess(ts_->data_, transformDirect(x[i], i), i + 1);
            }
            ts_->interpolation_.update();
            std::vector<Real> result(numberHelpers_);
            for (Size i = 0; i < numberHelpers_; ++i) {
                result[i] = ts_->instruments_[firstHelper_ + i]->quote()->value() -
                            ts_->instruments_[firstHelper_ + i]->impliedQuote();
            }
            if (!(additionalErrors_ == QL_NULL_FUNCTION)) {
                Array tmp = additionalErrors_();
                result.resize(numberHelpers_ + tmp.size());
                for (Size i = 0; i < tmp.size(); ++i) {
                    result[numberHelpers_ + i] = tmp[i];
                }
            }
            Array asArray(result.begin(), result.end());
            return asArray;
        }

      private:
        Size firstHelper_, numberHelpers_;
        ext::function<Array()> additionalErrors_;
        Curve *ts_;
        const std::vector<Real> lowerBounds_, upperBounds_;
    };
    TargetFunction cost(firstHelper_, numberHelpers_, additionalErrors_, ts_, lowerBounds, upperBounds);

    // setup guess
    Array guess(numberHelpers_ + numberAdditionalDates_);
    for (Size i = 0; i < guess.size(); ++i) {
        // just pass zero as the first alive helper, it's not used in the standard QL traits anyway
        guess[i] = cost.transformInverse(Traits::guess(i + 1, ts_, validCurve_, 0), i);
    }

    // setup problem
    NoConstraint noConstraint;
    Problem problem(cost, noConstraint, guess);

    // run optimization
    optimizer.minimize(problem, ec);

    // evaluate target function on best value found to ensure that data_ contains the optimal value
    Real finalTargetError = cost.value(problem.currentValue());

    // check final error
    QL_REQUIRE(finalTargetError <= accuracy,
               "global bootstrap failed, error is " << finalTargetError << ", accuracy is " << accuracy);

    // set valid flag
    validCurve_ = true;
}

} // namespace QuantLib

#endif
