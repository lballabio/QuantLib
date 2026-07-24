/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2006, 2008 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni
 Copyright (C) 2026 Aaditya Panikath

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
#include "swaptionvolstructuresutilities.hpp"
#include "utilities.hpp"
#include <ql/indexes/swap/euriborswap.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/swaption/interpolatedswaptionvolatilitycube.hpp>
#include <ql/termstructures/volatility/swaption/sabrswaptionvolatilitycube.hpp>
#include <ql/termstructures/volatility/swaption/zabrswaptionvolatilitycube.hpp>
#include <ql/termstructures/volatility/swaption/spreadedswaptionvol.hpp>
#include <ql/termstructures/volatility/sabrsmilesection.hpp>
#include <ql/termstructures/volatility/zabrsmilesection.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(SwaptionVolatilityCubeTests)

struct CommonVars {
    // global data
    SwaptionMarketConventions conventions;
    AtmVolatility atm;
    RelinkableHandle<SwaptionVolatilityStructure> atmVolMatrix;
    RelinkableHandle<SwaptionVolatilityStructure> normalVolMatrix;
    VolatilityCube cube;

    RelinkableHandle<YieldTermStructure> termStructure;

    ext::shared_ptr<SwapIndex> swapIndexBase, shortSwapIndexBase;
    bool vegaWeighedSmileFit;

    // utilities
    std::vector<std::vector<Handle<Quote>>> makeZabrParametersGuess(
            Real alpha, Real beta, Real nu, Real rho, Real gamma) const {
        Size n = cube.tenors.options.size() * cube.tenors.swaps.size();
        std::vector<std::vector<Handle<Quote>>> guess(n);
        for (Size i = 0; i < n; ++i) {
            guess[i] = {
                Handle<Quote>(ext::make_shared<SimpleQuote>(alpha)),
                Handle<Quote>(ext::make_shared<SimpleQuote>(beta)),
                Handle<Quote>(ext::make_shared<SimpleQuote>(nu)),
                Handle<Quote>(ext::make_shared<SimpleQuote>(rho)),
                Handle<Quote>(ext::make_shared<SimpleQuote>(gamma))
            };
        }
        return guess;
    }

    void makeAtmVolTest(const SwaptionVolatilityCube& volCube,
                        Real tolerance) {

        for (auto& option : atm.tenors.options) {
            for (auto& swap : atm.tenors.swaps) {
                Rate strike = volCube.atmStrike(option, swap);
                Volatility expVol = atmVolMatrix->volatility(option, swap, strike, true);
                Volatility actVol = volCube.volatility(option, swap, strike, true);
                Volatility error = std::abs(expVol - actVol);
                if (error > tolerance)
                    BOOST_ERROR("\nrecovery of atm vols failed:"
                                "\nexpiry time = "
                                << option << "\nswap length = " << swap
                                << "\n atm strike = " << io::rate(strike)
                                << "\n   exp. vol = " << io::volatility(expVol)
                                << "\n actual vol = " << io::volatility(actVol)
                                << "\n      error = " << io::volatility(error)
                                << "\n  tolerance = " << tolerance);
            }
        }
    }

    void makeVolSpreadsTest(const SwaptionVolatilityCube& volCube,
                            Real tolerance) {

        for (Size i=0; i<cube.tenors.options.size(); i++) {
            for (Size j=0; j<cube.tenors.swaps.size(); j++) {
                for (Size k=0; k<cube.strikeSpreads.size(); k++) {
                    Rate atmStrike = volCube.atmStrike(cube.tenors.options[i],
                                                       cube.tenors.swaps[j]);
                    Volatility atmVol =
                        atmVolMatrix->volatility(cube.tenors.options[i],
                                                 cube.tenors.swaps[j],
                                                 atmStrike, true);
                    Volatility vol =
                        volCube.volatility(cube.tenors.options[i],
                                           cube.tenors.swaps[j],
                                           atmStrike+cube.strikeSpreads[k], true);
                    Volatility spread = vol-atmVol;
                    Volatility expVolSpread =
                        cube.volSpreads[i*cube.tenors.swaps.size()+j][k];
                    Volatility error = std::abs(expVolSpread-spread);
                    if (error>tolerance)
                        BOOST_FAIL("\nrecovery of smile vol spreads failed:"
                                   "\n    option tenor = " << cube.tenors.options[i] <<
                                   "\n      swap tenor = " << cube.tenors.swaps[j] <<
                                   "\n      atm strike = " << io::rate(atmStrike) <<
                                   "\n   strike spread = " << io::rate(cube.strikeSpreads[k]) <<
                                   "\n         atm vol = " << io::volatility(atmVol) <<
                                   "\n      smiled vol = " << io::volatility(vol) <<
                                   "\n      vol spread = " << io::volatility(spread) <<
                                   "\n exp. vol spread = " << io::volatility(expVolSpread) <<
                                   "\n           error = " << io::volatility(error) <<
                                   "\n       tolerance = " << tolerance);
                }
            }
        }
    }

    CommonVars() {

        conventions.setConventions();

        // ATM swaptionvolmatrix
        atm.setMarketData();

        atmVolMatrix = RelinkableHandle<SwaptionVolatilityStructure>(
                ext::shared_ptr<SwaptionVolatilityStructure>(new
                    SwaptionVolatilityMatrix(conventions.calendar,
                                             conventions.optionBdc,
                                             atm.tenors.options,
                                             atm.tenors.swaps,
                                             atm.volsHandle,
                                             conventions.dayCounter)));

        normalVolMatrix = RelinkableHandle<SwaptionVolatilityStructure>(
                ext::shared_ptr<SwaptionVolatilityStructure>(new SwaptionVolatilityMatrix(
                    conventions.calendar, conventions.optionBdc, atm.tenors.options,
                    atm.tenors.swaps, atm.volsHandle, conventions.dayCounter, false, VolatilityType::Normal)));

        // Swaptionvolcube
        cube.setMarketData();

        termStructure.linkTo(flatRate(0.05, Actual365Fixed()));

        swapIndexBase = ext::shared_ptr<SwapIndex>(new
                EuriborSwapIsdaFixA(2*Years, termStructure));
        shortSwapIndexBase = ext::shared_ptr<SwapIndex>(new
                EuriborSwapIsdaFixA(1*Years, termStructure));

        vegaWeighedSmileFit=false;
    }
};


BOOST_AUTO_TEST_CASE(testSabrNormalVolatility) {

    BOOST_TEST_MESSAGE("Testing sabr normal volatility...");

    CommonVars vars;

    std::vector<std::vector<Handle<Quote> > > parametersGuess(vars.cube.tenors.options.size() *
                                                              vars.cube.tenors.swaps.size());
    for (Size i = 0; i < vars.cube.tenors.options.size() * vars.cube.tenors.swaps.size(); i++) {
        parametersGuess[i] = std::vector<Handle<Quote> >(4);
        parametersGuess[i][0] = Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.2)));
        parametersGuess[i][1] = Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.5)));
        parametersGuess[i][2] = Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.4)));
        parametersGuess[i][3] = Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.0)));
    }
    std::vector<bool> isParameterFixed(4, false);

    SabrSwaptionVolatilityCube volCube(vars.normalVolMatrix, vars.cube.tenors.options, vars.cube.tenors.swaps,
                             vars.cube.strikeSpreads, vars.cube.volSpreadsHandle,
                             vars.swapIndexBase, vars.shortSwapIndexBase, vars.vegaWeighedSmileFit,
                             parametersGuess, isParameterFixed, true);
    Real tolerance = 7.0e-4;
    vars.makeAtmVolTest(volCube, tolerance);
}

