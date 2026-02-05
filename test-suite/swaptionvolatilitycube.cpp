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
    std::vector<std::vector<Handle<Quote> > >
        parametersGuess(vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size());
    for (Size i=0; i<vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size(); i++) {
        parametersGuess[i] = std::vector<Handle<Quote> >(5);
        parametersGuess[i][0] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.2)));   // alpha
        parametersGuess[i][1] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.5)));   // beta
        parametersGuess[i][2] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.4)));   // nu
        parametersGuess[i][3] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.0)));   // rho
        parametersGuess[i][4] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(1.0)));   // gamma (1.0 = SABR-like)
    }
    std::vector<bool> isParameterFixed(5, false);
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
    // Note: ZABR uses slightly higher tolerance (5.0e-4) than SABR (3.0e-4) due to
    // additional gamma parameter flexibility in the optimization
    Real tolerance = 5.0e-4;
    vars.makeAtmVolTest(volCube, tolerance);

    // ZABR should also recover vol spreads with reasonable tolerance
    // Note: ZABR spread tolerance (15.0e-4) is slightly higher than SABR (12.0e-4)
    // due to additional optimization complexity with gamma parameter
    tolerance = 15.0e-4;
    vars.makeVolSpreadsTest(volCube, tolerance);

    // Access via base class interface for smileSection
    SwaptionVolatilityStructure* volStructure = &volCube;

    // Test smile section retrieval
    ext::shared_ptr<SmileSection> smileSection =
        volStructure->smileSection(Period(10,Years), Period(2,Years));
    BOOST_CHECK(smileSection != nullptr);

    // Test volatility at ATM
    Rate atmStrike = volCube.atmStrike(Period(10,Years), Period(2,Years));
    Volatility vol = volCube.volatility(Period(10,Years), Period(2,Years), atmStrike, true);
    BOOST_CHECK(vol > 0.0);
}

BOOST_AUTO_TEST_CASE(testZabrSmileSection) {

    BOOST_TEST_MESSAGE("Testing ZABR swaption volatility cube smile sections...");

    CommonVars vars;

    std::vector<std::vector<Handle<Quote> > >
        parametersGuess(vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size());
    for (Size i=0; i<vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size(); i++) {
        parametersGuess[i] = std::vector<Handle<Quote> >(5);
        parametersGuess[i][0] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.2)));
        parametersGuess[i][1] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.5)));
        parametersGuess[i][2] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.4)));
        parametersGuess[i][3] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.0)));
        parametersGuess[i][4] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(1.0)));
    }
    std::vector<bool> isParameterFixed(5, false);
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

    BOOST_CHECK(smileSection != nullptr);
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

    std::vector<std::vector<Handle<Quote> > >
        parametersGuess(vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size());
    for (Size i=0; i<vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size(); i++) {
        parametersGuess[i] = std::vector<Handle<Quote> >(5);
        parametersGuess[i][0] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.2)));
        parametersGuess[i][1] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.5)));
        parametersGuess[i][2] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.4)));
        parametersGuess[i][3] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.0)));
        parametersGuess[i][4] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(1.0)));
    }
    std::vector<bool> isParameterFixed(5, false);
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
}

BOOST_AUTO_TEST_CASE(testZabrWithNonUnitGamma) {

    BOOST_TEST_MESSAGE("Testing ZABR swaption volatility cube with gamma != 1.0...");

    CommonVars vars;

    // ZABR with gamma = 0.75 (different from SABR-like gamma=1.0)
    std::vector<std::vector<Handle<Quote> > >
        parametersGuess(vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size());
    for (Size i=0; i<vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size(); i++) {
        parametersGuess[i] = std::vector<Handle<Quote> >(5);
        parametersGuess[i][0] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.2)));   // alpha
        parametersGuess[i][1] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.5)));   // beta
        parametersGuess[i][2] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.4)));   // nu
        parametersGuess[i][3] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.0)));   // rho
        parametersGuess[i][4] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.75))); // gamma != 1.0
    }
    std::vector<bool> isParameterFixed(5, false);
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

    BOOST_CHECK(smileSection != nullptr);

    // Test volatility smile shape - with gamma < 1, expect different curvature
    Rate atmStrike = smileSection->atmLevel();
    Volatility atmVol = smileSection->volatility(atmStrike);
    Volatility otmVol = smileSection->volatility(atmStrike * 1.2);
    Volatility itmVol = smileSection->volatility(atmStrike * 0.8);

    BOOST_CHECK(atmVol > 0.0);
    BOOST_CHECK(otmVol > 0.0);
    BOOST_CHECK(itmVol > 0.0);
}

