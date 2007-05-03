/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Marco Bianchetti
 Copyright (C) 2007 Giorgio Facchinetti

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "optimizers.hpp"
#include "utilities.hpp"
#include <ql/math/optimization/simplex.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(OptimizersTest)

std::vector<boost::shared_ptr<CostFunction> > costFunctions_;
std::vector<boost::shared_ptr<Constraint> > constraints_;
std::vector<Array> initialValues_;
std::vector<Size> maxIterations_, maxStationaryStateIterations_;
std::vector<Real> rootEpsilons_, functionEpsilons_, gradientNormEpsilons_;
std::vector<boost::shared_ptr<EndCriteria> > endCriterias_;
std::vector<std::vector<boost::shared_ptr<OptimizationMethod> > > optimizationMethods_;
std::vector<Array> xMinExpected_, yMinExpected_;

class OneDimensionalPolynomialDegreeN : public CostFunction {
  public:
    OneDimensionalPolynomialDegreeN(const Array& coefficients)
    : coefficients_(coefficients), 
      polynomialDegree_(coefficients.size()-1) {}

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
};

enum OptimizationMethodType {simplex, levenbergMarquardt};

std::string optimizationMethodTypeToString(OptimizationMethodType type) {
    switch (type) {
      case simplex:
          return "Simplex";
      case levenbergMarquardt:
          return "Levenberg Marquardt";
      default:
        QL_FAIL("unknown OptimizationMethod type");
    }
}

boost::shared_ptr<OptimizationMethod> makeOptimizationMethod(
    OptimizationMethodType optimizationMethodType, 
    Real simplexLambda, 
    Real levenbergMarquardtEpsfcn,
    Real levenbergMarquardtXtol,
    Real levenbergMarquardtGtol)
{
    switch (optimizationMethodType) {
        case simplex:
            return boost::shared_ptr<OptimizationMethod>(
                new Simplex(simplexLambda));
        case levenbergMarquardt:
            return boost::shared_ptr<OptimizationMethod>(
                new LevenbergMarquardt(levenbergMarquardtEpsfcn,
                                       levenbergMarquardtXtol,
                                       levenbergMarquardtGtol));
        default:
            QL_FAIL("unknown OptimizationMethod type");
    }
}


std::vector<boost::shared_ptr<OptimizationMethod> > makeOptimizationMethods(
    OptimizationMethodType optimizationMethodTypes[],
    Real simplexLambda, 
    Real levenbergMarquardtEpsfcn,
    Real levenbergMarquardtXtol,
    Real levenbergMarquardtGtol)
{
    std::vector<boost::shared_ptr<OptimizationMethod> > results;
    Size length = LENGTH(optimizationMethodTypes);
    for (Size i=0; i<LENGTH(optimizationMethodTypes); ++i) {
        results.push_back(makeOptimizationMethod(
            optimizationMethodTypes[i],
            simplexLambda, 
            levenbergMarquardtEpsfcn,
            levenbergMarquardtXtol,
            levenbergMarquardtGtol));
    }
    return results;
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
    initialValue[0] = -100.0;
    initialValues_.push_back(initialValue);
    // Set end criteria for optimizer
    maxIterations_.push_back(1000);                 // maxIterations
    maxStationaryStateIterations_.push_back(100);   // MaxStationaryStateIterations
    rootEpsilons_.push_back(1e-8);                  // rootEpsilon
    functionEpsilons_.push_back(1e-16);             // functionEpsilon
    gradientNormEpsilons_.push_back(1e-8);          // gradientNormEpsilon
    endCriterias_.push_back(boost::shared_ptr<EndCriteria>(
        new EndCriteria(maxIterations_.back(), maxStationaryStateIterations_.back(), 
                        rootEpsilons_.back(), functionEpsilons_.back(),
                        gradientNormEpsilons_.back())));
    // Set optimization methods for optimizer
    // WARNING: AT THE MOMENT IT WORKS WITH 1 OPTIMIZER AT ONCE
    OptimizationMethodType optimizationMethodTypes[] = {
        simplex}; //, levenbergMarquardt simplex};
    Real simplexLambda = 0.1;                   // characteristic search length for simplex
    Real levenbergMarquardtEpsfcn = 1.0e-8;     // parameters specific for Levenberg-Marquardt
    Real levenbergMarquardtXtol   = 1.0e-8;     //
    Real levenbergMarquardtGtol   = 1.0e-8;     //
    optimizationMethods_.push_back(makeOptimizationMethods(
        optimizationMethodTypes,
        simplexLambda, levenbergMarquardtEpsfcn, levenbergMarquardtXtol, levenbergMarquardtGtol));
    // Set expected results for optimizer
    Array xMinExpected(1),yMinExpected(1);
    xMinExpected[0] = -b/(2.0*a);
    yMinExpected[0] = -(b*b-4.0*a*c)/(4.0*a);
    xMinExpected_.push_back(xMinExpected);
    yMinExpected_.push_back(yMinExpected);
}

QL_END_TEST_LOCALS(OptimizersTest)

void OptimizersTest::test() {
    BOOST_MESSAGE("Testing optimizers...");
    QL_TEST_SETUP
    for (Size i=0; i<costFunctions_.size(); ++i) {
        Problem problem(*costFunctions_[i], *constraints_[i], initialValues_[i]);
        for (Size j=0; j<(optimizationMethods_[i]).size(); ++j) {
            EndCriteria::Type endCriteriaResult = 
                optimizationMethods_[i][j]->minimize(problem, *endCriterias_[i]);
        Array xMinCalculated = problem.currentValue();
        Array yMinCalculated = problem.values(xMinCalculated);
        // Check optimization results vs known solution 
        for (Size k=0; k < xMinCalculated.size(); ++k) {
            if (endCriteriaResult==EndCriteria::None || 
                endCriteriaResult==EndCriteria::MaxIterations ||
                endCriteriaResult==EndCriteria::Unknown) {
            BOOST_MESSAGE("costFunction = " << i << "\n"
                              "optimizer =  " << j<< "\n"
                              << "    x expected:    " << xMinExpected_[i][k] << "\n"
                              << "    x calculated:  " << std::setprecision(9) << xMinCalculated[k] << "\n"
                              << "    x difference:  " <<  xMinExpected_[i][k]- xMinCalculated[k] << "\n"
                              << "    y expected:    " << yMinExpected_[i][k] << "\n"
                              << "    y calculated:  " << std::setprecision(9) << yMinCalculated[k] << "\n"
                              << "    y difference:  " <<  yMinExpected_[i][k]- yMinCalculated[k] << "\n"
                              << "    functionEpsilon:   " << std::setprecision(9) << functionEpsilons_[i] << "\n"
                              << "    endCriteriaResult:  " << endCriteriaResult);
                }
            }
        }
    }
}


test_suite* OptimizersTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Optimizers tests");
    suite->add(BOOST_TEST_CASE(&OptimizersTest::test));
    return suite;
}

