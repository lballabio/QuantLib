/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Gary Kennedy
 Copyright (C) 2015 Peter Caspers
 Copyright (C) 2017 Klaus Spanderen

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


#include "blackformula.hpp"
#include "utilities.hpp"
#include <ql/pricingengines/blackformula.hpp>

#include <boost/math/special_functions/fpclassify.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;


void BlackFormulaTest::testBachelierImpliedVol(){


    BOOST_TEST_MESSAGE("Testing Bachelier implied vol...");

    Real forward = 1.0;
    Real bpvol = 0.01;
    Real tte = 10.0;
    Real stdDev = bpvol*std::sqrt(tte);
    Option::Type optionType = Option::Call;
    Real discount = 0.95;

    Real d[] = {-3.0, -2.0, -1.0, -0.5, 0.0, 0.5, 1.0, 2.0, 3.0};
    for(Size i=0;i<LENGTH(d);++i){


        Real strike = forward - d[i] * bpvol * std::sqrt(tte);

        Real callPrem = bachelierBlackFormula(optionType, strike, forward, stdDev, discount);

        Real impliedBpVol = bachelierBlackFormulaImpliedVol(optionType,strike, forward, tte, callPrem, discount);

        if (std::fabs(bpvol-impliedBpVol)>1.0e-12){
            BOOST_ERROR("Failed, expected " << bpvol << " realised " << impliedBpVol );
        }
    }
    return;
}

void BlackFormulaTest::testChambersImpliedVol() {

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

    for (Size i1 = 0; i1 < LENGTH(types); ++i1) {
        for (Size i2 = 0; i2 < LENGTH(displacements); ++i2) {
            for (Size i3 = 0; i3 < LENGTH(forwards); ++i3) {
                for (Size i4 = 0; i4 < LENGTH(strikes); ++i4) {
                    for (Size i5 = 0; i5 < LENGTH(stdDevs); ++i5) {
                        for (Size i6 = 0; i6 < LENGTH(discounts); ++i6) {
                            if (forwards[i3] + displacements[i2] > 0.0 &&
                                strikes[i4] + displacements[i2] > 0.0) {
                                Real premium = blackFormula(
                                    types[i1], strikes[i4], forwards[i3],
                                    stdDevs[i5], discounts[i6],
                                    displacements[i2]);
                                Real atmPremium = blackFormula(
                                    types[i1], forwards[i3], forwards[i3],
                                    stdDevs[i5], discounts[i6],
                                    displacements[i2]);
                                Real iStdDev =
                                    blackFormulaImpliedStdDevChambers(
                                        types[i1], strikes[i4], forwards[i3],
                                        premium, atmPremium, discounts[i6],
                                        displacements[i2]);
                                Real moneyness = (strikes[i4] + displacements[i2]) /
                                             (forwards[i3] + displacements[i2]);
                                if(moneyness > 1.0) moneyness = 1.0 / moneyness;
                                Real error = (iStdDev - stdDevs[i5]) / stdDevs[i5] * moneyness;
                                if(error > tol)
                                    BOOST_ERROR(
                                        "Failed to verify Chambers-Nawalkha "
                                        "approximation for "
                                        << types[i1]
                                        << " displacement=" << displacements[i2]
                                        << " forward=" << forwards[i3]
                                        << " strike=" << strikes[i4]
                                        << " discount=" << discounts[i6]
                                        << " stddev=" << stdDevs[i5]
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

void BlackFormulaTest::testRadoicicStefanicaImpliedVol() {

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

    for (Size i=0; i < LENGTH(strikes); ++i) {
        const Real strike = strikes[i];
        for (Size j=0; j < LENGTH(types); ++j) {
            const Option::Type type = types[j];

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

void BlackFormulaTest::testRadoicicStefanicaLowerBound() {

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
        if (boost::math::isnan(estimate) || std::fabs(error) > 0.05) {
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

void BlackFormulaTest::testImpliedVolAdaptiveSuccessiveOverRelaxation() {
    BOOST_TEST_MESSAGE("Testing implied volatility calculation via "
        "adaptive successive over-relaxation...");

    SavedSettings backup;

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

    for (Size i=0; i < LENGTH(strikes); ++i) {
        const Real strike = strikes[i];

        for (Size j=0; j < LENGTH(types); ++j) {
            const Option::Type type = types[j];

            const ext::shared_ptr<PlainVanillaPayoff> payoff(
                ext::make_shared<PlainVanillaPayoff>(type, strike));

            for (Size k=0; k < LENGTH(displacements); ++k) {

                const Real displacement = displacements[k];
                const Real marketValue = blackFormula(
                    payoff, forward, stdDev, df, displacement);

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


test_suite* BlackFormulaTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Black formula tests");

    suite->add(QUANTLIB_TEST_CASE(
        &BlackFormulaTest::testBachelierImpliedVol));
    suite->add(QUANTLIB_TEST_CASE(
        &BlackFormulaTest::testChambersImpliedVol));
    suite->add(QUANTLIB_TEST_CASE(
        &BlackFormulaTest::testRadoicicStefanicaImpliedVol));
    suite->add(QUANTLIB_TEST_CASE(
        &BlackFormulaTest::testRadoicicStefanicaLowerBound));
    suite->add(QUANTLIB_TEST_CASE(
        &BlackFormulaTest::testImpliedVolAdaptiveSuccessiveOverRelaxation));

    return suite;
}
