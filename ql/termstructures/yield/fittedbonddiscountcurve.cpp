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

#include <ql/termstructures/yield/fittedbonddiscountcurve.hpp>
#include <ql/pricingengines/bond/bondfunctions.hpp>
#include <ql/math/optimization/simplex.hpp>
#include <ql/math/optimization/costfunction.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/time/daycounters/simpledaycounter.hpp>

using boost::shared_ptr;
using std::vector;

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
        mutable vector<Size> firstCashFlow_;
    };


    FittedBondDiscountCurve::FittedBondDiscountCurve (
                 Natural settlementDays,
                 const Calendar& calendar,
                 const vector<shared_ptr<BondHelper> >& bondHelpers,
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
      bondHelpers_(bondHelpers),
      fittingMethod_(fittingMethod) {

        fittingMethod_->curve_ = this;
        setup();
    }


    FittedBondDiscountCurve::FittedBondDiscountCurve (
                 const Date& referenceDate,
                 const vector<shared_ptr<BondHelper> >& bondHelpers,
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
      bondHelpers_(bondHelpers),
      fittingMethod_(fittingMethod) {

        fittingMethod_->curve_ = this;
        setup();
    }


    FittedBondDiscountCurve::FittedBondDiscountCurve (
                 Natural settlementDays,
                 const Calendar& calendar,
                 const vector<shared_ptr<FixedRateBondHelper> >& bondHelpers,
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
      bondHelpers_(bondHelpers.size()),
      fittingMethod_(fittingMethod) {

        for (Size i=0; i<bondHelpers_.size(); ++i) {
            bondHelpers_[i] = boost::dynamic_pointer_cast<BondHelper>(bondHelpers[i]);
        }

        fittingMethod_->curve_ = this;
        setup();
    }


    FittedBondDiscountCurve::FittedBondDiscountCurve (
                 const Date& referenceDate,
                 const vector<shared_ptr<FixedRateBondHelper> >& bondHelpers,
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
      bondHelpers_(bondHelpers.size()),
      fittingMethod_(fittingMethod) {

        for (Size i=0; i<bondHelpers_.size(); ++i) {
            bondHelpers_[i] = boost::dynamic_pointer_cast<BondHelper>(bondHelpers[i]);
        }

        fittingMethod_->curve_ = this;
        setup();
    }


    void FittedBondDiscountCurve::performCalculations() const {

        QL_REQUIRE(!bondHelpers_.empty(), "no bondHelpers given");

        maxDate_ = Date::minDate();
        Date refDate = referenceDate();

        // double check bond quotes still valid and/or instruments not expired
        for (Size i=0; i<bondHelpers_.size(); ++i) {
            shared_ptr<Bond> bond = bondHelpers_[i]->bond();
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
            maxDate_ = std::max(maxDate_, bondHelpers_[i]->latestDate());
            bondHelpers_[i]->setTermStructure(
                                  const_cast<FittedBondDiscountCurve*>(this));
        }

        fittingMethod_->init();
        fittingMethod_->calculate();
    }


    FittedBondDiscountCurve::FittingMethod::FittingMethod(bool constrainAtZero)
    : constrainAtZero_(constrainAtZero) {}


    void FittedBondDiscountCurve::FittingMethod::init() {

        // yield conventions
        DayCounter yieldDC = curve_->dayCounter();
        Compounding yieldComp = Compounded;
        Frequency yieldFreq = Annual;

        Size n = curve_->bondHelpers_.size();
        costFunction_ = shared_ptr<FittingCost>(new FittingCost(this));
        costFunction_->firstCashFlow_.resize(n);
        weights_ = Array(n);
        Real squaredSum = 0.0;
        for (Size i=0; i<curve_->bondHelpers_.size(); ++i) {
            shared_ptr<Bond> bond = curve_->bondHelpers_[i]->bond();

            Leg leg = bond->cashflows();
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

            const Leg& cf = bond->cashflows();
            for (Size k=0; k<cf.size(); ++k) {
                if (!cf[k]->hasOccurred(bondSettlement, false)) {
                    costFunction_->firstCashFlow_[i] = k;
                    break;
                }
            }
        }
        weights_ /= std::sqrt(squaredSum);

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
                                                       const Array& x) const {

        Date refDate  = fittingMethod_->curve_->referenceDate();
        const DayCounter& dc = fittingMethod_->curve_->dayCounter();

        Real squaredError = 0.0;
        Size n = fittingMethod_->curve_->bondHelpers_.size();
        for (Size i=0; i<n; ++i) {

            shared_ptr<Bond> bond =
                            fittingMethod_->curve_->bondHelpers_[i]->bond();
            Date bondSettlement = bond->settlementDate();

            // CleanPrice_i = sum( cf_k * d(t_k) ) - accruedAmount
            Real modelPrice = - bond->accruedAmount(bondSettlement);
            const Leg& cf = bond->cashflows();
            for (Size k=firstCashFlow_[i]; k<cf.size(); ++k) {
                Time tenor = dc.yearFraction(refDate, cf[k]->date());
                modelPrice += cf[k]->amount() *
                                    fittingMethod_->discountFunction(x, tenor);
            }

            // adjust price (NPV) for forward settlement
            if (bondSettlement != refDate ) {
                Time tenor = dc.yearFraction(refDate, bondSettlement);
                modelPrice /= fittingMethod_->discountFunction(x, tenor);
            }
            Real marketPrice =
                fittingMethod_->curve_->bondHelpers_[i]->quote()->value();
            Real error = modelPrice - marketPrice;
            Real weightedError = fittingMethod_->weights_[i] * error;
            squaredError += weightedError * weightedError;
        }
        return squaredError;
    }

    Disposable<Array>
    FittedBondDiscountCurve::FittingMethod::FittingCost::values(
                                                       const Array &x) const {
        Array y(1, value(x));
        return y;
    }

}