BOOST_AUTO_TEST_CASE(testSabrNormalTwoPassUsesStrikeSpreadMoneyness) {

    BOOST_TEST_MESSAGE("Testing K-F interpolation in a two-pass normal SABR cube...");

    CommonVars vars;
    const Date today = Settings::instance().evaluationDate();
    const std::vector<Date> dates = {
        today,
        vars.conventions.calendar.advance(today, 1 * Years),
        vars.conventions.calendar.advance(today, 5 * Years),
        vars.conventions.calendar.advance(today, 15 * Years),
        vars.conventions.calendar.advance(today, 70 * Years)
    };
    const std::vector<Rate> zeroRates = { 0.01, 0.015, 0.025, 0.035, 0.045 };
    RelinkableHandle<YieldTermStructure> termStructure;
    termStructure.linkTo(ext::make_shared<ZeroCurve>(
        dates, zeroRates, Actual365Fixed(), vars.conventions.calendar));

    const auto swapIndex = ext::make_shared<EuriborSwapIsdaFixA>(2 * Years, termStructure);
    const auto shortSwapIndex = ext::make_shared<EuriborSwapIsdaFixA>(1 * Years, termStructure);
    const Size sections = vars.cube.tenors.options.size() * vars.cube.tenors.swaps.size();
    Matrix normalVols = 0.05 * vars.atm.vols;
    Handle<SwaptionVolatilityStructure> normalAtm(
        ext::make_shared<SwaptionVolatilityMatrix>(
            vars.conventions.calendar, vars.conventions.optionBdc,
            vars.atm.tenors.options, vars.atm.tenors.swaps, normalVols,
            vars.conventions.dayCounter, false, VolatilityType::Normal));
    std::vector<std::vector<Handle<Quote>>> volSpreads(
        sections, std::vector<Handle<Quote>>(vars.cube.strikeSpreads.size()));
    for (Size section = 0; section < sections; ++section) {
        for (Size strike = 0; strike < vars.cube.strikeSpreads.size(); ++strike) {
            volSpreads[section][strike] = Handle<Quote>(ext::make_shared<SimpleQuote>(
                0.03 * vars.cube.volSpreads[section][strike]));
        }
    }
    std::vector<std::vector<Handle<Quote>>> parametersGuess(
        sections, std::vector<Handle<Quote>>(4));
    for (auto& guess : parametersGuess) {
        guess[0] = Handle<Quote>(ext::make_shared<SimpleQuote>(0.05));
        guess[1] = Handle<Quote>(ext::make_shared<SimpleQuote>(0.5));
        guess[2] = Handle<Quote>(ext::make_shared<SimpleQuote>(0.4));
        guess[3] = Handle<Quote>(ext::make_shared<SimpleQuote>(0.0));
    }

    SabrSwaptionVolatilityCube volCube(
        normalAtm, vars.cube.tenors.options, vars.cube.tenors.swaps,
        vars.cube.strikeSpreads, volSpreads, swapIndex,
        shortSwapIndex, false, parametersGuess,
        { false, true, false, false }, true);

    const Period targetOption = 5 * Years;
    const Period targetSwap = 5 * Years;
    const Time targetTime = normalAtm->timeFromReference(
        normalAtm->optionDateFromTenor(targetOption));
    const Time targetLength = normalAtm->swapLength(targetSwap);
    const Rate targetForward = volCube.atmStrike(targetOption, targetSwap);
    const Spread targetStrikeSpread = 0.02;
    const Rate targetStrike = targetForward + targetStrikeSpread;

    const Matrix sparseParameters = volCube.sparseSabrParameters();
    const auto findRow = [](const Matrix& matrix, Time optionTime, Time swapLength) -> Size {
        for (Size row = 0; row < matrix.rows(); ++row) {
            if (close_enough(matrix[row][0], swapLength) &&
                close_enough(matrix[row][1], optionTime))
                return row;
        }
        QL_FAIL("parameter row not found");
    };
    const std::vector<Time> optionNodes = {
        normalAtm->timeFromReference(normalAtm->optionDateFromTenor(1 * Years)),
        normalAtm->timeFromReference(normalAtm->optionDateFromTenor(10 * Years))
    };
    const std::vector<Time> swapNodes = {
        normalAtm->swapLength(2 * Years),
        normalAtm->swapLength(10 * Years)
    };

    const auto interpolatedSpread = [&](bool ratioMoneyness) {
        Matrix cornerSpreads(2, 2, 0.0);
        for (Size i = 0; i < 2; ++i) {
            for (Size j = 0; j < 2; ++j) {
                const Size row = findRow(sparseParameters, optionNodes[i], swapNodes[j]);
                const Rate sourceForward = sparseParameters[row][6];
                const std::vector<Real> parameters = {
                    sparseParameters[row][2], sparseParameters[row][3],
                    sparseParameters[row][4], sparseParameters[row][5]
                };
                SabrSmileSection smile(optionNodes[i], sourceForward, parameters,
                                       0.0, VolatilityType::Normal);
                const Rate sourceStrike = ratioMoneyness ?
                    sourceForward * targetStrike / targetForward :
                    sourceForward + targetStrikeSpread;
                const Volatility sourceAtm = normalAtm->volatility(
                    optionNodes[i], swapNodes[j], sourceForward, true);
                cornerSpreads[i][j] = smile.volatility(sourceStrike) - sourceAtm;
            }
        }
        const Real optionWeight = (targetTime - optionNodes[0]) /
                                  (optionNodes[1] - optionNodes[0]);
        const Real swapWeight = (targetLength - swapNodes[0]) /
                                (swapNodes[1] - swapNodes[0]);
        return (1.0 - optionWeight) *
                   ((1.0 - swapWeight) * cornerSpreads[0][0] +
                    swapWeight * cornerSpreads[0][1]) +
               optionWeight *
                   ((1.0 - swapWeight) * cornerSpreads[1][0] +
                    swapWeight * cornerSpreads[1][1]);
    };

    const Matrix denseMarketVols = volCube.volCubeAtmCalibrated();
    const Size targetRow = findRow(denseMarketVols, targetTime, targetLength);
    const Volatility targetAtm = normalAtm->volatility(
        targetTime, targetLength, targetForward, true);
    const Volatility actual = denseMarketVols[targetRow][6];
    const Volatility expected = targetAtm + interpolatedSpread(false);
    const Volatility oldRatioResult = targetAtm + interpolatedSpread(true);

    BOOST_CHECK_SMALL(actual - expected, 1.0e-12);
    BOOST_CHECK_GT(std::fabs(actual - oldRatioResult), 1.0e-8);
}

