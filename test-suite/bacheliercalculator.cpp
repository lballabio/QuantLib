/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*

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
#include <ql/pricingengines/bacheliercalculator.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/math/comparison.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <cmath>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(BachelierCalculatorTests)

struct BachelierCalculatorTestData {
    Option::Type type;
    Real strike;
    Real forward;
    Real stdDev;        // Absolute volatility
    Real discount;
    Real tolerance;
    Real refValue;
};

BOOST_AUTO_TEST_CASE(testBachelierCalculatorBasicValues) {
    BOOST_TEST_MESSAGE("Testing BachelierCalculator basic option values...");

    BachelierCalculatorTestData values[] = {
        // type, strike, forward, stdDev (absolute), discount, tolerance
        {Option::Call, 100.0, 100.0, 20.0, 1.0, 1e-8, 7.9788456080286538}, // ATM Call
        {Option::Put, 100.0, 100.0, 20.0, 1.0, 1e-8, 7.9788456080286538},  // ATM Put
        {Option::Call, 90.0, 100.0, 20.0, 1.0, 1e-8, 13.955931148026121},  // ITM Call
        {Option::Put, 110.0, 100.0, 20.0, 1.0, 1e-8, 13.955931148026121},  // ITM Put
        {Option::Call, 110.0, 100.0, 20.0, 1.0, 1e-8, 3.9559311480261217}, // OTM Call
        {Option::Put, 90.0, 100.0, 20.0, 1.0, 1e-8, 3.9559311480261217},   // OTM Put
        { Option::Call, 100.0, 100.0, 0.0,  1.0, 1e-8, 0.0 },  // Zero vol Call
        { Option::Put,  100.0, 100.0, 0.0,  1.0, 1e-8, 0.0 },  // Zero vol Put
        {Option::Call, 0.0, 100.0, 20.0, 1.0, 1e-8, 100.00000106923312},   // Zero strike
    };

    for (auto& data : values) {
        // Test constructor with Option::Type
        BachelierCalculator calc1(data.type, data.strike, data.forward, data.stdDev, data.discount);
        Real value1 = calc1.value();
        
        // Test constructor with Payoff
        ext::shared_ptr<StrikedTypePayoff> payoff(
            new PlainVanillaPayoff(data.type, data.strike));
        BachelierCalculator calc2(payoff, data.forward, data.stdDev, data.discount);
        Real value2 = calc2.value();

        // Both constructors should give the same result
        Real error = std::fabs(value1 - value2);
        if (error > data.tolerance) {
            BOOST_ERROR("BachelierCalculator constructor mismatch for "
                       << (data.type == Option::Call ? "Call" : "Put")
                       << " strike=" << data.strike << " forward=" << data.forward
                       << " stdDev=" << data.stdDev << " discount=" << data.discount
                       << " value1=" << value1 << " value2=" << value2
                       << " error=" << error);
        }

        Real error2 = std::fabs(value1 - data.refValue);
        if (error2 > data.tolerance) {
            BOOST_ERROR("BachelierCalculator constructor rf value error for "
                        << (data.type == Option::Call ? "Call" : "Put") << " strike=" << data.strike
                        << " forward=" << data.forward << " stdDev=" << data.stdDev
                        << " discount=" << data.discount << " value1=" << value1
                        << " value2=" << value2 << " error=" << error);
        }

        // Basic sanity checks
        if (data.stdDev == 0.0) {
            // With zero volatility, option value should be intrinsic value
            Real intrinsic = data.discount * std::max(0.0, 
                data.type == Option::Call ? data.forward - data.strike : data.strike - data.forward);
            if (std::fabs(value1 - intrinsic) > data.tolerance) {
                BOOST_ERROR("BachelierCalculator zero volatility test failed for "
                           << (data.type == Option::Call ? "Call" : "Put")
                           << " expected=" << intrinsic << " calculated=" << value1);
            }
        }

        // Option value should be non-negative
        if (value1 < -data.tolerance) {
            BOOST_ERROR("BachelierCalculator negative option value: " << value1);
        }
    }
}

