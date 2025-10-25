/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2019 SoftSolutions! S.r.l.
 Copyright (C) 2025 Peter Caspers

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

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
#include <ql/termstructures/bootstraphelper.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <algorithm>
#include <utility>

namespace QuantLib {

class MultiCurveBootstrap;

class MultiCurveBootstrapContributor {
public:
    virtual ~MultiCurveBootstrapContributor() {}
    virtual void
    setParentBootstrapper(const QuantLib::ext::shared_ptr<MultiCurveBootstrap>& b) const = 0;
    virtual Array guess() const = 0;
    virtual void setupCostFunction() const = 0;
    virtual void setCostFunctionArgument(const Array& v) const = 0;
    virtual Array evaluateCostFunction() const = 0;
    virtual void setToValid() const = 0;
};

class MultiCurveBootstrap : public QuantLib::ext::enable_shared_from_this<MultiCurveBootstrap> {
  public:
    explicit MultiCurveBootstrap(Real accuracy);
    explicit MultiCurveBootstrap(ext::shared_ptr<OptimizationMethod> optimizer = nullptr,
                        ext::shared_ptr<EndCriteria> endCriteria = nullptr);
    void add(const MultiCurveBootstrapContributor* c);
    void runMultiCurveBootstrap();
    void setOtherContributorsToValid() const;
    void finalizeCalculation();

