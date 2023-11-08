/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Peter Caspers

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
#include <ql/termstructures/volatility/sabrsmilesection.hpp>
#include <ql/experimental/volatility/noarbsabrsmilesection.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {
void checkD0(const Real sigmaI, const Real beta, const Real rho, const Real nu,
             const Real tau, const unsigned int absorptions) {

    Real forward = 0.03; // does not matter in the end
    Real alpha = sigmaI / std::pow(forward, beta - 1.0);

    QuantLib::detail::D0Interpolator d(forward, tau, alpha, beta, nu, rho);

    if (std::fabs(d() * QuantLib::detail::NoArbSabrModel::nsim - (Real)absorptions) > 0.1)
        BOOST_ERROR("failed to reproduce number of absorptions at sigmaI="
                    << sigmaI << ", beta=" << beta << ", rho=" << rho << ", nu="
                    << nu << " tau=" << tau << ": D0Interpolator says "
                    << d() * QuantLib::detail::NoArbSabrModel::nsim
                    << " while the reference value is " << absorptions);
}
}

BOOST_FIXTURE_TEST_SUITE(QuantLibTest, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(NoArbSabrExperimentalTest)

BOOST_AUTO_TEST_CASE(testAbsorptionMatrix) {

    BOOST_TEST_MESSAGE("Testing no-arbitrage Sabr absorption matrix...");

    // check some points explicitly against the external file's contents

    // sigmaI, beta, rho, nu, tau, absorptions
    checkD0(1,0.01,0.75,0.1,0.25,60342); // upper left corner
    checkD0(0.8,0.01,0.75,0.1,0.25,12148);
    checkD0(0.05,0.01,0.75,0.1,0.25,0);
    checkD0(1,0.01,0.75,0.1,10.0,1890509);
    checkD0(0.8,0.01,0.75,0.1,10.0,1740233);
    checkD0(0.05,0.01,0.75,0.1,10.0,0);
    checkD0(1,0.01,0.75,0.1,30.0,2174176);
    checkD0(0.8,0.01,0.75,0.1,30.0,2090672);
    checkD0(0.05,0.01,0.75,0.1,30.0,31);
    checkD0(0.35,0.10,-0.75,0.1,0.25,0);
    checkD0(0.35,0.10,-0.75,0.1,14.75,1087841);
    checkD0(0.35,0.10,-0.75,0.1,30.0,1406569);
    checkD0(0.24,0.90,0.50,0.8,1.25,27);
    checkD0(0.24,0.90,0.50,0.8,25.75,167541);
    checkD0(0.05,0.90,-0.75,0.8,2.0,17);
    checkD0(0.05,0.90,-0.75,0.8,30.0,42100); // lower right corner

}

BOOST_AUTO_TEST_CASE(testConsistencyWithHagan) {

    BOOST_TEST_MESSAGE("Testing consistency of noarb-sabr with Hagan et al (2002)");

    // parameters taken from Doust's paper, figure 3

    Real tau = 1.0;
    Real beta = 0.5;
    Real alpha = 0.026;
    Real rho = -0.1;
    Real nu = 0.4;
    Real f = 0.0488;

    SabrSmileSection sabr(tau, f, {alpha, beta, nu, rho});
    NoArbSabrSmileSection noarbsabr(tau, f, {alpha, beta, nu, rho});

    Real absProb=noarbsabr.model()->absorptionProbability();
    if( absProb > 1E-10 || absProb < 0.0 )
        BOOST_ERROR("absorption probability should be close to zero, but is " << absProb);

    Real strike = 0.0001;
    while (strike < 0.15) {
        // test vanilla prices
        Real sabrPrice = sabr.optionPrice(strike);
        Real noarbsabrPrice = noarbsabr.optionPrice(strike);
        if (std::fabs(sabrPrice - noarbsabrPrice) > 1e-5)
            BOOST_ERROR("incosistent Hagan price ("
                        << sabrPrice << ") and noarb-sabr price ("
                        << noarbsabrPrice << ") at strike " << strike);
        // test digitals
        Real sabrDigital = sabr.digitalOptionPrice(strike);
        Real noarbsabrDigital = noarbsabr.digitalOptionPrice(strike);
        if (std::fabs(sabrDigital - noarbsabrDigital) > 1e-3)
            BOOST_ERROR("incosistent Hagan digital ("
                        << sabrDigital << ") and noarb-sabr digital ("
                        << noarbsabrDigital << ") at strike " << strike);
        // test density
        Real sabrDensity = sabr.density(strike);
        Real noarbsabrDensity = noarbsabr.density(strike);
        if (std::fabs(sabrDensity - noarbsabrDensity) > 1e-0)
            BOOST_ERROR("incosistent Hagan density ("
                        << sabrDensity << ") and noarb-sabr density ("
                        << noarbsabrDensity << ") at strike " << strike);
        strike += 0.0001;
    }

}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()