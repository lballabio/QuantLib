
/*
 Copyright (C) 2003 Ferdinando Ametrano
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

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    double average = 1.0, sigma = 2.0;

    double gaussian(double x) {
        double normFact = sigma*QL_SQRT(2*M_PI);
        double dx = x-average;
        return QL_EXP(-dx*dx/(2.0*sigma*sigma))/normFact;
    }

    double gaussianDerivative(double x) {
        double normFact = sigma*sigma*sigma*QL_SQRT(2*M_PI);
        double dx = x-average;
        return -dx*QL_EXP(-dx*dx/(2.0*sigma*sigma))/normFact;
    }

}

void DistributionTest::testNormal() {

    BOOST_MESSAGE("Testing normal distributions...");

    NormalDistribution normal(average,sigma);
    CumulativeNormalDistribution cum(average,sigma);
    InverseCumulativeNormal invCum(average,sigma);

    int numberOfStandardDeviation = 6;
    double xMin = average - numberOfStandardDeviation*sigma,
           xMax = average + numberOfStandardDeviation*sigma;
    Size N = 100001;
    double h = (xMax-xMin)/(N-1);

    std::vector<double> x(N), y(N), yd(N), temp(N), diff(N);

    Size i;
    for (i=0; i<N; i++)
        x[i] = xMin+h*i;
    std::transform(x.begin(),x.end(),y.begin(),std::ptr_fun(gaussian));
    std::transform(x.begin(),x.end(),yd.begin(),
                   std::ptr_fun(gaussianDerivative));

    // check that normal = Gaussian
    std::transform(x.begin(),x.end(),temp.begin(),normal);
    std::transform(y.begin(),y.end(),temp.begin(),diff.begin(),
                   std::minus<double>());
    double e = norm(diff.begin(),diff.end(),h);
    if (e > 1.0e-16) {
        char s[10];
        QL_SPRINTF(s,"%5.2e",e);
        BOOST_FAIL(
            "norm of C++ NormalDistribution minus analytic Gaussian: "
            + std::string(s) + "\n"
            "tolerance exceeded");
    }

    // check that invCum . cum = identity
    std::transform(x.begin(),x.end(),temp.begin(),cum);
    std::transform(temp.begin(),temp.end(),temp.begin(),invCum);
    std::transform(x.begin(),x.end(),temp.begin(),diff.begin(),
                   std::minus<double>());
    e = norm(diff.begin(),diff.end(),h);
    if (e > 1.0e-8) {
        char s[10];
        QL_SPRINTF(s,"%5.2e",e);
        BOOST_FAIL(
            "norm of invCum . cum minus identity: "
            + std::string(s) + "\n"
            "tolerance exceeded");
    }

    // check that cum.derivative = Gaussian
    for (i=0; i<x.size(); i++)
        temp[i] = cum.derivative(x[i]);
    std::transform(y.begin(),y.end(),temp.begin(),diff.begin(),
                   std::minus<double>());
    e = norm(diff.begin(),diff.end(),h);
    if (e > 1.0e-16) {
        char s[10];
        QL_SPRINTF(s,"%5.2e",e);
        BOOST_FAIL(
            "norm of C++ Cumulative.derivative minus analytic Gaussian: "
            + std::string(s) + "\n"
            "tolerance exceeded");
    }

    // check that normal.derivative = gaussianDerivative
    for (i=0; i<x.size(); i++)
        temp[i] = normal.derivative(x[i]);
    std::transform(yd.begin(),yd.end(),temp.begin(),diff.begin(),
                   std::minus<double>());
    e = norm(diff.begin(),diff.end(),h);
    if (e > 1.0e-16) {
        char s[10];
        QL_SPRINTF(s,"%5.2e",e);
        BOOST_FAIL(
            "norm of C++ Normal.derivative minus analytic derivative: "
            + std::string(s) + "\n"
            "tolerance exceeded");
    }
}

void DistributionTest::testBivariate() {

    BOOST_MESSAGE("Testing bivariate cumulative normal distribution...");

    struct BivariateTestData {
        double a;
        double b;
        double rho;
        double result;
    };

    /* The data below are from
       "Option pricing formulas", E.G. Haug, McGraw-Hill 1998
       pag 193
    */
    BivariateTestData values[] = {
        {  0.0,  0.0,  0.0, 0.250000 },
        {  0.0,  0.0, -0.5, 0.166667 },
        {  0.0,  0.0,  0.5, 0.333333 },
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
        {  0.5,  0.5,  0.5, 0.546244 }
    };


    for (Size i=0; i<LENGTH(values); i++) {

        BivariateCumulativeNormalDistribution bcd(values[i].rho);
        double value = bcd(values[i].a, values[i].b);

        if (QL_FABS(value-values[i].result)>=1e-6) {
          BOOST_FAIL("BivariateCumulativeDistribution \n"
              "case "
              + SizeFormatter::toString(i+1) + "\n"
              "    a:   "
              + DoubleFormatter::toString(values[i].a) + "\n"
              "    b:   "
              + DoubleFormatter::toString(values[i].b) + "\n"
              "    rho:           "
              + DoubleFormatter::toString(values[i].rho) +"\n"
              "    tabulated value:  "
              + DoubleFormatter::toString(values[i].result) + "\n"
              "    result:  "
              + DoubleFormatter::toString(value));
        }
    }

}


test_suite* DistributionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Distribution tests");
    suite->add(BOOST_TEST_CASE(&DistributionTest::testNormal));
    suite->add(BOOST_TEST_CASE(&DistributionTest::testBivariate));
    return suite;
}

