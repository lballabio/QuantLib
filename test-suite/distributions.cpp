
/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
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

#include "distributions.hpp"
#include "utilities.hpp"
#include <ql/Math/normaldistribution.hpp>
#include <ql/Math/bivariatenormaldistribution.hpp>
#include <ql/Math/poissondistribution.hpp>
#include <ql/Math/comparison.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    Real average = 1.0, sigma = 2.0;

    Real gaussian(Real x) {
        Real normFact = sigma*QL_SQRT(2*M_PI);
        Real dx = x-average;
        return QL_EXP(-dx*dx/(2.0*sigma*sigma))/normFact;
    }

    Real gaussianDerivative(Real x) {
        Real normFact = sigma*sigma*sigma*QL_SQRT(2*M_PI);
        Real dx = x-average;
        return -dx*QL_EXP(-dx*dx/(2.0*sigma*sigma))/normFact;
    }

}

void DistributionTest::testNormal() {

    BOOST_MESSAGE("Testing normal distributions...");

    InverseCumulativeNormal invCumStandardNormal;
    Real check = invCumStandardNormal(0.5);
    if (check != 0.0e0) {
        BOOST_FAIL("C++ inverse cumulative of the standard normal at 0.5 is "
                   + DecimalFormatter::toExponential(check,2) +
                   "\n instead of zero: something is wrong!");
    }

    NormalDistribution normal(average,sigma);
    CumulativeNormalDistribution cum(average,sigma);
    InverseCumulativeNormal invCum(average,sigma);

    Size numberOfStandardDeviation = 6;
    Real xMin = average - numberOfStandardDeviation*sigma,
         xMax = average + numberOfStandardDeviation*sigma;
    Size N = 100001;
    Real h = (xMax-xMin)/(N-1);

    std::vector<Real> x(N), y(N), yd(N), temp(N), diff(N);

    Size i;
    for (i=0; i<N; i++)
        x[i] = xMin+h*i;
    std::transform(x.begin(),x.end(),y.begin(),std::ptr_fun(gaussian));
    std::transform(x.begin(),x.end(),yd.begin(),
                   std::ptr_fun(gaussianDerivative));

    // check that normal = Gaussian
    std::transform(x.begin(),x.end(),temp.begin(),normal);
    std::transform(y.begin(),y.end(),temp.begin(),diff.begin(),
                   std::minus<Real>());
    Real e = norm(diff.begin(),diff.end(),h);
    if (e > 1.0e-16) {
        BOOST_FAIL("norm of C++ NormalDistribution minus analytic Gaussian: "
                   + DecimalFormatter::toExponential(e,2) + "\n"
                   "tolerance exceeded");
    }

    // check that invCum . cum = identity
    std::transform(x.begin(),x.end(),temp.begin(),cum);
    std::transform(temp.begin(),temp.end(),temp.begin(),invCum);
    std::transform(x.begin(),x.end(),temp.begin(),diff.begin(),
                   std::minus<Real>());
    e = norm(diff.begin(),diff.end(),h);
    if (e > 1.0e-8) {
        BOOST_FAIL("norm of invCum . cum minus identity: "
                   + DecimalFormatter::toExponential(e,2) + "\n"
                   "tolerance exceeded");
    }

    // check that cum.derivative = Gaussian
    for (i=0; i<x.size(); i++)
        temp[i] = cum.derivative(x[i]);
    std::transform(y.begin(),y.end(),temp.begin(),diff.begin(),
                   std::minus<Real>());
    e = norm(diff.begin(),diff.end(),h);
    if (e > 1.0e-16) {
        BOOST_FAIL(
            "norm of C++ Cumulative.derivative minus analytic Gaussian: "
            + DecimalFormatter::toExponential(e,2) + "\n"
            "tolerance exceeded");
    }

    // check that normal.derivative = gaussianDerivative
    for (i=0; i<x.size(); i++)
        temp[i] = normal.derivative(x[i]);
    std::transform(yd.begin(),yd.end(),temp.begin(),diff.begin(),
                   std::minus<Real>());
    e = norm(diff.begin(),diff.end(),h);
    if (e > 1.0e-16) {
        BOOST_FAIL("norm of C++ Normal.derivative minus analytic derivative: "
                   + DecimalFormatter::toExponential(e,2) + "\n"
                   "tolerance exceeded");
    }
}

