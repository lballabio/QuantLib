
/*
 Copyright (C) 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "operators.hpp"
#include "utilities.hpp"
#include <ql/Math/normaldistribution.hpp>
#include <ql/FiniteDifferences/dzero.hpp>
#include <ql/FiniteDifferences/dplusdminus.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    double average = 0.0, sigma = 1.0;

}

void OperatorTest::testConsistency() {

    BOOST_MESSAGE("Testing differential operators...");

    NormalDistribution normal(average,sigma);
    CumulativeNormalDistribution cum(average,sigma);

    double xMin = average - 4*sigma,
           xMax = average + 4*sigma;
    Size N = 10001;
    double h = (xMax-xMin)/(N-1);

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
                   std::minus<double>());
    double e = norm(diff.begin(),diff.end(),h);
    if (e > 1.0e-6) {
        char s[10];
        QL_SPRINTF(s,"%5.2e",e);
        BOOST_FAIL("norm of 1st derivative of cum minus Gaussian: "
                   + std::string(s) + "\n"
                   "tolerance exceeded");
    }

    // check that the second derivative of cum is normal.derivative
    temp = D2.applyTo(yi);
    std::transform(yd.begin(),yd.end(),temp.begin(),diff.begin(),
                   std::minus<double>());
    e = norm(diff.begin(),diff.end(),h);
    if (e > 1.0e-4) {
        char s[10];
        QL_SPRINTF(s,"%5.2e",e);
        BOOST_FAIL("norm of 2nd derivative of cum minus Gaussian derivative: "
                   + std::string(s) + "\n"
                   "tolerance exceeded");
    }
}


test_suite* OperatorTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Operator tests");
    suite->add(BOOST_TEST_CASE(&OperatorTest::testConsistency));
    return suite;
}

