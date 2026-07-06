/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Colin Alberts

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/exercise.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/math/distributions/gammadistribution.hpp>
#include <ql/math/ode/fractionaladams.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/models/equity/hestonmodelhelper.hpp>
#include <ql/models/equity/roughhestonmodel.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <ql/pricingengines/vanilla/analyticroughhestonengine.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <cmath>
#include <complex>
#include <vector>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(RoughHestonModelTests)

BOOST_AUTO_TEST_CASE(testFractionalAdamsScheme) {
    BOOST_TEST_MESSAGE(
        "Testing fractional Adams scheme against known solutions...");

    // D^a y = Gamma(5)/Gamma(5-a) t^(4-a) + t^4 - y with y(0) = 0
    // has the exact solution y(t) = t^4
    const Real alpha = 0.6;
    const Real g = GammaFunction().value(5.0)
        /GammaFunction().value(5.0 - alpha);

    const Real tMax = 1.0;

    const auto maxError = [g, alpha, tMax](Size steps) -> Real {
        const std::vector<Real> y = FractionalAdams<>(alpha).solve(
            [g, alpha](Real t, const Real& x) -> Real {
                return g*std::pow(t, 4 - alpha) + t*t*t*t - x;
            },
            0.0, tMax, steps);

        Real error = 0.0;
        for (Size i = 0; i <= steps; ++i) {
            const Real t = Real(i)*tMax/steps;
            error = std::max(error, std::fabs(y[i] - t*t*t*t));
        }
        return error;
    };

    const Real coarseError = maxError(100), fineError = maxError(200);

    const Real tol = 3e-4;
    if (fineError > tol)
        BOOST_ERROR("failed to reproduce known fractional ODE solution"
                    << "\n    max error:  " << fineError
                    << "\n    tolerance:  " << tol);

    // at convergence order 1 + alpha = 1.6, halving the step size
    // should reduce the error by about a factor of three
    const Real minRatio = 2.2;
    if (coarseError/fineError < minRatio)
        BOOST_ERROR("failed to reproduce fractional Adams convergence order"
                    << "\n    error (n=100):  " << coarseError
                    << "\n    error (n=200):  " << fineError
                    << "\n    ratio:          " << coarseError/fineError
                    << "\n    expected ratio: > " << minRatio);

    // the linear problem D^a y = -y with y(0) = 1 has the solution
    // y(t) = E_a(-t^a), the Mittag-Leffler function, computed here
    // by direct series summation
    const Real mittagLefflerRef = [alpha]() -> Real {
        Real sum = 0.0, sgn = 1.0;
        for (Size k = 0; k < 100; ++k, sgn = -sgn)
            sum += sgn/GammaFunction().value(alpha*k + 1.0);
        return sum;
    }();

    const std::vector<Real> w = FractionalAdams<>(alpha).solve(
        [](Real, const Real& x) -> Real { return -x; }, 1.0, 1.0, 400);

    const Real mlTol = 1e-4;
    if (std::fabs(w.back() - mittagLefflerRef) > mlTol)
        BOOST_ERROR("failed to reproduce Mittag-Leffler solution of the "
                    "linear fractional ODE"
                    << "\n    calculated: " << w.back()
                    << "\n    expected:   " << mittagLefflerRef
                    << "\n    tolerance:  " << mlTol);

    // the pricing engine only ever instantiates FractionalAdams<complex<Real>>;
    // D^a z = i omega z with z(0) = 1 has the solution z(t) = E_a(i omega t^a)
    const std::complex<Real> lambda(0.0, 1.5);
    const std::complex<Real> mittagLefflerComplexRef = [alpha, lambda]() {
        std::complex<Real> sum = 0.0, term = 1.0;
        for (Size k = 0; k < 100; ++k, term *= lambda)
            sum += term/GammaFunction().value(alpha*k + 1.0);
        return sum;
    }();

    const std::vector<std::complex<Real>> wc =
        FractionalAdams<std::complex<Real>>(alpha).solve(
            [lambda](Real, const std::complex<Real>& x) { return lambda*x; },
            std::complex<Real>(1.0), 1.0, 400);

    if (std::abs(wc.back() - mittagLefflerComplexRef) > mlTol)
        BOOST_ERROR("failed to reproduce complex Mittag-Leffler solution of "
                    "the linear fractional ODE"
                    << "\n    calculated: " << wc.back()
                    << "\n    expected:   " << mittagLefflerComplexRef
                    << "\n    tolerance:  " << mlTol);
}