void DistributionTest::testBivariate() {

    BOOST_MESSAGE("Testing bivariate cumulative normal distribution...");

    struct BivariateTestData {
        Real a;
        Real b;
        Real rho;
        Real result;
    };

    BivariateTestData values[] = {
        /* The data below are from
           "Option pricing formulas", E.G. Haug, McGraw-Hill 1998
           pag 193
        */
        {  0.0,  0.0,  0.0, 0.250000 },
        {  0.0,  0.0, -0.5, 0.166667 },
        {  0.0,  0.0,  0.5, 1.0/3    },
        {  0.0, -0.5,  0.0, 0.154269 },
        {  0.0, -0.5, -0.5, 0.081660 },
        {  0.0, -0.5,  0.5, 0.226878 },
        {  0.0,  0.5,  0.0, 0.345731 },
        {  0.0,  0.5, -0.5, 0.273122 },
        {  0.0,  0.5,  0.5, 0.418340 },

        { -0.5,  0.0,  0.0, 0.154269 },
        { -0.5,  0.0, -0.5, 0.081660 },
        { -0.5,  0.0,  0.5, 0.226878 },
        { -0.5, -0.5,  0.0, 0.095195 },
        { -0.5, -0.5, -0.5, 0.036298 },
        { -0.5, -0.5,  0.5, 0.163319 },
        { -0.5,  0.5,  0.0, 0.213342 },
        { -0.5,  0.5, -0.5, 0.145218 },
        { -0.5,  0.5,  0.5, 0.272239 },

        {  0.5,  0.0,  0.0, 0.345731 },
        {  0.5,  0.0, -0.5, 0.273122 },
        {  0.5,  0.0,  0.5, 0.418340 },
        {  0.5, -0.5,  0.0, 0.213342 },
        {  0.5, -0.5, -0.5, 0.145218 },
        {  0.5, -0.5,  0.5, 0.272239 },
        {  0.5,  0.5,  0.0, 0.478120 },
        {  0.5,  0.5, -0.5, 0.419223 },
        {  0.5,  0.5,  0.5, 0.546244 },

        // known analytical values
        {  0.0, 0.0, QL_SQRT(1/2.0), 3.0/8},

//      {  0.0,  big,  any, 0.500000 },
        {  0.0,   30, -1.0, 0.500000 },
        {  0.0,   30,  0.0, 0.500000 },
        {  0.0,   30,  1.0, 0.500000 },

//      { big,  big,   any, 1.000000 },
        {  30,   30,  -1.0, 1.000000 },
        {  30,   30,   0.0, 1.000000 },
        {  30,   30,   1.0, 1.000000 },

//      {-big,  any,   any, 0.000000 }
        { -30, -1.0,  -1.0, 0.000000 },
        { -30,  0.0,  -1.0, 0.000000 },
        { -30,  1.0,  -1.0, 0.000000 },
        { -30, -1.0,   0.0, 0.000000 },
        { -30,  0.0,   0.0, 0.000000 },
        { -30,  1.0,   0.0, 0.000000 },
        { -30, -1.0,   1.0, 0.000000 },
        { -30,  0.0,   1.0, 0.000000 },
        { -30,  1.0,   1.0, 0.000000 }
    };


    for (Size i=0; i<LENGTH(values); i++) {

        BivariateCumulativeNormalDistribution bcd(values[i].rho);
        Real value = bcd(values[i].a, values[i].b);

        if (QL_FABS(value-values[i].result)>=1e-6) {
          BOOST_FAIL("BivariateCumulativeDistribution \n"
              "case "
              + SizeFormatter::toString(i+1) + "\n"
              "    a:   "
              + DecimalFormatter::toString(values[i].a) + "\n"
              "    b:   "
              + DecimalFormatter::toString(values[i].b) + "\n"
              "    rho:           "
              + DecimalFormatter::toString(values[i].rho) +"\n"
              "    tabulated value:  "
              + DecimalFormatter::toString(values[i].result) + "\n"
              "    result:  "
              + DecimalFormatter::toString(value));
        }
    }

}