  private:
    ext::shared_ptr<OptimizationMethod> optimizer_;
    ext::shared_ptr<EndCriteria> endCriteria_;
    std::vector<const MultiCurveBootstrapContributor*> contributors_;
};

class AdditionalBootstrapVariables {
  public:
    virtual ~AdditionalBootstrapVariables() = default;
    // Initialize variables to initial guesses and return them.
    virtual Array initialize(bool validData) = 0;
    // Update variables to given values.
    virtual void update(const Array& x) = 0;
};

/*! Global boostrapper, with additional restrictions

  The additionalDates functor must return a set of additional dates to add to the
  interpolation grid. These dates must only depend on the global evaluation date.

  The additionalPenalties functor must yield at least as many values such that

  number of (usual, alive) rate helpers + number of additional values >= number of data points - 1

  (note that the data points contain t=0). These values are treated as additional
  error terms in the optimization. The usual rate helpers return quoteError here.
  All error terms are equally weighted.

  The additionalHelpers are registered with the curve like the usual rate helpers,
  but no pillar dates or error terms are added for them. Pillars and error terms
  have to be added by additionalDates and additionalPenalties.

  The additionalVariables interface manages a set of additional variables to add
  to the optimization. This is useful to optimize model parameters used by rate
  helpers, for example, convexity adjustments for futures. See SimpleQuoteVariables
  for a concrete implementation of this interface.

  WARNING: This class is known to work with Traits Discount, ZeroYield, Forward,
  i.e. the usual IR curves traits in QL. It requires Traits::transformDirect()
  and Traits::transformInverse() to be implemented. Also, check the usage of
  Traits::updateGuess(), Traits::guess() in this class.
*/
template <class Curve> class GlobalBootstrap : public MultiCurveBootstrapContributor {
    typedef typename Curve::traits_type Traits;             // ZeroYield, Discount, ForwardRate
    typedef typename Curve::interpolator_type Interpolator; // Linear, LogLinear, ...
    typedef std::function<Array(const std::vector<Time>&, const std::vector<Real>&)>
        AdditionalPenalties;

  public:
    GlobalBootstrap(Real accuracy = Null<Real>(),
                    ext::shared_ptr<OptimizationMethod> optimizer = nullptr,
                    ext::shared_ptr<EndCriteria> endCriteria = nullptr);
    GlobalBootstrap(std::vector<ext::shared_ptr<typename Traits::helper> > additionalHelpers,
                    std::function<std::vector<Date>()> additionalDates,
                    AdditionalPenalties additionalPenalties,
                    Real accuracy = Null<Real>(),
                    ext::shared_ptr<OptimizationMethod> optimizer = nullptr,
                    ext::shared_ptr<EndCriteria> endCriteria = nullptr,
                    ext::shared_ptr<AdditionalBootstrapVariables> additionalVariables = nullptr);
    GlobalBootstrap(std::vector<ext::shared_ptr<typename Traits::helper> > additionalHelpers,
                    std::function<std::vector<Date>()> additionalDates,
                    std::function<Array()> additionalPenalties,
                    Real accuracy = Null<Real>(),
                    ext::shared_ptr<OptimizationMethod> optimizer = nullptr,
                    ext::shared_ptr<EndCriteria> endCriteria = nullptr,
                    ext::shared_ptr<AdditionalBootstrapVariables> additionalVariables = nullptr);
    void setup(Curve *ts);
    void calculate() const;

  private:
    void initialize() const;
    void
    setParentBootstrapper(const QuantLib::ext::shared_ptr<MultiCurveBootstrap>& b) const override;
    Array guess() const override;
    void setupCostFunction() const override;
    void setCostFunctionArgument(const Array& v) const override;
    Array evaluateCostFunction() const override;
    void setToValid() const override;
    Curve* ts_;
    Real accuracy_;
    ext::shared_ptr<OptimizationMethod> optimizer_;
    ext::shared_ptr<EndCriteria> endCriteria_;
    mutable std::vector<ext::shared_ptr<typename Traits::helper> > additionalHelpers_;
    std::function<std::vector<Date>()> additionalDates_;
    AdditionalPenalties additionalPenalties_;
    ext::shared_ptr<AdditionalBootstrapVariables> additionalVariables_;
    mutable bool initialized_ = false, validCurve_ = false;
    mutable Size firstHelper_ = 0, numberHelpers_ = 0;
    mutable Size firstAdditionalHelper_ = 0, numberAdditionalHelpers_= 0;
    mutable QuantLib::ext::shared_ptr<MultiCurveBootstrap> parentBootstrapper_ = nullptr;
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
    AdditionalPenalties additionalPenalties,
    Real accuracy,
    ext::shared_ptr<OptimizationMethod> optimizer,
    ext::shared_ptr<EndCriteria> endCriteria,
    ext::shared_ptr<AdditionalBootstrapVariables> additionalVariables)
: ts_(nullptr), accuracy_(accuracy), optimizer_(std::move(optimizer)),
  endCriteria_(std::move(endCriteria)), additionalHelpers_(std::move(additionalHelpers)),
  additionalDates_(std::move(additionalDates)),
  additionalPenalties_(std::move(additionalPenalties)),
  additionalVariables_(std::move(additionalVariables)) {}

template <class Curve>
GlobalBootstrap<Curve>::GlobalBootstrap(
    std::vector<ext::shared_ptr<typename Traits::helper>> additionalHelpers,
    std::function<std::vector<Date>()> additionalDates,
    std::function<Array()> additionalPenalties,
    Real accuracy,
    ext::shared_ptr<OptimizationMethod> optimizer,
    ext::shared_ptr<EndCriteria> endCriteria,
    ext::shared_ptr<AdditionalBootstrapVariables> additionalVariables)
: GlobalBootstrap(std::move(additionalHelpers), std::move(additionalDates),
                  additionalPenalties
                    ? [f=std::move(additionalPenalties)](const std::vector<Time>&, const std::vector<Real>&) {
                        return f();
                    }
                    : AdditionalPenalties(),
                  accuracy, std::move(optimizer), std::move(endCriteria),
                  std::move(additionalVariables)) {}

template <class Curve>
void GlobalBootstrap<Curve>::setParentBootstrapper(const QuantLib::ext::shared_ptr<MultiCurveBootstrap>& b) const {
    parentBootstrapper_ = b;
}

template <class Curve> void GlobalBootstrap<Curve>::setToValid() const { validCurve_ = true; }

template <class Curve> void GlobalBootstrap<Curve>::setup(Curve* ts) {
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
        validCurve_ = false;
    }
    initialized_ = true;
}

