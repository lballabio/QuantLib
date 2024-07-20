/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Gary Kennedy
 Copyright (C) 2015, 2024 Peter Caspers
 Copyright (C) 2017 Klaus Spanderen
 Copyright (C) 2020 Marcin Rybacki

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
#include <ql/pricingengines/blackformula.hpp>
#include <cmath>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(BlackFormulaTests)

BOOST_AUTO_TEST_CASE(testBachelierImpliedVol) {

    BOOST_TEST_MESSAGE("Testing Bachelier implied vol...");

    Real forward = 1.0;
    Real bpvol = 0.01;
    Real tte = 10.0;
    Real stdDev = bpvol * std::sqrt(tte);
    Option::Type optionType = Option::Call;
    Real discount = 0.95;

    Real d[] = {-3.0, -2.0, -1.0, -0.5, 0.0, 0.5, 1.0, 2.0, 3.0};
    for (Real i : d) {


        Real strike = forward - i * bpvol * std::sqrt(tte);

        Real callPrem = bachelierBlackFormula(optionType, strike, forward, stdDev, discount);

        Real impliedBpVol =
            bachelierBlackFormulaImpliedVol(optionType, strike, forward, tte, callPrem, discount);

        if (std::fabs(bpvol - impliedBpVol) > 1.0e-12) {
            BOOST_ERROR("Failed, expected " << bpvol << " realised " << impliedBpVol);
        }

        Real impliedBpVolExact = bachelierBlackFormulaImpliedVolExact(optionType, strike, forward,
                                                                      tte, callPrem, discount);

        if (std::fabs(bpvol - impliedBpVolExact) > 1.0e-15) {
            BOOST_ERROR("Failed, expected " << bpvol << " realised " << impliedBpVolExact);
        }
    }
}

