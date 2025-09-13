/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024

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
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/math/comparison.hpp>
#include <cmath>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(BlackCalculatorTests)

struct BlackCalculatorTestData {
    Option::Type type;
    Real strike;
    Real forward;
    Real stdDev;
    Real discount;
    Real tolerance;
};

BOOST_AUTO_TEST_CASE(testBlackCalculatorBasicValues) {
    BOOST_TEST_MESSAGE("Testing BlackCalculator basic option values...");

    BlackCalculatorTestData values[] = {
        // type, strike, forward, stdDev, discount, tolerance
        { Option::Call, 100.0, 100.0, 0.20, 1.0, 1e-8 },  // ATM Call
        { Option::Put,  100.0, 100.0, 0.20, 1.0, 1e-8 },  // ATM Put
        { Option::Call, 90.0,  100.0, 0.20, 1.0, 1e-8 },  // ITM Call
        { Option::Put,  110.0, 100.0, 0.20, 1.0, 1e-8 },  // ITM Put
        { Option::Call, 110.0, 100.0, 0.20, 1.0, 1e-8 },  // OTM Call
        { Option::Put,  90.0,  100.0, 0.20, 1.0, 1e-8 },  // OTM Put
        { Option::Call, 100.0, 100.0, 0.0,  1.0, 1e-8 },  // Zero vol Call
        { Option::Put,  100.0, 100.0, 0.0,  1.0, 1e-8 },  // Zero vol Put
    };

    for (auto& data : values) {
        // Test constructor with Option::Type
        BlackCalculator calc1(data.type, data.strike, data.forward, data.stdDev, data.discount);
        Real value1 = calc1.value();
        
        // Test constructor with Payoff
        ext::shared_ptr<StrikedTypePayoff> payoff(
            new PlainVanillaPayoff(data.type, data.strike));
        BlackCalculator calc2(payoff, data.forward, data.stdDev, data.discount);
        Real value2 = calc2.value();

        // Both constructors should give the same result
        Real error = std::fabs(value1 - value2);
        if (error > data.tolerance) {
            BOOST_ERROR("BlackCalculator constructor mismatch for "
                       << (data.type == Option::Call ? "Call" : "Put")
                       << " strike=" << data.strike << " forward=" << data.forward
                       << " stdDev=" << data.stdDev << " discount=" << data.discount
                       << " value1=" << value1 << " value2=" << value2
                       << " error=" << error);
        }

        // Basic sanity checks
        if (data.stdDev == 0.0) {
            // With zero volatility, option value should be intrinsic value
            Real intrinsic = data.discount * std::max(0.0, 
                data.type == Option::Call ? data.forward - data.strike : data.strike - data.forward);
            if (std::fabs(value1 - intrinsic) > data.tolerance) {
                BOOST_ERROR("BlackCalculator zero volatility test failed for "
                           << (data.type == Option::Call ? "Call" : "Put")
                           << " expected=" << intrinsic << " calculated=" << value1);
            }
        }

        // Option value should be non-negative
        if (value1 < -data.tolerance) {
            BOOST_ERROR("BlackCalculator negative option value: " << value1);
        }
    }
}

BOOST_AUTO_TEST_CASE(testBlackCalculatorGreeks) {
    BOOST_TEST_MESSAGE("Testing BlackCalculator Greeks calculations...");

    Real forward = 100.0;
    Real strike = 100.0;
    Real stdDev = 0.20;
    Real discount = 0.95;
    Real spot = 98.0;
    Real maturity = 1.0;
    Real tolerance = 1e-6;

    BlackCalculator calc(Option::Call, strike, forward, stdDev, discount);

    // Test delta
    Real delta = calc.delta(spot);
    Real deltaForward = calc.deltaForward();
    
    if (std::isnan(delta) || std::isnan(deltaForward)) {
        BOOST_ERROR("BlackCalculator produced NaN delta values");
    }

    // Delta should be between 0 and 1 for calls
    if (delta < -tolerance || delta > 1.0 + tolerance) {
        BOOST_ERROR("BlackCalculator call delta out of range: " << delta);
    }

    // Test gamma
    Real gamma = calc.gamma(spot);
    Real gammaForward = calc.gammaForward();
    
    if (std::isnan(gamma) || std::isnan(gammaForward)) {
        BOOST_ERROR("BlackCalculator produced NaN gamma values");
    }

    // Gamma should be non-negative
    if (gamma < -tolerance || gammaForward < -tolerance) {
        BOOST_ERROR("BlackCalculator negative gamma: spot=" << gamma 
                   << " forward=" << gammaForward);
    }

    // Test theta
    Real theta = calc.theta(spot, maturity);
    if (std::isnan(theta)) {
        BOOST_ERROR("BlackCalculator produced NaN theta");
    }

    // Test vega
    Real vega = calc.vega(maturity);
    if (std::isnan(vega)) {
        BOOST_ERROR("BlackCalculator produced NaN vega");
    }

    // Vega should be non-negative for ATM options
    if (vega < -tolerance) {
        BOOST_ERROR("BlackCalculator negative vega: " << vega);
    }

    // Test rho
    Real rho = calc.rho(maturity);
    if (std::isnan(rho)) {
        BOOST_ERROR("BlackCalculator produced NaN rho");
    }

    // Test other Greeks
    Real dividendRho = calc.dividendRho(maturity);
    Real strikeSensitivity = calc.strikeSensitivity();
    Real strikeGamma = calc.strikeGamma();
    
    if (std::isnan(dividendRho) || std::isnan(strikeSensitivity) || std::isnan(strikeGamma)) {
        BOOST_ERROR("BlackCalculator produced NaN values for extended Greeks");
    }
}

