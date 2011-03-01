/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2005, 2006 Klaus Spanderen
 Copyright (C) 2007 Giorgio Facchinetti

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

#include <ql/termstructures/volatility/abcdcalibration.hpp>
#include <ql/math/optimization/method.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/termstructures/volatility/abcd.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/interpolations/abcdinterpolation.hpp>

namespace QuantLib {

    AbcdCalibration::AbcdCalibration(
               const std::vector<Real>& t,
               const std::vector<Real>& blackVols,
               Real a, Real b, Real c, Real d,
               bool aIsFixed, bool bIsFixed, bool cIsFixed, bool dIsFixed,
               bool vegaWeighted,
               const boost::shared_ptr<EndCriteria>& endCriteria,
               const boost::shared_ptr<OptimizationMethod>& optMethod)
    : aIsFixed_(aIsFixed), bIsFixed_(bIsFixed),
      cIsFixed_(cIsFixed), dIsFixed_(dIsFixed),
      a_(a), b_(b), c_(c), d_(d),
      abcdEndCriteria_(EndCriteria::None), endCriteria_(endCriteria),
      optMethod_(optMethod), weights_(blackVols.size(), 1.0/blackVols.size()),
      vegaWeighted_(vegaWeighted),
      times_(t), blackVols_(blackVols) {

        QL_REQUIRE(blackVols.size()==t.size(),
                       "mismatch between number of times (" << t.size() <<
                       ") and blackVols (" << blackVols.size() << ")");

        // if no optimization method or endCriteria is provided, we provide one
        if (!optMethod_)
            optMethod_ = boost::shared_ptr<OptimizationMethod>(new
                LevenbergMarquardt(1e-8, 1e-8, 1e-8));
            //method_ = boost::shared_ptr<OptimizationMethod>(new
            //    Simplex(0.01));
        if (!endCriteria_)
            //endCriteria_ = boost::shared_ptr<EndCriteria>(new
            //    EndCriteria(60000, 100, 1e-8, 1e-8, 1e-8));
            endCriteria_ = boost::shared_ptr<EndCriteria>(new
                EndCriteria(1000, 100, 1.0e-8, 0.3e-4, 0.3e-4));   // Why 0.3e-4 ?
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
            transformation_ = boost::shared_ptr<ParametersTransformation>(new
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
            a_ = result[0];
            b_ = result[1];
            c_ = result[2];
            d_ = result[3];

            validateAbcdParameters(a_, b_, c_, d_);
        }
    }

    Real AbcdCalibration::a() const {
        return a_;
    }

    Real AbcdCalibration::b() const {
        return b_;
    }

    Real AbcdCalibration::c() const {
        return c_;
    }

    Real AbcdCalibration::d() const {
        return d_;
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
            squaredError += error * error *(weights_[i]);
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
    Disposable<Array> AbcdCalibration::errors() const {
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
