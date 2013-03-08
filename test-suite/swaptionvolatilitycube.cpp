/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2006, 2008 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni

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

#include "swaptionvolatilitycube.hpp"
#include "swaptionvolstructuresutilities.hpp"
#include "utilities.hpp"
#include <ql/indexes/swap/euriborswap.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolcube2.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolcube1.hpp>
#include <ql/termstructures/volatility/swaption/spreadedswaptionvol.hpp>
#include <ql/utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    struct CommonVars {
        // global data
        SwaptionMarketConventions conventions;
        AtmVolatility atm;
        RelinkableHandle<SwaptionVolatilityStructure> atmVolMatrix;
        VolatilityCube cube;

        RelinkableHandle<YieldTermStructure> termStructure;

        boost::shared_ptr<SwapIndex> swapIndexBase, shortSwapIndexBase;
        bool vegaWeighedSmileFit;

        // cleanup
        SavedSettings backup;

        // utilities
        void makeAtmVolTest(const SwaptionVolatilityCube& volCube,
                            Real tolerance) {

            for (Size i=0; i<atm.tenors.options.size(); i++) {
              for (Size j=0; j<atm.tenors.swaps.size(); j++) {
                Rate strike = volCube.atmStrike(atm.tenors.options[i],
                                                atm.tenors.swaps[j]);
                Volatility expVol =
                    atmVolMatrix->volatility(atm.tenors.options[i],
                                             atm.tenors.swaps[j],
                                             strike, true);
                Volatility actVol = volCube.volatility(atm.tenors.options[i],
                                                       atm.tenors.swaps[j],
                                                       strike, true);
                Volatility error = std::abs(expVol-actVol);
                if (error>tolerance)
                  BOOST_ERROR("\nrecovery of atm vols failed:"
                              "\nexpiry time = " << atm.tenors.options[i] <<
                              "\nswap length = " << atm.tenors.swaps[j] <<
                              "\n atm strike = " << io::rate(strike) <<
                              "\n   exp. vol = " << io::volatility(expVol) <<
                              "\n actual vol = " << io::volatility(actVol) <<
                              "\n      error = " << io::volatility(error) <<
                              "\n  tolerance = " << tolerance);
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
                boost::shared_ptr<SwaptionVolatilityStructure>(new
                    SwaptionVolatilityMatrix(conventions.calendar,
                                             conventions.optionBdc,
                                             atm.tenors.options,
                                             atm.tenors.swaps,
                                             atm.volsHandle,
                                             conventions.dayCounter)));
            // Swaptionvolcube
            cube.setMarketData();

            termStructure.linkTo(flatRate(0.05, Actual365Fixed()));

            swapIndexBase = boost::shared_ptr<SwapIndex>(new
                EuriborSwapIsdaFixA(2*Years, termStructure));
            shortSwapIndexBase = boost::shared_ptr<SwapIndex>(new
                EuriborSwapIsdaFixA(1*Years, termStructure));

            vegaWeighedSmileFit=false;
        }
    };

}


