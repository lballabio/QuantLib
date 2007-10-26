/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2006 Ferdinando Ametrano
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
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/interestrate/swaption/swaptionvolcube2.hpp>
#include <ql/termstructures/volatility/interestrate/swaption/swaptionvolcube1.hpp>
#include <ql/termstructures/volatility/interestrate/swaption/spreadedswaptionvol.hpp>
#include <ql/utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(SwaptionVolatilityCubeTest)

// global data

Date referenceDate_;
SwaptionMarketConventions conventions_;
AtmVolatility atm_;
RelinkableHandle<SwaptionVolatilityStructure> atmVolMatrix_;
VolatilityCube cube_;

Natural swapSettlementDays_;
Frequency fixedLegFrequency_;
BusinessDayConvention fixedLegConvention_;
DayCounter fixedLegDayCounter_;
RelinkableHandle<YieldTermStructure> termStructure_;
boost::shared_ptr<IborIndex> iborIndex_;
boost::shared_ptr<SwapIndex> swapIndexBase_;
Time shortTenor_;
boost::shared_ptr<IborIndex> iborIndexShortTenor_;

bool vegaWeightedSmileFit_;

// utilities


void makeAtmVolTest(const SwaptionVolatilityCube& volCube,
                    Real tolerance) {

    for (Size i=0; i<atm_.tenors.options.size(); i++) {
      for (Size j=0; j<atm_.tenors.swaps.size(); j++) {
          Rate strike = volCube.atmStrike(atm_.tenors.options[i], atm_.tenors.swaps[j]);
          Volatility expVol = atmVolMatrix_->volatility(
              atm_.tenors.options[i], atm_.tenors.swaps[j], strike, true);
          Volatility actVol = volCube.volatility(
              atm_.tenors.options[i], atm_.tenors.swaps[j], strike, true);
          Volatility error = std::abs(expVol-actVol);
          if (error>tolerance)
              BOOST_ERROR("\nrecovery of atm vols failed:"
                          "\nexpiry time = " << atm_.tenors.options[i] <<
                          "\nswap length = " << atm_.tenors.swaps[j] <<
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

    for (Size i=0; i<cube_.tenors.options.size(); i++) {
      for (Size j=0; j<cube_.tenors.swaps.size(); j++) {
          for (Size k=0; k<cube_.strikeSpreads.size(); k++) {
              Rate atmStrike = volCube.atmStrike(cube_.tenors.options[i], cube_.tenors.swaps[j]);
              Volatility atmVol = atmVolMatrix_->volatility(
                  cube_.tenors.options[i], cube_.tenors.swaps[j], atmStrike, true);
              Volatility vol = volCube.volatility(
                  cube_.tenors.options[i], cube_.tenors.swaps[j], atmStrike+cube_.strikeSpreads[k], true);
              Volatility spread = vol-atmVol;
              Volatility expVolSpread = cube_.volSpreads[i*cube_.tenors.swaps.size()+j][k];
              Volatility error = std::abs(expVolSpread-spread);
              if (error>tolerance)
                  BOOST_ERROR("\nrecovery of smile vol spreads failed:"
                              "\n    option tenor = " << cube_.tenors.options[i] <<
                              "\n      swap tenor = " << cube_.tenors.swaps[j] <<
                              "\n      atm strike = " << io::rate(atmStrike) <<
                              "\n   strike spread = " << io::rate(cube_.strikeSpreads[k]) <<
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

void setup() {

    referenceDate_ = Settings::instance().evaluationDate();
    //referenceDate_ = Date(6, September, 2006);
    Settings::instance().evaluationDate() = referenceDate_;

    conventions_.setConventions();

    // ATM swaptionvolmatrix
    atm_.setMarketData();

    atmVolMatrix_ = RelinkableHandle<SwaptionVolatilityStructure>(
        boost::shared_ptr<SwaptionVolatilityStructure>(new
            SwaptionVolatilityMatrix(conventions_.calendar,
                                     atm_.tenors.options,
                                     atm_.tenors.swaps,
                                     atm_.volsHandle,
                                     conventions_.dayCounter,
                                     conventions_.optionBdc)));

    // Swaptionvolcube
    cube_.setMarketData();

    swapSettlementDays_ = 2;
    fixedLegFrequency_ = Annual;
    fixedLegConvention_ = Unadjusted;
    fixedLegDayCounter_ = Thirty360();
    termStructure_.linkTo(flatRate(referenceDate_, 0.05, Actual365Fixed()));
    iborIndex_ = boost::shared_ptr<IborIndex>(new Euribor6M(termStructure_));
    shortTenor_ = 2;
    iborIndexShortTenor_ = boost::shared_ptr<IborIndex>(new
        Euribor3M(termStructure_));
    swapIndexBase_ = boost::shared_ptr<SwapIndex>(new
        SwapIndex("EurliborSwapFixA",
                  10*Years,
                  swapSettlementDays_,
                  iborIndex_->currency(),
                  conventions_.calendar,
                  Period(fixedLegFrequency_),
                  fixedLegConvention_,
                  iborIndex_->dayCounter(),
                  iborIndex_));

    vegaWeightedSmileFit_=false;
}

QL_END_TEST_LOCALS(SwaptionVolatilityCubeTest)


void SwaptionVolatilityCubeTest::testAtmVols() {

    BOOST_MESSAGE("Testing swaption volatility cube (atm vols)...");

    SavedSettings backup;

    setup();

    SwaptionVolCube2 volCube(atmVolMatrix_,
                             cube_.tenors.options,
                             cube_.tenors.swaps,
                             cube_.strikeSpreads,
                             cube_.volSpreadsHandle,
                             swapIndexBase_,
                             vegaWeightedSmileFit_);

    Real tolerance = 1.0e-16;
    makeAtmVolTest(volCube, tolerance);
}

void SwaptionVolatilityCubeTest::testSmile() {

    BOOST_MESSAGE("Testing swaption volatility cube (smile)...");

    SavedSettings backup;

    setup();

    SwaptionVolCube2 volCube(atmVolMatrix_,
                             cube_.tenors.options,
                             cube_.tenors.swaps,
                             cube_.strikeSpreads,
                             cube_.volSpreadsHandle,
                             swapIndexBase_,
                             vegaWeightedSmileFit_);

    Real tolerance = 1.0e-16;
    makeVolSpreadsTest(volCube, tolerance);
}

void SwaptionVolatilityCubeTest::testSabrVols() {

    BOOST_MESSAGE("Testing swaption volatility cube (sabr interpolation)...");

    SavedSettings backup;

    setup();

    std::vector<std::vector<Handle<Quote> > >
        parametersGuess(cube_.tenors.options.size()*cube_.tenors.swaps.size());
    for (Size i=0; i<cube_.tenors.options.size()*cube_.tenors.swaps.size(); i++) {
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

    SwaptionVolCube1 volCube(atmVolMatrix_,
                             cube_.tenors.options,
                             cube_.tenors.swaps,
                             cube_.strikeSpreads,
                             cube_.volSpreadsHandle,
                             swapIndexBase_,
                             vegaWeightedSmileFit_,
                             parametersGuess,
                             isParameterFixed,
                             true);
    Real tolerance = 3.0e-4;
    makeAtmVolTest(volCube, tolerance);

    tolerance = 12.0e-4;
    makeVolSpreadsTest(volCube, tolerance);
}

void SwaptionVolatilityCubeTest::testSpreadedCube() {

    BOOST_MESSAGE("Testing spreaded swaption volatility cube...");

    SavedSettings backup;

    setup();

    std::vector<std::vector<Handle<Quote> > >
        parametersGuess(cube_.tenors.options.size()*cube_.tenors.swaps.size());
    for (Size i=0; i<cube_.tenors.options.size()*cube_.tenors.swaps.size(); i++) {
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
        SwaptionVolCube1(atmVolMatrix_,
                         cube_.tenors.options,
                         cube_.tenors.swaps,
                         cube_.strikeSpreads,
                         cube_.volSpreadsHandle,
                         swapIndexBase_,
                         vegaWeightedSmileFit_,
                         parametersGuess,
                         isParameterFixed,
                         true)));

    boost::shared_ptr<SimpleQuote> spread (new SimpleQuote(0.0001));
    Handle<Quote> spreadHandle(spread);
    boost::shared_ptr<SwaptionVolatilityStructure> spreadedVolCube
        (new SpreadedSwaptionVolatilityStructure(volCube, spreadHandle));
    std::vector<Real> strikes;
    for (Size k=1; k<100; k++)
        strikes.push_back(k*.01);
    for (Size i=0; i<cube_.tenors.options.size(); i++) {
        for (Size j=0; j<cube_.tenors.swaps.size(); j++) {
            boost::shared_ptr<SmileSection> smileSectionByCube =
                volCube->smileSection(cube_.tenors.options[i], cube_.tenors.swaps[j]);
            boost::shared_ptr<SmileSection> smileSectionBySpreadedCube =
                spreadedVolCube->smileSection(cube_.tenors.options[i], cube_.tenors.swaps[j]);
            for (Size k=0; k<strikes.size(); k++) {
                Real strike = strikes[k];
                Real diff = spreadedVolCube->volatility(cube_.tenors.options[i], cube_.tenors.swaps[j], strike)
                            - volCube->volatility(cube_.tenors.options[i], cube_.tenors.swaps[j], strike);
                if (std::fabs(diff-spread->value())>1e-16)
                    BOOST_ERROR("\ndiff!=spread in volatility method:"
                                "\nexpiry time = " << cube_.tenors.options[i] <<
                                "\nswap length = " << cube_.tenors.swaps[j] <<
                                "\n atm strike = " << io::rate(strike) <<
                                "\ndiff = " << diff <<
                                "\nspread = " << spread->value());

                diff = smileSectionBySpreadedCube->volatility(strike)
                       - smileSectionByCube->volatility(strike);
                if (std::fabs(diff-spread->value())>1e-16)
                    BOOST_ERROR("\ndiff!=spread in smile section method:"
                                "\nexpiry time = " << cube_.tenors.options[i] <<
                                "\nswap length = " << cube_.tenors.swaps[j] <<
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
    BOOST_MESSAGE("Testing volatility cube observability...");

    SavedSettings backup;

    setup();

    std::vector<std::vector<Handle<Quote> > >
        parametersGuess(cube_.tenors.options.size()*cube_.tenors.swaps.size());
    for (Size i=0; i<cube_.tenors.options.size()*cube_.tenors.swaps.size(); i++) {
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
    volCube1_0 = boost::shared_ptr<SwaptionVolCube1>(new SwaptionVolCube1(atmVolMatrix_,
                                                                cube_.tenors.options,
                                                                cube_.tenors.swaps,
                                                                cube_.strikeSpreads,
                                                                cube_.volSpreadsHandle,
                                                                swapIndexBase_,
                                                                vegaWeightedSmileFit_,
                                                                parametersGuess,
                                                                isParameterFixed,
                                                                true));

    Date referenceDate = Settings::instance().evaluationDate();
    Settings::instance().evaluationDate() =
        conventions_.calendar.advance(referenceDate, Period(1, Days), conventions_.optionBdc);

    // VolCube created after change of reference date
    volCube1_1 = boost::shared_ptr<SwaptionVolCube1>(new SwaptionVolCube1(atmVolMatrix_,
                                                                cube_.tenors.options,
                                                                cube_.tenors.swaps,
                                                                cube_.strikeSpreads,
                                                                cube_.volSpreadsHandle,
                                                                swapIndexBase_,
                                                                vegaWeightedSmileFit_,
                                                                parametersGuess,
                                                                isParameterFixed,
                                                                true));
    Rate dummyStrike = 0.03;
    for (Size i=0;i<cube_.tenors.options.size(); i++ ) {
        for (Size j=0; j<cube_.tenors.swaps.size(); j++) {
            for (Size k=0; k<cube_.strikeSpreads.size(); k++) {

                Volatility v0 = volCube1_0->volatility(cube_.tenors.options[i],
                                                       cube_.tenors.swaps[j],
                                                       dummyStrike + cube_.strikeSpreads[k],
                                                       false);
                Volatility v1 = volCube1_1->volatility(cube_.tenors.options[i],
                                                       cube_.tenors.swaps[j],
                                                       dummyStrike + cube_.strikeSpreads[k],
                                                       false);
                if (v0 != v1)
                    BOOST_ERROR(description <<
                                " option tenor = " << cube_.tenors.options[i] <<
                                " swap tenor = " << cube_.tenors.swaps[j] <<
                                " strike = " << io::rate(dummyStrike+cube_.strikeSpreads[k])<<
                                "  v0 = " << io::volatility(v0) <<
                                "  v1 = " << io::volatility(v1));
            }
        }
    }

    Settings::instance().evaluationDate() = referenceDate;

    boost::shared_ptr<SwaptionVolCube2> volCube2_0, volCube2_1;
    // VolCube created before change of reference date
    volCube2_0 = boost::shared_ptr<SwaptionVolCube2>(new SwaptionVolCube2(atmVolMatrix_,
                                                                cube_.tenors.options,
                                                                cube_.tenors.swaps,
                                                                cube_.strikeSpreads,
                                                                cube_.volSpreadsHandle,
                                                                swapIndexBase_,
                                                                vegaWeightedSmileFit_));
    Settings::instance().evaluationDate() =
        conventions_.calendar.advance(referenceDate, Period(1, Days), conventions_.optionBdc);

    // VolCube created after change of reference date
    volCube2_1 = boost::shared_ptr<SwaptionVolCube2>(new SwaptionVolCube2(atmVolMatrix_,
                                                                cube_.tenors.options,
                                                                cube_.tenors.swaps,
                                                                cube_.strikeSpreads,
                                                                cube_.volSpreadsHandle,
                                                                swapIndexBase_,
                                                                vegaWeightedSmileFit_));

    for (Size i=0;i<cube_.tenors.options.size(); i++ ) {
        for (Size j=0; j<cube_.tenors.swaps.size(); j++) {
            for (Size k=0; k<cube_.strikeSpreads.size(); k++) {

                Volatility v0 = volCube2_0->volatility(cube_.tenors.options[i],
                                                       cube_.tenors.swaps[j],
                                                       dummyStrike + cube_.strikeSpreads[k],
                                                       false);
                Volatility v1 = volCube2_1->volatility(cube_.tenors.options[i],
                                                       cube_.tenors.swaps[j],
                                                       dummyStrike + cube_.strikeSpreads[k],
                                                       false);
                if (v0 != v1)
                    BOOST_ERROR(description <<
                                " option tenor = " << cube_.tenors.options[i] <<
                                " swap tenor = " << cube_.tenors.swaps[j] <<
                                " strike = " << io::rate(dummyStrike+cube_.strikeSpreads[k])<<
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
    suite->add(BOOST_TEST_CASE(&SwaptionVolatilityCubeTest::testAtmVols));
    // SwaptionVolCubeByLinear reproduces smile spreads with machine precision
    suite->add(BOOST_TEST_CASE(&SwaptionVolatilityCubeTest::testSmile));

    // SwaptionVolCubeBySabr reproduces ATM vol with given tolerance
    // SwaptionVolCubeBySabr reproduces smile spreads with given tolerance
    suite->add(BOOST_TEST_CASE(&SwaptionVolatilityCubeTest::testSabrVols));
    suite->add(BOOST_TEST_CASE(&SwaptionVolatilityCubeTest::testSpreadedCube));

    suite->add(BOOST_TEST_CASE(&SwaptionVolatilityCubeTest::testObservability));

    return suite;
}
