/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include <ql/termstructures/yield/fittedbonddiscountcurve.hpp>
#include <ql/math/optimization/simplex.hpp>
#include <ql/math/optimization/costfunction.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/utilities/dataformatters.hpp>

namespace QuantLib {

    class FittedBondDiscountCurve::FittingMethod::FittingCost
        : public CostFunction {
        friend class FittedBondDiscountCurve::FittingMethod;
      public:
        FittingCost(FittedBondDiscountCurve::FittingMethod* fittingMethod);
        Real value(const Array& x) const;
        Disposable<Array> values(const Array& x) const;
      private:
        FittedBondDiscountCurve::FittingMethod* fittingMethod_;
        mutable Date refDate_;
        mutable std::vector<Integer> startingCashFlowIndex_;
    };


    FittedBondDiscountCurve::FittedBondDiscountCurve (
                 Natural settlementDays,
                 const Calendar& calendar,
                 const std::vector<boost::shared_ptr<FixedRateBondHelper> >&
                                                                  instruments,
                 const DayCounter& dayCounter,
                 const FittingMethod& fittingMethod,
                 Real accuracy,
                 Size maxEvaluations,
                 const Array& guess,
                 Real simplexLambda)
    : YieldTermStructure(settlementDays, calendar, dayCounter),
      accuracy_(accuracy),
      maxEvaluations_(maxEvaluations),
      simplexLambda_(simplexLambda),
      guessSolution_(guess),
      instruments_(instruments),
      fittingMethod_(fittingMethod) {

        fittingMethod_->curve_ = this;
        setup();
    }


    FittedBondDiscountCurve::FittedBondDiscountCurve (
                 const Date& referenceDate,
                 const std::vector<boost::shared_ptr<FixedRateBondHelper> >&
                                                                  instruments,
                 const DayCounter& dayCounter,
                 const FittingMethod& fittingMethod,
                 Real accuracy,
                 Size maxEvaluations,
                 const Array& guess,
                 Real simplexLambda)
    : YieldTermStructure(referenceDate, Calendar(), dayCounter),
      accuracy_(accuracy),
      maxEvaluations_(maxEvaluations),
      simplexLambda_(simplexLambda),
      guessSolution_(guess),
      instruments_(instruments),
      fittingMethod_(fittingMethod) {

        fittingMethod_->curve_ = this;
        setup();
    }


    Size FittedBondDiscountCurve::numberOfBonds() const {
        return instruments_.size();
    }

    Date FittedBondDiscountCurve::maxDate() const {
        calculate();
        return maxDate_;
    }

    const FittedBondDiscountCurve::FittingMethod&
    FittedBondDiscountCurve::fitResults() const {
        calculate();
        return *fittingMethod_;
    }

    void FittedBondDiscountCurve::update() {
        TermStructure::update();
        LazyObject::update();
    }

    void FittedBondDiscountCurve::setup() {
        for (Size i=0; i<instruments_.size(); ++i)
            registerWith(instruments_[i]);
    }

    void FittedBondDiscountCurve::performCalculations() const {

        QL_REQUIRE(!instruments_.empty(), "no instruments given");

        // double check bond quotes still valid and/or instruments not expired
        for (Size i=0; i<instruments_.size(); ++i) {
            QL_REQUIRE(instruments_[i]->quoteIsValid(),
                       io::ordinal(i) << " instrument has an invalid quote");
            instruments_[i]->setTermStructure(
                                  const_cast<FittedBondDiscountCurve*>(this));
            boost::shared_ptr<Bond> bond = instruments_[i]->bond();
            QL_REQUIRE(!bond->isExpired(),
                       io::ordinal(i) << " bond is expired");
        }

        maxDate_ = Date::minDate();
        for (Size i=0; i<instruments_.size(); ++i)
            maxDate_ = std::max(maxDate_, instruments_[i]->latestDate());

        fittingMethod_->init();
        fittingMethod_->calculate();
    }


    DiscountFactor FittedBondDiscountCurve::discountImpl(Time t) const {
        calculate();
        return fittingMethod_->discountFunction(fittingMethod_->solution_,t);
    }




    FittedBondDiscountCurve::FittingMethod::FittingMethod(bool constrainAtZero)
    : constrainAtZero_(constrainAtZero) {}

    Integer FittedBondDiscountCurve::FittingMethod::numberOfIterations() const {
        return numberOfIterations_;
    }

    Real FittedBondDiscountCurve::FittingMethod::minimumCostValue() const {
        return costValue_;
    }

    Array FittedBondDiscountCurve::FittingMethod::solution() const {
        return solution_;
    }

