/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2003 StatPro Italia srl
 Copyright (C) 2005 Gary Kennedy
 Copyright (C) 2013 Fabien Le Floc'h

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

#include "distributions.hpp"
#include "utilities.hpp"
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/distributions/bivariatenormaldistribution.hpp>
#include <ql/math/distributions/poissondistribution.hpp>
#include <ql/math/comparison.hpp>
#include <ql/math/functional.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    Real average = 1.0, sigma = 2.0;

    Real gaussian(Real x) {
        Real normFact = sigma*std::sqrt(2*M_PI);
        Real dx = x-average;
        return std::exp(-dx*dx/(2.0*sigma*sigma))/normFact;
    }

    Real gaussianDerivative(Real x) {
        Real normFact = sigma*sigma*sigma*std::sqrt(2*M_PI);
        Real dx = x-average;
        return -dx*std::exp(-dx*dx/(2.0*sigma*sigma))/normFact;
    }

    struct BivariateTestData {
        Real a;
        Real b;
        Real rho;
        Real result;
    };

    template <class Bivariate>
    void checkBivariate(const char* tag) {

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
            {  0.0, 0.0, std::sqrt(1/2.0), 3.0/8},

            // {  0.0,  big,  any, 0.500000 },
            {  0.0,   30, -1.0, 0.500000 },
            {  0.0,   30,  0.0, 0.500000 },
            {  0.0,   30,  1.0, 0.500000 },

            // { big,  big,   any, 1.000000 },
            {  30,   30,  -1.0, 1.000000 },
            {  30,   30,   0.0, 1.000000 },
            {  30,   30,   1.0, 1.000000 },

            // {-big,  any,   any, 0.000000 }
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
            Bivariate bcd(values[i].rho);
            Real value = bcd(values[i].a, values[i].b);

            Real tolerance = 1.0e-6;
            if (std::fabs(value-values[i].result) >= tolerance) {
                BOOST_ERROR(tag << " bivariate cumulative distribution\n"
                            << "    case: " << i+1 << "\n"
                            << QL_FIXED
                            << "    a:    " << values[i].a << "\n"
                            << "    b:    " << values[i].b << "\n"
                            << "    rho:  " << values[i].rho <<"\n"
                            << QL_SCIENTIFIC
                            << "    tabulated value:  "
                            << values[i].result << "\n"
                            << "    result:           " << value);
            }
        }
    }

    template <class Bivariate>
    void checkBivariateAtZero(const char* tag, Real tolerance) {

        /*
          BVN(0.0,0.0,rho) = 1/4 + arcsin(rho)/(2*M_PI)
          "Handbook of the Normal Distribution",
          J.K. Patel & C.B.Read, 2nd Ed, 1996
        */
        const Real rho[] = { 0.0, 0.1, 0.2, 0.3, 0.4, 0.5,
                             0.6, 0.7, 0.8, 0.9, 0.99999 };
        const Real x(0.0);
        const Real y(0.0);

        for (Size i=0;i<LENGTH(rho);i++) {
            for (Integer sgn=-1; sgn < 2; sgn+=2) {
                Bivariate bvn(sgn*rho[i]);
                Real expected = 0.25 + std::asin(sgn*rho[i]) / (2*M_PI) ;
                Real realised = bvn(x,y);

                if (std::fabs(realised-expected)>=tolerance) {
                    BOOST_ERROR(tag << " bivariate cumulative distribution\n"
                                << QL_SCIENTIFIC
                                << "    rho: " << sgn*rho[i] << "\n"
                                << "    expected:  " << expected << "\n"
                                << "    realised:  " << realised << "\n"
                                << "    tolerance: " << tolerance);
                }
            }
        }
    }

    template <class Bivariate>
    void checkBivariateTail(const char* tag, Real tolerance) {

        /* make sure numerical greeks are sensible, numerical error in
         * the tails can make garbage greeks for partial time barrier
         * option */
        Real x = -6.9;
        Real y = 6.9;
        Real corr = -0.999;
        Bivariate bvn(corr);
        for (int i = 0; i<10;i++) {
            Real cdf0 = bvn(x,y);
            y = y + tolerance;
            double cdf1 = bvn(x,y);
            if (cdf0 > cdf1) {
                BOOST_ERROR(tag << " cdf must be decreasing in the tails\n"
                            << QL_SCIENTIFIC
                            << "    cdf0: " << cdf0 << "\n"
                            << "    cdf1: " << cdf1 << "\n"
                            << "    x: " << x << "\n"
                            << "    y: " << y << "\n"
                            << "    rho: " << corr);
            }
        }
    }

}