template <class Curve> void GlobalBootstrap<Curve>::setupCostFunction() const {

    // for single-curve boostrap, this was done in LazyObject::calculate() already, but for
    // multi-curve boostrap we have to do this manually for all contributing curves except
    // the main one, because calculate() is never triggered for them
    ts_->setCalculated(true);

    // we might have to call initialize even if the curve is initialized
    // and not moving, just because helpers might be date relative and change
    // with evaluation date change.
    // anyway it makes little sense to use date relative helpers with a
    // non-moving curve if the evaluation date changes
    if (!initialized_ || ts_->moving_)
        initialize();

    // setup helpers
    for (Size j = 0; j < numberHelpers_; ++j) {
        const ext::shared_ptr<typename Traits::helper>& helper = ts_->instruments_[firstHelper_ + j];
        // check for valid quote
        QL_REQUIRE(helper->quote()->isValid(), io::ordinal(j + 1)
                                                   << " instrument (maturity: " << helper->maturityDate()
                                                   << ", pillar: " << helper->pillarDate() << ") has an invalid quote");
        // don't try this at home!
        // This call creates helpers, and removes "const".
        // There is a significant interaction with observability.
        helper->setTermStructure(const_cast<Curve*>(ts_));
    }

    // setup additional helpers
    for (Size j = 0; j < numberAdditionalHelpers_; ++j) {
        const ext::shared_ptr<typename Traits::helper>& helper = additionalHelpers_[firstAdditionalHelper_ + j];
        QL_REQUIRE(helper->quote()->isValid(), io::ordinal(j + 1)
                                                   << " additional instrument (maturity: " << helper->maturityDate()
                                                   << ") has an invalid quote");
        helper->setTermStructure(const_cast<Curve*>(ts_));
    }

    // setup interpolation
    if (!validCurve_) {
        ts_->interpolation_ =
            ts_->interpolator_.interpolate(ts_->times_.begin(), ts_->times_.end(), ts_->data_.begin());
    }
}

template <class Curve>
Array GlobalBootstrap<Curve>::guess() const {
    // Initial guess. We have guesses for the curve values first (numberPillars),
    // followed by guesses for the additional variables.
    Array additionalGuesses;
    if (additionalVariables_) {
        additionalGuesses = additionalVariables_->initialize(validCurve_);
    }
    Array guess(ts_->times_.size() - 1 + additionalGuesses.size());
    for (Size i = 0; i < ts_->times_.size() - 1; ++i) {
        // just pass zero as the first alive helper, it's not used in the standard QL traits anyway
        // update ts_->data_ since Traits::guess() usually depends on previous values
        Traits::updateGuess(ts_->data_, Traits::guess(i + 1, ts_, validCurve_, 0), i + 1);
        guess[i] = Traits::transformInverse(ts_->data_[i + 1], i + 1, ts_);
    }
    std::copy(additionalGuesses.begin(), additionalGuesses.end(),
              guess.begin() + ts_->times_.size() - 1);
    return guess;
}

template <class Curve>
void GlobalBootstrap<Curve>::setCostFunctionArgument(const Array& x) const {
    // x has the same layout as guess above: the first numberPillars values go into
    // the curve, while the rest are new values for the additional variables.
    for (Size i = 0; i < ts_->times_.size() - 1; ++i) {
        Traits::updateGuess(ts_->data_, Traits::transformDirect(x[i], i + 1, ts_), i + 1);
    }
    ts_->interpolation_.update();
    if (additionalVariables_) {
        additionalVariables_->update(Array(x.begin() + ts_->times_.size() - 1, x.end()));
    }
}

template <class Curve>
Array GlobalBootstrap<Curve>::evaluateCostFunction() const {
    Array additionalErrors;
    if (additionalPenalties_) {
        additionalErrors = additionalPenalties_(ts_->times_, ts_->data_);
    }
    Array result(numberHelpers_ + additionalErrors.size());
    std::transform(ts_->instruments_.begin() + firstHelper_, ts_->instruments_.end(),
                   result.begin(), [](const auto& helper) { return helper->quoteError(); });
    std::copy(additionalErrors.begin(), additionalErrors.end(), result.begin() + numberHelpers_);
    return result;
}
    
template <class Curve> void GlobalBootstrap<Curve>::calculate() const {

  if (parentBootstrapper_) {
        parentBootstrapper_->runMultiCurveBootstrap();
        return;
    }

    // single curve boostrap

    setupCostFunction();

    NoConstraint noConstraint;

    SimpleCostFunction costFunction([this](const Array& x) {
        this->setCostFunctionArgument(x);
        return this->evaluateCostFunction();
    });

    Problem problem(costFunction, noConstraint, guess());
    EndCriteria::Type endType = optimizer_->minimize(problem, *endCriteria_);
    QL_REQUIRE(EndCriteria::succeeded(endType),
               "global bootstrap failed to minimize to required accuracy: " << endType);
    validCurve_ = true;
}

} // namespace QuantLib

#endif
