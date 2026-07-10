/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Marco Bianchetti
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2012 Ralph Schreyer
 Copyright (C) 2012 Mateusz Kapturski

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

#include "preconditions.hpp"
#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/math/optimization/bfgs.hpp>
#include <ql/math/optimization/conjugategradient.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <ql/math/optimization/costfunction.hpp>
#include <ql/math/optimization/differentialevolution.hpp>
#include <ql/math/optimization/goldstein.hpp>
#include <ql/math/optimization/lbfgsb.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/optimization/simplex.hpp>
#include <ql/math/optimization/steepestdescent.hpp>
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>
#include <ql/experimental/math/cmaes.hpp>
#include <ql/math/matrix.hpp>
#include <ql/mathconstants.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

using std::pow;
using std::cos;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(OptimizersTests)

struct NamedOptimizationMethod {
    ext::shared_ptr<OptimizationMethod> optimizationMethod;
    std::string name;
};

std::vector<ext::shared_ptr<CostFunction> > costFunctions_;
std::vector<ext::shared_ptr<Constraint> > constraints_;
std::vector<Array> initialValues_;
std::vector<Size> maxIterations_, maxStationaryStateIterations_;
std::vector<Real> rootEpsilons_, functionEpsilons_, gradientNormEpsilons_;
std::vector<ext::shared_ptr<EndCriteria> > endCriterias_;
std::vector<std::vector<NamedOptimizationMethod> > optimizationMethods_;
std::vector<Array> xMinExpected_, yMinExpected_;

class OneDimensionalPolynomialDegreeN : public CostFunction {
  public:
    explicit OneDimensionalPolynomialDegreeN(const Array& coefficients)
    : coefficients_(coefficients),
      polynomialDegree_(coefficients.size()-1) {}

    Real value(const Array& x) const override {
        QL_REQUIRE(x.size()==1,"independent variable must be 1 dimensional");
        Real y = 0;
        for (Size i=0; i<=polynomialDegree_; ++i)
            y += coefficients_[i]*std::pow(x[0],static_cast<int>(i));
        return y;
    }

    Array values(const Array& x) const override {
        QL_REQUIRE(x.size()==1,"independent variable must be 1 dimensional");
        return Array(1, value(x));
    }

  private:
    const Array coefficients_;
    const Size polynomialDegree_;
};


// The goal of this cost function is simply to call another optimization inside
// in order to test nested optimizations
class OptimizationBasedCostFunction : public CostFunction {
  public:
    Real value(const Array&) const override { return 1.0; }

    Array values(const Array&) const override {
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
        return Array(1, 0);
    }
};


enum OptimizationMethodType {simplex,
                             levenbergMarquardt,
                             levenbergMarquardt2,
                             conjugateGradient,
                             conjugateGradient_goldstein,
                             steepestDescent,
                             steepestDescent_goldstein,
                             bfgs,
                             bfgs_goldstein,
                             lbfgsb};

std::string optimizationMethodTypeToString(OptimizationMethodType type) {
    switch (type) {
      case simplex:
        return "Simplex";
      case levenbergMarquardt:
        return "Levenberg Marquardt";
      case levenbergMarquardt2:
        return "Levenberg Marquardt (cost function's jacbobian)";
      case conjugateGradient:
        return "Conjugate Gradient";
      case steepestDescent:
        return "Steepest Descent";
      case bfgs:
        return "BFGS";
      case lbfgsb:
        return "L-BFGS-B";
      case conjugateGradient_goldstein:
        return "Conjugate Gradient (Goldstein line search)";
      case steepestDescent_goldstein:
        return "Steepest Descent (Goldstein line search)";
      case bfgs_goldstein:
        return "BFGS (Goldstein line search)";
      default:
        QL_FAIL("unknown OptimizationMethod type");
    }
}


ext::shared_ptr<OptimizationMethod> makeOptimizationMethod(
                                                           OptimizationMethodType optimizationMethodType,
                                                           Real simplexLambda,
                                                           Real levenbergMarquardtEpsfcn,
                                                           Real levenbergMarquardtXtol,
                                                           Real levenbergMarquardtGtol) {
    switch (optimizationMethodType) {
      case simplex:
        return ext::shared_ptr<OptimizationMethod>(
                new Simplex(simplexLambda));
      case levenbergMarquardt:
        return ext::shared_ptr<OptimizationMethod>(
                new LevenbergMarquardt(levenbergMarquardtEpsfcn,
                                       levenbergMarquardtXtol,
                                       levenbergMarquardtGtol));
      case levenbergMarquardt2:
        return ext::shared_ptr<OptimizationMethod>(
                new LevenbergMarquardt(levenbergMarquardtEpsfcn,
                                       levenbergMarquardtXtol,
                                       levenbergMarquardtGtol,
                                       true));
      case conjugateGradient:
        return ext::make_shared<ConjugateGradient>();
      case steepestDescent:
        return ext::make_shared<SteepestDescent>();
      case bfgs:
        return ext::make_shared<BFGS>();
      case lbfgsb:
        return ext::make_shared<LBFGSB>();
      case conjugateGradient_goldstein:
        return ext::shared_ptr<OptimizationMethod>(new ConjugateGradient(ext::make_shared<GoldsteinLineSearch>()));
      case steepestDescent_goldstein:
        return ext::shared_ptr<OptimizationMethod>(new SteepestDescent(ext::make_shared<GoldsteinLineSearch>()));
      case bfgs_goldstein:
        return ext::shared_ptr<OptimizationMethod>(new BFGS(ext::make_shared<GoldsteinLineSearch>()));
      default:
        QL_FAIL("unknown OptimizationMethod type");
    }
}


