/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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
#include "utilities.hpp"
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatilities/swaption/swaptionvolmatrix.hpp>
#include <ql/termstructures/volatilities/swaption/swaptionvolcube2.hpp>
#include <ql/termstructures/volatilities/swaption/swaptionvolcube1.hpp>
#include <ql/termstructures/volatilities/swaption/spreadedswaptionvolstructure.hpp>
#include <ql/utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(SwaptionVolatilityCubeTest)

// global data

Date referenceDate_;
Calendar calendar_;

BusinessDayConvention optionBDC_;
std::vector<Period> atmOptionTenors_;
std::vector<Period> atmSwapTenors_;
Matrix atmVols_;
DayCounter dayCounter_;
Handle<SwaptionVolatilityStructure> atmVolMatrix_;

std::vector<Period> optionTenors_;
std::vector<Period> swapTenors_;
std::vector<Spread> strikeSpreads_;
Matrix volSpreadsMatrix_;
std::vector<std::vector<Handle<Quote> > > volSpreads_;
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

    for (Size i=0; i<atmOptionTenors_.size(); i++) {
      for (Size j=0; j<atmSwapTenors_.size(); j++) {
          Rate strike = volCube.atmStrike(atmOptionTenors_[i], atmSwapTenors_[j]);
          Volatility expVol = atmVolMatrix_->volatility(
              atmOptionTenors_[i], atmSwapTenors_[j], strike, true);
          Volatility actVol = volCube.volatility(
              atmOptionTenors_[i], atmSwapTenors_[j], strike, true);
          Volatility error = std::abs(expVol-actVol);
          if (error>tolerance)
              BOOST_ERROR("\nrecovery of atm vols failed:"
                          "\nexpiry time = " << atmOptionTenors_[i] <<
                          "\nswap length = " << atmSwapTenors_[j] <<
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

    for (Size i=0; i<optionTenors_.size(); i++) {
      for (Size j=0; j<swapTenors_.size(); j++) {
          for (Size k=0; k<strikeSpreads_.size(); k++) {
              Rate atmStrike = volCube.atmStrike(optionTenors_[i], swapTenors_[j]);
              Volatility atmVol = atmVolMatrix_->volatility(
                  optionTenors_[i], swapTenors_[j], atmStrike, true);
              Volatility vol = volCube.volatility(
                  optionTenors_[i], swapTenors_[j], atmStrike+strikeSpreads_[k], true);
              Volatility spread = vol-atmVol;
              Volatility expVolSpread = volSpreadsMatrix_[i*swapTenors_.size()+j][k];
              Volatility error = std::abs(expVolSpread-spread);
              if (error>tolerance)
                  BOOST_ERROR("\nrecovery of smile vol spreads failed:"
                              "\n    option tenor = " << optionTenors_[i] <<
                              "\n      swap tenor = " << swapTenors_[j] <<
                              "\n      atm strike = " << io::rate(atmStrike) <<
                              "\n   strike spread = " << io::rate(strikeSpreads_[k]) <<
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

    referenceDate_ = Date(6, September, 2006);
    calendar_ = TARGET();

    Settings::instance().evaluationDate() = referenceDate_;

    optionBDC_ = Following;

    // ATM swaptionvolmatrix

    atmOptionTenors_ = std::vector<Period>();
    atmOptionTenors_.push_back(Period(1, Months));
    atmOptionTenors_.push_back(Period(6, Months));
    atmOptionTenors_.push_back(Period(1, Years));
    atmOptionTenors_.push_back(Period(5, Years));
    atmOptionTenors_.push_back(Period(10, Years));
    atmOptionTenors_.push_back(Period(30, Years));

    atmSwapTenors_ = std::vector<Period>();
    atmSwapTenors_.push_back(Period(1, Years));
    atmSwapTenors_.push_back(Period(5, Years));
    atmSwapTenors_.push_back(Period(10, Years));
    atmSwapTenors_.push_back(Period(30, Years));

    atmVols_ = Matrix(atmOptionTenors_.size(), atmSwapTenors_.size());
    atmVols_[0][0]=0.1300; atmVols_[0][1]=0.1560; atmVols_[0][2]=0.1390; atmVols_[0][3]=0.1220;
    atmVols_[1][0]=0.1440; atmVols_[1][1]=0.1580; atmVols_[1][2]=0.1460; atmVols_[1][3]=0.1260;
    atmVols_[2][0]=0.1600; atmVols_[2][1]=0.1590; atmVols_[2][2]=0.1470; atmVols_[2][3]=0.1290;
    atmVols_[3][0]=0.1640; atmVols_[3][1]=0.1470; atmVols_[3][2]=0.1370; atmVols_[3][3]=0.1220;
    atmVols_[4][0]=0.1400; atmVols_[4][1]=0.1300; atmVols_[4][2]=0.1250; atmVols_[4][3]=0.1100;
    atmVols_[5][0]=0.1130; atmVols_[5][1]=0.1090; atmVols_[5][2]=0.1070; atmVols_[5][3]=0.0930;

    Size nRowsAtmVols = atmVols_.rows();
    Size nColsAtmVols = atmVols_.columns();
    std::vector<std::vector<Handle<Quote> > > atmVolsHandle_;
    atmVolsHandle_ = std::vector<std::vector<Handle<Quote> > >(nRowsAtmVols);
    Size i;
    for (i=0; i<nRowsAtmVols; i++){
        atmVolsHandle_[i] = std::vector<Handle<Quote> >(nColsAtmVols);
        for (Size j=0; j<nColsAtmVols; j++) {
            // every handle must be reassigned, as the ones created by
            // default are all linked together.
            atmVolsHandle_[i][j] = Handle<Quote>(boost::shared_ptr<Quote>(new
                SimpleQuote(atmVols_[i][j])));
        }
    }

    dayCounter_ = Actual365Fixed();

    atmVolMatrix_ = Handle<SwaptionVolatilityStructure>(
        boost::shared_ptr<SwaptionVolatilityStructure>(new
            SwaptionVolatilityMatrix(calendar_,
                                     atmOptionTenors_,
                                     atmSwapTenors_,
                                     atmVolsHandle_,
                                     dayCounter_,
                                     optionBDC_)));

    //swaptionvolcube

    optionTenors_ = std::vector<Period>();
    optionTenors_.push_back(Period(1, Years));
    optionTenors_.push_back(Period(10, Years));
    optionTenors_.push_back(Period(30, Years));

    swapTenors_ = std::vector<Period>();
    swapTenors_.push_back(Period(2, Years));
    swapTenors_.push_back(Period(10, Years));
    swapTenors_.push_back(Period(30, Years));

    strikeSpreads_ = std::vector<Rate>();
    strikeSpreads_.push_back(-0.020);
    strikeSpreads_.push_back(-0.005);
    strikeSpreads_.push_back(+0.000);
    strikeSpreads_.push_back(+0.005);
    strikeSpreads_.push_back(+0.020);

    Size nRows = optionTenors_.size()*swapTenors_.size();
    Size nCols = strikeSpreads_.size();
    volSpreadsMatrix_ = Matrix(nRows, nCols);
    volSpreadsMatrix_[0][0]=0.0599; volSpreadsMatrix_[0][1]=0.0049;
    volSpreadsMatrix_[0][2]=0.0000;
    volSpreadsMatrix_[0][3]=-0.0001; volSpreadsMatrix_[0][4]=0.0127;

    volSpreadsMatrix_[1][0]=0.0729; volSpreadsMatrix_[1][1]=0.0086;
    volSpreadsMatrix_[1][2]=0.0000;
    volSpreadsMatrix_[1][3]=-0.0024; volSpreadsMatrix_[1][4]=0.0098;

    volSpreadsMatrix_[2][0]=0.0738; volSpreadsMatrix_[2][1]=0.0102;
    volSpreadsMatrix_[2][2]=0.0000;
    volSpreadsMatrix_[2][3]=-0.0039; volSpreadsMatrix_[2][4]=0.0065;

    volSpreadsMatrix_[3][0]=0.0465; volSpreadsMatrix_[3][1]=0.0063;
    volSpreadsMatrix_[3][2]=0.0000;
    volSpreadsMatrix_[3][3]=-0.0032; volSpreadsMatrix_[3][4]=-0.0010;

    volSpreadsMatrix_[4][0]=0.0558; volSpreadsMatrix_[4][1]=0.0084;
    volSpreadsMatrix_[4][2]=0.0000;
    volSpreadsMatrix_[4][3]=-0.0050; volSpreadsMatrix_[4][4]=-0.0057;

    volSpreadsMatrix_[5][0]=0.0576; volSpreadsMatrix_[5][1]=0.0083;
    volSpreadsMatrix_[5][2]=0.0000;
    volSpreadsMatrix_[5][3]=-0.0043; volSpreadsMatrix_[5][4]=-0.0014;

    volSpreadsMatrix_[6][0]=0.0437; volSpreadsMatrix_[6][1]=0.0059;
    volSpreadsMatrix_[6][2]=0.0000;
    volSpreadsMatrix_[6][3]=-0.0030; volSpreadsMatrix_[6][4]=-0.0006;

    volSpreadsMatrix_[7][0]=0.0533; volSpreadsMatrix_[7][1]=0.0078;
    volSpreadsMatrix_[7][2]=0.0000;
    volSpreadsMatrix_[7][3]=-0.0045; volSpreadsMatrix_[7][4]=-0.0046;

    volSpreadsMatrix_[8][0]=0.0545; volSpreadsMatrix_[8][1]=0.0079;
    volSpreadsMatrix_[8][2]=0.0000;
    volSpreadsMatrix_[8][3]=-0.0042; volSpreadsMatrix_[8][4]=-0.0020;

    volSpreads_ = std::vector<std::vector<Handle<Quote> > >(nRows);
    for (i=0; i<optionTenors_.size()*swapTenors_.size(); i++){
        volSpreads_[i] = std::vector<Handle<Quote> >(nCols);
        for (Size j=0; j<strikeSpreads_.size(); j++) {
            // every handle must be reassigned, as the ones created by
            // default are all linked together.
            volSpreads_[i][j] = Handle<Quote>(boost::shared_ptr<Quote>(new
                SimpleQuote(volSpreadsMatrix_[i][j])));
        }
    }

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
                  calendar_,
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
                                           optionTenors_,
                                           swapTenors_,
                                           strikeSpreads_,
                                           volSpreads_,
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
                                           optionTenors_,
                                           swapTenors_,
                                           strikeSpreads_,
                                           volSpreads_,
                                           swapIndexBase_,
                                           vegaWeightedSmileFit_);

    Real tolerance = 1.0e-16;
    makeVolSpreadsTest(volCube, tolerance);
}

void SwaptionVolatilityCubeTest::testSabrVols() {

    BOOST_MESSAGE("Testing swaption volatility cube (sabr interpolation)...");

    SavedSettings backup;

    setup();

    std::vector<std::vector<Handle<Quote> > > parametersGuess(optionTenors_.size()*swapTenors_.size());
    for (Size i=0; i<optionTenors_.size()*swapTenors_.size(); i++) {
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
                                         optionTenors_,
                                         swapTenors_,
                                         strikeSpreads_,
                                         volSpreads_,
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

    std::vector<std::vector<Handle<Quote> > > parametersGuess(optionTenors_.size()*swapTenors_.size());
    for (Size i=0; i<optionTenors_.size()*swapTenors_.size(); i++) {
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
                         optionTenors_,
                         swapTenors_,
                         strikeSpreads_,
                         volSpreads_,
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
    for (Size i=0; i<optionTenors_.size(); i++) {
        for (Size j=0; j<swapTenors_.size(); j++) {
            boost::shared_ptr<SmileSection> smileSectionByCube =
                volCube->smileSection(optionTenors_[i], swapTenors_[j]);
            boost::shared_ptr<SmileSection> smileSectionBySpreadedCube =
                spreadedVolCube->smileSection(optionTenors_[i], swapTenors_[j]);
            for (Size k=0; k<strikes.size(); k++) {
                Real strike = strikes[k];
                Real diff = spreadedVolCube->volatility(optionTenors_[i], swapTenors_[j], strike)
                            - volCube->volatility(optionTenors_[i], swapTenors_[j], strike);
                if (std::fabs(diff-spread->value())>1e-16)
                    BOOST_ERROR("\ndiff!=spread in volatility method:"
                                "\nexpiry time = " << optionTenors_[i] <<
                                "\nswap length = " << swapTenors_[j] <<
                                "\n atm strike = " << io::rate(strike) <<
                                "\ndiff = " << diff <<
                                "\nspread = " << spread->value());

                diff = smileSectionBySpreadedCube->volatility(strike)
                       - smileSectionByCube->volatility(strike);
                if (std::fabs(diff-spread->value())>1e-16)
                    BOOST_ERROR("\ndiff!=spread in smile section method:"
                                "\nexpiry time = " << optionTenors_[i] <<
                                "\nswap length = " << swapTenors_[j] <<
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
        BOOST_ERROR("spreadedCapletVolatilityStructure "
                    << "does not propagate notifications");
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

    return suite;
}