BOOST_AUTO_TEST_CASE(testBlackCalculatorPutCallParity) {
    BOOST_TEST_MESSAGE("Testing BlackCalculator put-call parity...");

    Real forward = 100.0;
    Real strike = 105.0;
    Real stdDev = 0.25;
    Real discount = 0.95;
    Real tolerance = 1e-10;

    BlackCalculator callCalc(Option::Call, strike, forward, stdDev, discount);
    BlackCalculator putCalc(Option::Put, strike, forward, stdDev, discount);

    Real callValue = callCalc.value();
    Real putValue = putCalc.value();

    // Put-Call parity: C - P = discount * (F - K)
    Real paritylhs = callValue - putValue;
    Real parityrhs = discount * (forward - strike);
    Real parityError = std::fabs(paritylhs - parityrhs);

    if (parityError > tolerance) {
        BOOST_ERROR("BlackCalculator put-call parity violation: "
                   << "C-P=" << paritylhs << " discount*(F-K)=" << parityrhs
                   << " error=" << parityError);
    }
}

BOOST_AUTO_TEST_CASE(testBlackCalculatorEdgeCases) {
    BOOST_TEST_MESSAGE("Testing BlackCalculator edge cases...");

    Real tolerance = 1e-10;

    // Test very low volatility
    {
        BlackCalculator calc(Option::Call, 100.0, 100.0, 1e-8, 1.0);
        Real value = calc.value();
        if (value < 0 || std::isnan(value)) {
            BOOST_ERROR("BlackCalculator failed for very low volatility: " << value);
        }
    }

    // Test very high volatility
    {
        BlackCalculator calc(Option::Call, 100.0, 100.0, 2.0, 1.0);
        Real value = calc.value();
        if (value < 0 || std::isnan(value)) {
            BOOST_ERROR("BlackCalculator failed for very high volatility: " << value);
        }
    }

    // Test deep ITM call
    {
        BlackCalculator calc(Option::Call, 50.0, 100.0, 0.20, 1.0);
        Real value = calc.value();
        Real intrinsicValue = 100.0 - 50.0;  // Should be close to intrinsic
        if (value < intrinsicValue - tolerance) {
            BOOST_ERROR("BlackCalculator deep ITM call below intrinsic: " 
                       << value << " vs " << intrinsicValue);
        }
    }

    // Test deep OTM call
    {
        BlackCalculator calc(Option::Call, 150.0, 100.0, 0.20, 1.0);
        Real value = calc.value();
        if (value < 0 || value > 10.0) {  // Should be small positive value
            BOOST_ERROR("BlackCalculator deep OTM call unreasonable: " << value);
        }
    }
}

BOOST_AUTO_TEST_CASE(testBlackCalculatorNumericalDerivatives) {
    BOOST_TEST_MESSAGE("Testing BlackCalculator numerical derivative consistency...");

    Real forward = 100.0;
    Real strike = 100.0;
    Real stdDev = 0.20;
    Real discount = 0.95;
    Real spot = 98.0;
    Real maturity = 1.0;
    Real bump = 1e-4;
    Real tolerance = 1e-3;

    BlackCalculator calc(Option::Call, strike, forward, stdDev, discount);

    // Test delta via finite differences
    BlackCalculator calcUp(Option::Call, strike, forward + bump, stdDev, discount);
    BlackCalculator calcDown(Option::Call, strike, forward - bump, stdDev, discount);
    
    Real analyticalDelta = calc.deltaForward();
    Real numericalDelta = (calcUp.value() - calcDown.value()) / (2.0 * bump);
    Real deltaError = std::fabs(analyticalDelta - numericalDelta);

    if (deltaError > tolerance) {
        BOOST_ERROR("BlackCalculator delta finite difference test failed: "
                   << "analytical=" << analyticalDelta 
                   << " numerical=" << numericalDelta 
                   << " error=" << deltaError);
    }

    // Test gamma via finite differences
    Real analyticalGamma = calc.gammaForward();
    Real numericalGamma = (calcUp.deltaForward() - calcDown.deltaForward()) / (2.0 * bump);
    Real gammaError = std::fabs(analyticalGamma - numericalGamma);

    if (gammaError > tolerance) {
        BOOST_ERROR("BlackCalculator gamma finite difference test failed: "
                   << "analytical=" << analyticalGamma 
                   << " numerical=" << numericalGamma 
                   << " error=" << gammaError);
    }
}

BOOST_AUTO_TEST_CASE(testBlackCalculatorProbabilities) {
    BOOST_TEST_MESSAGE("Testing BlackCalculator ITM probabilities...");

    Real forward = 100.0;
    Real strike = 100.0;
    Real stdDev = 0.20;
    Real discount = 1.0;
    Real tolerance = 1e-8;

    BlackCalculator calc(Option::Call, strike, forward, stdDev, discount);

    Real cashProb = calc.itmCashProbability();
    Real assetProb = calc.itmAssetProbability();

    // Probabilities should be between 0 and 1
    if (cashProb < -tolerance || cashProb > 1.0 + tolerance) {
        BOOST_ERROR("BlackCalculator cash ITM probability out of range: " << cashProb);
    }
    
    if (assetProb < -tolerance || assetProb > 1.0 + tolerance) {
        BOOST_ERROR("BlackCalculator asset ITM probability out of range: " << assetProb);
    }

    // For ATM options, probabilities should be close to 0.5
    if (std::fabs(cashProb - 0.5) > 0.1) {
        BOOST_ERROR("BlackCalculator ATM cash probability far from 0.5: " << cashProb);
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()