void DistributionTest::testNormal() {

    BOOST_TEST_MESSAGE("Testing normal distributions...");

    InverseCumulativeNormal invCumStandardNormal;
    Real check = invCumStandardNormal(0.5);
    if (check != 0.0e0) {
        BOOST_ERROR("C++ inverse cumulative of the standard normal at 0.5 is "
                    << QL_SCIENTIFIC << check
                    << "\n instead of zero: something is wrong!");
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
        BOOST_ERROR("norm of C++ NormalDistribution minus analytic Gaussian: "
                    << QL_SCIENTIFIC << e << "\n"
                    << "tolerance exceeded");
    }

    // check that invCum . cum = identity
    std::transform(x.begin(),x.end(),temp.begin(),cum);
    std::transform(temp.begin(),temp.end(),temp.begin(),invCum);
    std::transform(x.begin(),x.end(),temp.begin(),diff.begin(),
                   std::minus<Real>());
    e = norm(diff.begin(),diff.end(),h);
    if (e > 1.0e-7) {
        BOOST_ERROR("norm of invCum . cum minus identity: "
                    << QL_SCIENTIFIC << e << "\n"
                    << "tolerance exceeded");
    }

    MaddockInverseCumulativeNormal mInvCum(average, sigma);
    std::transform(x.begin(),x.end(), x.begin(), diff.begin(),
    			   compose3(std::minus<Real>(),
    				  identity<Real>(), compose(mInvCum, cum)));

    e = norm(diff.begin(), diff.end(), h);
    if (e > 1.0e-7) {
        BOOST_ERROR("norm of MaddokInvCum . cum minus identity: "
                    << QL_SCIENTIFIC << e << "\n"
                    << "tolerance exceeded");
    }

    // check that cum.derivative = Gaussian
    for (i=0; i<x.size(); i++)
        temp[i] = cum.derivative(x[i]);
    std::transform(y.begin(),y.end(),temp.begin(),diff.begin(),
                   std::minus<Real>());
    e = norm(diff.begin(),diff.end(),h);
    if (e > 1.0e-16) {
        BOOST_ERROR(
            "norm of C++ Cumulative.derivative minus analytic Gaussian: "
            << QL_SCIENTIFIC << e << "\n"
            << "tolerance exceeded");
    }

    // check that normal.derivative = gaussianDerivative
    for (i=0; i<x.size(); i++)
        temp[i] = normal.derivative(x[i]);
    std::transform(yd.begin(),yd.end(),temp.begin(),diff.begin(),
                   std::minus<Real>());
    e = norm(diff.begin(),diff.end(),h);
    if (e > 1.0e-16) {
        BOOST_ERROR("norm of C++ Normal.derivative minus analytic derivative: "
                    << QL_SCIENTIFIC << e << "\n"
                    << "tolerance exceeded");
    }
}

void DistributionTest::testBivariate() {

    BOOST_TEST_MESSAGE("Testing bivariate cumulative normal distribution...");

    checkBivariateAtZero<BivariateCumulativeNormalDistributionDr78>(
                                                      "Drezner 1978", 1.0e-6);
    checkBivariate<BivariateCumulativeNormalDistributionDr78>("Drezner 1978");

    // due to relative low accuracy of Dr78, it does not pass with a
    // smaller perturbation
    checkBivariateTail<BivariateCumulativeNormalDistributionDr78>(
                                                        "Drezner 1978", 1.0e-5);

    checkBivariateAtZero<BivariateCumulativeNormalDistributionWe04DP>(
                                                        "West 2004", 1.0e-15);
    checkBivariate<BivariateCumulativeNormalDistributionWe04DP>("West 2004");

    checkBivariateTail<BivariateCumulativeNormalDistributionWe04DP>(
                                                        "West 2004", 1.0e-6);
    checkBivariateTail<BivariateCumulativeNormalDistributionWe04DP>(
                                                        "West 2004", 1.0e-8);
}


