/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2015 Ferdinando Ametrano
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2005, 2006 Klaus Spanderen
 Copyright (C) 2007 Giorgio Facchinetti
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

#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/interpolations/abcdinterpolation.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/optimization/method.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/termstructures/volatility/abcd.hpp>
#include <ql/termstructures/volatility/abcdcalibration.hpp>
#include <utility>

namespace QuantLib {

    // to constrained <- from unconstrained
    Array AbcdCalibration::AbcdParametersTransformation::direct(const Array& x) const {
        y_[1] = x[1];
        y_[2] = std::exp(x[2]);
        y_[3] = std::exp(x[3]);
        y_[0] = std::exp(x[0]) - y_[3];
        return y_;
    }

    // to unconstrained <- from constrained
    Array AbcdCalibration::AbcdParametersTransformation::inverse(const Array& x) const {
        y_[1] = x[1];
        y_[2] = std::log(x[2]);
        y_[3] = std::log(x[3]);
        y_[0] = std::log(x[0] + x[3]);
        return y_;
    }

    // to constrained <- from unconstrained

    AbcdCalibration::AbcdCalibration(const std::vector<Real>& t,
                                     const std::vector<Real>& blackVols,
                                     Real a,
                                     Real b,
                                     Real c,
                                     Real d,
                                     bool aIsFixed,
                                     bool bIsFixed,
                                     bool cIsFixed,
                                     bool dIsFixed,
                                     bool vegaWeighted,
                                     std::shared_ptr<EndCriteria> endCriteria,
                                     std::shared_ptr<OptimizationMethod> optMethod)
    : aIsFixed_(aIsFixed), bIsFixed_(bIsFixed), cIsFixed_(cIsFixed), dIsFixed_(dIsFixed), a_(a),
      b_(b), c_(c), d_(d), abcdEndCriteria_(EndCriteria::None),
      endCriteria_(std::move(endCriteria)), optMethod_(std::move(optMethod)),
      weights_(blackVols.size(), 1.0 / blackVols.size()), vegaWeighted_(vegaWeighted), times_(t),
      blackVols_(blackVols) {

        AbcdMathFunction::validate(a, b, c, d);

        QL_REQUIRE(blackVols.size()==t.size(),
                       "mismatch between number of times (" << t.size() <<
                       ") and blackVols (" << blackVols.size() << ")");

        // if no optimization method or endCriteria is provided, we provide one
        if (!optMethod_) {
            Real epsfcn = 1.0e-8;
            Real xtol = 1.0e-8;
            Real gtol = 1.0e-8;
            bool useCostFunctionsJacobian = false;
            optMethod_ = std::shared_ptr<OptimizationMethod>(new
                LevenbergMarquardt(epsfcn, xtol, gtol, useCostFunctionsJacobian));
        }
        if (!endCriteria_) {
            Size maxIterations = 10000;
            Size maxStationaryStateIterations = 1000;
            Real rootEpsilon = 1.0e-8;
            Real functionEpsilon = 0.3e-4;     // Why 0.3e-4 ?
            Real gradientNormEpsilon = 0.3e-4; // Why 0.3e-4 ?
            endCriteria_ = std::make_shared<EndCriteria>(maxIterations, maxStationaryStateIterations,
                            rootEpsilon, functionEpsilon, gradientNormEpsilon);
        }
    }

    void AbcdCalibration::compute() {
        if (vegaWeighted_) {
            Real weightsSum = 0.0;
            for (Size i=0; i<times_.size() ; i++) {
                Real stdDev = std::sqrt(blackVols_[i]* blackVols_[i]* times_[i]);
                // when strike==forward, the blackFormulaStdDevDerivative becomes
                weights_[i] = CumulativeNormalDistribution().derivative(.5*stdDev);
                weightsSum += weights_[i];
            }
            // weight normalization
            for (Size i=0; i<times_.size() ; i++) {
                weights_[i] /= weightsSum;
            }
        }

        // there is nothing to optimize
        if (aIsFixed_ && bIsFixed_ && cIsFixed_ && dIsFixed_) {
            abcdEndCriteria_ = EndCriteria::None;
            //error_ = interpolationError();
            //maxError_ = interpolationMaxError();
            return;
        } else {

            AbcdError costFunction(this);
            transformation_ = std::shared_ptr<ParametersTransformation>(new
                AbcdParametersTransformation);

            Array guess(4);
            guess[0] = a_;
            guess[1] = b_;
            guess[2] = c_;
            guess[3] = d_;

            std::vector<bool> parameterAreFixed(4);
            parameterAreFixed[0] = aIsFixed_;
            parameterAreFixed[1] = bIsFixed_;
            parameterAreFixed[2] = cIsFixed_;
            parameterAreFixed[3] = dIsFixed_;

            Array inversedTransformatedGuess(transformation_->inverse(guess));

            ProjectedCostFunction projectedAbcdCostFunction(costFunction,
                            inversedTransformatedGuess, parameterAreFixed);

            Array projectedGuess
                (projectedAbcdCostFunction.project(inversedTransformatedGuess));

            NoConstraint constraint;
            Problem problem(projectedAbcdCostFunction, constraint, projectedGuess);
            abcdEndCriteria_ = optMethod_->minimize(problem, *endCriteria_);
            Array projectedResult(problem.currentValue());
            Array transfResult(projectedAbcdCostFunction.include(projectedResult));

            Array result = transformation_->direct(transfResult);
            AbcdMathFunction::validate(a_, b_, c_, d_);
            a_ = result[0];
            b_ = result[1];
            c_ = result[2];
            d_ = result[3];

        }
    }

    Real AbcdCalibration::value(Real x) const {
        return abcdBlackVolatility(x,a_,b_,c_,d_);
    }

    std::vector<Real> AbcdCalibration::k(const std::vector<Real>& t,
                                         const std::vector<Real>& blackVols) const {
        QL_REQUIRE(blackVols.size()==t.size(),
               "mismatch between number of times (" << t.size() <<
               ") and blackVols (" << blackVols.size() << ")");
        std::vector<Real> k(t.size());
        for (Size i=0; i<t.size() ; i++) {
            k[i]=blackVols[i]/value(t[i]);
        }
        return k;
    }

    Real AbcdCalibration::error() const {
        Size n = times_.size();
        Real error, squaredError = 0.0;
        for (Size i=0; i<times_.size() ; i++) {
            error = (value(times_[i]) - blackVols_[i]);
            squaredError += error * error * weights_[i];
        }
        return std::sqrt(n*squaredError/(n-1));
    }

    Real AbcdCalibration::maxError() const {
        Real error, maxError = QL_MIN_REAL;
        for (Size i=0; i<times_.size() ; i++) {
            error = std::fabs(value(times_[i]) - blackVols_[i]);
            maxError = std::max(maxError, error);
        }
        return maxError;
    }

    // calculate weighted differences
    Array AbcdCalibration::errors() const {
        Array results(times_.size());
        for (Size i=0; i<times_.size() ; i++) {
            results[i] = (value(times_[i]) - blackVols_[i])* std::sqrt(weights_[i]);
        }
        return results;
    }

    EndCriteria::Type AbcdCalibration::endCriteria() const{
        return abcdEndCriteria_;
    }

}
