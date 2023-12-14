/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/experimental/risk/creditriskplus.hpp>
#include <ql/math/comparison.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTest, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(CreditRiskPlusTest)

BOOST_AUTO_TEST_CASE(testReferenceValues) {

    BOOST_TEST_MESSAGE(
        "Testing extended credit risk plus model against reference values...");

    static const Real tol = 1E-8; 

    /* Reference Values are taken from [1] Integrating Correlations, Risk,
       July 1999, table A, table B and figure 1 */

    std::vector<Real> sector1Exposure(1000, 1.0);
    std::vector<Real> sector1Pd(1000, 0.04);
    std::vector<Size> sector1Sector(1000, 0);

    std::vector<Real> sector2Exposure(1000, 2.0);
    std::vector<Real> sector2Pd(1000, 0.02);
    std::vector<Size> sector2Sector(1000, 1);

    std::vector<Real> exposure;
    exposure.insert(exposure.end(), sector1Exposure.begin(),
                    sector1Exposure.end());
    exposure.insert(exposure.end(), sector2Exposure.begin(),
                    sector2Exposure.end());

    std::vector<Real> pd;
    pd.insert(pd.end(), sector1Pd.begin(), sector1Pd.end());
    pd.insert(pd.end(), sector2Pd.begin(), sector2Pd.end());

    std::vector<Size> sector;
    sector.insert(sector.end(), sector1Sector.begin(), sector1Sector.end());
    sector.insert(sector.end(), sector2Sector.begin(), sector2Sector.end());

    std::vector<Real> relativeDefaultVariance;
    relativeDefaultVariance.push_back(0.75 * 0.75);
    relativeDefaultVariance.push_back(0.75 * 0.75);

    Matrix rho(2, 2);
    rho[0][0] = rho[1][1] = 1.0;
    rho[0][1] = rho[1][0] = 0.50;

    Real unit = 0.1;

    CreditRiskPlus cr(exposure, pd, sector, relativeDefaultVariance, rho, unit);

    if ( std::fabs(cr.sectorExposures()[0] - 1000.0) > tol )
        BOOST_FAIL("failed to reproduce sector 1 exposure ("
                   << cr.sectorExposures()[0] << ", should be 1000)");

    if ( std::fabs(cr.sectorExposures()[1] - 2000.0) > tol )
        BOOST_FAIL("failed to reproduce sector 2 exposure ("
                   << cr.sectorExposures()[1] << ", should be 2000)");

    if ( std::fabs(cr.sectorExpectedLoss()[0] - 40.0) > tol )
        BOOST_FAIL("failed to reproduce sector 1 expected loss ("
                   << cr.sectorExpectedLoss()[0] << ", should be 40)");

    if ( std::fabs(cr.sectorExpectedLoss()[1] - 40.0) > tol )
        BOOST_FAIL("failed to reproduce sector 2 expected loss ("
                   << cr.sectorExpectedLoss()[1] << ", should be 40)");

    if ( std::fabs(cr.sectorUnexpectedLoss()[0] - 30.7) > 0.05 )
        BOOST_FAIL("failed to reproduce sector 1 unexpected loss ("
                   << cr.sectorUnexpectedLoss()[0] << ", should be 30.7)");

    if ( std::fabs(cr.sectorUnexpectedLoss()[1] - 31.3) > 0.05 )
        BOOST_FAIL("failed to reproduce sector 2 unexpected loss ("
                   << cr.sectorUnexpectedLoss()[1] << ", should be 31.3)");

    if ( std::fabs(cr.exposure() - 3000.0) > tol )
        BOOST_FAIL("failed to reproduce overall exposure ("
                   << cr.exposure() << ", should be 3000)");

    if ( std::fabs(cr.expectedLoss() - 80.0) > tol )
        BOOST_FAIL("failed to reproduce overall expected loss ("
                   << cr.expectedLoss() << ", should be 80)");

    if ( std::fabs(cr.unexpectedLoss() - 53.1) > 0.01 )
        BOOST_FAIL("failed to reproduce overall unexpected loss ("
                   << cr.unexpectedLoss() << ", should be 53.1)");

    // the overall relative default variance in the paper seems generously rounded,
    // but since EL and UL is matching closely and the former is retrieved
    // as a simple expression in the latter, we do not suspect a problem in our
    // calculation

    if ( std::fabs(cr.relativeDefaultVariance() - 0.65 * 0.65) > 0.001 )
        BOOST_FAIL("failed to reproduce overall relative default variance ("
                   << cr.relativeDefaultVariance() << ", should be 0.4225)");

    if ( std::fabs(cr.lossQuantile(0.99) - 250) > 0.5 )
        BOOST_FAIL("failed to reproduce overall 99 percentile ("
                   << cr.lossQuantile(0.99) << ", should be 250)");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
