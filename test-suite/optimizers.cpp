/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Marco Bianchetti
 Copyright (C) 2007 François du Vignaud
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

#include "optimizers.hpp"
#include "utilities.hpp"
#include <ql/math/optimization/simplex.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/optimization/conjugategradient.hpp>
#include <ql/math/optimization/steepestdescent.hpp>
#include <ql/math/optimization/bfgs.hpp>
#include <ql/math/optimization/constraint.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    struct NamedOptimizationMethod;

    std::vector<boost::shared_ptr<CostFunction> > costFunctions_;
    std::vector<boost::shared_ptr<Constraint> > constraints_;
    std::vector<Array> initialValues_;
    std::vector<Size> maxIterations_, maxStationaryStateIterations_;
    std::vector<Real> rootEpsilons_, functionEpsilons_, gradientNormEpsilons_;
    std::vector<boost::shared_ptr<EndCriteria> > endCriterias_;
    std::vector<std::vector<NamedOptimizationMethod> > optimizationMethods_;
    std::vector<Array> xMinExpected_, yMinExpected_;

    class OneDimensionalPolynomialDegreeN : public CostFunction {
      public:
        OneDimensionalPolynomialDegreeN(const Array& coefficients)
        : coefficients_(coefficients),
          polynomialDegree_(coefficients.size()-1),odd(true) {}

        Real value(const Array& x) const {
            QL_REQUIRE(x.size()==1,"independent variable must be 1 dimensional");
            Real y = 0;
            for (Size i=0; i<=polynomialDegree_; ++i)
                y += coefficients_[i]*std::pow(x[0],static_cast<int>(i));
            return y;
        }

        Disposable<Array> values(const Array& x) const{
            QL_REQUIRE(x.size()==1,"independent variable must be 1 dimensional");
            Array y(1);
            y[0] = value(x);
            return y;
        }

      private:
        const Array coefficients_;
        const Size polynomialDegree_;
        mutable bool odd;
    };


    // The goal of this cost function is simply to call another optimization inside
    // in order to test nested optimizations
    class OptimizationBasedCostFunction : public CostFunction {
      public:
        Real value(const Array&) const { return 1.0; }

        Disposable<Array> values(const Array&) const{
            // dummy nested optimization
            Array coefficients(3, 1.0);
            OneDimensionalPolynomialDegreeN oneDimensionalPolynomialDegreeN(coefficients);
            NoConstraint constraint;
            Array initialValues(1, 100.0);
            Problem problem(oneDimensionalPolynomialDegreeN, constraint,
                            initialValues);
            LevenbergMarquardt optimizationMethod;
            //Simplex optimizationMethod(0.1);
            //ConjugateGradient optimizationMethod;
            //SteepestDescent optimizationMethod;
            EndCriteria endCriteria(1000, 100, 1e-5, 1e-5, 1e-5);
            optimizationMethod.minimize(problem, endCriteria);
            // return dummy result
            Array dummy(1,0);
            return dummy;
        }
    };


    enum OptimizationMethodType {simplex,
                                 levenbergMarquardt,
                                 conjugateGradient,
                                 steepestDescent,
                                 bfgs};

    std::string optimizationMethodTypeToString(OptimizationMethodType type) {
        switch (type) {
          case simplex:
            return "Simplex";
          case levenbergMarquardt:
            return "Levenberg Marquardt";
          case conjugateGradient:
            return "Conjugate Gradient";
          case steepestDescent:
            return "Steepest Descent";
          case bfgs:
            return "BFGS";
          default:
            QL_FAIL("unknown OptimizationMethod type");
        }
    }

    struct NamedOptimizationMethod {
        boost::shared_ptr<OptimizationMethod> optimizationMethod;
        std::string name;
    };


    boost::shared_ptr<OptimizationMethod> makeOptimizationMethod(
                                OptimizationMethodType optimizationMethodType,
                                Real simplexLambda,
                                Real levenbergMarquardtEpsfcn,
                                Real levenbergMarquardtXtol,
                                Real levenbergMarquardtGtol) {
        switch (optimizationMethodType) {
          case simplex:
            return boost::shared_ptr<OptimizationMethod>(
                new Simplex(simplexLambda));
          case levenbergMarquardt:
            return boost::shared_ptr<OptimizationMethod>(
                new LevenbergMarquardt(levenbergMarquardtEpsfcn,
                                       levenbergMarquardtXtol,
                                       levenbergMarquardtGtol));
          case conjugateGradient:
            return boost::shared_ptr<OptimizationMethod>(new ConjugateGradient);
          case steepestDescent:
            return boost::shared_ptr<OptimizationMethod>(new SteepestDescent);
          case bfgs:
            return boost::shared_ptr<OptimizationMethod>(new BFGS);
          default:
            QL_FAIL("unknown OptimizationMethod type");
        }
    }


    std::vector<NamedOptimizationMethod> makeOptimizationMethods(
                             OptimizationMethodType optimizationMethodTypes[],
                             Size optimizationMethodNb,
                             Real simplexLambda,
                             Real levenbergMarquardtEpsfcn,
                             Real levenbergMarquardtXtol,
                             Real levenbergMarquardtGtol) {
        std::vector<NamedOptimizationMethod> results;
        for (Size i=0; i<optimizationMethodNb; ++i) {
            NamedOptimizationMethod namedOptimizationMethod;
            namedOptimizationMethod.optimizationMethod = makeOptimizationMethod(
                optimizationMethodTypes[i],
                simplexLambda,
                levenbergMarquardtEpsfcn,
                levenbergMarquardtXtol,
                levenbergMarquardtGtol);
            namedOptimizationMethod.name
                = optimizationMethodTypeToString(optimizationMethodTypes[i]);
            results.push_back(namedOptimizationMethod);
        }
        return results;
    }

    Real maxDifference(const Array& a, const Array& b) {
        Array diff = a-b;
        Real maxDiff = 0.0;
        for (Size i=0; i<diff.size(); ++i)
            maxDiff = std::max(maxDiff, std::fabs(diff[i]));
        return maxDiff;
    }

    // Set up, for each cost function, all the ingredients for optimization:
    // constraint, initial guess, end criteria, optimization methods.
    void setup() {

        // Cost function n. 1: 1D polynomial of degree 2 (parabolic function y=a*x^2+b*x+c)
        const Real a = 1;   // required a > 0
        const Real b = 1;
        const Real c = 1;
        Array coefficients(3);
        coefficients[0]= c;
        coefficients[1]= b;
        coefficients[2]= a;
        costFunctions_.push_back(boost::shared_ptr<CostFunction>(
            new OneDimensionalPolynomialDegreeN(coefficients)));
        // Set constraint for optimizers: unconstrained problem
        constraints_.push_back(boost::shared_ptr<Constraint>(new NoConstraint()));
        // Set initial guess for optimizer
        Array initialValue(1);
        initialValue[0] = -100;
        initialValues_.push_back(initialValue);
        // Set end criteria for optimizer
        maxIterations_.push_back(10000);                // maxIterations
        maxStationaryStateIterations_.push_back(100);   // MaxStationaryStateIterations
        rootEpsilons_.push_back(1e-8);                  // rootEpsilon
        functionEpsilons_.push_back(1e-8);              // functionEpsilon
        gradientNormEpsilons_.push_back(1e-8);          // gradientNormEpsilon
        endCriterias_.push_back(boost::shared_ptr<EndCriteria>(
            new EndCriteria(maxIterations_.back(), maxStationaryStateIterations_.back(),
                            rootEpsilons_.back(), functionEpsilons_.back(),
                            gradientNormEpsilons_.back())));
        // Set optimization methods for optimizer
        OptimizationMethodType optimizationMethodTypes[] = {
            simplex, levenbergMarquardt, conjugateGradient, bfgs//, steepestDescent
        };
        Real simplexLambda = 0.1;                   // characteristic search length for simplex
        Real levenbergMarquardtEpsfcn = 1.0e-8;     // parameters specific for Levenberg-Marquardt
        Real levenbergMarquardtXtol   = 1.0e-8;     //
        Real levenbergMarquardtGtol   = 1.0e-8;     //
        optimizationMethods_.push_back(makeOptimizationMethods(
            optimizationMethodTypes, LENGTH(optimizationMethodTypes),
            simplexLambda, levenbergMarquardtEpsfcn, levenbergMarquardtXtol,
            levenbergMarquardtGtol));
        // Set expected results for optimizer
        Array xMinExpected(1),yMinExpected(1);
        xMinExpected[0] = -b/(2.0*a);
        yMinExpected[0] = -(b*b-4.0*a*c)/(4.0*a);
        xMinExpected_.push_back(xMinExpected);
        yMinExpected_.push_back(yMinExpected);
    }

}