BOOST_AUTO_TEST_CASE(testSabrAtmCalibrationWithAsymmetricSmile) {

    BOOST_TEST_MESSAGE("Testing exact SABR ATM calibration with asymmetric smiles...");

    CommonVars vars;

    const auto checkAtmRecovery = [&](const Handle<SwaptionVolatilityStructure>& atmVols,
                                      const std::vector<Volatility>& smileSpreads,
                                      Real alphaGuess) {
        const Size sections = vars.cube.tenors.options.size() * vars.cube.tenors.swaps.size();
        std::vector<std::vector<Handle<Quote>>> volSpreads(
            sections, std::vector<Handle<Quote>>(smileSpreads.size()));
        for (auto& section : volSpreads) {
            for (Size i = 0; i < smileSpreads.size(); ++i)
                section[i] = Handle<Quote>(ext::make_shared<SimpleQuote>(smileSpreads[i]));
        }

        std::vector<std::vector<Handle<Quote>>> parametersGuess(
            sections, std::vector<Handle<Quote>>(4));
        for (auto& guess : parametersGuess) {
            guess[0] = Handle<Quote>(ext::make_shared<SimpleQuote>(alphaGuess));
            guess[1] = Handle<Quote>(ext::make_shared<SimpleQuote>(0.5));
            guess[2] = Handle<Quote>(ext::make_shared<SimpleQuote>(0.4));
            guess[3] = Handle<Quote>(ext::make_shared<SimpleQuote>(0.0));
        }

        SabrSwaptionVolatilityCube volCube(
            atmVols, vars.cube.tenors.options, vars.cube.tenors.swaps,
            vars.cube.strikeSpreads, volSpreads, vars.swapIndexBase,
            vars.shortSwapIndexBase, false, parametersGuess,
            { false, true, false, false }, true,
            {}, Null<Real>(), {}, Null<Real>(), false, 50, false, 0.0001, true);

        for (const auto& option : vars.cube.tenors.options) {
            for (const auto& swap : vars.cube.tenors.swaps) {
                const Rate strike = volCube.atmStrike(option, swap);
                const Volatility expected = atmVols->volatility(option, swap, strike, true);
                const Volatility calculated = volCube.volatility(option, swap, strike, true);
                BOOST_CHECK_SMALL(calculated - expected, 1.0e-10);
                const auto smile = ext::dynamic_pointer_cast<SabrSmileSection>(
                    volCube.smileSection(option, swap));
                BOOST_REQUIRE(smile);
                BOOST_CHECK_SMALL(smile->beta() - 0.5, 1.0e-12);
            }
        }

        const Matrix parameters = volCube.denseSabrParameters();
        const Matrix marketVols = volCube.volCubeAtmCalibrated();
        bool hasRatioMoneynessAdjustment = false;
        for (Size row = 0; row < parameters.rows(); ++row) {
            const Time optionTime = parameters[row][1];
            const Time swapLength = parameters[row][0];
            const Rate forward = parameters[row][6];
            const std::vector<Real> sabrParameters = {
                parameters[row][2], parameters[row][3],
                parameters[row][4], parameters[row][5]
            };
            SabrSmileSection smile(
                optionTime, forward, sabrParameters,
                atmVols->shift(optionTime, swapLength), atmVols->volatilityType());

            Real squaredError = 0.0;
            Real maxError = 0.0;
            for (Size i = 0; i < vars.cube.strikeSpreads.size(); ++i) {
                const Real error = smile.volatility(forward + vars.cube.strikeSpreads[i]) -
                                   marketVols[row][i + 2];
                squaredError += error * error;
                maxError = std::max(maxError, std::fabs(error));
            }
            const Size n = vars.cube.strikeSpreads.size();
            const Real rmsError = std::sqrt(squaredError / (n - 1));
            BOOST_CHECK_SMALL(parameters[row][7] - rmsError, 1.0e-12);
            BOOST_CHECK_SMALL(parameters[row][8] - maxError, 1.0e-12);

            if (atmVols->volatilityType() == VolatilityType::Normal) {
                const Volatility atmVol = atmVols->volatility(
                    optionTime, swapLength, forward, true);
                for (Size i = 0; i < smileSpreads.size(); ++i) {
                    BOOST_CHECK_SMALL(
                        marketVols[row][i + 2] - atmVol - smileSpreads[i],
                        1.0e-12);
                }
            } else {
                const Volatility atmVol = atmVols->volatility(
                    optionTime, swapLength, forward, true);
                for (Size i = 0; i < smileSpreads.size(); ++i) {
                    hasRatioMoneynessAdjustment = hasRatioMoneynessAdjustment ||
                        std::fabs(marketVols[row][i + 2] - atmVol - smileSpreads[i]) >
                            1.0e-10;
                }
            }
        }
        if (atmVols->volatilityType() == VolatilityType::ShiftedLognormal)
            BOOST_CHECK(hasRatioMoneynessAdjustment);

        SabrSwaptionVolatilityCube twoPassCube(
            atmVols, vars.cube.tenors.options, vars.cube.tenors.swaps,
            vars.cube.strikeSpreads, volSpreads, vars.swapIndexBase,
            vars.shortSwapIndexBase, false, parametersGuess,
            { false, true, false, false }, true);
        bool hasUnanchoredAtmNode = false;
        for (const auto& option : vars.cube.tenors.options) {
            for (const auto& swap : vars.cube.tenors.swaps) {
                const Rate strike = twoPassCube.atmStrike(option, swap);
                const Volatility expected = atmVols->volatility(option, swap, strike, true);
                const Volatility calculated = twoPassCube.volatility(
                    option, swap, strike, true);
                hasUnanchoredAtmNode = hasUnanchoredAtmNode ||
                    std::fabs(calculated - expected) > 1.0e-8;
            }
        }
        BOOST_CHECK(hasUnanchoredAtmNode);

        SabrSwaptionVolatilityCube unanchoredSinglePassCube(
            atmVols, vars.cube.tenors.options, vars.cube.tenors.swaps,
            vars.cube.strikeSpreads, volSpreads, vars.swapIndexBase,
            vars.shortSwapIndexBase, false, parametersGuess,
            { false, true, false, false }, false,
            {}, Null<Real>(), {}, Null<Real>(), false, 50, false, 0.0001, true);
        bool singlePassHasUnanchoredAtmNode = false;
        for (const auto& option : vars.cube.tenors.options) {
            for (const auto& swap : vars.cube.tenors.swaps) {
                const Rate strike = unanchoredSinglePassCube.atmStrike(option, swap);
                const Volatility expected = atmVols->volatility(option, swap, strike, true);
                const Volatility calculated = unanchoredSinglePassCube.volatility(
                    option, swap, strike, true);
                singlePassHasUnanchoredAtmNode = singlePassHasUnanchoredAtmNode ||
                    std::fabs(calculated - expected) > 1.0e-8;
            }
        }
        BOOST_CHECK(singlePassHasUnanchoredAtmNode);

        if (atmVols->volatilityType() == VolatilityType::ShiftedLognormal) {
            const Matrix calibrationParameters =
                unanchoredSinglePassCube.denseSabrParameters();
            Real largestCalibrationError = 0.0;
            Real largestAnchoredError = 0.0;
            for (Size row = 0; row < parameters.rows(); ++row) {
                largestCalibrationError = std::max(
                    largestCalibrationError, calibrationParameters[row][7]);
                largestAnchoredError = std::max(
                    largestAnchoredError, parameters[row][7]);
            }
            BOOST_REQUIRE_GT(largestAnchoredError, largestCalibrationError);
            const Real tolerance =
                0.5 * (largestCalibrationError + largestAnchoredError);
            BOOST_CHECK_NO_THROW(([&] {
                SabrSwaptionVolatilityCube toleranceCube(
                    atmVols, vars.cube.tenors.options, vars.cube.tenors.swaps,
                    vars.cube.strikeSpreads, volSpreads, vars.swapIndexBase,
                    vars.shortSwapIndexBase, false, parametersGuess,
                    { false, true, false, false }, true,
                    {}, tolerance, {}, 0.0020, false, 50, false, 0.0001, true);
                toleranceCube.denseSabrParameters();
            }()));
        }
    };

    Matrix shifts(vars.atm.tenors.options.size(), vars.atm.tenors.swaps.size());
    for (Size i = 0; i < shifts.rows(); ++i) {
        for (Size j = 0; j < shifts.columns(); ++j)
            shifts[i][j] = 0.001 * (i + 2 * j);
    }
    Handle<SwaptionVolatilityStructure> shiftedLognormalAtm(
        ext::make_shared<SwaptionVolatilityMatrix>(
            vars.conventions.calendar, vars.conventions.optionBdc,
            vars.atm.tenors.options, vars.atm.tenors.swaps, vars.atm.vols,
            vars.conventions.dayCounter, false, VolatilityType::ShiftedLognormal,
            shifts));
    checkAtmRecovery(
        shiftedLognormalAtm, { 0.045, 0.015, 0.0, 0.010, 0.030 }, 0.05);

    std::vector<std::vector<Handle<Quote>>> normalAtmVols(vars.atm.tenors.options.size());
    for (Size i = 0; i < normalAtmVols.size(); ++i) {
        normalAtmVols[i].resize(vars.atm.tenors.swaps.size());
        for (Size j = 0; j < normalAtmVols[i].size(); ++j) {
            normalAtmVols[i][j] = Handle<Quote>(
                ext::make_shared<SimpleQuote>(0.05 * vars.atm.vols[i][j]));
        }
    }
    Handle<SwaptionVolatilityStructure> normalAtm(
        ext::make_shared<SwaptionVolatilityMatrix>(
            vars.conventions.calendar, vars.conventions.optionBdc,
            vars.atm.tenors.options, vars.atm.tenors.swaps, normalAtmVols,
            vars.conventions.dayCounter, false, VolatilityType::Normal));
    checkAtmRecovery(normalAtm, { 0.00135, 0.00045, 0.0, 0.00030, 0.00090 }, 0.05);
}

