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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/cashflows/cashflows.hpp>
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


    void FittedBondDiscountCurve::performCalculations() const {

        QL_REQUIRE(!bondHelpers_.empty(), "no bondHelpers given");

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
        fittingMethod_->init();
        fittingMethod_->calculate();
    }


    FittedBondDiscountCurve::FittingMethod::FittingMethod(
        bool constrainAtZero,
        const Array& weights,
        ext::shared_ptr<OptimizationMethod> optimizationMethod,
        Array l2,
        const Real minCutoffTime,
        const Real maxCutoffTime)
    : constrainAtZero_(constrainAtZero), weights_(weights), l2_(std::move(l2)),
      calculateWeights_(weights.empty()), optimizationMethod_(std::move(optimizationMethod)),
      minCutoffTime_(minCutoffTime), maxCutoffTime_(maxCutoffTime) {}

    void FittedBondDiscountCurve::FittingMethod::init() {
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

                Real cleanPrice = curve_->bondHelpers_[i]->quote()->value();

                Date bondSettlement = bond->settlementDate();
                Rate ytm = BondFunctions::yield(*bond, cleanPrice,
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
        }
    }

    void FittedBondDiscountCurve::FittingMethod::setConstraint(const Constraint& constraint) {
        constraint_ = constraint;
    }

    void FittedBondDiscountCurve::FittingMethod::calculate() {

        FittingCost& costFunction = *costFunction_;

        // start with the guess solution, if it exists
        Array x(size(), 0.0);
        if (!curve_->guessSolution_.empty()) {
            x = curve_->guessSolution_;
        }

        if (curve_->maxEvaluations_ == 0)
        {
            // Don't calculate, simply use the given parameters to provide a fitted curve.
            // This turns the fittedbonddiscountcurve into an evaluator of the parametric
            // curve, for example allowing to use the parameters for a credit spread curve
            // calculated with bonds in one currency to be coupled to a discount curve in
            // another currency.

            QL_REQUIRE(!curve_->guessSolution_.empty(), "no guess provided");

            solution_ = curve_->guessSolution_;

            numberOfIterations_ = 0;
            costValue_ = costFunction.value(solution_);
            errorCode_ = EndCriteria::None;

            return;
        }

        //workaround for backwards compatibility
        ext::shared_ptr<OptimizationMethod> optimization = optimizationMethod_;
        if(!optimization){
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
