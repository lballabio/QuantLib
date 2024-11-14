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
*/

#ifndef quantlib_global_bootstrap_hpp
#define quantlib_global_bootstrap_hpp

#include <ql/functional.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/termstructures/bootstraperror.hpp>
#include <ql/termstructures/bootstraphelper.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <algorithm>
#include <utility>

namespace QuantLib {

//! Global boostrapper, with additional restrictions
template <class Curve> class GlobalBootstrap {
    typedef typename Curve::traits_type Traits;             // ZeroYield, Discount, ForwardRate
    typedef typename Curve::interpolator_type Interpolator; // Linear, LogLinear, ...

  public:
    GlobalBootstrap(Real accuracy = Null<Real>(),
                    ext::shared_ptr<OptimizationMethod> optimizer = nullptr,
                    ext::shared_ptr<EndCriteria> endCriteria = nullptr);
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
                    std::function<std::vector<Date>()> additionalDates,
                    std::function<Array()> additionalErrors,
                    Real accuracy = Null<Real>(),
                    ext::shared_ptr<OptimizationMethod> optimizer = nullptr,
                    ext::shared_ptr<EndCriteria> endCriteria = nullptr);
    void setup(Curve *ts);
    void calculate() const;

  private:
    void initialize() const;
    Curve *ts_;
    Real accuracy_;
    ext::shared_ptr<OptimizationMethod> optimizer_;
    ext::shared_ptr<EndCriteria> endCriteria_;
    mutable std::vector<ext::shared_ptr<typename Traits::helper> > additionalHelpers_;
    std::function<std::vector<Date>()> additionalDates_;
    std::function<Array()> additionalErrors_;
    mutable bool initialized_ = false, validCurve_ = false;
    mutable Size firstHelper_, numberHelpers_;
    mutable Size firstAdditionalHelper_, numberAdditionalHelpers_;
};

// template definitions

template <class Curve>
GlobalBootstrap<Curve>::GlobalBootstrap(
    Real accuracy,
    ext::shared_ptr<OptimizationMethod> optimizer,
    ext::shared_ptr<EndCriteria> endCriteria)
: ts_(nullptr), accuracy_(accuracy), optimizer_(std::move(optimizer)),
  endCriteria_(std::move(endCriteria)) {}

template <class Curve>
GlobalBootstrap<Curve>::GlobalBootstrap(
    std::vector<ext::shared_ptr<typename Traits::helper> > additionalHelpers,
    std::function<std::vector<Date>()> additionalDates,
    std::function<Array()> additionalErrors,
    Real accuracy,
    ext::shared_ptr<OptimizationMethod> optimizer,
    ext::shared_ptr<EndCriteria> endCriteria)
: ts_(nullptr), accuracy_(accuracy), optimizer_(std::move(optimizer)),
  endCriteria_(std::move(endCriteria)), additionalHelpers_(std::move(additionalHelpers)),
  additionalDates_(std::move(additionalDates)), additionalErrors_(std::move(additionalErrors)) {}

template <class Curve> void GlobalBootstrap<Curve>::setup(Curve *ts) {
    ts_ = ts;
    for (Size j = 0; j < ts_->instruments_.size(); ++j)
        ts_->registerWithObservables(ts_->instruments_[j]);
    for (Size j = 0; j < additionalHelpers_.size(); ++j)
        ts_->registerWithObservables(additionalHelpers_[j]);

    // setup optimizer and EndCriteria
    Real accuracy = accuracy_ != Null<Real>() ? accuracy_ : ts_->accuracy_;
    if (!optimizer_) {
        optimizer_ = ext::make_shared<LevenbergMarquardt>(accuracy, accuracy, accuracy);
    }
    if (!endCriteria_) {
        endCriteria_ = ext::make_shared<EndCriteria>(1000, 10, accuracy, accuracy, accuracy);
    }

    // do not initialize yet: instruments could be invalid here
    // but valid later when bootstrapping is actually required
}