// SwaptionVolCubeByLinear reproduces ATM vol with machine precision
BOOST_AUTO_TEST_CASE(testAtmVols) {

    BOOST_TEST_MESSAGE("Testing swaption volatility cube (atm vols)...");

    CommonVars vars;

    InterpolatedSwaptionVolatilityCube volCube(vars.atmVolMatrix,
                             vars.cube.tenors.options,
                             vars.cube.tenors.swaps,
                             vars.cube.strikeSpreads,
                             vars.cube.volSpreadsHandle,
                             vars.swapIndexBase,
                             vars.shortSwapIndexBase,
                             vars.vegaWeighedSmileFit);

    Real tolerance = 1.0e-16;
    vars.makeAtmVolTest(volCube, tolerance);
}

// SwaptionVolCubeByLinear reproduces smile spreads with machine precision
BOOST_AUTO_TEST_CASE(testSmile) {

    BOOST_TEST_MESSAGE("Testing swaption volatility cube (smile)...");

    CommonVars vars;

    InterpolatedSwaptionVolatilityCube volCube(vars.atmVolMatrix,
                             vars.cube.tenors.options,
                             vars.cube.tenors.swaps,
                             vars.cube.strikeSpreads,
                             vars.cube.volSpreadsHandle,
                             vars.swapIndexBase,
                             vars.shortSwapIndexBase,
                             vars.vegaWeighedSmileFit);

    Real tolerance = 1.0e-16;
    vars.makeVolSpreadsTest(volCube, tolerance);
}

// SwaptionVolCubeBySabr reproduces ATM vol with given tolerance
// SwaptionVolCubeBySabr reproduces smile spreads with given tolerance
BOOST_AUTO_TEST_CASE(testSabrVols) {

    BOOST_TEST_MESSAGE("Testing swaption volatility cube (sabr interpolation)...");

    CommonVars vars;

    std::vector<std::vector<Handle<Quote> > >
        parametersGuess(vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size());
    for (Size i=0; i<vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size(); i++) {
        parametersGuess[i] = std::vector<Handle<Quote> >(4);
        parametersGuess[i][0] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.2)));
        parametersGuess[i][1] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.5)));
        parametersGuess[i][2] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.4)));
        parametersGuess[i][3] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.0)));
    }
    std::vector<bool> isParameterFixed(4, false);

    SabrSwaptionVolatilityCube volCube(vars.atmVolMatrix,
                             vars.cube.tenors.options,
                             vars.cube.tenors.swaps,
                             vars.cube.strikeSpreads,
                             vars.cube.volSpreadsHandle,
                             vars.swapIndexBase,
                             vars.shortSwapIndexBase,
                             vars.vegaWeighedSmileFit,
                             parametersGuess,
                             isParameterFixed,
                             true);
    Real tolerance = 3.0e-4;
    vars.makeAtmVolTest(volCube, tolerance);

    tolerance = 12.0e-4;
    vars.makeVolSpreadsTest(volCube, tolerance);
}

BOOST_AUTO_TEST_CASE(testSpreadedCube) {

    BOOST_TEST_MESSAGE("Testing spreaded swaption volatility cube...");

    CommonVars vars;

    std::vector<std::vector<Handle<Quote> > >
        parametersGuess(vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size());
    for (Size i=0; i<vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size(); i++) {
        parametersGuess[i] = std::vector<Handle<Quote> >(4);
        parametersGuess[i][0] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.2)));
        parametersGuess[i][1] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.5)));
        parametersGuess[i][2] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.4)));
        parametersGuess[i][3] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.0)));
    }
    std::vector<bool> isParameterFixed(4, false);

    Handle<SwaptionVolatilityStructure> volCube( ext::shared_ptr<SwaptionVolatilityStructure>(new
        SabrSwaptionVolatilityCube(vars.atmVolMatrix,
                         vars.cube.tenors.options,
                         vars.cube.tenors.swaps,
                         vars.cube.strikeSpreads,
                         vars.cube.volSpreadsHandle,
                         vars.swapIndexBase,
                         vars.shortSwapIndexBase,
                         vars.vegaWeighedSmileFit,
                         parametersGuess,
                         isParameterFixed,
                         true)));

    ext::shared_ptr<SimpleQuote> spread (new SimpleQuote(0.0001));
    Handle<Quote> spreadHandle(spread);
    ext::shared_ptr<SwaptionVolatilityStructure> spreadedVolCube(new
        SpreadedSwaptionVolatility(volCube, spreadHandle));
    std::vector<Real> strikes;
    for (Size k=1; k<100; k++)
        strikes.push_back(k*.01);
    for (Size i=0; i<vars.cube.tenors.options.size(); i++) {
        for (Size j=0; j<vars.cube.tenors.swaps.size(); j++) {
            ext::shared_ptr<SmileSection> smileSectionByCube =
                volCube->smileSection(vars.cube.tenors.options[i], vars.cube.tenors.swaps[j]);
            ext::shared_ptr<SmileSection> smileSectionBySpreadedCube =
                spreadedVolCube->smileSection(vars.cube.tenors.options[i], vars.cube.tenors.swaps[j]);
            for (Real strike : strikes) {
                Real diff = spreadedVolCube->volatility(vars.cube.tenors.options[i],
                                                        vars.cube.tenors.swaps[j], strike) -
                            volCube->volatility(vars.cube.tenors.options[i],
                                                vars.cube.tenors.swaps[j], strike);
                if (std::fabs(diff-spread->value())>1e-16)
                    BOOST_ERROR("\ndiff!=spread in volatility method:"
                                "\nexpiry time = " << vars.cube.tenors.options[i] <<
                                "\nswap length = " << vars.cube.tenors.swaps[j] <<
                                "\n atm strike = " << io::rate(strike) <<
                                "\ndiff = " << diff <<
                                "\nspread = " << spread->value());

                diff = smileSectionBySpreadedCube->volatility(strike)
                       - smileSectionByCube->volatility(strike);
                if (std::fabs(diff-spread->value())>1e-16)
                    BOOST_ERROR("\ndiff!=spread in smile section method:"
                                "\nexpiry time = " << vars.cube.tenors.options[i] <<
                                "\nswap length = " << vars.cube.tenors.swaps[j] <<
                                "\n atm strike = " << io::rate(strike) <<
                                "\ndiff = " << diff <<
                                "\nspread = " << spread->value());
            }
        }
    }

    //testing observability
    Flag f;
    f.registerWith(spreadedVolCube);
    volCube->update();
    if(!f.isUp())
        BOOST_ERROR("SpreadedSwaptionVolatilityStructure "
                    << "does not propagate notifications");
    f.lower();
    spread->setValue(.001);
    if(!f.isUp())
        BOOST_ERROR("SpreadedSwaptionVolatilityStructure "
                    << "does not propagate notifications");
}

