/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ferdinando Ametrano
 Copyright (C) 2007 Allen Kuo

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

#include <ql/cashflows/cashflows.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <ql/math/optimization/costfunction.hpp>
#include <ql/math/optimization/simplex.hpp>
#include <ql/pricingengines/bond/bondfunctions.hpp>
#include <ql/termstructures/yield/fittedbonddiscountcurve.hpp>
#include <ql/time/daycounters/simpledaycounter.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <utility>

using std::vector;

namespace QuantLib {

    class FittedBondDiscountCurve::FittingMethod::FittingCost
        : public CostFunction {
        friend class FittedBondDiscountCurve::FittingMethod;
      public:
        explicit FittingCost(
                       FittedBondDiscountCurve::FittingMethod* fittingMethod);
        Real value(const Array& x) const override;
        Array values(const Array& x) const override;

      private:
        FittedBondDiscountCurve::FittingMethod* fittingMethod_;
    };


    FittedBondDiscountCurve::FittedBondDiscountCurve(
        Natural settlementDays,
        const Calendar& calendar,
        vector<ext::shared_ptr<BondHelper> > bondHelpers,
        const DayCounter& dayCounter,
        const FittingMethod& fittingMethod,
        Real accuracy,
        Size maxEvaluations,
        Array guess,
        Real simplexLambda,
        Size maxStationaryStateIterations)
    : YieldTermStructure(settlementDays, calendar, dayCounter), accuracy_(accuracy),
      maxEvaluations_(maxEvaluations), simplexLambda_(simplexLambda),
      maxStationaryStateIterations_(maxStationaryStateIterations), guessSolution_(std::move(guess)),
      bondHelpers_(std::move(bondHelpers)), fittingMethod_(fittingMethod) {
        fittingMethod_->curve_ = this;
        setup();
    }


    FittedBondDiscountCurve::FittedBondDiscountCurve(
        const Date& referenceDate,
        vector<ext::shared_ptr<BondHelper> > bondHelpers,
        const DayCounter& dayCounter,
        const FittingMethod& fittingMethod,
        Real accuracy,
        Size maxEvaluations,
        Array guess,
        Real simplexLambda,
        Size maxStationaryStateIterations)
    : YieldTermStructure(referenceDate, Calendar(), dayCounter), accuracy_(accuracy),
      maxEvaluations_(maxEvaluations), simplexLambda_(simplexLambda),
      maxStationaryStateIterations_(maxStationaryStateIterations), guessSolution_(std::move(guess)),
      bondHelpers_(std::move(bondHelpers)), fittingMethod_(fittingMethod) {

        fittingMethod_->curve_ = this;
        setup();
    }

    FittedBondDiscountCurve::FittedBondDiscountCurve(
                            Natural settlementDays,
                            const Calendar& calendar,
                            const FittingMethod& fittingMethod,
                            Array parameters,
                            Date maxDate,
                            const DayCounter& dayCounter)
    : YieldTermStructure(settlementDays, calendar, dayCounter), accuracy_(1e-10),
      maxEvaluations_(0), guessSolution_(std::move(parameters)),
      maxDate_(maxDate), fittingMethod_(fittingMethod) {

        fittingMethod_->curve_ = this;
        setup();
    }

    FittedBondDiscountCurve::FittedBondDiscountCurve(
                                const Date& referenceDate,
                                const FittingMethod& fittingMethod,
                                Array parameters,
                                Date maxDate,
                                const DayCounter& dayCounter)
    : YieldTermStructure(referenceDate, Calendar(), dayCounter), accuracy_(1e-10),
      maxEvaluations_(0), guessSolution_(std::move(parameters)),
      maxDate_(maxDate), fittingMethod_(fittingMethod) {

        fittingMethod_->curve_ = this;
        setup();
    }


    void FittedBondDiscountCurve::resetGuess(const Array& guess) {
        QL_REQUIRE(guess.empty() || guess.size() == fittingMethod_->size(), "guess is of wrong size");
        guessSolution_ = guess;
        update();
    }