BOOST_AUTO_TEST_CASE(testBachelierCalculatorGreeks) {
    BOOST_TEST_MESSAGE("Testing BachelierCalculator Greeks calculations...");

    Real forward = 100.0;
    Real strike = 105.0;
    Real stdDev = 20.0;  // Absolute volatility
    Real discount = 0.95;
    Real spot = 98.0;
    Real maturity = 1.0;
    Real tolerance = 1e-6;
    Real refDelta = 0.38900917408288;
    Real refDeltaFwd = 0.38122899060122245;
    Real refGamma = 0.019124047842706517;
    Real refGammaFwd = 0.018366735548135338;
    Real refTheta = -4.3159316452046594;
    Real refVega = 0.36733471096270676;
    Real refRho = 32.682349793874224;
    Real refElasticity = 7.0071783554334042;
    Real refElasticityFwd = 7.0071783554334051;
    Real refItmCashProb = 0.4012936743170763;
    Real refItmAssetProb = 0.4012936743170763;
    Real refDividendRho = -38.122899060122243;
    Real refStrikeSensitivity = -0.38122899060122245;
    Real refStrikeGamma = 0.018366735548135338;

    BachelierCalculator calc(Option::Call, strike, forward, stdDev, discount);

    Real delta = calc.delta(spot);
    Real deltaForward = calc.deltaForward();
    Real gamma = calc.gamma(spot);
    Real gammaForward = calc.gammaForward();
    Real theta = calc.theta(spot, maturity);
    Real vega = calc.vega(maturity);
    Real rho = calc.rho(maturity);
    Real elasticity = calc.elasticity(spot);
    Real elasticityForward = calc.elasticityForward();
    Real itmCashProb = calc.itmCashProbability();
    Real itmAssetProb = calc.itmAssetProbability();
    Real dividendRho = calc.dividendRho(maturity);
    Real strikeSensitivity = calc.strikeSensitivity();
    Real strikeGamma = calc.strikeGamma();
    
    if (std::fabs(deltaForward - refDeltaFwd) > tolerance) {
        BOOST_ERROR("BachelierCalculator call fwd delta error");
    }

    if (std::fabs(delta - refDelta) > tolerance) {
        BOOST_ERROR("BachelierCalculator call delta error");
    }

    if (std::fabs(gammaForward - refGammaFwd) > tolerance) {
        BOOST_ERROR("BachelierCalculator call fwd gamma error");
    }

    if (std::fabs(gamma - refGamma) > tolerance) {
        BOOST_ERROR("BachelierCalculator call gamma error");
    }

    if (std::fabs(theta - refTheta) > tolerance) {
        BOOST_ERROR("BachelierCalculator call theta error");
    }

    if (std::fabs(vega - refVega) > tolerance) {
        BOOST_ERROR("BachelierCalculator call vega error");
    }

    if (std::fabs(rho - refRho) > tolerance) {
        BOOST_ERROR("BachelierCalculator call rho error");
    }

    if (std::fabs(elasticityForward - refElasticityFwd) > tolerance) {
        BOOST_ERROR("BachelierCalculator call fwd elasticity error");
    }

    if (std::fabs(elasticity - refElasticity) > tolerance) {
        BOOST_ERROR("BachelierCalculator call elasticity error");
    }

    if (std::fabs(itmCashProb - refItmCashProb) > tolerance) {
        BOOST_ERROR("BachelierCalculator call itm cash probability error");
    }

    if (std::fabs(itmAssetProb - refItmAssetProb) > tolerance) {
        BOOST_ERROR("BachelierCalculator call itm asset probability error");
    }

    if (std::fabs(dividendRho - refDividendRho) > tolerance) {
        BOOST_ERROR("BachelierCalculator call dividend rho error");
    }

    if (std::fabs(strikeSensitivity - refStrikeSensitivity) > tolerance) {
        BOOST_ERROR("BachelierCalculator call strike sensitivity error");
    }

    if (std::fabs(strikeGamma - refStrikeGamma) > tolerance) {
        BOOST_ERROR("BachelierCalculator call strike gamma error");
    }

}

BOOST_AUTO_TEST_CASE(testBachelierCalculatorPutCallParity) {
    BOOST_TEST_MESSAGE("Testing BachelierCalculator put-call parity...");

    Real forward = 100.0;
    Real strike = 105.0;
    Real stdDev = 25.0;  // Absolute volatility
    Real discount = 0.95;
    Real tolerance = 1e-10;

    BachelierCalculator callCalc(Option::Call, strike, forward, stdDev, discount);
    BachelierCalculator putCalc(Option::Put, strike, forward, stdDev, discount);

    Real callValue = callCalc.value();
    Real putValue = putCalc.value();

    // Put-Call parity: C - P = discount * (F - K)
    Real paritylhs = callValue - putValue;
    Real parityrhs = discount * (forward - strike);
    Real parityError = std::fabs(paritylhs - parityrhs);

    if (parityError > tolerance) {
        BOOST_ERROR("BachelierCalculator put-call parity violation: "
                   << "C-P=" << paritylhs << " discount*(F-K)=" << parityrhs
                   << " error=" << parityError);
    }
}