BOOST_AUTO_TEST_CASE(testObservability) {
    BOOST_TEST_MESSAGE("Testing volatility cube observability...");

    CommonVars vars;

    std::vector<std::vector<Handle<Quote> > >
        parametersGuess(vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size());
    for (Size i=0; i<vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size(); i++) {
        parametersGuess[i] = std::vector<Handle<Quote> >(4);
        parametersGuess[i][0] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.2)));
        parametersGuess[i][1] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.5)));
        parametersGuess[i][2] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.4)));
        parametersGuess[i][3] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.0)));
    }
    std::vector<bool> isParameterFixed(4, false);

    std::string description;
    ext::shared_ptr<SabrSwaptionVolatilityCube> volCube1_0, volCube1_1;
    // VolCube created before change of reference date
    volCube1_0 = ext::make_shared<SabrSwaptionVolatilityCube>(vars.atmVolMatrix,
                                                                vars.cube.tenors.options,
                                                                vars.cube.tenors.swaps,
                                                                vars.cube.strikeSpreads,
                                                                vars.cube.volSpreadsHandle,
                                                                vars.swapIndexBase,
                                                                vars.shortSwapIndexBase,
                                                                vars.vegaWeighedSmileFit,
                                                                parametersGuess,
                                                                isParameterFixed,
                                                                true);

    Date referenceDate = Settings::instance().evaluationDate();
    Settings::instance().evaluationDate() =
        vars.conventions.calendar.advance(referenceDate, Period(1, Days),
                                          vars.conventions.optionBdc);

    // VolCube created after change of reference date
    volCube1_1 = ext::make_shared<SabrSwaptionVolatilityCube>(vars.atmVolMatrix,
                                                                vars.cube.tenors.options,
                                                                vars.cube.tenors.swaps,
                                                                vars.cube.strikeSpreads,
                                                                vars.cube.volSpreadsHandle,
                                                                vars.swapIndexBase,
                                                                vars.shortSwapIndexBase,
                                                                vars.vegaWeighedSmileFit,
                                                                parametersGuess,
                                                                isParameterFixed,
                                                                true);
    Rate dummyStrike = 0.03;
    for (Size i=0;i<vars.cube.tenors.options.size(); i++ ) {
        for (Size j=0; j<vars.cube.tenors.swaps.size(); j++) {
            for (Size k=0; k<vars.cube.strikeSpreads.size(); k++) {

                Volatility v0 = volCube1_0->volatility(vars.cube.tenors.options[i],
                                                       vars.cube.tenors.swaps[j],
                                                       dummyStrike + vars.cube.strikeSpreads[k],
                                                       false);
                Volatility v1 = volCube1_1->volatility(vars.cube.tenors.options[i],
                                                       vars.cube.tenors.swaps[j],
                                                       dummyStrike + vars.cube.strikeSpreads[k],
                                                       false);
                if (std::fabs(v0 - v1) > 1e-14)
                    BOOST_ERROR(description <<
                                " option tenor = " << vars.cube.tenors.options[i] <<
                                " swap tenor = " << vars.cube.tenors.swaps[j] <<
                                " strike = " << io::rate(dummyStrike+vars.cube.strikeSpreads[k])<<
                                "  v0 = " << io::volatility(v0) <<
                                "  v1 = " << io::volatility(v1) <<
                                "  error = " << std::fabs(v1-v0));
            }
        }
    }

    Settings::instance().evaluationDate() = referenceDate;

    ext::shared_ptr<InterpolatedSwaptionVolatilityCube> volCube2_0, volCube2_1;
    // VolCube created before change of reference date
    volCube2_0 = ext::make_shared<InterpolatedSwaptionVolatilityCube>(vars.atmVolMatrix,
                                                                vars.cube.tenors.options,
                                                                vars.cube.tenors.swaps,
                                                                vars.cube.strikeSpreads,
                                                                vars.cube.volSpreadsHandle,
                                                                vars.swapIndexBase,
                                                                vars.shortSwapIndexBase,
                                                                vars.vegaWeighedSmileFit);
    Settings::instance().evaluationDate() =
        vars.conventions.calendar.advance(referenceDate, Period(1, Days),
                                          vars.conventions.optionBdc);

    // VolCube created after change of reference date
    volCube2_1 = ext::make_shared<InterpolatedSwaptionVolatilityCube>(vars.atmVolMatrix,
                                                                vars.cube.tenors.options,
                                                                vars.cube.tenors.swaps,
                                                                vars.cube.strikeSpreads,
                                                                vars.cube.volSpreadsHandle,
                                                                vars.swapIndexBase,
                                                                vars.shortSwapIndexBase,
                                                                vars.vegaWeighedSmileFit);

    for (Size i=0;i<vars.cube.tenors.options.size(); i++ ) {
        for (Size j=0; j<vars.cube.tenors.swaps.size(); j++) {
            for (Size k=0; k<vars.cube.strikeSpreads.size(); k++) {

                Volatility v0 = volCube2_0->volatility(vars.cube.tenors.options[i],
                                                       vars.cube.tenors.swaps[j],
                                                       dummyStrike + vars.cube.strikeSpreads[k],
                                                       false);
                Volatility v1 = volCube2_1->volatility(vars.cube.tenors.options[i],
                                                       vars.cube.tenors.swaps[j],
                                                       dummyStrike + vars.cube.strikeSpreads[k],
                                                       false);
                if (std::fabs(v0 - v1) > 1e-14)
                    BOOST_ERROR(description <<
                                " option tenor = " << vars.cube.tenors.options[i] <<
                                " swap tenor = " << vars.cube.tenors.swaps[j] <<
                                " strike = " << io::rate(dummyStrike+vars.cube.strikeSpreads[k])<<
                                "  v0 = " << io::volatility(v0) <<
                                "  v1 = " << io::volatility(v1) <<
                                "  error = " << std::fabs(v1-v0));
            }
        }
    }

    Settings::instance().evaluationDate() = referenceDate;
}