    void FittedBondDiscountCurve::performCalculations() const {

        if (maxEvaluations_!= 0) {
            // we need to fit, so we require helpers
            QL_REQUIRE(!bondHelpers_.empty(), "no bond helpers given");
        }

        if (maxEvaluations_ == 0) {
            // no fit, but we need either an explicit max date or
            // helpers from which to deduce it
            QL_REQUIRE(maxDate_ != Date() || !bondHelpers_.empty(),
                       "no bond helpers or max date given");
        }

        if (!bondHelpers_.empty()) {
            maxDate_ = Date::minDate();
            Date refDate = referenceDate();

            // double check bond quotes still valid and/or instruments not expired
            for (Size i=0; i<bondHelpers_.size(); ++i) {
                ext::shared_ptr<Bond> bond = bondHelpers_[i]->bond();
                QL_REQUIRE(bondHelpers_[i]->quote()->isValid(),
                           io::ordinal(i+1) << " bond (maturity: " <<
                           bond->maturityDate() << ") has an invalid price quote");
                Date bondSettlement = bond->settlementDate();
                QL_REQUIRE(bondSettlement>=refDate,
                           io::ordinal(i+1) << " bond settlemente date (" <<
                           bondSettlement << ") before curve reference date (" <<
                           refDate << ")");
                QL_REQUIRE(BondFunctions::isTradable(*bond, bondSettlement),
                           io::ordinal(i+1) << " bond non tradable at " <<
                           bondSettlement << " settlement date (maturity"
                           " being " << bond->maturityDate() << ")");
                maxDate_ = std::max(maxDate_, bondHelpers_[i]->pillarDate());
                bondHelpers_[i]->setTermStructure(
                                                  const_cast<FittedBondDiscountCurve*>(this));
            }
        }

        fittingMethod_->init();
        fittingMethod_->calculate();
    }


    FittedBondDiscountCurve::FittingMethod::FittingMethod(
        bool constrainAtZero,
        const Array& weights,
        ext::shared_ptr<OptimizationMethod> optimizationMethod,
        Array l2,
        const Real minCutoffTime,
        const Real maxCutoffTime,
        Constraint constraint)
    : constrainAtZero_(constrainAtZero), weights_(weights), l2_(std::move(l2)),
      calculateWeights_(weights.empty()), optimizationMethod_(std::move(optimizationMethod)),
      constraint_(std::move(constraint)),
      minCutoffTime_(minCutoffTime), maxCutoffTime_(maxCutoffTime) {
        if (constraint_.empty())
            constraint_ = NoConstraint();
    }

    void FittedBondDiscountCurve::FittingMethod::init() {

        if (curve_->maxEvaluations_ == 0)
            return; // we can skip the rest

        // yield conventions
        DayCounter yieldDC = curve_->dayCounter();
        Compounding yieldComp = Compounded;
        Frequency yieldFreq = Annual;

        Size n = curve_->bondHelpers_.size();
        costFunction_ = ext::make_shared<FittingCost>(this);

        for (auto& bondHelper : curve_->bondHelpers_) {
            bondHelper->setTermStructure(curve_);
        }

        if (calculateWeights_) {
            if (weights_.empty())
                weights_ = Array(n);

            Real squaredSum = 0.0;
            for (Size i=0; i<curve_->bondHelpers_.size(); ++i) {
                ext::shared_ptr<Bond> bond = curve_->bondHelpers_[i]->bond();

                Real amount = curve_->bondHelpers_[i]->quote()->value();
                Bond::Price price(amount, curve_->bondHelpers_[i]->priceType());

                Date bondSettlement = bond->settlementDate();
                Rate ytm = BondFunctions::yield(*bond, price,
                                                yieldDC, yieldComp, yieldFreq,
                                                bondSettlement);

                Time dur = BondFunctions::duration(*bond, ytm,
                                                   yieldDC, yieldComp, yieldFreq,
                                                   Duration::Modified,
                                                   bondSettlement);
                weights_[i] = 1.0/dur;
                squaredSum += weights_[i]*weights_[i];
            }
            weights_ /= std::sqrt(squaredSum);
        }

        QL_REQUIRE(weights_.size() == n,
                   "Given weights do not cover all boostrapping helpers");

        if (!l2_.empty()) {
            QL_REQUIRE(l2_.size() == size(),
                       "Given penalty factors do not cover all parameters");

            QL_REQUIRE(!curve_->guessSolution_.empty(), "L2 penalty requires a guess");
        }
    }

