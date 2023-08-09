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
#include <ql/termstructures/volatility/swaption/interpolatedswaptionvolatilitycube.hpp>
#include <ql/termstructures/volatility/swaption/sabrswaptionvolatilitycube.hpp>
#include <ql/termstructures/volatility/swaption/spreadedswaptionvol.hpp>
#include <ql/termstructures/volatility/sabrsmilesection.hpp>
#include <ql/utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace swaption_volatility_cube_test {

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

}

void SwaptionVolatilityCubeTest::testSabrNormalVolatility() {

    BOOST_TEST_MESSAGE("Testing sabr normal volatility...");

    using namespace swaption_volatility_cube_test;

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

void SwaptionVolatilityCubeTest::testAtmVols() {

    BOOST_TEST_MESSAGE("Testing swaption volatility cube (atm vols)...");

    using namespace swaption_volatility_cube_test;

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

void SwaptionVolatilityCubeTest::testSmile() {

    BOOST_TEST_MESSAGE("Testing swaption volatility cube (smile)...");

    using namespace swaption_volatility_cube_test;

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

void SwaptionVolatilityCubeTest::testSabrVols() {

    BOOST_TEST_MESSAGE("Testing swaption volatility cube (sabr interpolation)...");

    using namespace swaption_volatility_cube_test;

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

void SwaptionVolatilityCubeTest::testSpreadedCube() {

    BOOST_TEST_MESSAGE("Testing spreaded swaption volatility cube...");

    using namespace swaption_volatility_cube_test;

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


void SwaptionVolatilityCubeTest::testObservability() {
    BOOST_TEST_MESSAGE("Testing volatility cube observability...");

    using namespace swaption_volatility_cube_test;

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

void SwaptionVolatilityCubeTest::testSabrParameters() {
    BOOST_TEST_MESSAGE("Testing interpolation of SABR smile sections...");

    using namespace swaption_volatility_cube_test;

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


test_suite* SwaptionVolatilityCubeTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Swaption Volatility Cube tests");

    suite->add(QUANTLIB_TEST_CASE(&SwaptionVolatilityCubeTest::testSabrNormalVolatility));
    // SwaptionVolCubeByLinear reproduces ATM vol with machine precision
    suite->add(QUANTLIB_TEST_CASE(&SwaptionVolatilityCubeTest::testAtmVols));
    // SwaptionVolCubeByLinear reproduces smile spreads with machine precision
    suite->add(QUANTLIB_TEST_CASE(&SwaptionVolatilityCubeTest::testSmile));

    // SwaptionVolCubeBySabr reproduces ATM vol with given tolerance
    // SwaptionVolCubeBySabr reproduces smile spreads with given tolerance
    suite->add(QUANTLIB_TEST_CASE(&SwaptionVolatilityCubeTest::testSabrVols));
    suite->add(QUANTLIB_TEST_CASE(&SwaptionVolatilityCubeTest::testSpreadedCube));
    suite->add(QUANTLIB_TEST_CASE(&SwaptionVolatilityCubeTest::testObservability));
    suite->add(QUANTLIB_TEST_CASE(&SwaptionVolatilityCubeTest::testSabrParameters));


    return suite;
}