BOOST_AUTO_TEST_CASE(testSabrParameters) {
    BOOST_TEST_MESSAGE("Testing interpolation of SABR smile sections...");

    CommonVars vars;

    std::vector<std::vector<Handle<Quote> > >
        parametersGuess(vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size());
    for (Size i=0; i<vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size(); i++) {
        parametersGuess[i] = std::vector<Handle<Quote> >(4);
        parametersGuess[i][0] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.2)));
        parametersGuess[i][1] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.5)));
        parametersGuess[i][2] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.4)));
        parametersGuess[i][3] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.0)));
    }
    std::vector<bool> isParameterFixed(4, false);

    SabrSwaptionVolatilityCube volCube(vars.atmVolMatrix,
                             vars.cube.tenors.options,
                             vars.cube.tenors.swaps,
                             vars.cube.strikeSpreads,
                             vars.cube.volSpreadsHandle,
                             vars.swapIndexBase,
                             vars.shortSwapIndexBase,
                             vars.vegaWeighedSmileFit,
                             parametersGuess,
                             isParameterFixed,
                             true);

    SwaptionVolatilityStructure* volStructure = &volCube;
    Real tolerance = 1.0e-4;

    //Interpolating between two SmileSection objects

    //First section: maturity = 10Y, tenor = 2Y
    ext::shared_ptr<SmileSection> smileSection1 = volStructure->smileSection(Period(10,Years), Period(2,Years));

    //Second section: maturity = 10Y, tenor = 4Y
    ext::shared_ptr<SmileSection> smileSection2 = volStructure->smileSection(Period(10,Years), Period(4,Years));

    //Third section in the middle: maturity = 10Y, tenor = 3Y
    ext::shared_ptr<SmileSection> smileSection3 = volStructure->smileSection(Period(10,Years), Period(3,Years));

    //test alpha interpolation
    Real alpha1 = ext::dynamic_pointer_cast<SabrSmileSection>(smileSection1)->alpha();
    Real alpha2 = ext::dynamic_pointer_cast<SabrSmileSection>(smileSection2)->alpha();
    Real alpha3 = ext::dynamic_pointer_cast<SabrSmileSection>(smileSection3)->alpha();
    Real alpha12 = 0.5*(alpha1+alpha2);
    if (std::abs(alpha3 - alpha12) > tolerance) {
             BOOST_ERROR("\nChecking interpolation of alpha parameters:"
                         "\nexpected = " << alpha12 <<
                         "\nobserved = " << alpha3);
    }

    //test beta interpolation
     Real beta1 = ext::dynamic_pointer_cast<SabrSmileSection>(smileSection1)->beta();
     Real beta2 = ext::dynamic_pointer_cast<SabrSmileSection>(smileSection2)->beta();
     Real beta3 = ext::dynamic_pointer_cast<SabrSmileSection>(smileSection3)->beta();
     Real beta12 = 0.5*(beta1+beta2);
     if (std::abs(beta3 - beta12) > tolerance) {
              BOOST_ERROR("\nChecking interpolation of beta parameters:"
                          "\nexpected = " << beta12 <<
                          "\nobserved = " << beta3);
     }

     //test rho interpolation
       Real rho1 = ext::dynamic_pointer_cast<SabrSmileSection>(smileSection1)->rho();
       Real rho2 = ext::dynamic_pointer_cast<SabrSmileSection>(smileSection2)->rho();
       Real rho3 = ext::dynamic_pointer_cast<SabrSmileSection>(smileSection3)->rho();
       Real rho12 = 0.5*(rho1+rho2);
       if (std::abs(rho3 - rho12) > tolerance) {
                BOOST_ERROR("\nChecking interpolation of rho parameters:"
                            "\nexpected = " << rho12 <<
                            "\nobserved = " << rho3);
       }

       //test nu interpolation
         Real nu1 = ext::dynamic_pointer_cast<SabrSmileSection>(smileSection1)->nu();
         Real nu2 = ext::dynamic_pointer_cast<SabrSmileSection>(smileSection2)->nu();
         Real nu3 = ext::dynamic_pointer_cast<SabrSmileSection>(smileSection3)->nu();
         Real nu12 = 0.5*(nu1+nu2);
         if (std::abs(nu3 - nu12) > tolerance) {
                  BOOST_ERROR("\nChecking interpolation of nu parameters:"
                              "\nexpected = " << nu12 <<
                              "\nobserved = " << nu3);
         }

         //test forward interpolation
           Real forward1 = smileSection1->atmLevel();
           Real forward2 = smileSection2->atmLevel();
           Real forward3 = smileSection3->atmLevel();
           Real forward12 = 0.5*(forward1+forward2);
           if (std::abs(forward3 - forward12) > tolerance) {
                    BOOST_ERROR("\nChecking interpolation of forward parameters:"
                                "\nexpected = " << forward12 <<
                                "\nobserved = " << forward3);
           }

}

BOOST_AUTO_TEST_CASE(testZabrVols) {

    BOOST_TEST_MESSAGE("Testing swaption volatility cube (ZABR interpolation)...");

    CommonVars vars;

    // ZABR requires 5 parameters: alpha, beta, nu, rho, gamma
    // gamma=1.0 gives SABR-like behaviour
    auto parametersGuess = vars.makeZabrParametersGuess(0.2, 0.5, 0.4, 0.0, 1.0);
    std::vector<bool> isParameterFixed(5, false);
    isParameterFixed[1] = true;  // Fix beta (standard practice, as in SABR)
    isParameterFixed[4] = true;  // Fix gamma for stability

    ZabrSwaptionVolatilityCube volCube(vars.atmVolMatrix,
                             vars.cube.tenors.options,
                             vars.cube.tenors.swaps,
                             vars.cube.strikeSpreads,
                             vars.cube.volSpreadsHandle,
                             vars.swapIndexBase,
                             vars.shortSwapIndexBase,
                             vars.vegaWeighedSmileFit,
                             parametersGuess,
                             isParameterFixed,
                             true);

    // ZABR should recover ATM vols with reasonable tolerance
    Real tolerance = 5.0e-4;
    vars.makeAtmVolTest(volCube, tolerance);

    // ZABR should also recover vol spreads with reasonable tolerance
    tolerance = 15.0e-4;
    vars.makeVolSpreadsTest(volCube, tolerance);

    // Access via base class interface for smileSection
    SwaptionVolatilityStructure* volStructure = &volCube;

    // Test smile section retrieval
    ext::shared_ptr<SmileSection> smileSection =
        volStructure->smileSection(Period(10,Years), Period(2,Years));
    BOOST_REQUIRE(smileSection != nullptr);

    // Test volatility at ATM
    Rate atmStrike = volCube.atmStrike(Period(10,Years), Period(2,Years));
    Volatility vol = volCube.volatility(Period(10,Years), Period(2,Years), atmStrike, true);
    BOOST_CHECK(vol > 0.0);
}

BOOST_AUTO_TEST_CASE(testZabrSmileSection) {

    BOOST_TEST_MESSAGE("Testing ZABR swaption volatility cube smile sections...");

    CommonVars vars;

    auto parametersGuess = vars.makeZabrParametersGuess(0.2, 0.5, 0.4, 0.0, 1.0);
    std::vector<bool> isParameterFixed(5, false);
    isParameterFixed[1] = true;  // Fix beta (standard practice)
    isParameterFixed[4] = true;

    ZabrSwaptionVolatilityCube volCube(vars.atmVolMatrix,
                             vars.cube.tenors.options,
                             vars.cube.tenors.swaps,
                             vars.cube.strikeSpreads,
                             vars.cube.volSpreadsHandle,
                             vars.swapIndexBase,
                             vars.shortSwapIndexBase,
                             vars.vegaWeighedSmileFit,
                             parametersGuess,
                             isParameterFixed,
                             true);

    // Access via base class interface
    SwaptionVolatilityStructure* volStructure = &volCube;

    // Get smile section and test its properties
    ext::shared_ptr<SmileSection> smileSection =
        volStructure->smileSection(Period(5,Years), Period(5,Years));

    BOOST_REQUIRE(smileSection != nullptr);
    BOOST_CHECK(smileSection->atmLevel() > 0.0);

    // Test volatility at different strikes
    Rate atmStrike = smileSection->atmLevel();
    std::vector<Real> strikes = {atmStrike * 0.8, atmStrike, atmStrike * 1.2};

    for (Real strike : strikes) {
        Volatility vol = smileSection->volatility(strike);
        BOOST_CHECK_MESSAGE(vol > 0.0,
            "ZABR smile section volatility at strike " << strike << " should be positive");
    }
}