    void FittedBondDiscountCurve::FittingMethod::calculate() {

        if (curve_->maxEvaluations_ == 0)
        {
            // Don't calculate, simply use the given parameters to
            // provide a fitted curve.  This turns the instance into
            // an evaluator of the parametric curve, for example
            // allowing to use the parameters for a credit spread
            // curve calculated with bonds in one currency to be
            // coupled to a discount curve in another currency.

            QL_REQUIRE(curve_->guessSolution_.size() == size(),
                       "wrong number of parameters");

            solution_ = curve_->guessSolution_;

            numberOfIterations_ = 0;
            costValue_ = Null<Real>();
            errorCode_ = EndCriteria::None;

            return;
        }

        FittingCost& costFunction = *costFunction_;

        // start with the guess solution, if it exists
        Array x(size(), 0.0);
        if (!curve_->guessSolution_.empty()) {
            QL_REQUIRE(curve_->guessSolution_.size() == size(), "wrong size for guess");
            x = curve_->guessSolution_;
        }

        // workaround for backwards compatibility
        ext::shared_ptr<OptimizationMethod> optimization = optimizationMethod_;
        if (!optimization) {
            optimization = ext::make_shared<Simplex>(curve_->simplexLambda_);
        }
        Problem problem(costFunction, constraint_, x);

        Real rootEpsilon = curve_->accuracy_;
        Real functionEpsilon =  curve_->accuracy_;
        Real gradientNormEpsilon = curve_->accuracy_;

        EndCriteria endCriteria(curve_->maxEvaluations_,
                                curve_->maxStationaryStateIterations_,
                                rootEpsilon,
                                functionEpsilon,
                                gradientNormEpsilon);

        errorCode_ = optimization->minimize(problem,endCriteria);
        solution_ = problem.currentValue();

        numberOfIterations_ = problem.functionEvaluation();
        costValue_ = problem.functionValue();

        // save the results as the guess solution, in case of recalculation
        curve_->guessSolution_ = solution_;
    }


    FittedBondDiscountCurve::FittingMethod::FittingCost::FittingCost(
                        FittedBondDiscountCurve::FittingMethod* fittingMethod)
    : fittingMethod_(fittingMethod) {}


    Real FittedBondDiscountCurve::FittingMethod::FittingCost::value(
                                                       const Array& x) const {
        Real squaredError = 0.0;
        Array vals = values(x);
        for (Real val : vals) {
            squaredError += val;
        }
        return squaredError;
    }

    Array FittedBondDiscountCurve::FittingMethod::FittingCost::values(const Array &x) const {
        Size n = fittingMethod_->curve_->bondHelpers_.size();
        Size N = fittingMethod_->l2_.size();

        // set solution so that fittingMethod_->curve_ represents the current trial
        // the final solution will be set in FittingMethod::calculate() later on
        fittingMethod_->solution_ = x;

        Array values(n + N);
        for (Size i=0; i<n; ++i) {
            ext::shared_ptr<BondHelper> helper = fittingMethod_->curve_->bondHelpers_[i];
            Real error = helper->impliedQuote() - helper->quote()->value();
            Real weightedError = fittingMethod_->weights_[i] * error;
            values[i] = weightedError * weightedError;
        }

        if (N != 0) {
            for (Size i = 0; i < N; ++i) {
                Real error = x[i] - fittingMethod_->curve_->guessSolution_[i];
                values[i + n] = fittingMethod_->l2_[i] * error * error;
            }
        }
        return values;
    }

}