BOOST_AUTO_TEST_CASE(testRiemannLiouvilleIntegral) {
    BOOST_TEST_MESSAGE(
        "Testing product-trapezoidal Riemann-Liouville integration...");

    // I^a t^p = Gamma(p+1)/Gamma(p+1+a) t^(p+a)
    const Size steps = 500;
    const Real tMax = 2.0;
    const Real dt = tMax/steps;

    for (const Real p : {1.0, 2.0, 3.0}) {
        std::vector<Real> y(steps + 1);
        for (Size i = 0; i <= steps; ++i)
            y[i] = std::pow(Real(i)*dt, p);

        for (const Real a : {0.0, 0.25, 0.6, 1.0}) {
            const Real calculated = riemannLiouvilleIntegral(y, a, dt);
            const Real expected = GammaFunction().value(p + 1.0)
                /GammaFunction().value(p + 1.0 + a)*std::pow(tMax, p + a);

            const Real tol = (p == 1.0) ? 1e-12 : 5e-5;
            if (std::fabs(calculated - expected) > tol)
                BOOST_ERROR("failed to reproduce fractional integral of t^p"
                            << "\n    p:          " << p
                            << "\n    alpha:      " << a
                            << "\n    calculated: " << calculated
                            << "\n    expected:   " << expected
                            << "\n    tolerance:  " << tol);

            // FractionalAdams only ever integrates complex-valued vectors in
            // production; check the templated complex path directly
            std::vector<std::complex<Real>> yc(y.size());
            for (Size i = 0; i <= steps; ++i)
                yc[i] = std::complex<Real>(1.0, 1.0)*y[i];

            const std::complex<Real> calculatedC =
                riemannLiouvilleIntegral(yc, a, dt);
            const std::complex<Real> expectedC =
                std::complex<Real>(1.0, 1.0)*expected;

            if (std::abs(calculatedC - expectedC) > tol)
                BOOST_ERROR("failed to reproduce complex fractional integral "
                            "of t^p"
                            << "\n    p:          " << p
                            << "\n    alpha:      " << a
                            << "\n    calculated: " << calculatedC
                            << "\n    expected:   " << expectedC
                            << "\n    tolerance:  " << tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testEquivalenceWithHestonModel) {
    BOOST_TEST_MESSAGE(
        "Testing rough Heston engine against the classical Heston engine "
        "for H = 0.5...");

    const Date today(2, July, 2026);
    Settings::instance().evaluationDate() = today;
    const DayCounter dc = Actual365Fixed();

    const Handle<YieldTermStructure> rTS(
        ext::make_shared<FlatForward>(today, 0.05, dc));
    const Handle<YieldTermStructure> qTS(
        ext::make_shared<FlatForward>(today, 0.02, dc));
    const Handle<Quote> s0(ext::make_shared<SimpleQuote>(100.0));

    // three parameter sets: the original strongly-correlated case, one
    // with near-zero correlation, and one with a small vol-of-vol
    struct Params { Real v0, kappa, theta, sigma, rho; };
    const Params paramSets[] = {
        {0.05, 1.5, 0.04, 0.5, -0.75},
        {0.05, 1.5, 0.04, 0.5,  0.02},
        {0.05, 1.5, 0.04, 0.1, -0.75},
    };

    for (const auto& p : paramSets) {
        const auto roughModel = ext::make_shared<RoughHestonModel>(
            rTS, qTS, s0, p.v0, p.kappa, p.theta, p.sigma, p.rho, 0.5);
        const auto roughEngine = ext::make_shared<AnalyticRoughHestonEngine>(
            roughModel, 128, 512);

        const auto hestonModel = ext::make_shared<HestonModel>(
            ext::make_shared<HestonProcess>(
                rTS, qTS, s0, p.v0, p.kappa, p.theta, p.sigma, p.rho));
        const auto hestonEngine =
            ext::make_shared<AnalyticHestonEngine>(hestonModel, 192);

        // characteristic function comparison along the integration contour
        for (const Real u : {0.5, 1.0, 5.0, 10.0, 25.0, 50.0}) {
            for (const Time t : {0.5, 2.0}) {
                const std::complex<Real> z(u, -0.5);
                const std::complex<Real> calculated = roughEngine->chF(z, t);
                const std::complex<Real> expected = hestonEngine->chF(z, t);

                const Real tol = 1e-5;
                if (std::abs(calculated - expected) > tol)
                    BOOST_ERROR("failed to reproduce Heston characteristic "
                                "function for H = 0.5"
                                << "\n    rho:        " << p.rho
                                << "\n    sigma:      " << p.sigma
                                << "\n    u:          " << u
                                << "\n    t:          " << t
                                << "\n    calculated: " << calculated
                                << "\n    expected:   " << expected
                                << "\n    tolerance:  " << tol);
            }
        }

        // price comparison through the full pricing-engine interface
        for (const auto& optionType : {Option::Call, Option::Put}) {
            for (const Real strike : {80.0, 90.0, 100.0, 110.0, 125.0}) {
                for (const Size months : {6, 12, 24}) {
                    const Date maturity = today + Period(months, Months);

                    VanillaOption option(
                        ext::make_shared<PlainVanillaPayoff>(optionType, strike),
                        ext::make_shared<EuropeanExercise>(maturity));

                    option.setPricingEngine(roughEngine);
                    const Real calculated = option.NPV();

                    option.setPricingEngine(hestonEngine);
                    const Real expected = option.NPV();

                    const Real tol = 5e-4;
                    if (std::fabs(calculated - expected) > tol)
                        BOOST_ERROR("failed to reproduce Heston price for "
                                    "H = 0.5"
                                    << "\n    rho:        " << p.rho
                                    << "\n    sigma:      " << p.sigma
                                    << "\n    option:     " << optionType
                                    << "\n    strike:     " << strike
                                    << "\n    maturity:   " << maturity
                                    << "\n    calculated: " << calculated
                                    << "\n    expected:   " << expected
                                    << "\n    tolerance:  " << tol);
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testIntegrationAlgorithmConsistency) {
    BOOST_TEST_MESSAGE(
        "Testing consistency of rough Heston prices across Fourier "
        "integration algorithms...");

    const Date today(2, July, 2026);
    Settings::instance().evaluationDate() = today;
    const DayCounter dc = Actual365Fixed();

    const Handle<YieldTermStructure> rTS(
        ext::make_shared<FlatForward>(today, 0.03, dc));
    const Handle<YieldTermStructure> qTS(
        ext::make_shared<FlatForward>(today, 0.0, dc));
    const Handle<Quote> s0(ext::make_shared<SimpleQuote>(100.0));

    const auto model = ext::make_shared<RoughHestonModel>(
        rTS, qTS, s0, 0.04, 0.3, 0.04, 0.4, -0.7, 0.1);

    const Size timeSteps = 256;
    const auto laguerreEngine = ext::make_shared<AnalyticRoughHestonEngine>(
        model, 128, timeSteps);
    const auto lobattoEngine = ext::make_shared<AnalyticRoughHestonEngine>(
        model,
        AnalyticRoughHestonEngine::Integration::gaussLobatto(
            1e-10, 1e-10, 100000),
        timeSteps);

    for (const Real strike : {75.0, 100.0, 130.0}) {
        for (const Size months : {3, 12}) {
            const Date maturity = today + Period(months, Months);

            VanillaOption option(
                ext::make_shared<PlainVanillaPayoff>(Option::Call, strike),
                ext::make_shared<EuropeanExercise>(maturity));

            option.setPricingEngine(laguerreEngine);
            const Real laguerreNPV = option.NPV();

            option.setPricingEngine(lobattoEngine);
            const Real lobattoNPV = option.NPV();

            const Real tol = 1e-6;
            if (std::fabs(laguerreNPV - lobattoNPV) > tol)
                BOOST_ERROR("Gauss-Laguerre and Gauss-Lobatto rough Heston "
                            "prices differ"
                            << "\n    strike:        " << strike
                            << "\n    maturity:      " << maturity
                            << "\n    Gauss-Laguerre: " << laguerreNPV
                            << "\n    Gauss-Lobatto:  " << lobattoNPV
                            << "\n    tolerance:      " << tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testKnownReferenceValues) {
    BOOST_TEST_MESSAGE(
        "Testing rough Heston prices against an independent reference "
        "implementation...");

    const Date today(2, July, 2026);
    Settings::instance().evaluationDate() = today;
    const DayCounter dc = Actual365Fixed();

    const Handle<YieldTermStructure> rTS(
        ext::make_shared<FlatForward>(today, 0.03, dc));
    const Handle<YieldTermStructure> qTS(
        ext::make_shared<FlatForward>(today, 0.0, dc));
    const Handle<Quote> s0(ext::make_shared<SimpleQuote>(100.0));

    const auto model = ext::make_shared<RoughHestonModel>(
        rTS, qTS, s0, 0.04, 0.3, 0.04, 0.4, -0.7, 0.1);
    const auto engine = ext::make_shared<AnalyticRoughHestonEngine>(
        model, 128, 512);

    // reference prices from a from-scratch NumPy/SciPy reimplementation,
    // independent of this engine's C++ code, converged to ~1e-4
    struct Reference { Time maturity; Real strike; Real price; };
    const Reference references[] = {
        {0.25,  75.0, 25.762400},
        {0.25, 100.0,  3.793313},
        {0.25, 130.0,  0.001586},
        {1.00,  75.0, 28.394103},
        {1.00, 100.0,  8.336634},
        {1.00, 130.0,  0.202295},
        {2.00,  75.0, 31.520600},
        {2.00, 100.0, 12.722717},
        {2.00, 130.0,  1.285820},
    };

    for (const auto& ref : references) {
        const Real calculated = engine->priceVanillaPayoff(
            ext::make_shared<PlainVanillaPayoff>(Option::Call, ref.strike),
            ref.maturity);

        const Real tol = 5e-4;
        if (std::fabs(calculated - ref.price) > tol)
            BOOST_ERROR("failed to reproduce independent reference price"
                        << "\n    strike:     " << ref.strike
                        << "\n    maturity:   " << ref.maturity
                        << "\n    calculated: " << calculated
                        << "\n    expected:   " << ref.price
                        << "\n    tolerance:  " << tol);
    }
}

BOOST_AUTO_TEST_CASE(testPutCallParity) {
    BOOST_TEST_MESSAGE("Testing put-call parity for the rough Heston engine...");

    const Date today(2, July, 2026);
    Settings::instance().evaluationDate() = today;
    const DayCounter dc = Actual365Fixed();

    const Handle<YieldTermStructure> rTS(
        ext::make_shared<FlatForward>(today, 0.03, dc));
    const Handle<YieldTermStructure> qTS(
        ext::make_shared<FlatForward>(today, 0.01, dc));
    const Handle<Quote> s0(ext::make_shared<SimpleQuote>(100.0));

    for (const Real hurst : {0.1, 0.3, 0.5}) {
        const auto model = ext::make_shared<RoughHestonModel>(
            rTS, qTS, s0, 0.04, 0.3, 0.04, 0.4, -0.7, hurst);
        const auto engine = ext::make_shared<AnalyticRoughHestonEngine>(
            model, 128, 256);

        for (const Real strike : {70.0, 100.0, 130.0}) {
            for (const Time t : {0.1, 1.0, 2.0}) {
                const Real callPrice = engine->priceVanillaPayoff(
                    ext::make_shared<PlainVanillaPayoff>(Option::Call, strike), t);
                const Real putPrice = engine->priceVanillaPayoff(
                    ext::make_shared<PlainVanillaPayoff>(Option::Put, strike), t);

                const Real fwd = s0->value()*qTS->discount(t)/rTS->discount(t);
                const Real expected = (fwd - strike)*rTS->discount(t);

                const Real tol = 1e-6;
                if (std::fabs((callPrice - putPrice) - expected) > tol)
                    BOOST_ERROR("failed put-call parity"
                                << "\n    hurst:      " << hurst
                                << "\n    strike:     " << strike
                                << "\n    maturity:   " << t
                                << "\n    call - put: " << callPrice - putPrice
                                << "\n    expected:   " << expected
                                << "\n    tolerance:  " << tol);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testMonotonicityAndBounds) {
    BOOST_TEST_MESSAGE(
        "Testing rough Heston call-price monotonicity, convexity and "
        "no-arbitrage bounds...");

    const Date today(2, July, 2026);
    Settings::instance().evaluationDate() = today;
    const DayCounter dc = Actual365Fixed();

    const Handle<YieldTermStructure> rTS(
        ext::make_shared<FlatForward>(today, 0.03, dc));
    const Handle<YieldTermStructure> qTS(
        ext::make_shared<FlatForward>(today, 0.01, dc));
    const Handle<Quote> s0(ext::make_shared<SimpleQuote>(100.0));

    const auto model = ext::make_shared<RoughHestonModel>(
        rTS, qTS, s0, 0.04, 0.3, 0.04, 0.4, -0.7, 0.15);
    const auto engine = ext::make_shared<AnalyticRoughHestonEngine>(
        model, 128, 256);

    const Time t = 1.0;
    const Real fwd = s0->value()*qTS->discount(t)/rTS->discount(t);
    const Real df = rTS->discount(t);

    const std::vector<Real> strikes =
        {60.0, 70.0, 80.0, 90.0, 100.0, 110.0, 120.0, 130.0, 140.0};
    std::vector<Real> prices;
    for (Real k : strikes)
        prices.push_back(engine->priceVanillaPayoff(
            ext::make_shared<PlainVanillaPayoff>(Option::Call, k), t));

    for (Size i = 0; i < strikes.size(); ++i) {
        const Real lower = std::max(fwd - strikes[i], 0.0)*df;
        const Real upper = fwd*df;
        if (prices[i] < lower - 1e-8 || prices[i] > upper + 1e-8)
            BOOST_ERROR("call price violates no-arbitrage bounds"
                        << "\n    strike: " << strikes[i]
                        << "\n    price:  " << prices[i]
                        << "\n    lower:  " << lower
                        << "\n    upper:  " << upper);

        if (i > 0 && prices[i] > prices[i - 1] + 1e-8)
            BOOST_ERROR("call price is not monotonically decreasing in strike"
                        << "\n    strike: " << strikes[i - 1] << " -> " << strikes[i]
                        << "\n    price:  " << prices[i - 1] << " -> " << prices[i]);

        if (i > 0 && i + 1 < strikes.size()) {
            const Real secondDiff = prices[i + 1] - 2*prices[i] + prices[i - 1];
            if (secondDiff < -1e-6)
                BOOST_ERROR("call price is not convex in strike"
                            << "\n    strike:            " << strikes[i]
                            << "\n    second difference: " << secondDiff);
        }
    }
}

BOOST_AUTO_TEST_CASE(testInputValidation) {
    BOOST_TEST_MESSAGE(
        "Testing input validation of the rough Heston engine...");

    const Date today(2, July, 2026);
    Settings::instance().evaluationDate() = today;
    const DayCounter dc = Actual365Fixed();

    const Handle<YieldTermStructure> rTS(
        ext::make_shared<FlatForward>(today, 0.03, dc));
    const Handle<YieldTermStructure> qTS(
        ext::make_shared<FlatForward>(today, 0.0, dc));
    const Handle<Quote> s0(ext::make_shared<SimpleQuote>(100.0));

    const auto model = ext::make_shared<RoughHestonModel>(
        rTS, qTS, s0, 0.04, 0.3, 0.04, 0.4, -0.7, 0.15);

    BOOST_CHECK_THROW(
        AnalyticRoughHestonEngine(model, 128, 0), Error);

    const auto integration =
        AnalyticRoughHestonEngine::Integration::gaussLaguerre(128);
    BOOST_CHECK_THROW(
        AnalyticRoughHestonEngine(model, integration, 0), Error);
    BOOST_CHECK_THROW(
        AnalyticRoughHestonEngine(model, integration, 256, 1e-25, 0.0), Error);
    BOOST_CHECK_THROW(
        AnalyticRoughHestonEngine(model, integration, 256, 1e-25, -1.0), Error);
    BOOST_CHECK_THROW(
        AnalyticRoughHestonEngine(model, integration, 256, 1e-25, 0.5), Error);

    const auto engine = ext::make_shared<AnalyticRoughHestonEngine>(
        model, 128, 256);

    BOOST_CHECK_THROW(
        engine->priceVanillaPayoff(
            ext::make_shared<PlainVanillaPayoff>(Option::Call, 100.0), 0.0),
        Error);
    BOOST_CHECK_THROW(
        engine->priceVanillaPayoff(
            ext::make_shared<PlainVanillaPayoff>(Option::Call, 100.0), -1.0),
        Error);

    VanillaOption americanOption(
        ext::make_shared<PlainVanillaPayoff>(Option::Call, 100.0),
        ext::make_shared<AmericanExercise>(today, today + Period(1, Years)));
    americanOption.setPricingEngine(engine);
    BOOST_CHECK_THROW(americanOption.NPV(), Error);
}

BOOST_AUTO_TEST_CASE(testShortMaturitySkewExplosion) {
    BOOST_TEST_MESSAGE(
        "Testing rough Heston short-maturity skew explosion...");

    const Date today(2, July, 2026);
    Settings::instance().evaluationDate() = today;
    const DayCounter dc = Actual365Fixed();

    const Handle<YieldTermStructure> rTS(
        ext::make_shared<FlatForward>(today, 0.0, dc));
    const Handle<YieldTermStructure> qTS(
        ext::make_shared<FlatForward>(today, 0.0, dc));
    const Handle<Quote> s0(ext::make_shared<SimpleQuote>(100.0));

    // at-the-money implied volatility skew d(sigma_iv)/d(ln K)
    const auto atmSkew = [](const ext::shared_ptr<AnalyticRoughHestonEngine>& engine,
                            Time t) -> Real {
        const Real dk = 0.02;

        Real iv[2];
        for (Size i = 0; i < 2; ++i) {
            const Real strike = 100.0*std::exp((i == 0 ? -dk : dk));
            const Real price = engine->priceVanillaPayoff(
                ext::make_shared<PlainVanillaPayoff>(Option::Call, strike), t);
            iv[i] = blackFormulaImpliedStdDev(
                Option::Call, strike, 100.0, price)/std::sqrt(t);
        }
        return (iv[1] - iv[0])/(2*dk);
    };

    const Time t1 = 0.1, t2 = 0.4;

    // classical Heston (H = 0.5) reference for the "materially less
    // explosion" comparison below
    const auto flatModel = ext::make_shared<RoughHestonModel>(
        rTS, qTS, s0, 0.04, 0.3, 0.04, 0.4, -0.7, 0.5);
    const auto flatEngine = ext::make_shared<AnalyticRoughHestonEngine>(
        flatModel, 128, 512);
    const Real flatSkewRatio = atmSkew(flatEngine, t1)/atmSkew(flatEngine, t2);

    // El Euch, Fukasawa, Rosenbaum: the ATM skew of rough volatility
    // models explodes like T^(H - 1/2) for T -> 0
    for (const Real hurst : {0.05, 0.1, 0.3}) {
        const auto model = ext::make_shared<RoughHestonModel>(
            rTS, qTS, s0, 0.04, 0.3, 0.04, 0.4, -0.7, hurst);
        const auto engine = ext::make_shared<AnalyticRoughHestonEngine>(
            model, 128, 512);

        const Real skewRatio = atmSkew(engine, t1)/atmSkew(engine, t2);
        const Real expected = std::pow(t1/t2, hurst - 0.5);

        const Real tol = 0.15*expected;
        if (std::fabs(skewRatio - expected) > tol)
            BOOST_ERROR("failed to reproduce short-maturity skew power law"
                        << "\n    hurst:      " << hurst
                        << "\n    skew ratio: " << skewRatio
                        << "\n    expected:   " << expected
                        << "\n    tolerance:  " << tol);

        // classical Heston must show materially less short-maturity
        // skew steepening than any rough case
        if (flatSkewRatio > 0.5*(skewRatio + 1.0))
            BOOST_ERROR("classical limit shows unexpected skew explosion"
                        << "\n    hurst:                 " << hurst
                        << "\n    rough skew ratio:      " << skewRatio
                        << "\n    classical skew ratio:  " << flatSkewRatio);
    }
}

BOOST_AUTO_TEST_CASE(testCalibration) {
    BOOST_TEST_MESSAGE(
        "Testing rough Heston calibration to a synthetic surface...");

    const Date today(2, July, 2026);
    Settings::instance().evaluationDate() = today;
    const DayCounter dc = Actual365Fixed();
    const Calendar calendar = NullCalendar();

    const Handle<YieldTermStructure> rTS(
        ext::make_shared<FlatForward>(today, 0.02, dc));
    const Handle<YieldTermStructure> qTS(
        ext::make_shared<FlatForward>(today, 0.0, dc));
    const Handle<Quote> s0(ext::make_shared<SimpleQuote>(100.0));

    const Real v0    = 0.0225;
    const Real kappa = 0.6;
    const Real theta = 0.04;
    const Real sigma = 0.35;
    const Real rho   = -0.65;
    const Real hurst = 0.12;

    // the time steps must keep the fractional Adams scheme stable up to
    // frequencies where the characteristic function has decayed (u ~ 0.35 N/T)
    const Size integrationOrder = 64, timeSteps = 256;

    // synthetic market surface generated from known parameters
    const auto trueModel = ext::make_shared<RoughHestonModel>(
        rTS, qTS, s0, v0, kappa, theta, sigma, rho, hurst);
    const auto trueEngine = ext::make_shared<AnalyticRoughHestonEngine>(
        trueModel, integrationOrder, timeSteps);

    std::vector<ext::shared_ptr<CalibrationHelper>> helpers;
    for (const Size months : {3, 6, 12, 24}) {
        const Period maturity(months, Months);
        const Date maturityDate = today + maturity;
        const Time t = dc.yearFraction(today, maturityDate);
        const Real fwd = s0->value()*qTS->discount(maturityDate)
            /rTS->discount(maturityDate);
        const DiscountFactor df = rTS->discount(maturityDate);

        for (const Real strike : {80.0, 90.0, 100.0, 110.0, 120.0}) {
            const Real price = trueEngine->priceVanillaPayoff(
                ext::make_shared<PlainVanillaPayoff>(Option::Call, strike),
                maturityDate);
            const Volatility impliedVol = blackFormulaImpliedStdDev(
                Option::Call, strike, fwd, price, df)/std::sqrt(t);

            // implied-vol errors keep the objective well scaled; relative
            // price errors can strand the optimizer in local minima
            helpers.push_back(ext::make_shared<HestonModelHelper>(
                maturity, calendar, s0, strike,
                Handle<Quote>(ext::make_shared<SimpleQuote>(impliedVol)),
                rTS, qTS, BlackCalibrationHelper::ImpliedVolError));
        }
    }

    // calibration starts from deliberately wrong parameters
    const auto model = ext::make_shared<RoughHestonModel>(
        rTS, qTS, s0, 0.04, 1.0, 0.02, 0.25, -0.4, 0.2);
    const auto engine = ext::make_shared<AnalyticRoughHestonEngine>(
        model, integrationOrder, timeSteps);

    for (const auto& helper : helpers)
        ext::static_pointer_cast<BlackCalibrationHelper>(helper)
            ->setPricingEngine(engine);

    LevenbergMarquardt om(1e-8, 1e-8, 1e-8);
    model->calibrate(helpers, om,
                     EndCriteria(400, 40, 1.0e-8, 1.0e-8, 1.0e-8));

    Real sse = 0.0;
    for (const auto& helper : helpers) {
        const Real error =
            ext::static_pointer_cast<BlackCalibrationHelper>(helper)
                ->calibrationError();
        sse += error*error;
    }

    const Real expectedSse = 1e-8;
    if (sse > expectedSse)
        BOOST_ERROR("failed to calibrate rough Heston model to a synthetic "
                    "surface"
                    << "\n    sse:      " << sse
                    << "\n    expected: less than " << expectedSse
                    << "\n    v0:    " << model->v0()    << " vs " << v0
                    << "\n    kappa: " << model->kappa() << " vs " << kappa
                    << "\n    theta: " << model->theta() << " vs " << theta
                    << "\n    sigma: " << model->sigma() << " vs " << sigma
                    << "\n    rho:   " << model->rho()   << " vs " << rho
                    << "\n    hurst: " << model->hurst() << " vs " << hurst);

    // the Hurst exponent controls the skew term structure and should be
    // identified well
    const Real hurstTol = 0.02;
    if (std::fabs(model->hurst() - hurst) > hurstTol)
        BOOST_ERROR("failed to recover the Hurst exponent"
                    << "\n    calculated: " << model->hurst()
                    << "\n    expected:   " << hurst
                    << "\n    tolerance:  " << hurstTol);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
