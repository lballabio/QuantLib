
/*
 Copyright (C) 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
// $Id$

#include "distributions.hpp"
#include "utilities.hpp"

using namespace QuantLib;
using QuantLib::Math::NormalDistribution;
using QuantLib::Math::CumulativeNormalDistribution;
using QuantLib::Math::InvCumulativeNormalDistribution;

namespace {
    
    double average = 0.0, sigma = 1.0;
    
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

void DistributionTest::runTest() {

    NormalDistribution normal(average,sigma);
    CumulativeNormalDistribution cum(average,sigma);
    InvCumulativeNormalDistribution invCum(average,sigma);

    double xMin = average - 4*sigma,
           xMax = average + 4*sigma;
    Size N = 10001;
    double h = (xMax-xMin)/(N-1);

    std::vector<double> x(N), y(N), yd(N), temp(N), diff(N);

    int i;
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
        CPPUNIT_FAIL(
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
    if (e > 1.0e-3) {
        char s[10];
        QL_SPRINTF(s,"%5.2e",e);
        CPPUNIT_FAIL(
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
        CPPUNIT_FAIL(
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
        CPPUNIT_FAIL(
            "norm of C++ Normal.derivative minus analytic derivative: "
            + std::string(s) + "\n"
            "tolerance exceeded");
    }
}