std::vector<NamedOptimizationMethod> makeOptimizationMethods(
                                                             const std::vector<OptimizationMethodType>& optimizationMethodTypes,
                                                             Real simplexLambda,
                                                             Real levenbergMarquardtEpsfcn,
                                                             Real levenbergMarquardtXtol,
                                                             Real levenbergMarquardtGtol) {
    std::vector<NamedOptimizationMethod> results;
    for (auto optimizationMethodType : optimizationMethodTypes) {
        NamedOptimizationMethod namedOptimizationMethod;
        namedOptimizationMethod.optimizationMethod = makeOptimizationMethod(
                optimizationMethodType, simplexLambda, levenbergMarquardtEpsfcn,
                levenbergMarquardtXtol, levenbergMarquardtGtol);
        namedOptimizationMethod.name = optimizationMethodTypeToString(optimizationMethodType);
        results.push_back(namedOptimizationMethod);
    }
    return results;
}

Real maxDifference(const Array& a, const Array& b) {
    Array diff = a-b;
    Real maxDiff = 0.0;
    for (Real i : diff)
        maxDiff = std::max(maxDiff, std::fabs(i));
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
    costFunctions_.push_back(ext::make_shared<OneDimensionalPolynomialDegreeN>(coefficients));
    // Set constraint for optimizers: unconstrained problem
    constraints_.push_back(ext::make_shared<NoConstraint>());
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
    endCriterias_.push_back(ext::make_shared<EndCriteria>(
            maxIterations_.back(), maxStationaryStateIterations_.back(),
                            rootEpsilons_.back(), functionEpsilons_.back(),
                            gradientNormEpsilons_.back()));
    // Set optimization methods for optimizer
    std::vector<OptimizationMethodType> optimizationMethodTypes = {
        simplex, levenbergMarquardt, levenbergMarquardt2, conjugateGradient,
        bfgs, lbfgsb //, steepestDescent
    };
    Real simplexLambda = 0.1;                   // characteristic search length for simplex
    Real levenbergMarquardtEpsfcn = 1.0e-8;     // parameters specific for Levenberg-Marquardt
    Real levenbergMarquardtXtol   = 1.0e-8;     //
    Real levenbergMarquardtGtol   = 1.0e-8;     //
    optimizationMethods_.push_back(makeOptimizationMethods(
            optimizationMethodTypes,
            simplexLambda, levenbergMarquardtEpsfcn, levenbergMarquardtXtol,
            levenbergMarquardtGtol));
    // Set expected results for optimizer
    Array xMinExpected(1),yMinExpected(1);
    xMinExpected[0] = -b/(2.0*a);
    yMinExpected[0] = -(b*b-4.0*a*c)/(4.0*a);
    xMinExpected_.push_back(xMinExpected);
    yMinExpected_.push_back(yMinExpected);
}