template <class Curve> void GlobalBootstrap<Curve>::initialize() const {

    // ensure helpers are sorted
    std::sort(ts_->instruments_.begin(), ts_->instruments_.end(), detail::BootstrapHelperSorter());
    std::sort(additionalHelpers_.begin(), additionalHelpers_.end(), detail::BootstrapHelperSorter());

    // skip expired helpers
    const Date firstDate = Traits::initialDate(ts_);

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
    if (additionalDates_)
        additionalDates = additionalDates_();
    if (!additionalDates.empty()) {
        additionalDates.erase(
            std::remove_if(additionalDates.begin(), additionalDates.end(),
                           [=](const Date& date) { return date <= firstDate; }),
            additionalDates.end()
        );
    }
    const Size numberAdditionalDates = additionalDates.size();

    QL_REQUIRE(numberHelpers_ + numberAdditionalDates >= Interpolator::requiredPoints - 1,
               "not enough alive instruments (" << numberHelpers_ << ") + additional dates (" << numberAdditionalDates
                                                << ") = " << numberHelpers_ + numberAdditionalDates << " provided, "
                                                << Interpolator::requiredPoints - 1 << " required");

    // calculate dates and times
    std::vector<Date> &dates = ts_->dates_;
    std::vector<Time> &times = ts_->times_;

    // first populate the dates vector and make sure they are sorted and there are no duplicates
    dates.clear();
    dates.push_back(firstDate);
    for (Size j = 0; j < numberHelpers_; ++j)
        dates.push_back(ts_->instruments_[firstHelper_ + j]->pillarDate());
    dates.insert(dates.end(), additionalDates.begin(), additionalDates.end());
    std::sort(dates.begin(), dates.end());
    auto it = std::unique(dates.begin(), dates.end());
    QL_REQUIRE(it == dates.end(), "duplicate dates among alive instruments and additional dates");

    // build times vector
    times.clear();
    for (auto& date : dates)
        times.push_back(ts_->timeFromReference(date));

    // determine maxDate
    Date maxDate = dates.back();
    for (Size j = 0; j < numberHelpers_; ++j) {
        maxDate = std::max(ts_->instruments_[firstHelper_ + j]->latestRelevantDate(), maxDate);
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

    // setup interpolation
    if (!validCurve_) {
        ts_->interpolation_ =
            ts_->interpolator_.interpolate(ts_->times_.begin(), ts_->times_.end(), ts_->data_.begin());
    }

    // determine bounds, we use an unconstrained optimisation transforming the free variables to [lowerBound,upperBound]
    const Size numberBounds = ts_->times_.size() - 1;
    std::vector<Real> lowerBounds(numberBounds), upperBounds(numberBounds);
    for (Size i = 0; i < numberBounds; ++i) {
        // just pass zero as the first alive helper, it's not used in the standard QL traits anyway
        lowerBounds[i] = Traits::minValueAfter(i + 1, ts_, validCurve_, 0);
        upperBounds[i] = Traits::maxValueAfter(i + 1, ts_, validCurve_, 0);
    }

    // setup cost function
    class TargetFunction : public CostFunction {
      public:
        TargetFunction(const Size firstHelper,
                       const Size numberHelpers,
                       std::function<Array()> additionalErrors,
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

        Array values(const Array& x) const override {
            for (Size i = 0; i < x.size(); ++i) {
                Traits::updateGuess(ts_->data_, transformDirect(x[i], i), i + 1);
            }
            ts_->interpolation_.update();
            std::vector<Real> result(numberHelpers_);
            for (Size i = 0; i < numberHelpers_; ++i) {
                result[i] = ts_->instruments_[firstHelper_ + i]->quote()->value() -
                            ts_->instruments_[firstHelper_ + i]->impliedQuote();
            }
            if (additionalErrors_) {
                Array tmp = additionalErrors_();
                result.resize(numberHelpers_ + tmp.size());
                for (Size i = 0; i < tmp.size(); ++i) {
                    result[numberHelpers_ + i] = tmp[i];
                }
            }
            return Array(result.begin(), result.end());
        }

      private:
        Size firstHelper_, numberHelpers_;
        std::function<Array()> additionalErrors_;
        Curve *ts_;
        const std::vector<Real> lowerBounds_, upperBounds_;
    };
    TargetFunction cost(firstHelper_, numberHelpers_, additionalErrors_, ts_,
                        std::move(lowerBounds), std::move(upperBounds));

    // setup guess
    Array guess(numberBounds);
    for (Size i = 0; i < numberBounds; ++i) {
        // just pass zero as the first alive helper, it's not used in the standard QL traits anyway
        guess[i] = cost.transformInverse(Traits::guess(i + 1, ts_, validCurve_, 0), i);
    }

    // setup problem
    NoConstraint noConstraint;
    Problem problem(cost, noConstraint, guess);

    // run optimization
    EndCriteria::Type endType = optimizer_->minimize(problem, *endCriteria_);

    // check the end criteria
    QL_REQUIRE(EndCriteria::succeeded(endType),
               "global bootstrap failed to minimize to required accuracy: " << endType);

    // set valid flag
    validCurve_ = true;
}

} // namespace QuantLib

#endif