void DistributionTest::testPoisson() {

    BOOST_TEST_MESSAGE("Testing Poisson distribution...");

    for (Real mean=0.0; mean<=10.0; mean+=0.5) {
        BigNatural i = 0;
        PoissonDistribution pdf(mean);
        Real calculated = pdf(i);
        Real logHelper = -mean;
        Real expected = std::exp(logHelper);
        Real error = std::fabs(calculated-expected);
        if (error > 1.0e-16)
            BOOST_ERROR("Poisson pdf(" << mean << ")(" << i << ")\n"
                        << std::setprecision(16)
                        << "    calculated: " << calculated << "\n"
                        << "    expected:   " << expected << "\n"
                        << "    error:      " << error);

        for (i=1; i<25; i++) {
            calculated = pdf(i);
            if (mean == 0.0) {
                expected = 0.0;
            } else {
                logHelper = logHelper+std::log(mean)-std::log(Real(i));
                expected = std::exp(logHelper);
            }
            error = std::fabs(calculated-expected);
            if (error>1.0e-13)
                BOOST_ERROR("Poisson pdf(" << mean << ")(" << i << ")\n"
                            << std::setprecision(13)
                            << "    calculated: " << calculated << "\n"
                            << "    expected:   " << expected << "\n"
                            << "    error:      " << error);
        }
    }
}

void DistributionTest::testCumulativePoisson() {

    BOOST_TEST_MESSAGE("Testing cumulative Poisson distribution...");

    for (Real mean=0.0; mean<=10.0; mean+=0.5) {
        BigNatural i = 0;
        CumulativePoissonDistribution cdf(mean);
        Real cumCalculated = cdf(i);
        Real logHelper = -mean;
        Real cumExpected = std::exp(logHelper);
        Real error = std::fabs(cumCalculated-cumExpected);
        if (error>1.0e-13)
            BOOST_ERROR("Poisson cdf(" << mean << ")(" << i << ")\n"
                        << std::setprecision(13)
                        << "    calculated: " << cumCalculated << "\n"
                        << "    expected:   " << cumExpected << "\n"
                        << "    error:      " << error);
        for (i=1; i<25; i++) {
            cumCalculated = cdf(i);
            if (mean == 0.0) {
                cumExpected = 1.0;
            } else {
                logHelper = logHelper+std::log(mean)-std::log(Real(i));
                cumExpected += std::exp(logHelper);
            }
            error = std::fabs(cumCalculated-cumExpected);
            if (error>1.0e-12)
                BOOST_ERROR("Poisson cdf(" << mean << ")(" << i << ")\n"
                            << std::setprecision(12)
                            << "    calculated: " << cumCalculated << "\n"
                            << "    expected:   " << cumExpected << "\n"
                            << "    error:      " << error);
        }
    }
}

void DistributionTest::testInverseCumulativePoisson() {

    BOOST_TEST_MESSAGE("Testing inverse cumulative Poisson distribution...");

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
        if (!close(icp(data[i]), static_cast<Real>(i))) {
            BOOST_ERROR(std::setprecision(8)
                        << "failed to reproduce known value for x = "
                        << data[i] << "\n"
                        << "    calculated: " << icp(data[i]) << "\n"
                        << "    expected:   " << Real(i));
        }
    }
}


test_suite* DistributionTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Distribution tests");
    suite->add(QUANTLIB_TEST_CASE(&DistributionTest::testNormal));
    suite->add(QUANTLIB_TEST_CASE(&DistributionTest::testBivariate));
    suite->add(QUANTLIB_TEST_CASE(&DistributionTest::testPoisson));
    suite->add(QUANTLIB_TEST_CASE(&DistributionTest::testCumulativePoisson));
    suite->add(QUANTLIB_TEST_CASE(
                            &DistributionTest::testInverseCumulativePoisson));
    return suite;
}