BOOST_AUTO_TEST_CASE(testChambersImpliedVol) {

    BOOST_TEST_MESSAGE("Testing Chambers-Nawalkha implied vol approximation...");

    Option::Type types[] = {Option::Call, Option::Put};
    Real displacements[] = {0.0000, 0.0010, 0.0050, 0.0100, 0.0200};
    Real forwards[] = {-0.0010, 0.0000, 0.0050, 0.0100, 0.0200, 0.0500};
    Real strikes[] = {-0.0100, -0.0050, -0.0010, 0.0000, 0.0010, 0.0050,
                      0.0100,  0.0200,  0.0500,  0.1000};
    Real stdDevs[] = {0.10, 0.15, 0.20, 0.30, 0.50, 0.60, 0.70,
                      0.80, 1.00, 1.50, 2.00};
    Real discounts[] = {1.00, 0.95, 0.80, 1.10};

    Real tol = 5.0E-4;

    for (auto& type : types) {
        for (Real& displacement : displacements) {
            for (Real& forward : forwards) {
                for (Real& strike : strikes) {
                    for (Real& stdDev : stdDevs) {
                        for (Real& discount : discounts) {
                            if (forward + displacement > 0.0 && strike + displacement > 0.0) {
                                Real premium = blackFormula(type, strike, forward, stdDev, discount,
                                                            displacement);
                                Real atmPremium = blackFormula(type, forward, forward, stdDev,
                                                               discount, displacement);
                                Real iStdDev = blackFormulaImpliedStdDevChambers(
                                    type, strike, forward, premium, atmPremium, discount,
                                    displacement);
                                Real moneyness = (strike + displacement) / (forward + displacement);
                                if(moneyness > 1.0) moneyness = 1.0 / moneyness;
                                Real error = (iStdDev - stdDev) / stdDev * moneyness;
                                if(error > tol)
                                    BOOST_ERROR("Failed to verify Chambers-Nawalkha "
                                                "approximation for "
                                                << type << " displacement=" << displacement
                                                << " forward=" << forward << " strike=" << strike
                                                << " discount=" << discount << " stddev=" << stdDev
                                                << " result=" << iStdDev
                                                << " exceeds maximum error tolerance");
                            }
                        }
                    }
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testRadoicicStefanicaImpliedVol) {

    BOOST_TEST_MESSAGE(
        "Testing Radoicic-Stefanica implied vol approximation...");

    const Time T = 1.7;
    const Rate r = 0.1;
    const DiscountFactor df = std::exp(-r*T);

    const Real forward = 100;

    const Volatility vol = 0.3;
    const Real stdDev = vol * std::sqrt(T);

    const Option::Type types[] = { Option::Call, Option::Put };
    const Real strikes[] = {
        50, 60, 70, 80, 90, 100, 110, 125, 150, 200, 300 };

    const Real tol = 0.02;

    for (Real strike : strikes) {
        for (auto type : types) {
            const ext::shared_ptr<PlainVanillaPayoff> payoff(
                ext::make_shared<PlainVanillaPayoff>(type, strike));

            const Real marketValue = blackFormula(payoff, forward, stdDev, df);

            const Real estVol = blackFormulaImpliedStdDevApproximationRS(
                payoff, forward, marketValue, df) / std::sqrt(T);

            const Real error = std::fabs(estVol - vol);
            if (error > tol) {
                BOOST_ERROR("Failed to verify Radoicic-Stefanica"
                    "approximation for "
                    << type
                    << "\n forward     :" << forward
                    << "\n strike      :" << strike
                    << "\n discount    :" << df
                    << "\n implied vol :" << vol
                    << "\n result      :" << estVol
                    << "\n error       :" << error
                    << "\n tolerance   :" << tol);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testRadoicicStefanicaLowerBound) {

    BOOST_TEST_MESSAGE("Testing Radoicic-Stefanica lower bound...");

    // testing lower bound plot figure 3.1 from
    // "Tighter Bounds for Implied Volatility",
    // J. Gatheral, I. Matic, R. Radoicic, D. Stefanica
    // https://papers.ssrn.com/sol3/papers.cfm?abstract_id=2922742

    const Real forward = 1.0;
    const Real k = 1.2;

    for (Real s=0.17; s < 2.9; s+=0.01) {
        const Real strike = std::exp(k)*forward;
        const Real c = blackFormula(Option::Call, strike, forward, s);
        const Real estimate = blackFormulaImpliedStdDevApproximationRS(
            Option::Call, strike, forward, c);

        const Real error = s - estimate;
        if (std::isnan(estimate) || std::fabs(error) > 0.05) {
            BOOST_ERROR("Failed to lower bound Radoicic-Stefanica"
                "approximation for "
                << "\n forward     :" << forward
                << "\n strike      :" << k
                << "\n stdDev      :" << s
                << "\n result      :" << estimate
                << "\n error       :" << error);

        }

        if (c > 1e-6 && error < 0.0) {
            BOOST_ERROR("Failed to verify Radoicic-Stefanica is lower bound"
                    << "\n forward     :" << forward
                    << "\n strike      :" << k
                    << "\n stdDev      :" << s
                    << "\n result      :" << estimate
                    << "\n error       :" << error);
        }
    }
}

BOOST_AUTO_TEST_CASE(testImpliedVolAdaptiveSuccessiveOverRelaxation) {
    BOOST_TEST_MESSAGE("Testing implied volatility calculation via "
        "adaptive successive over-relaxation...");

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(12, July, 2017);
    Settings::instance().evaluationDate() = today;

    const Date exerciseDate = today + Period(15, Months);
    const Time exerciseTime = dc.yearFraction(today, exerciseDate);

    const ext::shared_ptr<YieldTermStructure> rTS = flatRate(0.10, dc);
    const ext::shared_ptr<YieldTermStructure> qTS = flatRate(0.06, dc);

    const DiscountFactor df = rTS->discount(exerciseDate);

    const Volatility vol = 0.20;
    const Real stdDev = vol * std::sqrt(exerciseTime);

    const Real s0     = 100;
    const Real forward= s0 * qTS->discount(exerciseDate)/df;

    const Option::Type types[] = { Option::Call, Option::Put };
    const Real strikes[] = { 50, 60, 70, 80, 90, 100, 110, 125, 150, 200 };
    const Real displacements[] = { 0, 25, 50, 100};

    const Real tol = 1e-8;

    for (Real strike : strikes) {
        for (auto type : types) {
            const ext::shared_ptr<PlainVanillaPayoff> payoff(
                ext::make_shared<PlainVanillaPayoff>(type, strike));

            for (Real displacement : displacements) {

                const Real marketValue = blackFormula(payoff, forward, stdDev, df, displacement);

                const Real impliedStdDev = blackFormulaImpliedStdDevLiRS(
                    payoff, forward, marketValue, df, displacement,
                    Null<Real>(), 1.0, tol, 100);

                const Real error = std::fabs(impliedStdDev - stdDev);
                if (error > 10*tol) {
                    BOOST_ERROR("Failed to calculated implied volatility"
                                " with adaptive successive over-relaxation"
                            << "\n forward     :" << forward
                            << "\n strike      :" << strike
                            << "\n stdDev      :" << stdDev
                            << "\n displacement:" << displacement
                            << "\n result      :" << impliedStdDev
                            << "\n error       :" << error
                            << "\n tolerance   :" << tol);
                }
            }
        }
    }
}

void assertBlackFormulaForwardDerivative(
    Option::Type optionType,
    const std::vector<Real> &strikes,
    Real bpvol)
{
    Real forward = 1.0;
    Real tte = 10.0;
    Real stdDev = bpvol * std::sqrt(tte);
    Real discount = 0.95;
    Real displacement = 0.01;
    Real bump = 0.0001;
    Real epsilon = 1.e-10;
    std::string type = optionType == Option::Call ? "Call" : "Put";

    for (Real strike : strikes) {
        Real delta = blackFormulaForwardDerivative(optionType, strike, forward, stdDev, discount,
                                                   displacement);
        Real bumpedDelta = blackFormulaForwardDerivative(
            optionType, strike, forward + bump, stdDev, discount, displacement);

        Real basePremium = blackFormula(
            optionType, strike, forward, stdDev, discount, displacement);
        Real bumpedPremium = blackFormula(
            optionType, strike, forward + bump, stdDev, discount, displacement);
        Real deltaApprox = (bumpedPremium - basePremium) / bump;

        /*! Based on the Mean Value Theorem, the below inequality
            should hold for any function that is monotonic in the
            area of the bump.
         */
        bool success =
            (std::max(delta, bumpedDelta) + epsilon > deltaApprox)
            &&  (deltaApprox > std::min(delta, bumpedDelta) - epsilon);

        if (!success)
        {
            BOOST_ERROR("Failed to calculate the derivative of the"
                        " Black formula w.r.t. forward"
                        << "\n option type       :" << type
                        << "\n forward           :" << forward
                        << "\n strike            :" << strike
                        << "\n stdDev            :" << stdDev
                        << "\n displacement      :" << displacement
                        << "\n analytical delta  :" << delta
                        << "\n approximated delta:" << deltaApprox);
        }
    }
}

BOOST_AUTO_TEST_CASE(testBlackFormulaForwardDerivative) {

    BOOST_TEST_MESSAGE("Testing forward derivative of the Black formula...");

    std::vector<Real> strikes;
    strikes.push_back(0.1);
    strikes.push_back(0.5);
    strikes.push_back(1.0);
    strikes.push_back(2.0);
    strikes.push_back(3.0);
    const Real vol = 0.1;
    assertBlackFormulaForwardDerivative(Option::Call, strikes, vol);
    assertBlackFormulaForwardDerivative(Option::Put, strikes, vol);
}

BOOST_AUTO_TEST_CASE(testBlackFormulaForwardDerivativeWithZeroStrike) {

    BOOST_TEST_MESSAGE("Testing forward derivative of the Black formula "
        "with zero strike...");

    std::vector<Real> strikes;
    strikes.push_back(0.0);
    const Real vol = 0.1;
    assertBlackFormulaForwardDerivative(Option::Call, strikes, vol);
    assertBlackFormulaForwardDerivative(Option::Put, strikes, vol);
}

BOOST_AUTO_TEST_CASE(testBlackFormulaForwardDerivativeWithZeroVolatility) {

    BOOST_TEST_MESSAGE("Testing forward derivative of the Black formula "
        "with zero volatility...");

    std::vector<Real> strikes;
    strikes.push_back(0.1);
    strikes.push_back(0.5);
    strikes.push_back(1.0);
    strikes.push_back(2.0);
    strikes.push_back(3.0);
    const Real vol = 0.0;
    assertBlackFormulaForwardDerivative(Option::Call, strikes, vol);
    assertBlackFormulaForwardDerivative(Option::Put, strikes, vol);
}

void assertBachelierBlackFormulaForwardDerivative(
    Option::Type optionType,
    const std::vector<Real> &strikes,
    Real bpvol)
{
    Real forward = 1.0;
    Real tte = 10.0;
    Real stdDev = bpvol * std::sqrt(tte);
    Real discount = 0.95;
    Real bump = 0.0001;
    Real epsilon = 1.e-10;
    std::string type = optionType == Option::Call ? "Call" : "Put";

    for (Real strike : strikes) {
        Real delta =
            bachelierBlackFormulaForwardDerivative(optionType, strike, forward, stdDev, discount);
        Real bumpedDelta = bachelierBlackFormulaForwardDerivative(
            optionType, strike, forward + bump, stdDev, discount);

        Real basePremium = bachelierBlackFormula(
            optionType, strike, forward, stdDev, discount);
        Real bumpedPremium = bachelierBlackFormula(
            optionType, strike, forward + bump, stdDev, discount);
        Real deltaApprox = (bumpedPremium - basePremium) / bump;

        /*! Based on the Mean Value Theorem, the below inequality
            should hold for any function that is monotonic in the
            area of the bump.
         */
        bool success =
            (std::max(delta, bumpedDelta) + epsilon > deltaApprox)
            &&  (deltaApprox > std::min(delta, bumpedDelta) - epsilon);

        if (!success)
        {
            BOOST_ERROR("Failed to calculate the derivative of the"
                        " Bachelier Black formula w.r.t. forward"
                        << "\n option type       :" << type
                        << "\n forward           :" << forward
                        << "\n strike            :" << strike
                        << "\n stdDev            :" << stdDev
                        << "\n analytical delta  :" << delta
                        << "\n approximated delta:" << deltaApprox);
        }
    }
}

BOOST_AUTO_TEST_CASE(testBachelierBlackFormulaForwardDerivative) {

    BOOST_TEST_MESSAGE("Testing forward derivative of the "
        "Bachelier Black formula...");

    std::vector<Real> strikes;
    strikes.push_back(-3.0);
    strikes.push_back(-2.0);
    strikes.push_back(-1.0);
    strikes.push_back(-0.5);
    strikes.push_back(0.0);
    strikes.push_back(0.5);
    strikes.push_back(1.0);
    strikes.push_back(2.0);
    strikes.push_back(3.0);
    const Real vol = 0.001;
    assertBachelierBlackFormulaForwardDerivative(Option::Call, strikes, vol);
    assertBachelierBlackFormulaForwardDerivative(Option::Put, strikes, vol);
}

BOOST_AUTO_TEST_CASE(testBachelierBlackFormulaForwardDerivativeWithZeroVolatility) {

    BOOST_TEST_MESSAGE("Testing forward derivative of the Bachelier Black formula "
        "with zero volatility...");

    std::vector<Real> strikes;
    strikes.push_back(-3.0);
    strikes.push_back(-2.0);
    strikes.push_back(-1.0);
    strikes.push_back(-0.5);
    strikes.push_back(0.0);
    strikes.push_back(0.5);
    strikes.push_back(1.0);
    strikes.push_back(2.0);
    strikes.push_back(3.0);
    const Real vol = 0.0;
    assertBachelierBlackFormulaForwardDerivative(Option::Call, strikes, vol);
    assertBachelierBlackFormulaForwardDerivative(Option::Put, strikes, vol);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