BOOST_AUTO_TEST_CASE(test) {
    BOOST_TEST_MESSAGE("Testing optimizers...");

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
                    break;
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

BOOST_AUTO_TEST_CASE(nestedOptimizationTest) {
    BOOST_TEST_MESSAGE("Testing nested optimizations...");
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


// Extended Rosenbrock function with analytic gradient; global minimum 0 at \vec{1}
class RosenbrockFunction : public CostFunction {
  public:
    Real value(const Array& x) const override {
        Real f = 0.0;
        for (Size i = 0; i + 1 < x.size(); ++i)
            f += 100.0 * std::pow(x[i+1] - x[i]*x[i], 2) + std::pow(1.0 - x[i], 2);
        return f;
    }
    Array values(const Array& x) const override { return Array(1, value(x)); }
    void gradient(Array& grad, const Array& x) const override {
        std::fill(grad.begin(), grad.end(), 0.0);
        for (Size i = 0; i + 1 < x.size(); ++i) {
            grad[i]   += -400.0*x[i]*(x[i+1]-x[i]*x[i]) - 2.0*(1.0-x[i]);
            grad[i+1] +=  200.0*(x[i+1]-x[i]*x[i]);
        }
    }
    Real valueAndGradient(Array& grad, const Array& x) const override {
        gradient(grad, x);
        return value(x);
    }
};

// Separable quadratic sum_i {w_i (x_i - c_i)^2}; unconstrained minimum c.
class WeightedQuadratic : public CostFunction {
  public:
    WeightedQuadratic(Array center, Array weight)
    : center_(std::move(center)), weight_(std::move(weight)) {}
    Real value(const Array& x) const override {
        Real f = 0.0;
        for (Size i = 0; i < x.size(); ++i)
            f += weight_[i] * std::pow(x[i] - center_[i], 2);
        return f;
    }
    Array values(const Array& x) const override { return Array(1, value(x)); }
    void gradient(Array& grad, const Array& x) const override {
        for (Size i = 0; i < x.size(); ++i)
            grad[i] = 2.0 * weight_[i] * (x[i] - center_[i]);
    }
    Real valueAndGradient(Array& grad, const Array& x) const override {
        gradient(grad, x);
        return value(x);
    }
  private:
    Array center_, weight_;
};

// Same separable quadratic but with no analytic gradient, forcing the
// optimizer onto the base-class central-difference gradient path.
class WeightedQuadraticValueOnly : public CostFunction {
  public:
    WeightedQuadraticValueOnly(Array center, Array weight)
    : center_(std::move(center)), weight_(std::move(weight)) {}
    Real value(const Array& x) const override {
        Real f = 0.0;
        for (Size i = 0; i < x.size(); ++i)
            f += weight_[i] * std::pow(x[i] - center_[i], 2);
        return f;
    }
    Array values(const Array& x) const override { return Array(1, value(x)); }
  private:
    Array center_, weight_;
};

// infinity norm of the projected gradient P(x - g, l, u) - x, the
// quantity that vanishes at a KKT point of a box-constrained problem.
Real projectedGradientNorm(const Array& x, const Array& g,
                           const Array& lo, const Array& hi) {
    Real norm = 0.0;
    for (Size i = 0; i < x.size(); ++i) {
        Real proj = std::min(std::max(x[i] - g[i], lo[i]), hi[i]) - x[i];
        norm = std::max(norm, std::fabs(proj));
    }
    return norm;
}

BOOST_AUTO_TEST_CASE(testLBFGSB) {
    BOOST_TEST_MESSAGE("Testing L-BFGS-B optimizer...");

    // Reference values below were produced with
    //   scipy.optimize.minimize(method='L-BFGS-B').
    // The unconstrained Rosenbrock and the box-constrained quadratics have
    // analytic minima, used directly as the expected results.

    EndCriteria endCriteria(1000, 100, 1e-12, 1e-12, 1e-10);

    // Unconstrained equivalence: Rosenbrock in 2 and 10 dimensions must
    // reach the analytic minimum \vec{1}, as plain (L-)BFGS does.
    for (Size n : {Size(2), Size(10)}) {
        RosenbrockFunction f;
        NoConstraint c;
        Array x0(n, -1.0);
        Problem problem(f, c, x0);
        LBFGSB optimizer(10, 1e-10, 1e1 * QL_EPSILON);
        optimizer.minimize(problem, endCriteria);
        Array x = problem.currentValue();
        Array expected(n, 1.0);
        Real xError = maxDifference(x, expected);
        if (xError > 1e-4)
            BOOST_ERROR("L-BFGS-B on unconstrained Rosenbrock-" << n << "D"
                        << "\n    calculated: " << x
                        << "\n    expected:   " << expected
                        << "\n    x error:    " << xError);
        if (problem.functionValue() > 1e-8)
            BOOST_ERROR("L-BFGS-B Rosenbrock-" << n << "D f = "
                        << problem.functionValue() << " (expected ~0)");
    }

    const Array center{3.0, -2.0, 0.5};
    const Array weight{1.0, 4.0, 0.25};

    // Interior minimum: with wide bounds enclosing the unconstrained
    // optimum the result must equal the unconstrained minimizer.
    {
        WeightedQuadratic f(center, weight);
        Array lo(3, -10.0), hi(3, 10.0);
        NonhomogeneousBoundaryConstraint c(lo, hi);
        Array x0(3, 0.0);
        Problem problem(f, c, x0);
        LBFGSB optimizer(10, 1e-10, 1e1 * QL_EPSILON);
        optimizer.minimize(problem, endCriteria);
        Real xError = maxDifference(problem.currentValue(), center);
        if (xError > 1e-6)
            BOOST_ERROR("L-BFGS-B interior-bound quadratic"
                        << "\n    calculated: " << problem.currentValue()
                        << "\n    expected:   " << center
                        << "\n    x error:    " << xError);
    }

    // Active-bound minimum: the box [0,1]^3 clips the unconstrained
    // optimum (3,-2,0.5), so the solution must sit on the boundary at
    // (1, 0, 0.5) with a vanishing projected gradient.
    {
        WeightedQuadratic f(center, weight);
        Array lo(3, 0.0), hi(3, 1.0);
        NonhomogeneousBoundaryConstraint c(lo, hi);
        Array x0(3, 0.5);
        Problem problem(f, c, x0);
        LBFGSB optimizer;
        EndCriteria::Type ret = optimizer.minimize(problem, endCriteria);
        // the operative stop must be the projected-gradient (KKT) test, not
        // the function-reduction fallback
        if (ret != EndCriteria::ZeroGradientNorm)
            BOOST_ERROR("L-BFGS-B active-bound quadratic ended with " << ret
                        << " (expected ZeroGradientNorm)");
        Array x = problem.currentValue();
        Array expected{1.0, 0.0, 0.5};
        Real xError = maxDifference(x, expected);
        if (xError > 1e-7)
            BOOST_ERROR("L-BFGS-B active-bound quadratic"
                        << "\n    calculated: " << x
                        << "\n    expected:   " << expected
                        << "\n    x error:    " << xError);
        Array g(3);
        f.gradient(g, x);
        Real pg = projectedGradientNorm(x, g, lo, hi);
        if (pg > 1e-6)
            BOOST_ERROR("L-BFGS-B active-bound projected gradient = " << pg
                        << " (expected ~0)");
    }

    // Bound-constrained Rosenbrock: the box [-2,0.5]^2 clips the optimum;
    // SciPy's L-BFGS-B converges to (0.5, 0.25) on the boundary.
    {
        RosenbrockFunction f;
        Array lo(2, -2.0), hi(2, 0.5);
        NonhomogeneousBoundaryConstraint c(lo, hi);
        Array x0(2, -1.0);
        Problem problem(f, c, x0);
        LBFGSB optimizer(10, 1e-10, 1e1 * QL_EPSILON);
        optimizer.minimize(problem, endCriteria);
        Array x = problem.currentValue();
        Array expected{0.5, 0.25};
        Real xError = maxDifference(x, expected);
        if (xError > 1e-5)
            BOOST_ERROR("L-BFGS-B bound-constrained Rosenbrock"
                        << "\n    calculated: " << x
                        << "\n    expected:   " << expected
                        << "\n    x error:    " << xError);
        Array g(2);
        f.gradient(g, x);
        Real pg = projectedGradientNorm(x, g, lo, hi);
        if (pg > 1e-6)
            BOOST_ERROR("L-BFGS-B bound-constrained Rosenbrock projected gradient = "
                        << pg << " (expected ~0)");
    }
}


BOOST_AUTO_TEST_CASE(testLBFGSBActiveBounds) {
    BOOST_TEST_MESSAGE("Testing L-BFGS-B with active bounds, corners and "
                       "pinned coordinates...");

    EndCriteria endCriteria(1000, 100, 1e-12, 1e-12, 1e-10);

    // All bounds active at a corner: the unconstrained optimum (5,5,5)
    // lies outside [0,1]^3, so every coordinate is pinned at its upper
    // bound and the solution is the corner (1,1,1) with a vanishing
    // projected gradient. Exercises the empty free-set subspace path.
    {
        WeightedQuadratic f(Array{5.0, 5.0, 5.0}, Array{1.0, 1.0, 1.0});
        Array lo(3, 0.0), hi(3, 1.0);
        NonhomogeneousBoundaryConstraint c(lo, hi);
        Array x0(3, 0.5);
        Problem problem(f, c, x0);
        LBFGSB optimizer(10, 1e-10, 1e1 * QL_EPSILON);
        optimizer.minimize(problem, endCriteria);
        Array x = problem.currentValue();
        Array expected(3, 1.0);
        Real xError = maxDifference(x, expected);
        Array g(3);
        f.gradient(g, x);
        Real pg = projectedGradientNorm(x, g, lo, hi);
        if (xError > 1e-8 || pg > 1e-8)
            BOOST_ERROR("L-BFGS-B all-active corner"
                        << "\n    calculated: " << x
                        << "\n    expected:   " << expected
                        << "\n    x error:    " << xError
                        << "\n    proj. grad: " << pg);
    }

    // Two simultaneously active bounds reached through distinct
    // breakpoints: the disparate weights make the projected-gradient
    // path hit the two upper bounds at different step lengths, so the
    // Cauchy search must traverse more than one breakpoint.
    {
        WeightedQuadratic f(Array{10.0, 10.0}, Array{1.0, 100.0});
        Array lo(2, 0.0), hi(2, 1.0);
        NonhomogeneousBoundaryConstraint c(lo, hi);
        Array x0{0.9, 0.1};
        Problem problem(f, c, x0);
        LBFGSB optimizer(10, 1e-10, 1e1 * QL_EPSILON);
        optimizer.minimize(problem, endCriteria);
        Array x = problem.currentValue();
        Array expected{1.0, 1.0};
        Real xError = maxDifference(x, expected);
        Array g(2);
        f.gradient(g, x);
        Real pg = projectedGradientNorm(x, g, lo, hi);
        if (xError > 1e-8 || pg > 1e-8)
            BOOST_ERROR("L-BFGS-B two-active-bound quadratic"
                        << "\n    calculated: " << x
                        << "\n    expected:   " << expected
                        << "\n    x error:    " << xError
                        << "\n    proj. grad: " << pg);
    }

    // Single variable (n = 1) with an active bound: the minimum of
    // (x-5)^2 over [0,1] is the boundary point x = 1.
    {
        WeightedQuadratic f(Array{5.0}, Array{1.0});
        Array lo(1, 0.0), hi(1, 1.0);
        NonhomogeneousBoundaryConstraint c(lo, hi);
        Array x0(1, 0.5);
        Problem problem(f, c, x0);
        LBFGSB optimizer(10, 1e-10, 1e1 * QL_EPSILON);
        optimizer.minimize(problem, endCriteria);
        Array x = problem.currentValue();
        Array g(1);
        f.gradient(g, x);
        Real xError = maxDifference(x, Array{1.0});
        Real pg = projectedGradientNorm(x, g, lo, hi);
        if (xError > 1e-8 || pg > 1e-8)
            BOOST_ERROR("L-BFGS-B n=1 active bound"
                        << "\n    calculated: " << x
                        << "\n    x error:    " << xError
                        << "\n    proj. grad: " << pg);
    }

    // Pinned coordinate (lower == upper): the third variable is frozen
    // at 0.25 while the first two reach their unconstrained optima.
    {
        WeightedQuadratic f(Array{3.0, -2.0, 0.5}, Array{1.0, 4.0, 0.25});
        Array lo{-10.0, -10.0, 0.25}, hi{10.0, 10.0, 0.25};
        NonhomogeneousBoundaryConstraint c(lo, hi);
        Array x0(3, 0.0);
        Problem problem(f, c, x0);
        LBFGSB optimizer(10, 1e-10, 1e1 * QL_EPSILON);
        optimizer.minimize(problem, endCriteria);
        Array x = problem.currentValue();
        Array expected{3.0, -2.0, 0.25};
        Real xError = maxDifference(x, expected);
        if (xError > 1e-6)
            BOOST_ERROR("L-BFGS-B pinned coordinate (lower==upper)"
                        << "\n    calculated: " << x
                        << "\n    expected:   " << expected
                        << "\n    x error:    " << xError);
    }
}


BOOST_AUTO_TEST_CASE(testLBFGSBCoverage) {
    BOOST_TEST_MESSAGE("Testing L-BFGS-B small memory, infeasible start and "
                       "finite-difference gradient...");

    EndCriteria endCriteria(1000, 100, 1e-12, 1e-12, 1e-10);

    // Limited memory smaller than the dimension (m < n): forces eviction
    // of correction pairs and repeated rebuilds of the compact
    // representation while still converging to \vec{1}.
    {
        const Size n = 20;
        RosenbrockFunction f;
        NoConstraint c;
        Array x0(n, -1.0);
        Problem problem(f, c, x0);
        LBFGSB optimizer(3, 1e-8, 1e1 * QL_EPSILON); // memory = 3 < n = 20
        optimizer.minimize(problem, endCriteria);
        Array x = problem.currentValue();
        Array expected(n, 1.0);
        Real xError = maxDifference(x, expected);
        if (xError > 1e-4)
            BOOST_ERROR("L-BFGS-B small-memory Rosenbrock-" << n << "D (m=3)"
                        << "\n    x error: " << xError);
        if (problem.functionValue() > 1e-8)
            BOOST_ERROR("L-BFGS-B small-memory Rosenbrock f = "
                        << problem.functionValue() << " (expected ~0)");
    }

    // Infeasible start point: x0 lies outside the box and must be clipped
    // into [l,u] before optimization; the solution is unaffected.
    {
        WeightedQuadratic f(Array{3.0, -2.0, 0.5}, Array{1.0, 4.0, 0.25});
        Array lo(3, 0.0), hi(3, 1.0);
        NonhomogeneousBoundaryConstraint c(lo, hi);
        Array x0(3, 5.0); // outside [0,1]^3
        Problem problem(f, c, x0);
        LBFGSB optimizer(10, 1e-10, 1e1 * QL_EPSILON);
        optimizer.minimize(problem, endCriteria);
        Array x = problem.currentValue();
        Array expected{1.0, 0.0, 0.5};
        Real xError = maxDifference(x, expected);
        if (xError > 1e-7)
            BOOST_ERROR("L-BFGS-B infeasible start"
                        << "\n    calculated: " << x
                        << "\n    expected:   " << expected
                        << "\n    x error:    " << xError);
    }

    // Finite-difference gradient with active bounds: the cost function
    // exposes no analytic gradient, so the optimizer uses central
    // differences. The interior coordinate (0.7) has to move off its
    // starting value, unlike the two clipped coordinates.
    {
        WeightedQuadraticValueOnly f(Array{3.0, -2.0, 0.7}, Array{1.0, 4.0, 0.25});
        Array lo(3, 0.0), hi(3, 1.0);
        NonhomogeneousBoundaryConstraint c(lo, hi);
        Array x0(3, 0.5);
        Problem problem(f, c, x0);
        LBFGSB optimizer(10, 1e-6, 1e7 * QL_EPSILON);
        optimizer.minimize(problem, endCriteria);
        Array x = problem.currentValue();
        Array expected{1.0, 0.0, 0.7};
        Real xError = maxDifference(x, expected);
        if (xError > 1e-5)
            BOOST_ERROR("L-BFGS-B finite-difference gradient with bounds"
                        << "\n    calculated: " << x
                        << "\n    expected:   " << expected
                        << "\n    x error:    " << xError);
    }
}


class FirstDeJong : public CostFunction {
  public:
    Array values(const Array& x) const override {
        return Array(x.size(),value(x));
    }
    Real value(const Array& x) const override { return DotProduct(x, x); }
};

class SecondDeJong : public CostFunction {
  public:
    Array values(const Array& x) const override {
        return Array(x.size(),value(x));
    }
    Real value(const Array& x) const override {
        return  100.0*(x[0]*x[0]-x[1])*(x[0]*x[0]-x[1])
            + (1.0-x[0])*(1.0-x[0]);
    }
};

class ModThirdDeJong : public CostFunction {
  public:
    Array values(const Array& x) const override {
        return Array(x.size(),value(x));
    }
    Real value(const Array& x) const override {
        Real fx = 0.0;
        for (Real i : x) {
            fx += std::floor(i) * std::floor(i);
        }
        return fx;
    }
};

class ModFourthDeJong : public CostFunction {
  public:
    ModFourthDeJong()
    : uniformRng_(MersenneTwisterUniformRng(4711)) {
    }
    Array values(const Array& x) const override {
        return Array(x.size(),value(x));
    }
    Real value(const Array& x) const override {
        Real fx = 0.0;
        for (Size i=0; i<x.size(); ++i) {
            fx += (i+1.0)*pow(x[i],4.0) + uniformRng_.nextReal();
        }
        return fx;
    }
    MersenneTwisterUniformRng uniformRng_;
};

class Griewangk : public CostFunction {
  public:
    Array values(const Array& x) const override {
        return Array(x.size(),value(x));
    }
    Real value(const Array& x) const override {
        Real fx = 0.0;
        for (Real i : x) {
            fx += i * i / 4000.0;
        }
        Real p = 1.0;
        for (Size i=0; i<x.size(); ++i) {
            p *= cos(x[i]/sqrt(i+1.0));
        }
        return fx - p + 1.0;
    }
};


class Rastrigin : public CostFunction {
  public:
    Real value(const Array& x) const override {
        Real fx = 10.0 * x.size();

        for (Real xi : x)
            fx += xi * xi - 10.0 * cos(M_TWOPI * xi);

        return fx;
    }

    Array values(const Array& x) const override { return Array(x.size(), value(x)); }
};


class QuadraticForm : public CostFunction {
  public:
    explicit QuadraticForm(Matrix m) : m_(std::move(m)) {}

    Real value(const Array& x) const override { return DotProduct(x, m_ * x); }
    Array values(const Array& x) const override { return Array(x.size(), value(x)); }

  private:
    Matrix m_;
};


BOOST_AUTO_TEST_CASE(testDifferentialEvolution) {
    BOOST_TEST_MESSAGE("Testing differential evolution...");

    /* Note:
    *
    * The "ModFourthDeJong" doesn't have a well defined optimum because
    * of its noisy part. It just has to be <= 15 in our example.
    * The concrete value might differ for a different input and
    * different random numbers.
    *
    * The "Griewangk" function is an example where the adaptive
    * version of DifferentialEvolution turns out to be more successful.
    */

    DifferentialEvolution::Configuration conf =
        DifferentialEvolution::Configuration()
        .withStepsizeWeight(0.4)
        .withBounds()
        .withCrossoverProbability(0.35)
        .withPopulationMembers(500)
        .withStrategy(DifferentialEvolution::BestMemberWithJitter)
        .withCrossoverType(DifferentialEvolution::Normal)
        .withAdaptiveCrossover()
        .withSeed(3242);
    DifferentialEvolution deOptim(conf);

    DifferentialEvolution::Configuration conf2 =
        DifferentialEvolution::Configuration()
        .withStepsizeWeight(1.8)
        .withBounds()
        .withCrossoverProbability(0.9)
        .withPopulationMembers(1000)
        .withStrategy(DifferentialEvolution::Rand1SelfadaptiveWithRotation)
        .withCrossoverType(DifferentialEvolution::Normal)
        .withAdaptiveCrossover()
        .withSeed(3242);
    DifferentialEvolution deOptim2(conf2);

    std::vector<DifferentialEvolution > diffEvolOptimisers = {
        deOptim,
        deOptim,
        deOptim,
        deOptim,
        deOptim2
    };

    std::vector<ext::shared_ptr<CostFunction> > costFunctions = {
        ext::shared_ptr<CostFunction>(new FirstDeJong),
        ext::shared_ptr<CostFunction>(new SecondDeJong),
        ext::shared_ptr<CostFunction>(new ModThirdDeJong),
        ext::shared_ptr<CostFunction>(new ModFourthDeJong),
        ext::shared_ptr<CostFunction>(new Griewangk)
    };

    std::vector<BoundaryConstraint> constraints = {
        {-10.0, 10.0},
        {-10.0, 10.0},
        {-10.0, 10.0},
        {-10.0, 10.0},
        {-600.0, 600.0}
    };

    std::vector<Array> initialValues = {
        Array(3, 5.0),
        Array(2, 5.0),
        Array(5, 5.0),
        Array(30, 5.0),
        Array(10, 100.0)
    };

    std::vector<EndCriteria> endCriteria = {
        {100, 10, 1e-10, 1e-8, Null<Real>()},
        {100, 10, 1e-10, 1e-8, Null<Real>()},
        {100, 10, 1e-10, 1e-8, Null<Real>()},
        {500, 100, 1e-10, 1e-8, Null<Real>()},
        {1000, 800, 1e-12, 1e-10, Null<Real>()}
    };

    std::vector<Real> minima = {
        0.0,
        0.0,
        0.0,
        10.9639796558,
        0.0
    };

    for (Size i = 0; i < costFunctions.size(); ++i) {
        Problem problem(*costFunctions[i], constraints[i], initialValues[i]);
        diffEvolOptimisers[i].minimize(problem, endCriteria[i]);

        if (i != 3) {
            // stable
            if (std::fabs(problem.functionValue() - minima[i]) > 1e-8) {
                BOOST_ERROR("costFunction # " << i
                            << "\ncalculated: " << problem.functionValue()
                            << "\nexpected:   " << minima[i]);
            }
        } else {
            // this case is unstable due to randomness; we're good as
            // long as the result is below 15
            if (problem.functionValue() > 15) {
                BOOST_ERROR("costFunction # " << i
                            << "\ncalculated: " << problem.functionValue()
                            << "\nexpected:   " << "less than 15");
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testCMAES) {
    BOOST_TEST_MESSAGE("Testing CMA-ES optimizer...");

    // Convex convergence: shifted, ill-conditioned separable quadratics
    // (2D, 10D) must reach the analytic minimum value 0.
    {
        EndCriteria endCriteria(5000, 100, 1e-14, 1e-14, Null<Real>());
        for (Size n : {Size(2), Size(10)}) {
            Array center(n), weight(n);

            for (Size i = 0; i < n; ++i) {
                center[i] = 1.0 + 0.5 * i;
                weight[i] = std::pow(10.0, 2.0 * i / Real(n));
            }

            WeightedQuadratic f(center, weight);
            NoConstraint c;
            Array x0(n, 0.0);
            Problem problem(f, c, x0);
            Cmaes optimizer(Cmaes::Configuration().withSigma(1.0).withSeed(42));

            optimizer.minimize(problem, endCriteria);

            if (problem.functionValue() > 1e-8)
                BOOST_ERROR("CMA-ES convex quadratic-" << n << "D f = "
                            << problem.functionValue() << " (expected < 1e-8)");
        }
    }

    // Non-convex convergence: Rosenbrock (2D, 10D) must reach the
    // minimizer (1,...,1) with f \approx 0.
    {
        EndCriteria endCriteria(20000, 300, 1e-14, 1e-14, Null<Real>());
        for (Size n : {Size(2), Size(10)}) {
            RosenbrockFunction f;
            NoConstraint c;
            Array x0(n, 0.0);
            Cmaes::Configuration cfg =
                Cmaes::Configuration().withSigma(0.5).withSeed(123);

            if (n >= 10)
                cfg.withPopulationSize(20); // larger population for 10D

            Problem problem(f, c, x0);
            Cmaes(cfg).minimize(problem, endCriteria);
            Array expected(n, 1.0);

            Real xError = maxDifference(problem.currentValue(), expected);
            if (problem.functionValue() > 1e-8 || xError > 1e-4)
                BOOST_ERROR("CMA-ES Rosenbrock-" << n << "D"
                            << "\n    f       = " << problem.functionValue()
                            << "\n    x error = " << xError);
        }
    }

    // Multimodal global search: Rastrigin 5D from a start away from the
    // origin must reach the global minimum, and match or beat DifferentialEvolution.
    {
        const Size n = 5;
        Rastrigin f;
        BoundaryConstraint c(-5.12, 5.12);
        Array x0(n, 3.0);
        EndCriteria endCriteria(2000, 200, 1e-12, 1e-12, Null<Real>());

        // Large population lets CMA-ES cross the local-minimum lattice to the
        // global basin. Small populations reliably stall one lattice step away.
        Cmaes::Configuration cfg = Cmaes::Configuration()
            .withSigma(4.0).withPopulationSize(150).withSeed(1);
        Problem cmaesProblem(f, c, x0);
        Cmaes(cfg).minimize(cmaesProblem, endCriteria);

        Real cmaesValue = cmaesProblem.functionValue();
        if (cmaesValue > 1e-4)
            BOOST_ERROR("CMA-ES Rastrigin-5D f = " << cmaesValue
                        << " (expected < 1e-4)");

        DifferentialEvolution::Configuration deConf =
            DifferentialEvolution::Configuration()
            .withStepsizeWeight(0.6)
            .withBounds()
            .withCrossoverProbability(0.9)
            .withPopulationMembers(60)
            .withStrategy(DifferentialEvolution::BestMemberWithJitter)
            .withSeed(7);
        DifferentialEvolution de(deConf);
        Problem deProblem(f, c, x0);

        de.minimize(deProblem, endCriteria);
        if (cmaesValue > deProblem.functionValue() + 1e-8)
            BOOST_ERROR("CMA-ES (" << cmaesValue << ") failed to match/beat "
                        << "DifferentialEvolution (" << deProblem.functionValue()
                        << ") on Rastrigin-5D");
    }

    // Rotational/affine invariance: an ill-conditioned quadratic x^T A x and
    // its rotation by an orthogonal Q must be solved in a comparable eval budget.
    {
        Matrix A(2, 2, 0.0);
        A[0][0] = 1.0;
        A[1][1] = 30.0;

        Real theta = 0.7;
        Matrix Q(2, 2, 0.0);
        Q[0][0] = std::cos(theta); Q[0][1] = -std::sin(theta);
        Q[1][0] = std::sin(theta); Q[1][1] =  std::cos(theta);
        Matrix Arot = transpose(Q) * A * Q;

        QuadraticForm f(A);
        QuadraticForm fRot(Arot);
        NoConstraint c;

        Array x0{2.0, 2.0};
        Array x0Rot = transpose(Q) * x0; // s.t. Q * x0Rot = x0

        EndCriteria endCriteria(5000, 100, 1e-14, 1e-14, Null<Real>());
        Cmaes::Configuration cfg =
            Cmaes::Configuration().withSigma(1.0).withSeed(99);

        Problem pA(f, c, x0);
        Cmaes(cfg).minimize(pA, endCriteria);

        Problem pRot(fRot, c, x0Rot);
        Cmaes(cfg).minimize(pRot, endCriteria);

        if (pA.functionValue() > 1e-8 || pRot.functionValue() > 1e-8)
            BOOST_ERROR("CMA-ES invariance: values " << pA.functionValue()
                        << " and " << pRot.functionValue() << " (expected < 1e-8)");

        Real evalsA = Real(pA.functionEvaluation());
        Real evalsRot = Real(pRot.functionEvaluation());
        Real ratio = std::max(evalsA, evalsRot) / std::min(evalsA, evalsRot);
        if (ratio > 2.0)
            BOOST_ERROR("CMA-ES rotational invariance: eval budgets differ by "
                        << ratio << "x (" << evalsA << " vs " << evalsRot << ")");
    }

    // Determinism check: runs must be bit-identical
    {
        WeightedQuadratic f(Array{1.0, -2.0, 0.5}, Array{1.0, 3.0, 7.0});
        NoConstraint c;
        Array x0(3, 0.0);
        EndCriteria endCriteria(500, 50, 1e-12, 1e-12, Null<Real>());
        Cmaes::Configuration cfg =
            Cmaes::Configuration().withSigma(1.0).withSeed(2024);

        Problem p1(f, c, x0);
        Cmaes(cfg).minimize(p1, endCriteria);
        Problem p2(f, c, x0);
        Cmaes(cfg).minimize(p2, endCriteria);

        if (p1.functionValue() != p2.functionValue())
            BOOST_ERROR("CMA-ES not reproducible: " << p1.functionValue()
                        << " != " << p2.functionValue());
    }

    // Box bounds: unconstrained minimum (3,-2) lies outside [0, 1] ^ 2.
    // The returned point must be feasible and sit at the constrained min (1,0).
    {
        WeightedQuadratic f(Array{3.0, -2.0}, Array{1.0, 1.0});
        BoundaryConstraint c(0.0, 1.0);
        Array x0(2, 0.5);
        EndCriteria endCriteria(3000, 100, 1e-14, 1e-14, Null<Real>());
        Cmaes::Configuration cfg =
            Cmaes::Configuration().withSigma(0.3).withSeed(5);
        Problem problem(f, c, x0);

        Cmaes(cfg).minimize(problem, endCriteria);

        Array x = problem.currentValue();

        Array expected{1.0, 0.0};
        if (!c.test(x))
            BOOST_ERROR("CMA-ES box-bounds returned infeasible point " << x);
        Real xError = maxDifference(x, expected);
        if (xError > 1e-5)
            BOOST_ERROR("CMA-ES box-bounds"
                        << "\n    calculated: " << x
                        << "\n    expected:   " << expected
                        << "\n    x error:    " << xError);
    }
}

BOOST_AUTO_TEST_SUITE_END()