    void FittedBondDiscountCurve::FittingMethod::init() {

        Array tempWeights(curve_->instruments_.size(), 0.0);
        Date today  = curve_->referenceDate();
        Real squaredSum = 0.0;

        for (Size k=0; k<curve_->instruments_.size(); ++k) {
            boost::shared_ptr<Bond> bond = curve_->instruments_[k]->bond();

            Leg leg = bond->cashflows();
            Real cleanPrice = curve_->instruments_[k]->quoteValue();
            Rate ytm = bond->yield(cleanPrice,
                                   curve_->instruments_[k]->bond()->dayCounter(),
                                   Compounded,
                                   curve_->instruments_[k]->bond()->frequency(),
                                   today);
            InterestRate r(ytm,
                           curve_->instruments_[k]->bond()->dayCounter(),
                           Compounded,
                           curve_->instruments_[k]->bond()->frequency());

            Date settlement = bond->settlementDate(today);
            Time duration =
                CashFlows::duration(leg, r, Duration::Modified, settlement);
            tempWeights[k] = 1.0/duration;
            squaredSum += tempWeights[k]*tempWeights[k];
        }
        weights_ = tempWeights/std::sqrt(squaredSum);

        // set cost function related below
        costFunction_ = boost::shared_ptr<FittingCost>(new FittingCost(this));
        costFunction_->refDate_  = curve_->referenceDate();
        costFunction_->startingCashFlowIndex_.clear();

        for (Size i=0; i<curve_->instruments_.size(); ++i) {
            boost::shared_ptr<Bond> bond = curve_->instruments_[i]->bond();
            Date settlementDate = bond->settlementDate(today);
            Leg cf = bond->cashflows();
            for (Size k=0; k<cf.size(); ++k) {
                if (!cf[k]->hasOccurred(settlementDate)) {
                    costFunction_->startingCashFlowIndex_.push_back(k);
                    break;
                }
            }
        }
    }

    void FittedBondDiscountCurve::FittingMethod::calculate() {

        FittingCost& costFunction = *costFunction_;
        Constraint constraint = NoConstraint();

        // start with the guess solution, if it exists
        Array x(size(), 0.0);
        if (!curve_->guessSolution_.empty()) {
            x = curve_->guessSolution_;
        }

        Simplex simplex(curve_->simplexLambda_);
        Problem problem(costFunction, constraint, x);

        Natural maxStationaryStateIterations = 100;
        Real rootEpsilon = curve_->accuracy_;
        Real functionEpsilon =  curve_->accuracy_;
        Real gradientNormEpsilon = curve_->accuracy_;

        EndCriteria endCriteria(curve_->maxEvaluations_,
                                maxStationaryStateIterations,
                                rootEpsilon,
                                functionEpsilon,
                                gradientNormEpsilon);

        simplex.minimize(problem,endCriteria);
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
                                                       const Array &x) const {

        // speed optimization by setting some of the below in the constructor
        // rather than here

        Size numberOfBonds = fittingMethod_->curve_->instruments_.size();
        Date today  = fittingMethod_->curve_->referenceDate();

        Array trialDirtyPrice(numberOfBonds,0.);
        Real squaredError = 0.0;

        for (Size i=0; i<numberOfBonds;++i) {
            boost::shared_ptr<Bond> bond =
                fittingMethod_->curve_->instruments_[i]->bond();
            Real quotedPrice =
                fittingMethod_->curve_->instruments_[i]->quoteValue();

            Date settlement = bond->settlementDate(today);
            Real dirtyPrice = quotedPrice + bond->accruedAmount(settlement);

            DayCounter bondDayCount =
                fittingMethod_->curve_->instruments_[i]->bond()->dayCounter();
            Leg cf = bond->cashflows();

            // loop over cashFlows: P_j = sum( cf_i * d(t_i))
            for (Size k=startingCashFlowIndex_[i]; k<cf.size(); ++k) {
                Time tenor = bondDayCount.yearFraction(today, cf[k]->date());
                trialDirtyPrice[i] += cf[k]->amount() *
                                      fittingMethod_->discountFunction(x,tenor);
            }
            // adjust dirty price (NPV) for a forward settlement
            if (settlement != today ) {
                Time tenor = bondDayCount.yearFraction(today, settlement);
                trialDirtyPrice[i] = trialDirtyPrice[i]/
                                     fittingMethod_->discountFunction(x,tenor);
            }
            squaredError = squaredError +
                           std::pow(fittingMethod_->weights_[i]*
                                    (trialDirtyPrice[i] - dirtyPrice),2);
        }
        return squaredError;

    }

    Disposable<Array>
    FittedBondDiscountCurve::FittingMethod::FittingCost::values(
                                                       const Array &x) const {
        Array y(1);
        y[0] = value(x);
        return y;
    }

}