BOOST_AUTO_TEST_CASE(testBachelierCalculatorEdgeCases) {
    BOOST_TEST_MESSAGE("Testing BachelierCalculator edge cases...");

    Real tolerance = 1e-8;

    // Test zero volatility
    {
        BachelierCalculator calc(Option::Call, 100.0, 100.0, 0.0, 1.0);
        Real value = calc.value();
        Real refValue = 0.0;
        if (std::fabs(value - refValue) > tolerance) {
            BOOST_ERROR("BachelierCalculator failed for zero volatility: " << value);
        }
    }

    // Test very high volatility
    {
        BachelierCalculator calc(Option::Call, 100.0, 100.0, 200.0, 1.0);
        Real value = calc.value();
        Real refValue = 79.788456080286537;
        if (std::fabs(value - refValue) > tolerance) {
            BOOST_ERROR("BachelierCalculator failed for very high volatility: " << value);
        }
    }

    // Test negative strikes (valid in Bachelier model)
    {
        BachelierCalculator calc(Option::Call, -50.0, 100.0, 20.0, 1.0);
        Real value = calc.value();
        Real intrinsicValue = 100.0 - (-50.0);  // Should be close to intrinsic
        if (value < intrinsicValue - 10.0) {  // Allow some time value
            BOOST_ERROR("BachelierCalculator negative strike call unreasonable: " 
                       << value << " vs intrinsic " << intrinsicValue);
        }
    }

    // Test negative forward (valid in Bachelier model)
    {
        BachelierCalculator calc(Option::Call, 50.0, -100.0, 20.0, 1.0);
        Real value = calc.value();
        Real intrinsicValue = -100.0 - 50.0; // Should be close to intrinsic
        if (value < intrinsicValue + 10.0) {   // Allow some time value
            BOOST_ERROR("BachelierCalculator negative strike call unreasonable: "
                        << value << " vs intrinsic " << intrinsicValue);
        }
    }

    // Test deep ITM call
    {
        BachelierCalculator calc(Option::Call, 50.0, 100.0, 20.0, 1.0);
        Real value = calc.value();
        Real intrinsicValue = 100.0 - 50.0;  // Should be close to intrinsic
        if (value < intrinsicValue - tolerance) {
            BOOST_ERROR("BachelierCalculator deep ITM call below intrinsic: " 
                       << value << " vs " << intrinsicValue);
        }
    }

    // Test deep OTM call
    {
        BachelierCalculator calc(Option::Call, 150.0, 100.0, 20.0, 1.0);
        Real value = calc.value();
        Real refValue = 0.040082743582562863;
        if (std::fabs(value - refValue) > tolerance) {
            BOOST_ERROR("BachelierCalculator deep OTM call negative: " << value);
        }
    }
}

BOOST_AUTO_TEST_CASE(testBachelierCalculatorNumericalDerivatives) {
    BOOST_TEST_MESSAGE("Testing BachelierCalculator numerical derivative consistency...");

    Real forward = 100.0;
    Real strike = 100.0;
    Real stdDev = 20.0;  // Absolute volatility
    Real discount = 0.95;
    Real maturity = 1.0;
    Real bump = 1e-4;
    Real tolerance = 1e-3;

    BachelierCalculator calc(Option::Call, strike, forward, stdDev, discount);

    // Test delta via finite differences
    BachelierCalculator calcUp(Option::Call, strike, forward + bump, stdDev, discount);
    BachelierCalculator calcDown(Option::Call, strike, forward - bump, stdDev, discount);
    
    Real analyticalDelta = calc.deltaForward();
    Real numericalDelta = (calcUp.value() - calcDown.value()) / (2.0 * bump);
    Real deltaError = std::fabs(analyticalDelta - numericalDelta);

    if (deltaError > tolerance) {
        BOOST_ERROR("BachelierCalculator delta finite difference test failed: "
                   << "analytical=" << analyticalDelta 
                   << " numerical=" << numericalDelta 
                   << " error=" << deltaError);
    }

    // Test gamma via finite differences
    Real analyticalGamma = calc.gammaForward();
    Real numericalGamma = (calcUp.deltaForward() - calcDown.deltaForward()) / (2.0 * bump);
    Real gammaError = std::fabs(analyticalGamma - numericalGamma);

    if (gammaError > tolerance) {
        BOOST_ERROR("BachelierCalculator gamma finite difference test failed: "
                   << "analytical=" << analyticalGamma 
                   << " numerical=" << numericalGamma 
                   << " error=" << gammaError);
    }

    // Test vega via finite differences
    BachelierCalculator calcVolUp(Option::Call, strike, forward, stdDev + bump, discount);
    BachelierCalculator calcVolDown(Option::Call, strike, forward, stdDev - bump, discount);
    
    Real analyticalVega = calc.vega(maturity);
    Real numericalVega = (calcVolUp.value() - calcVolDown.value()) / (2.0 * bump);
    Real vegaError = std::fabs(analyticalVega - numericalVega * std::sqrt(maturity));

    if (vegaError > tolerance) {
        BOOST_ERROR("BachelierCalculator vega finite difference test failed: "
                   << "analytical=" << analyticalVega 
                   << " numerical=" << numericalVega * std::sqrt(maturity)
                   << " error=" << vegaError);
    }
}