BOOST_AUTO_TEST_CASE(testZabrParameters) {

    BOOST_TEST_MESSAGE("Testing interpolation of ZABR smile section parameters...");

    CommonVars vars;

    auto parametersGuess = vars.makeZabrParametersGuess(0.2, 0.5, 0.4, 0.0, 1.0);
    std::vector<bool> isParameterFixed(5, false);
    isParameterFixed[1] = true;  // Fix beta (standard practice)
    isParameterFixed[4] = true;

    ZabrSwaptionVolatilityCube volCube(vars.atmVolMatrix,
                             vars.cube.tenors.options,
                             vars.cube.tenors.swaps,
                             vars.cube.strikeSpreads,
                             vars.cube.volSpreadsHandle,
                             vars.swapIndexBase,
                             vars.shortSwapIndexBase,
                             vars.vegaWeighedSmileFit,
                             parametersGuess,
                             isParameterFixed,
                             true);

    SwaptionVolatilityStructure* volStructure = &volCube;

    // Test parameter interpolation by comparing volatilities at intermediate points
    // If parameters interpolate correctly, volatilities at intermediate tenors should
    // be smooth interpolations of neighboring volatilities

    // Section 1: maturity = 10Y, tenor = 2Y
    ext::shared_ptr<SmileSection> smileSection1 =
        volStructure->smileSection(Period(10,Years), Period(2,Years));

    // Section 2: maturity = 10Y, tenor = 4Y
    ext::shared_ptr<SmileSection> smileSection2 =
        volStructure->smileSection(Period(10,Years), Period(4,Years));

    // Section 3: maturity = 10Y, tenor = 3Y (intermediate)
    ext::shared_ptr<SmileSection> smileSection3 =
        volStructure->smileSection(Period(10,Years), Period(3,Years));

    // Test forward rate interpolation (which depends on parameter interpolation)
    Real forward1 = smileSection1->atmLevel();
    Real forward2 = smileSection2->atmLevel();
    Real forward3 = smileSection3->atmLevel();
    Real forward12 = 0.5*(forward1+forward2);

    Real tolerance = 1.0e-4;
    if (std::abs(forward3 - forward12) > tolerance) {
        BOOST_ERROR("\nChecking interpolation of ZABR forward parameters:"
                    "\nforward at 2Y = " << forward1 <<
                    "\nforward at 4Y = " << forward2 <<
                    "\nexpected at 3Y = " << forward12 <<
                    "\nobserved at 3Y = " << forward3);
    }

    // Test ATM volatility smoothness (indirect parameter interpolation test)
    Volatility vol1 = smileSection1->volatility(forward1);
    Volatility vol2 = smileSection2->volatility(forward2);
    Volatility vol3 = smileSection3->volatility(forward3);

    // Verify that interpolated section produces reasonable volatility
    // (between the two boundary sections or close to their average)
    Real vol12 = 0.5*(vol1+vol2);
    Real volTolerance = 5.0e-3;  // 50 bps tolerance for vol interpolation

    if (std::abs(vol3 - vol12) > volTolerance) {
        BOOST_ERROR("\nChecking interpolation of ZABR ATM volatility:"
                    "\nvol at 2Y = " << io::volatility(vol1) <<
                    "\nvol at 4Y = " << io::volatility(vol2) <<
                    "\nexpected at 3Y = " << io::volatility(vol12) <<
                    "\nobserved at 3Y = " << io::volatility(vol3));
    }

    // Downcast to access individual calibrated ZABR parameters,
    // mirroring testSabrParameters which checks alpha/beta/rho/nu directly.
    // This catches bugs such as parameters stored in wrong order in params vector.
    using ZabrSection = ZabrSmileSection<ZabrShortMaturityLognormal>;
    auto zs1 = ext::dynamic_pointer_cast<ZabrSection>(smileSection1);
    auto zs2 = ext::dynamic_pointer_cast<ZabrSection>(smileSection2);
    auto zs3 = ext::dynamic_pointer_cast<ZabrSection>(smileSection3);
    BOOST_REQUIRE(zs1 != nullptr);
    BOOST_REQUIRE(zs2 != nullptr);
    BOOST_REQUIRE(zs3 != nullptr);

    // alpha interpolation
    Real alpha1 = zs1->model()->alpha(), alpha2 = zs2->model()->alpha();
    Real alpha3 = zs3->model()->alpha();
    if (std::abs(alpha3 - 0.5*(alpha1+alpha2)) > tolerance)
        BOOST_ERROR("\nChecking interpolation of ZABR alpha:"
                    "\nalpha at 2Y = " << alpha1 <<
                    "\nalpha at 4Y = " << alpha2 <<
                    "\nexpected at 3Y = " << 0.5*(alpha1+alpha2) <<
                    "\nobserved at 3Y = " << alpha3);

    // nu interpolation
    Real nu1 = zs1->model()->nu(), nu2 = zs2->model()->nu();
    Real nu3 = zs3->model()->nu();
    if (std::abs(nu3 - 0.5*(nu1+nu2)) > tolerance)
        BOOST_ERROR("\nChecking interpolation of ZABR nu:"
                    "\nnu at 2Y = " << nu1 <<
                    "\nnu at 4Y = " << nu2 <<
                    "\nexpected at 3Y = " << 0.5*(nu1+nu2) <<
                    "\nobserved at 3Y = " << nu3);

    // rho interpolation
    Real rho1 = zs1->model()->rho(), rho2 = zs2->model()->rho();
    Real rho3 = zs3->model()->rho();
    if (std::abs(rho3 - 0.5*(rho1+rho2)) > tolerance)
        BOOST_ERROR("\nChecking interpolation of ZABR rho:"
                    "\nrho at 2Y = " << rho1 <<
                    "\nrho at 4Y = " << rho2 <<
                    "\nexpected at 3Y = " << 0.5*(rho1+rho2) <<
                    "\nobserved at 3Y = " << rho3);

    // Note: beta and gamma interpolation are not tested here because both
    // parameters are fixed (isParameterFixed[1]=true, isParameterFixed[4]=true),
    // so their layer values do not change between sections.
}

BOOST_AUTO_TEST_CASE(testZabrWithNonUnitGamma) {

    BOOST_TEST_MESSAGE("Testing ZABR swaption volatility cube with gamma != 1.0...");

    CommonVars vars;

    // ZABR with gamma = 0.75 (different from SABR-like gamma=1.0)
    auto parametersGuess = vars.makeZabrParametersGuess(0.2, 0.5, 0.4, 0.0, 0.75);
    std::vector<bool> isParameterFixed(5, false);
    isParameterFixed[1] = true;  // Fix beta (standard practice)
    isParameterFixed[4] = true;  // Fix gamma to test non-unit value behavior

    ZabrSwaptionVolatilityCube volCube(vars.atmVolMatrix,
                             vars.cube.tenors.options,
                             vars.cube.tenors.swaps,
                             vars.cube.strikeSpreads,
                             vars.cube.volSpreadsHandle,
                             vars.swapIndexBase,
                             vars.shortSwapIndexBase,
                             vars.vegaWeighedSmileFit,
                             parametersGuess,
                             isParameterFixed,
                             true);

    // With gamma != 1.0, ZABR should still produce valid volatilities
    // Tolerance may be slightly higher due to different smile shape
    Real tolerance = 8.0e-4;
    vars.makeAtmVolTest(volCube, tolerance);

    // Verify smile section works with non-unit gamma
    SwaptionVolatilityStructure* volStructure = &volCube;
    ext::shared_ptr<SmileSection> smileSection =
        volStructure->smileSection(Period(5,Years), Period(5,Years));

    BOOST_REQUIRE(smileSection != nullptr);

    // Test volatility smile shape - with gamma < 1, expect different curvature
    Rate atmStrike = smileSection->atmLevel();
    Volatility atmVol = smileSection->volatility(atmStrike);
    Volatility otmVol = smileSection->volatility(atmStrike * 1.2);
    Volatility itmVol = smileSection->volatility(atmStrike * 0.8);

    BOOST_CHECK(atmVol > 0.0);
    BOOST_CHECK(otmVol > 0.0);
    BOOST_CHECK(itmVol > 0.0);
}

BOOST_AUTO_TEST_CASE(testZabrWithFreeGamma) {

    BOOST_TEST_MESSAGE("Testing ZABR cube with free gamma calibration...");

    CommonVars vars;

    // gamma=1.0 is only the initial guess; optimizer will calibrate it freely
    auto parametersGuess = vars.makeZabrParametersGuess(0.2, 0.5, 0.4, 0.0, 1.0);
    std::vector<bool> isParameterFixed(5, false);
    isParameterFixed[1] = true;   // fix beta

    ZabrSwaptionVolatilityCube cube(vars.atmVolMatrix,
                             vars.cube.tenors.options,
                             vars.cube.tenors.swaps,
                             vars.cube.strikeSpreads,
                             vars.cube.volSpreadsHandle,
                             vars.swapIndexBase,
                             vars.shortSwapIndexBase,
                             vars.vegaWeighedSmileFit,
                             parametersGuess,
                             isParameterFixed,
                             true);

    // Get a smile section and verify gamma was actually calibrated
    Period optionTenor = vars.cube.tenors.options[0];
    Period swapTenor = vars.cube.tenors.swaps[0];
    ext::shared_ptr<SmileSection> section =
        cube.smileSection(optionTenor, swapTenor);
    BOOST_REQUIRE(section != nullptr);

    // Downcast to ZabrSmileSection to access calibrated gamma
    auto zabrSection =
        ext::dynamic_pointer_cast<ZabrSmileSection<ZabrShortMaturityLognormal> >(section);
    BOOST_REQUIRE(zabrSection != nullptr);

    Real calibratedGamma = zabrSection->model()->gamma();

    // Gamma should be within valid bounds: ZabrSpecs::direct() caps gamma at 1.9
    BOOST_CHECK(calibratedGamma > 0.1);
    BOOST_CHECK(calibratedGamma < 1.9);

    // Verify gamma actually moved from the initial guess of 1.0
    BOOST_CHECK(std::abs(calibratedGamma - 1.0) > 1e-6);

    // Verify ATM vol is positive
    Rate atmStrike = section->atmLevel();
    Volatility atmVol = section->volatility(atmStrike);
    BOOST_CHECK(atmVol > 0.0);
}


