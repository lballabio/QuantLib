/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl

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

#include "operators.hpp"
#include "utilities.hpp"
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/methods/finitedifferences/dzero.hpp>
#include <ql/methods/finitedifferences/dplusdminus.hpp>
#include <ql/methods/finitedifferences/bsmoperator.hpp>
#include <ql/methods/finitedifferences/bsmtermoperator.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

Real average = 0.0, sigma = 1.0;

}


void OperatorTest::testConsistency() {

    BOOST_MESSAGE("Testing differential operators...");

    NormalDistribution normal(average,sigma);
    CumulativeNormalDistribution cum(average,sigma);

    Real xMin = average - 4*sigma,
         xMax = average + 4*sigma;
    Size N = 10001;
    // FLOATING_POINT_EXCEPTION
    Real h = (xMax-xMin)/(N-1);

    Array x(N), y(N), yi(N), yd(N), temp(N), diff(N);

    Size i;
    for (i=0; i<N; i++)
        x[i] = xMin+h*i;
    std::transform(x.begin(),x.end(),y.begin(),normal);
    std::transform(x.begin(),x.end(),yi.begin(),cum);
    for (i=0; i<x.size(); i++)
        yd[i] = normal.derivative(x[i]);

    // define the differential operators
    DZero D(N,h);
    DPlusDMinus D2(N,h);

    // check that the derivative of cum is Gaussian
    temp = D.applyTo(yi);
    std::transform(y.begin(),y.end(),temp.begin(),diff.begin(),
                   std::minus<Real>());
    Real e = norm(diff.begin(),diff.end(),h);
    if (e > 1.0e-6) {
        BOOST_FAIL("norm of 1st derivative of cum minus Gaussian: " << e
                   << "\ntolerance exceeded");
    }

    // check that the second derivative of cum is normal.derivative
    temp = D2.applyTo(yi);
    std::transform(yd.begin(),yd.end(),temp.begin(),diff.begin(),
                   std::minus<Real>());
    e = norm(diff.begin(),diff.end(),h);
    if (e > 1.0e-4) {
        BOOST_FAIL("norm of 2nd derivative of cum minus Gaussian derivative: "
                   << e << "\ntolerance exceeded");
    }
}

void OperatorTest::testBSMOperatorConsistency() {
    BOOST_MESSAGE("Testing consistency of BSM operators...");

    Array grid(10);
    Real price = 20.0;
    Real factor = 1.1;
    Size i;
    for (i = 0; i < grid.size(); i++) {
        grid[i] = price;
        // FLOATING_POINT_EXCEPTION
        price *= factor;
    }
    Real dx = std::log(factor);
    Rate r = 0.05;
    Rate q = 0.01;
    Volatility sigma = 0.5;

    BSMOperator ref(grid.size(), dx, r, q, sigma);

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();
    Date exercise = today + 2*Years;
    Time residualTime = dc.yearFraction(today,exercise);

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(today, q, dc);
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(today, r, dc);
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, sigma, dc);
    boost::shared_ptr<GeneralizedBlackScholesProcess> stochProcess(
        new GeneralizedBlackScholesProcess(
                                       Handle<Quote>(spot),
                                       Handle<YieldTermStructure>(qTS),
                                       Handle<YieldTermStructure>(rTS),
                                       Handle<BlackVolTermStructure>(volTS)));
    BSMOperator op1(grid, stochProcess, residualTime);
    BSMTermOperator op2(grid, stochProcess, residualTime);

    Real tolerance = 1.0e-6;
    Array lderror = ref.lowerDiagonal() - op1.lowerDiagonal();
    Array derror = ref.diagonal() - op1.diagonal();
    Array uderror = ref.upperDiagonal() - op1.upperDiagonal();

    for (i=2; i<grid.size()-2; i++) {
        if (std::fabs(lderror[i]) > tolerance ||
            std::fabs(derror[i]) > tolerance ||
            std::fabs(uderror[i]) > tolerance) {
            BOOST_FAIL("inconsistency between BSM operators:\n"
                       << io::ordinal(i) << " row:\n"
                       << "expected:   "
                       << ref.lowerDiagonal()[i] << ", "
                       << ref.diagonal()[i] << ", "
                       << ref.upperDiagonal()[i] << "\n"
                       << "calculated: "
                       << op1.lowerDiagonal()[i] << ", "
                       << op1.diagonal()[i] << ", "
                       << op1.upperDiagonal()[i]);
        }
    }
    lderror = ref.lowerDiagonal() - op2.lowerDiagonal();
    derror = ref.diagonal() - op2.diagonal();
    uderror = ref.upperDiagonal() - op2.upperDiagonal();

    for (i=2; i<grid.size()-2; i++) {
        if (std::fabs(lderror[i]) > tolerance ||
            std::fabs(derror[i]) > tolerance ||
            std::fabs(uderror[i]) > tolerance) {
            BOOST_FAIL("inconsistency between BSM operators:\n"
                       << io::ordinal(i) << " row:\n"
                       << "expected:   "
                       << ref.lowerDiagonal()[i] << ", "
                       << ref.diagonal()[i] << ", "
                       << ref.upperDiagonal()[i] << "\n"
                       << "calculated: "
                       << op2.lowerDiagonal()[i] << ", "
                       << op2.diagonal()[i] << ", "
                       << op2.upperDiagonal()[i]);
        }
    }
}


test_suite* OperatorTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Operator tests");
    // FLOATING_POINT_EXCEPTION
    suite->add(QUANTLIB_TEST_CASE(&OperatorTest::testConsistency));
    // FLOATING_POINT_EXCEPTION
    suite->add(QUANTLIB_TEST_CASE(&OperatorTest::testBSMOperatorConsistency));
    return suite;
}