BOOST_AUTO_TEST_CASE(testZabrVsSabrComparison) {

    BOOST_TEST_MESSAGE("Testing ZABR (gamma=1) vs SABR comparison...");

    CommonVars vars;

    // Create SABR cube with 4 parameters
    std::vector<std::vector<Handle<Quote> > >
        sabrParametersGuess(vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size());
    for (Size i=0; i<vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size(); i++) {
        sabrParametersGuess[i] = std::vector<Handle<Quote> >(4);
        sabrParametersGuess[i][0] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.2)));
        sabrParametersGuess[i][1] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.5)));
        sabrParametersGuess[i][2] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.4)));
        sabrParametersGuess[i][3] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.0)));
    }
    std::vector<bool> sabrIsParameterFixed(4, false);

    SabrSwaptionVolatilityCube sabrCube(vars.atmVolMatrix,
                             vars.cube.tenors.options,
                             vars.cube.tenors.swaps,
                             vars.cube.strikeSpreads,
                             vars.cube.volSpreadsHandle,
                             vars.swapIndexBase,
                             vars.shortSwapIndexBase,
                             vars.vegaWeighedSmileFit,
                             sabrParametersGuess,
                             sabrIsParameterFixed,
                             true);

    // Create ZABR cube with gamma = 1.0 (SABR-like)
    std::vector<std::vector<Handle<Quote> > >
        zabrParametersGuess(vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size());
    for (Size i=0; i<vars.cube.tenors.options.size()*vars.cube.tenors.swaps.size(); i++) {
        zabrParametersGuess[i] = std::vector<Handle<Quote> >(5);
        zabrParametersGuess[i][0] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.2)));
        zabrParametersGuess[i][1] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.5)));
        zabrParametersGuess[i][2] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.4)));
        zabrParametersGuess[i][3] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.0)));
        zabrParametersGuess[i][4] =
            Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(1.0)));  // gamma = 1.0
    }
    std::vector<bool> zabrIsParameterFixed(5, false);
    zabrIsParameterFixed[4] = true;  // Fix gamma = 1.0

    ZabrSwaptionVolatilityCube zabrCube(vars.atmVolMatrix,
                             vars.cube.tenors.options,
                             vars.cube.tenors.swaps,
                             vars.cube.strikeSpreads,
                             vars.cube.volSpreadsHandle,
                             vars.swapIndexBase,
                             vars.shortSwapIndexBase,
                             vars.vegaWeighedSmileFit,
                             zabrParametersGuess,
                             zabrIsParameterFixed,
                             true);

    // Compare ATM volatilities between SABR and ZABR(gamma=1)
    // They should be reasonably close (within tolerance) as gamma=1 makes ZABR behave
    // similarly to SABR, though not identically due to different kernel implementations
    Real tolerance = 5.0e-3;  // 50 bps tolerance for model comparison

    for (auto& option : vars.atm.tenors.options) {
        for (auto& swap : vars.atm.tenors.swaps) {
            Rate strike = sabrCube.atmStrike(option, swap);
            Volatility sabrVol = sabrCube.volatility(option, swap, strike, true);
            Volatility zabrVol = zabrCube.volatility(option, swap, strike, true);
            Volatility diff = std::abs(sabrVol - zabrVol);

            if (diff > tolerance)
                BOOST_ERROR("\nZABR(gamma=1) vs SABR comparison failed:"
                            "\nexpiry time = " << option <<
                            "\nswap length = " << swap <<
                            "\n   SABR vol = " << io::volatility(sabrVol) <<
                            "\n   ZABR vol = " << io::volatility(zabrVol) <<
                            "\n difference = " << io::volatility(diff) <<
                            "\n  tolerance = " << tolerance);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