BOOST_AUTO_TEST_CASE(testBachelierCalculatorAgainstAnalyticalFormula) {
    BOOST_TEST_MESSAGE("Testing BachelierCalculator against analytical Bachelier formula...");

    Real forward = 100.0;
    Real strike = 95.0;
    Real stdDev = 15.0;  // Absolute volatility
    Real discount = 0.98;
    Real tolerance = 1e-10;

    BachelierCalculator calc(Option::Call, strike, forward, stdDev, discount);
    Real calculatedValue = calc.value();

    // Analytical Bachelier formula: C = (F-K)*N(d) + ?*n(d)
    // where d = (F-K)/?
    Real d = (forward - strike) / stdDev;
    CumulativeNormalDistribution N;
    NormalDistribution n;
    
    Real analyticalValue = discount * ((forward - strike) * N(d) + stdDev * n(d));
    Real error = std::fabs(calculatedValue - analyticalValue);

    if (error > tolerance) {
        BOOST_ERROR("BachelierCalculator analytical formula test failed: "
                   << "calculated=" << calculatedValue 
                   << " analytical=" << analyticalValue
                   << " error=" << error);
    }
}

BOOST_AUTO_TEST_CASE(testBachelierCalculatorZeroVolatilityGreeks) {
    BOOST_TEST_MESSAGE("Testing BachelierCalculator Greeks with zero volatility...");

    Real tolerance = 1e-10;
    Real forward = 100.0;
    Real discount = 1.0;
    Real spot = 98.0;
    Real maturity = 1.0;
    Real stdDev = 0.0;  // Zero absolute volatility

    // Test different moneyness scenarios
    struct ZeroVolTestCase {
        Option::Type type;
        Real strike;
        std::string description;
        Real expectedDelta;
        Real expectedGamma;
        Real expectedVega;
        Real expectedTheta;
    };

    ZeroVolTestCase testCases[] = {
        // ITM options should have delta = 1 for calls, -1 for puts (approximately)
        {Option::Call, 90.0, "ITM Call", 1.0, 0.0, 0.0, 0.0},
        {Option::Put, 110.0, "ITM Put", -1.0, 0.0, 0.0, 0.0},
        // ATM options in Bachelier model
        {Option::Call, 100.0, "ATM Call", 0.5, 0.0, 0.0, 0.0},
        {Option::Put, 100.0, "ATM Put", -0.5, 0.0, 0.0, 0.0},
        // OTM options should have delta = 0
        {Option::Call, 90.0, "OTM Call", 0.0, 0.0, 0.0, 0.0},
        {Option::Put, 110.0, "OTM Put", 0.0, 0.0, 0.0, 0.0},
        // Test negative strikes (valid in Bachelier model)
        {Option::Call, -10.0, "Negative Strike Call", 1.0, 0.0, 0.0, 0.0},
        {Option::Put, 200.0, "High Strike Put", -1.0, 0.0, 0.0, 0.0}
    };

    for (const auto& testCase : testCases) {
        BachelierCalculator calc(testCase.type, testCase.strike, forward, stdDev, discount);

        Real deltaForward = calc.deltaForward();
        Real delta = calc.delta(spot);
        Real gammaForward = calc.gammaForward();
        Real gamma = calc.gamma(spot);
        Real vega = calc.vega(maturity);
        Real theta = calc.theta(spot, maturity);
        Real rho = calc.rho(maturity);
        Real dividendRho = calc.dividendRho(maturity);

        // All Greeks should be finite (not NaN or infinite)
        if (!std::isfinite(deltaForward) || !std::isfinite(delta) || 
            !std::isfinite(gammaForward) || !std::isfinite(gamma) ||
            !std::isfinite(vega) || !std::isfinite(theta) || 
            !std::isfinite(rho) || !std::isfinite(dividendRho)) {
            BOOST_ERROR("BachelierCalculator " << testCase.description 
                       << " produced non-finite Greeks with zero volatility");
        }

        // Gamma should be zero (no convexity with zero vol)
        if (std::fabs(gammaForward) > tolerance || std::fabs(gamma) > tolerance) {
            BOOST_ERROR("BachelierCalculator " << testCase.description 
                       << " gamma should be zero with zero volatility: "
                       << "gammaForward=" << gammaForward << " gamma=" << gamma);
        }

        // Vega should be zero (no vol sensitivity)  
        if (std::fabs(vega) > tolerance) {
            BOOST_ERROR("BachelierCalculator " << testCase.description 
                       << " vega should be zero with zero volatility: " << vega);
        }

        // For clearly ITM/OTM cases, check delta bounds
        if (testCase.strike < forward - 5.0) { // Clearly ITM call
            if (testCase.type == Option::Call && (deltaForward < 0.99 || deltaForward > 1.01)) {
                BOOST_ERROR("BachelierCalculator ITM call deltaForward should be ~1.0 with zero vol: " 
                           << deltaForward);
            }
        }
        if (testCase.strike > forward + 5.0) { // Clearly OTM call
            if (testCase.type == Option::Call && std::fabs(deltaForward) > tolerance) {
                BOOST_ERROR("BachelierCalculator OTM call deltaForward should be ~0.0 with zero vol: " 
                           << deltaForward);
            }
        }

        // Strike sensitivities should be finite
        Real strikeSens = calc.strikeSensitivity();
        Real strikeGamma = calc.strikeGamma();
        
        if (!std::isfinite(strikeSens) || !std::isfinite(strikeGamma)) {
            BOOST_ERROR("BachelierCalculator " << testCase.description 
                       << " strike sensitivities should be finite with zero volatility");
        }

        // Test ITM probabilities
        Real itmCashProb = calc.itmCashProbability();
        Real itmAssetProb = calc.itmAssetProbability();
        
        if (!std::isfinite(itmCashProb) || !std::isfinite(itmAssetProb)) {
            BOOST_ERROR("BachelierCalculator " << testCase.description 
                       << " ITM probabilities should be finite with zero volatility");
        }

        // In Bachelier model with zero vol, ITM probabilities should be 0 or 1
        Real expectedProb = (testCase.type == Option::Call) ? 
                           (forward > testCase.strike ? 1.0 : (forward == testCase.strike ? 0.5 : 0.0)) :
                           (forward < testCase.strike ? 1.0 : (forward == testCase.strike ? 0.5 : 0.0));
        
        if (std::fabs(itmCashProb - expectedProb) > tolerance) {
            BOOST_ERROR("BachelierCalculator " << testCase.description 
                       << " ITM cash probability incorrect with zero vol: expected=" 
                       << expectedProb << " actual=" << itmCashProb);
        }
    }
}

BOOST_AUTO_TEST_CASE(testBachelierVsBlackConvergence) {
    BOOST_TEST_MESSAGE("Testing BachelierCalculator convergence to BlackCalculator for small relative volatilities...");

    Real forward = 100.0;
    Real strike = 100.0;
    Real relativeVol = 0.01;  // 1% relative volatility
    Real absoluteVol = relativeVol * forward;  // Convert to absolute
    Real discount = 1.0;
    Real tolerance = 1e-2;  // Looser tolerance for convergence test

    BachelierCalculator bachelierCalc(Option::Call, strike, forward, absoluteVol, discount);
    BlackCalculator blackCalc(Option::Call, strike, forward, relativeVol, discount);

    Real bachelierValue = bachelierCalc.value();
    Real blackValue = blackCalc.value();

    // For small relative volatilities, Bachelier should approximate Black-Scholes
    Real error = std::fabs(bachelierValue - blackValue);
    Real relativeError = error / blackValue;

    if (relativeError > tolerance) {
        BOOST_ERROR("BachelierCalculator vs BlackCalculator convergence test failed: "
                   << "bachelier=" << bachelierValue 
                   << " black=" << blackValue
                   << " relative error=" << relativeError);
    }
}


BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()