void OptimizersTest::test() {
    BOOST_MESSAGE("Testing optimizers...");

    setup();

    // Loop over problems (currently there is only 1 problem)
    for (Size i=0; i<costFunctions_.size(); ++i) {
        Problem problem(*costFunctions_[i], *constraints_[i],
                        initialValues_[i]);
        Array initialValues = problem.currentValue();
        // Loop over optimizers
        for (Size j=0; j<(optimizationMethods_[i]).size(); ++j) {
            Real rootEpsilon = endCriterias_[i]->rootEpsilon();
            Size endCriteriaTests = 1;
           // Loop over rootEpsilon
            for (Size k=0; k<endCriteriaTests; ++k) {
                problem.setCurrentValue(initialValues);
                EndCriteria endCriteria(
                            endCriterias_[i]->maxIterations(),
                            endCriterias_[i]->maxStationaryStateIterations(),
                            rootEpsilon,
                            endCriterias_[i]->functionEpsilon(),
                            endCriterias_[i]->gradientNormEpsilon());
                rootEpsilon *= .1;
                EndCriteria::Type endCriteriaResult =
                    optimizationMethods_[i][j].optimizationMethod->minimize(
                    problem, endCriteria);
                Array xMinCalculated = problem.currentValue();
                Array yMinCalculated = problem.values(xMinCalculated);

                // Check optimization results vs known solution
                bool completed;
                switch (endCriteriaResult) {
                  case EndCriteria::None:
                  case EndCriteria::MaxIterations:
                  case EndCriteria::Unknown:
                    completed = false;
                  default:
                    completed = true;
                }

                Real xError = maxDifference(xMinCalculated,xMinExpected_[i]);
                Real yError = maxDifference(yMinCalculated,yMinExpected_[i]);

                bool correct = (xError <= endCriteria.rootEpsilon() ||
                                yError <= endCriteria.functionEpsilon());

                if ((!completed) || (!correct))
                    BOOST_ERROR("costFunction # = " << i <<
                                "\nOptimizer: " <<
                                optimizationMethods_[i][j].name <<
                                "\n    function evaluations: " <<
                                problem.functionEvaluation()  <<
                                "\n    gradient evaluations: " <<
                                problem.gradientEvaluation() <<
                                "\n    x expected:           " <<
                                xMinExpected_[i] <<
                                "\n    x calculated:         " <<
                                std::setprecision(9) << xMinCalculated <<
                                "\n    x difference:         " <<
                                xMinExpected_[i]- xMinCalculated <<
                                "\n    rootEpsilon:          " <<
                                std::setprecision(9) <<
                                endCriteria.rootEpsilon() <<
                                "\n    y expected:           " <<
                                yMinExpected_[i] <<
                                "\n    y calculated:         " <<
                                std::setprecision(9) << yMinCalculated <<
                                "\n    y difference:         " <<
                                yMinExpected_[i]- yMinCalculated <<
                                "\n    functionEpsilon:      " <<
                                std::setprecision(9) <<
                                endCriteria.functionEpsilon() <<
                                "\n    endCriteriaResult:    " <<
                                endCriteriaResult);
            }
        }
    }
}


void OptimizersTest::nestedOptimizationTest() {
    BOOST_MESSAGE("Testing nested optimizations...");
    OptimizationBasedCostFunction optimizationBasedCostFunction;
    NoConstraint constraint;
    Array initialValues(1, 0.0);
    Problem problem(optimizationBasedCostFunction, constraint,
                    initialValues);
    LevenbergMarquardt optimizationMethod;
    //Simplex optimizationMethod(0.1);
    //ConjugateGradient optimizationMethod;
    //SteepestDescent optimizationMethod;
    EndCriteria endCriteria(1000, 100, 1e-5, 1e-5, 1e-5);
    optimizationMethod.minimize(problem, endCriteria);

}

test_suite* OptimizersTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Optimizers tests");
    suite->add(QUANTLIB_TEST_CASE(&OptimizersTest::test));
    suite->add(QUANTLIB_TEST_CASE(&OptimizersTest::nestedOptimizationTest));
    return suite;
}