void DistributionTest::testPoisson() {

    BOOST_MESSAGE("Testing Poisson distribution...");

    for (Real mean=0.0; mean<=10.0; mean+=0.5) {
        BigNatural i = 0;
        PoissonDistribution pdf(mean);
        Real calculated = pdf(i);
        Real logHelper = -mean;
        Real expected = QL_EXP(logHelper);
        Real error = QL_FABS(calculated-expected);
        if (error > 1.0e-16)
            BOOST_FAIL("Poisson pdf("
                       + DecimalFormatter::toString(mean) + ")("
                       + IntegerFormatter::toString(i) + ")\n"
                       "    calculated: "
                       + DecimalFormatter::toString(calculated,16) + "\n"
                       "    expected:   "
                       + DecimalFormatter::toString(expected,16)+
                       "    error:   "
                       + DecimalFormatter::toExponential(error));

        for (i=1; i<25; i++) {
            calculated = pdf(i);
            if (mean == 0.0) {
                expected = 0.0;
            } else {
                logHelper = logHelper+QL_LOG(mean)-QL_LOG(Real(i));
                expected = QL_EXP(logHelper);
            }
            error = QL_FABS(calculated-expected);
            if (error>1.0e-13)
                BOOST_FAIL("Poisson pdf("
                           + DecimalFormatter::toString(mean) + ")("
                           + IntegerFormatter::toString(i) + ")\n"
                           "    calculated: "
                           + DecimalFormatter::toString(calculated,13) + "\n"
                           "    expected:   "
                           + DecimalFormatter::toString(expected,13)+
                           "    error:   "
                           + DecimalFormatter::toExponential(error));
        }
    }
}

void DistributionTest::testCumulativePoisson() {

    BOOST_MESSAGE("Testing cumulative Poisson distribution...");

    for (Real mean=0.0; mean<=10.0; mean+=0.5) {
        BigNatural i = 0;
        CumulativePoissonDistribution cdf(mean);
        Real cumCalculated = cdf(i);
        Real logHelper = -mean;
        Real cumExpected = QL_EXP(logHelper);
        Real error = QL_FABS(cumCalculated-cumExpected);
        if (error>1.0e-13)
            BOOST_FAIL("Poisson cdf("
                       + DecimalFormatter::toString(mean) + ")("
                       + IntegerFormatter::toString(i) + ")\n"
                       "    calculated: "
                       + DecimalFormatter::toString(cumCalculated,13) + "\n"
                       "    expected:   "
                       + DecimalFormatter::toString(cumExpected,13)+
                       "    error:   "
                       + DecimalFormatter::toExponential(error));
        for (i=1; i<25; i++) {
            cumCalculated = cdf(i);
            if (mean == 0.0) {
                cumExpected = 1.0;
            } else {
                logHelper = logHelper+QL_LOG(mean)-QL_LOG(Real(i));
                cumExpected += QL_EXP(logHelper);
            }
            error = QL_FABS(cumCalculated-cumExpected);
            if (error>1.0e-12)
                BOOST_FAIL("Poisson cdf("
                           + DecimalFormatter::toString(mean) + ")("
                           + IntegerFormatter::toString(i) + ")\n"
                           "    calculated: "
                           + DecimalFormatter::toString(cumCalculated,12)+"\n"
                           "    expected:   "
                           + DecimalFormatter::toString(cumExpected,12)+
                           "    error:   "
                           + DecimalFormatter::toExponential(error));
        }
    }
}

void DistributionTest::testInverseCumulativePoisson() {

    BOOST_MESSAGE("Testing inverse cumulative Poisson distribution...");

    InverseCumulativePoisson icp(1.0);

    Real data[] = { 0.2,
                    0.5,
                    0.9,
                    0.98,
                    0.99,
                    0.999,
                    0.9999,
                    0.99995,
                    0.99999,
                    0.999999,
                    0.9999999,
                    0.99999999
    };

    for (Size i=0; i<LENGTH(data); i++) {
        if (!close(icp(data[i]), i)) {
            BOOST_FAIL("failed to reproduce known value for x = "
                       + DecimalFormatter::toString(data[i],8) + "\n"
                       "    calculated: "
                       + DecimalFormatter::toString(icp(data[i]),2) + "\n"
                       "    expected:   "
                       + DecimalFormatter::toString(Real(i),1));
        }
    }
}


test_suite* DistributionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Distribution tests");
    suite->add(BOOST_TEST_CASE(&DistributionTest::testNormal));
    suite->add(BOOST_TEST_CASE(&DistributionTest::testBivariate));
    suite->add(BOOST_TEST_CASE(&DistributionTest::testPoisson));
    suite->add(BOOST_TEST_CASE(&DistributionTest::testCumulativePoisson));
    suite->add(
            BOOST_TEST_CASE(&DistributionTest::testInverseCumulativePoisson));
    return suite;
}