BOOST_AUTO_TEST_CASE(testZabrShiftedVolThrows) {

    BOOST_TEST_MESSAGE("Testing that ZABR cube throws for non-zero ATM vol shift...");

    CommonVars vars;

    // Build an ATM vol matrix with a uniform 2% shift.
    // ZABR does not support shifted lognormal vols (QL_REQUIRE in createSmileSection).
    Size nOpts = vars.atm.tenors.options.size();
    Size nSwps = vars.atm.tenors.swaps.size();
    std::vector<std::vector<Real>> shifts(nOpts, std::vector<Real>(nSwps, 0.02));

    Handle<SwaptionVolatilityStructure> shiftedAtmVol(
        ext::make_shared<SwaptionVolatilityMatrix>(
            vars.conventions.calendar,
            vars.conventions.optionBdc,
            vars.atm.tenors.options,
            vars.atm.tenors.swaps,
            vars.atm.volsHandle,
            vars.conventions.dayCounter,
            false,
            ShiftedLognormal,
            shifts));

    auto parametersGuess = vars.makeZabrParametersGuess(0.2, 0.5, 0.4, 0.0, 1.0);
    std::vector<bool> isParameterFixed(5, false);
    isParameterFixed[1] = true;
    isParameterFixed[4] = true;

    ZabrSwaptionVolatilityCube volCube(shiftedAtmVol,
                             vars.cube.tenors.options,
                             vars.cube.tenors.swaps,
                             vars.cube.strikeSpreads,
                             vars.cube.volSpreadsHandle,
                             vars.swapIndexBase,
                             vars.shortSwapIndexBase,
                             vars.vegaWeighedSmileFit,
                             parametersGuess,
                             isParameterFixed,
                             true);

    // Querying the cube triggers lazy calibration and must throw
    // because the ATM vol structure has non-zero shift
    Rate strike = volCube.atmStrike(vars.cube.tenors.options[0],
                                    vars.cube.tenors.swaps[0]);
    BOOST_CHECK_THROW(
        volCube.volatility(vars.cube.tenors.options[0],
                           vars.cube.tenors.swaps[0], strike, true),
        QuantLib::Error);
}

BOOST_AUTO_TEST_CASE(testZabrAlternativeKernel) {

    BOOST_TEST_MESSAGE("Testing ZABR cube with ZabrShortMaturityNormal kernel...");

    CommonVars vars;

    auto parametersGuess = vars.makeZabrParametersGuess(0.2, 0.5, 0.4, 0.0, 1.0);
    std::vector<bool> isParameterFixed(5, false);
    isParameterFixed[1] = true;
    isParameterFixed[4] = true;

    // Instantiate with a non-default kernel — this exercises the kernel
    // template parameter added in zabrswaptionvolatilitycube.hpp.
    // Note: ZabrLocalVolatility and ZabrFullFd kernels are not covered here
    // due to the high cost of finite-difference evaluation.
    typedef XabrSwaptionVolatilityCube<
        SwaptionVolCubeZabrModel<ZabrShortMaturityNormal>
    > ZabrNormalKernelCube;

    ZabrNormalKernelCube volCube(vars.atmVolMatrix,
                             vars.cube.tenors.options,
                             vars.cube.tenors.swaps,
                             vars.cube.strikeSpreads,
                             vars.cube.volSpreadsHandle,
                             vars.swapIndexBase,
                             vars.shortSwapIndexBase,
                             vars.vegaWeighedSmileFit,
                             parametersGuess,
                             isParameterFixed,
                             true);

    // Smoke test: cube constructs and returns positive vol at ATM
    ext::shared_ptr<SmileSection> section =
        volCube.smileSection(Period(5, Years), Period(5, Years));
    BOOST_REQUIRE(section != nullptr);

    Rate atmStrike = section->atmLevel();
    BOOST_CHECK(atmStrike > 0.0);
    BOOST_CHECK(section->volatility(atmStrike) > 0.0);
}

BOOST_AUTO_TEST_CASE(testZabrObservability) {

    BOOST_TEST_MESSAGE("Testing ZABR volatility cube observability...");

    CommonVars vars;

    auto parametersGuess = vars.makeZabrParametersGuess(0.2, 0.5, 0.4, 0.0, 1.0);
    std::vector<bool> isParameterFixed(5, false);
    isParameterFixed[1] = true;
    isParameterFixed[4] = true;

    // VolCube created before reference date change
    auto volCube0 = ext::make_shared<ZabrSwaptionVolatilityCube>(
        vars.atmVolMatrix, vars.cube.tenors.options, vars.cube.tenors.swaps,
        vars.cube.strikeSpreads, vars.cube.volSpreadsHandle,
        vars.swapIndexBase, vars.shortSwapIndexBase,
        vars.vegaWeighedSmileFit, parametersGuess, isParameterFixed, true);

    Date referenceDate = Settings::instance().evaluationDate();
    Settings::instance().evaluationDate() =
        vars.conventions.calendar.advance(referenceDate, Period(1, Days),
                                          vars.conventions.optionBdc);

    // VolCube created after reference date change
    auto volCube1 = ext::make_shared<ZabrSwaptionVolatilityCube>(
        vars.atmVolMatrix, vars.cube.tenors.options, vars.cube.tenors.swaps,
        vars.cube.strikeSpreads, vars.cube.volSpreadsHandle,
        vars.swapIndexBase, vars.shortSwapIndexBase,
        vars.vegaWeighedSmileFit, parametersGuess, isParameterFixed, true);

    // Both cubes must give identical volatilities: the cube defined relative
    // to today re-calibrates when the reference date advances
    Rate dummyStrike = 0.03;
    for (Size i = 0; i < vars.cube.tenors.options.size(); ++i) {
        for (Size j = 0; j < vars.cube.tenors.swaps.size(); ++j) {
            for (Size k = 0; k < vars.cube.strikeSpreads.size(); ++k) {
                Volatility v0 = volCube0->volatility(
                    vars.cube.tenors.options[i], vars.cube.tenors.swaps[j],
                    dummyStrike + vars.cube.strikeSpreads[k], false);
                Volatility v1 = volCube1->volatility(
                    vars.cube.tenors.options[i], vars.cube.tenors.swaps[j],
                    dummyStrike + vars.cube.strikeSpreads[k], false);
                if (std::fabs(v0 - v1) > 1e-14)
                    BOOST_ERROR("ZABR cube observability failed:"
                                " option tenor = " << vars.cube.tenors.options[i] <<
                                " swap tenor = " << vars.cube.tenors.swaps[j] <<
                                " strike = " << io::rate(dummyStrike + vars.cube.strikeSpreads[k]) <<
                                "  v0 = " << io::volatility(v0) <<
                                "  v1 = " << io::volatility(v1) <<
                                "  error = " << std::fabs(v0 - v1));
            }
        }
    }

    Settings::instance().evaluationDate() = referenceDate;
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