void SwaptionVolatilityCubeTest::testAtmVols() {

    BOOST_TEST_MESSAGE("Testing swaption volatility cube (atm vols)...");

    CommonVars vars;

    SwaptionVolCube2 volCube(vars.atmVolMatrix,
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

void SwaptionVolatilityCubeTest::testSmile() {

    BOOST_TEST_MESSAGE("Testing swaption volatility cube (smile)...");

    CommonVars vars;

    SwaptionVolCube2 volCube(vars.atmVolMatrix,
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

void SwaptionVolatilityCubeTest::testSabrVols() {

    BOOST_TEST_MESSAGE("Testing swaption volatility cube (sabr interpolation)...");

    CommonVars vars;

    std::vector<std::vector<Handle<Quote> > >
        parametersGuess(vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size());
    for (Size i=0; i<vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size(); i++) {
        parametersGuess[i] = std::vector<Handle<Quote> >(4);
        parametersGuess[i][0] =
            Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(0.2)));
        parametersGuess[i][1] =
            Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(0.5)));
        parametersGuess[i][2] =
            Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(0.4)));
        parametersGuess[i][3] =
            Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(0.0)));
    }
    std::vector<bool> isParameterFixed(4, false);

    SwaptionVolCube1 volCube(vars.atmVolMatrix,
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

void SwaptionVolatilityCubeTest::testSpreadedCube() {

    BOOST_TEST_MESSAGE("Testing spreaded swaption volatility cube...");

    CommonVars vars;

    std::vector<std::vector<Handle<Quote> > >
        parametersGuess(vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size());
    for (Size i=0; i<vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size(); i++) {
        parametersGuess[i] = std::vector<Handle<Quote> >(4);
        parametersGuess[i][0] =
            Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(0.2)));
        parametersGuess[i][1] =
            Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(0.5)));
        parametersGuess[i][2] =
            Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(0.4)));
        parametersGuess[i][3] =
            Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(0.0)));
    }
    std::vector<bool> isParameterFixed(4, false);

    Handle<SwaptionVolatilityStructure> volCube( boost::shared_ptr<SwaptionVolatilityStructure>(new
        SwaptionVolCube1(vars.atmVolMatrix,
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

    boost::shared_ptr<SimpleQuote> spread (new SimpleQuote(0.0001));
    Handle<Quote> spreadHandle(spread);
    boost::shared_ptr<SwaptionVolatilityStructure> spreadedVolCube(new
        SpreadedSwaptionVolatility(volCube, spreadHandle));
    std::vector<Real> strikes;
    for (Size k=1; k<100; k++)
        strikes.push_back(k*.01);
    for (Size i=0; i<vars.cube.tenors.options.size(); i++) {
        for (Size j=0; j<vars.cube.tenors.swaps.size(); j++) {
            boost::shared_ptr<SmileSection> smileSectionByCube =
                volCube->smileSection(vars.cube.tenors.options[i], vars.cube.tenors.swaps[j]);
            boost::shared_ptr<SmileSection> smileSectionBySpreadedCube =
                spreadedVolCube->smileSection(vars.cube.tenors.options[i], vars.cube.tenors.swaps[j]);
            for (Size k=0; k<strikes.size(); k++) {
                Real strike = strikes[k];
                Real diff = spreadedVolCube->volatility(vars.cube.tenors.options[i], vars.cube.tenors.swaps[j], strike)
                            - volCube->volatility(vars.cube.tenors.options[i], vars.cube.tenors.swaps[j], strike);
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


void SwaptionVolatilityCubeTest::testObservability() {
    BOOST_TEST_MESSAGE("Testing volatility cube observability...");

    CommonVars vars;

    std::vector<std::vector<Handle<Quote> > >
        parametersGuess(vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size());
    for (Size i=0; i<vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size(); i++) {
        parametersGuess[i] = std::vector<Handle<Quote> >(4);
        parametersGuess[i][0] =
            Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(0.2)));
        parametersGuess[i][1] =
            Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(0.5)));
        parametersGuess[i][2] =
            Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(0.4)));
        parametersGuess[i][3] =
            Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(0.0)));
    }
    std::vector<bool> isParameterFixed(4, false);

    std::string description;
    boost::shared_ptr<SwaptionVolCube1> volCube1_0, volCube1_1;
    // VolCube created before change of reference date
    volCube1_0 = boost::shared_ptr<SwaptionVolCube1>(new SwaptionVolCube1(vars.atmVolMatrix,
                                                                vars.cube.tenors.options,
                                                                vars.cube.tenors.swaps,
                                                                vars.cube.strikeSpreads,
                                                                vars.cube.volSpreadsHandle,
                                                                vars.swapIndexBase,
                                                                vars.shortSwapIndexBase,
                                                                vars.vegaWeighedSmileFit,
                                                                parametersGuess,
                                                                isParameterFixed,
                                                                true));

    Date referenceDate = Settings::instance().evaluationDate();
    Settings::instance().evaluationDate() =
        vars.conventions.calendar.advance(referenceDate, Period(1, Days),
                                          vars.conventions.optionBdc);

    // VolCube created after change of reference date
    volCube1_1 = boost::shared_ptr<SwaptionVolCube1>(new SwaptionVolCube1(vars.atmVolMatrix,
                                                                vars.cube.tenors.options,
                                                                vars.cube.tenors.swaps,
                                                                vars.cube.strikeSpreads,
                                                                vars.cube.volSpreadsHandle,
                                                                vars.swapIndexBase,
                                                                vars.shortSwapIndexBase,
                                                                vars.vegaWeighedSmileFit,
                                                                parametersGuess,
                                                                isParameterFixed,
                                                                true));
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
                if (v0 != v1)
                    BOOST_ERROR(description <<
                                " option tenor = " << vars.cube.tenors.options[i] <<
                                " swap tenor = " << vars.cube.tenors.swaps[j] <<
                                " strike = " << io::rate(dummyStrike+vars.cube.strikeSpreads[k])<<
                                "  v0 = " << io::volatility(v0) <<
                                "  v1 = " << io::volatility(v1));
            }
        }
    }

    Settings::instance().evaluationDate() = referenceDate;

    boost::shared_ptr<SwaptionVolCube2> volCube2_0, volCube2_1;
    // VolCube created before change of reference date
    volCube2_0 = boost::shared_ptr<SwaptionVolCube2>(new SwaptionVolCube2(vars.atmVolMatrix,
                                                                vars.cube.tenors.options,
                                                                vars.cube.tenors.swaps,
                                                                vars.cube.strikeSpreads,
                                                                vars.cube.volSpreadsHandle,
                                                                vars.swapIndexBase,
                                                                vars.shortSwapIndexBase,
                                                                vars.vegaWeighedSmileFit));
    Settings::instance().evaluationDate() =
        vars.conventions.calendar.advance(referenceDate, Period(1, Days),
                                          vars.conventions.optionBdc);

    // VolCube created after change of reference date
    volCube2_1 = boost::shared_ptr<SwaptionVolCube2>(new SwaptionVolCube2(vars.atmVolMatrix,
                                                                vars.cube.tenors.options,
                                                                vars.cube.tenors.swaps,
                                                                vars.cube.strikeSpreads,
                                                                vars.cube.volSpreadsHandle,
                                                                vars.swapIndexBase,
                                                                vars.shortSwapIndexBase,
                                                                vars.vegaWeighedSmileFit));

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
                if (v0 != v1)
                    BOOST_ERROR(description <<
                                " option tenor = " << vars.cube.tenors.options[i] <<
                                " swap tenor = " << vars.cube.tenors.swaps[j] <<
                                " strike = " << io::rate(dummyStrike+vars.cube.strikeSpreads[k])<<
                                "  v0 = " << io::volatility(v0) <<
                                "  v1 = " << io::volatility(v1));
            }
        }
    }

    Settings::instance().evaluationDate() = referenceDate;
}

test_suite* SwaptionVolatilityCubeTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Swaption Volatility Cube tests");

    // SwaptionVolCubeByLinear reproduces ATM vol with machine precision
    suite->add(QUANTLIB_TEST_CASE(&SwaptionVolatilityCubeTest::testAtmVols));
    // SwaptionVolCubeByLinear reproduces smile spreads with machine precision
    suite->add(QUANTLIB_TEST_CASE(&SwaptionVolatilityCubeTest::testSmile));

    // SwaptionVolCubeBySabr reproduces ATM vol with given tolerance
    // SwaptionVolCubeBySabr reproduces smile spreads with given tolerance
    suite->add(QUANTLIB_TEST_CASE(&SwaptionVolatilityCubeTest::testSabrVols));
    suite->add(QUANTLIB_TEST_CASE(
                              &SwaptionVolatilityCubeTest::testSpreadedCube));

    suite->add(QUANTLIB_TEST_CASE(
                             &SwaptionVolatilityCubeTest::testObservability));

    return suite;
}
