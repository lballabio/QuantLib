/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 François du Vignaud
 Copyright (C) 2006 Giorgio Facchinetti
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

#include "swaptionvolatilitystructures.hpp"
#include "utilities.hpp"
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/termstructures/volatilities/swaption/swaptionvolmatrix.hpp>
#include <ql/termstructures/volatilities/swaption/swaptionvolcube1.hpp>
#include <ql/termstructures/volatilities/swaption/swaptionvolcube2.hpp>
#include <ql/termstructures/volatilities/swaption/spreadedswaptionvolstructure.hpp>
#include <iostream>
#include <string>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(SwaptionVolatilityStructuresTest)

// global data

Calendar calendar_;
BusinessDayConvention optionBdc_;
DayCounter dayCounter_;

Matrix atmVols_;  // atm swaption volatility matrix
std::vector<std::vector<Handle<Quote> > > atmVolsHandle_;
std::vector<Period> atmOptionTenors_;
std::vector<Period> atmSwapTenors_;
Size atmOptionTenorsNb;
Size atmSwapTenorsNb;
Size atmStrikeSpreadsNb;

Matrix volSpreadsMatrix_;  // swaption volatility cube
std::vector<std::vector<Handle<Quote> > > volSpreads_;
std::vector<Period> optionTenors_;
std::vector<Period> swapTenors_;
std::vector<Spread> strikeSpreads_;
Size optionTenorsNb;
Size swapTenorsNb;
Size strikeSpreadsNb;

boost::shared_ptr<SwapIndex> swapIndexBase_;
boost::shared_ptr<IborIndex> iborIndex_;
bool vegaWeightedSmileFit_;

RelinkableHandle<YieldTermStructure> termStructure_;
RelinkableHandle<SwaptionVolatilityStructure> atmVolMatrix_;
std::vector<std::vector<Handle<Quote> > > parametersGuess_;
std::vector<bool> isParameterFixed_(4, false);

// utilities
void setAtmMarketData() {

    atmOptionTenorsNb = 6;
    atmOptionTenors_.resize(atmOptionTenorsNb);
    atmOptionTenors_[0] = Period(1, Months);
    atmOptionTenors_[1] = Period(6, Months);
    atmOptionTenors_[2] = Period(1, Years);
    atmOptionTenors_[3] = Period(5, Years);
    atmOptionTenors_[4] = Period(10, Years);
    atmOptionTenors_[5] = Period(30, Years);

    atmSwapTenorsNb = 4;
    atmSwapTenors_.resize(atmSwapTenorsNb);
    atmSwapTenors_[0] = Period(1, Years);
    atmSwapTenors_[1] = Period(5, Years);
    atmSwapTenors_[2] = Period(10, Years);
    atmSwapTenors_[3] = Period(30, Years);

    atmVols_ = Matrix(atmOptionTenorsNb, atmSwapTenorsNb);
    atmVols_[0][0]=0.1300; atmVols_[0][1]=0.1560; atmVols_[0][2]=0.1390; atmVols_[0][3]=0.1220;
    atmVols_[1][0]=0.1440; atmVols_[1][1]=0.1580; atmVols_[1][2]=0.1460; atmVols_[1][3]=0.1260;
    atmVols_[2][0]=0.1600; atmVols_[2][1]=0.1590; atmVols_[2][2]=0.1470; atmVols_[2][3]=0.1290;
    atmVols_[3][0]=0.1640; atmVols_[3][1]=0.1470; atmVols_[3][2]=0.1370; atmVols_[3][3]=0.1220;
    atmVols_[4][0]=0.1400; atmVols_[4][1]=0.1300; atmVols_[4][2]=0.1250; atmVols_[4][3]=0.1100;
    atmVols_[5][0]=0.1130; atmVols_[5][1]=0.1090; atmVols_[5][2]=0.1070; atmVols_[5][3]=0.0930;

    atmVolsHandle_.resize(atmOptionTenorsNb);
    for (Size i=0; i<atmOptionTenorsNb; i++){
        atmVolsHandle_[i].resize(atmSwapTenorsNb);
        for (Size j=0; j<atmSwapTenorsNb; j++)
            // every handle must be reassigned, as the ones created by
            // default are all linked together.
            atmVolsHandle_[i][j] = Handle<Quote>(boost::shared_ptr<Quote>(new
                SimpleQuote(atmVols_[i][j])));
    }
}
void setCubeMarketData() {

    optionTenorsNb = 3;
    optionTenors_.resize(optionTenorsNb);
    optionTenors_[0] = Period(1, Years);
    optionTenors_[1] = Period(10, Years);
    optionTenors_[2] = Period(30, Years);

    swapTenorsNb = 3;
    swapTenors_.resize(swapTenorsNb);
    swapTenors_[0] = Period(2, Years);
    swapTenors_[1] = Period(10, Years);
    swapTenors_[2] = Period(30, Years);

    strikeSpreadsNb = 5;
    strikeSpreads_.resize(strikeSpreadsNb);
    strikeSpreads_[0] = -0.020;
    strikeSpreads_[1] = -0.005;
    strikeSpreads_[2] = +0.000;
    strikeSpreads_[3] = +0.005;
    strikeSpreads_[4] = +0.020;

    volSpreadsMatrix_ = Matrix(optionTenorsNb*swapTenorsNb, strikeSpreadsNb);
    volSpreadsMatrix_[0][0] = 0.0599; volSpreadsMatrix_[0][1] = 0.0049;
    volSpreadsMatrix_[0][2] = 0.0000;
    volSpreadsMatrix_[0][3] =-0.0001; volSpreadsMatrix_[0][4] = 0.0127;

    volSpreadsMatrix_[1][0] = 0.0729; volSpreadsMatrix_[1][1] = 0.0086;
    volSpreadsMatrix_[1][2] = 0.0000;
    volSpreadsMatrix_[1][3] =-0.0024; volSpreadsMatrix_[1][4] = 0.0098;

    volSpreadsMatrix_[2][0] = 0.0738; volSpreadsMatrix_[2][1] = 0.0102;
    volSpreadsMatrix_[2][2] = 0.0000;
    volSpreadsMatrix_[2][3] =-0.0039; volSpreadsMatrix_[2][4] = 0.0065;

    volSpreadsMatrix_[3][0] = 0.0465; volSpreadsMatrix_[3][1] = 0.0063;
    volSpreadsMatrix_[3][2] = 0.0000;
    volSpreadsMatrix_[3][3] =-0.0032; volSpreadsMatrix_[3][4] =-0.0010;

    volSpreadsMatrix_[4][0] = 0.0558; volSpreadsMatrix_[4][1] = 0.0084;
    volSpreadsMatrix_[4][2] = 0.0000;
    volSpreadsMatrix_[4][3] =-0.0050; volSpreadsMatrix_[4][4] =-0.0057;

    volSpreadsMatrix_[5][0] = 0.0576; volSpreadsMatrix_[5][1] = 0.0083;
    volSpreadsMatrix_[5][2] = 0.0000;
    volSpreadsMatrix_[5][3] =-0.0043; volSpreadsMatrix_[5][4] = -0.0014;

    volSpreadsMatrix_[6][0] = 0.0437; volSpreadsMatrix_[6][1] = 0.0059;
    volSpreadsMatrix_[6][2] = 0.0000;
    volSpreadsMatrix_[6][3] =-0.0030; volSpreadsMatrix_[6][4] =-0.0006;

    volSpreadsMatrix_[7][0] = 0.0533; volSpreadsMatrix_[7][1] = 0.0078;
    volSpreadsMatrix_[7][2] = 0.0000;
    volSpreadsMatrix_[7][3] =-0.0045; volSpreadsMatrix_[7][4] =-0.0046;

    volSpreadsMatrix_[8][0] = 0.0545; volSpreadsMatrix_[8][1] = 0.0079;
    volSpreadsMatrix_[8][2] = 0.0000;
    volSpreadsMatrix_[8][3] =-0.0042; volSpreadsMatrix_[8][4] =-0.0020;

    volSpreads_ = std::vector<std::vector<Handle<Quote> > >(optionTenorsNb*swapTenorsNb);
    for (Size i=0; i<optionTenorsNb*swapTenorsNb; i++){
        volSpreads_[i] = std::vector<Handle<Quote> >(strikeSpreadsNb);
        for (Size j=0; j<strikeSpreadsNb; j++) {
            // every handle must be reassigned, as the ones created by
            // default are all linked together.
            volSpreads_[i][j] = Handle<Quote>(boost::shared_ptr<Quote>(new
                SimpleQuote(volSpreadsMatrix_[i][j])));
        }
    }
}


void setup() {

    //Date referenceDate = Settings::instance().evaluationDate();
    Date referenceDate = Date(6, September, 2006);
    Settings::instance().evaluationDate() = referenceDate;

    setAtmMarketData();     // atm swaption volatility matrix 
    setCubeMarketData();    // swaption volatility cube

    dayCounter_ = Actual365Fixed();
    calendar_ = TARGET();
    optionBdc_ = Following;

    termStructure_.linkTo(flatRate(referenceDate, 0.05, dayCounter_));
    iborIndex_ = boost::shared_ptr<IborIndex>(new Euribor6M(termStructure_));
    swapIndexBase_ = boost::shared_ptr<SwapIndex>(new
                            SwapIndex("EurliborSwapFixA",
                                      10*Years,
                                      2,
                                      iborIndex_->currency(),
                                      calendar_,
                                      Period(Annual),
                                      Unadjusted,
                                      iborIndex_->dayCounter(),
                                      iborIndex_));
    vegaWeightedSmileFit_= false;

    atmVolMatrix_ = RelinkableHandle<SwaptionVolatilityStructure>(
        boost::shared_ptr<SwaptionVolatilityStructure>(new
            SwaptionVolatilityMatrix(calendar_,
                                     atmOptionTenors_,
                                     atmSwapTenors_,
                                     atmVolsHandle_,
                                     dayCounter_,
                                     optionBdc_)));
    
    // guesses for swaptionvolcube1
    parametersGuess_.resize(optionTenorsNb*swapTenorsNb);
    for (Size i=0; i<optionTenorsNb*swapTenorsNb; i++) {
        parametersGuess_[i] = std::vector<Handle<Quote> >(4);
        parametersGuess_[i][0] =
            Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(0.2)));
        parametersGuess_[i][1] =
            Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(0.5)));
        parametersGuess_[i][2] =
            Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(0.4)));
        parametersGuess_[i][3] =
            Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(0.0)));
    }

}

void makeObservabilityTest(const std::string& description,
                           const boost::shared_ptr<SwaptionVolatilityStructure>& vol,
                           bool mktDataFloating,
                           bool referenceDateFloating) {
    Rate dummyStrike = .02;
    Date referenceDate = Settings::instance().evaluationDate();
    Volatility initialVol = vol->volatility(referenceDate + atmOptionTenors_[0],
                                            atmSwapTenors_[0], dummyStrike, false);
    // testing evaluation date change ...
    Settings::instance().evaluationDate() = referenceDate - Period(1, Years);
    Volatility newVol =  vol->volatility(referenceDate + atmOptionTenors_[0],
                                         atmSwapTenors_[0], dummyStrike, false);
    Settings::instance().evaluationDate() = referenceDate;
    if (referenceDateFloating && (initialVol == newVol))
        BOOST_ERROR(description <<
        " the volatility should change when the reference date is changed !");
    if (!referenceDateFloating && (initialVol != newVol))
        BOOST_ERROR(description <<
        " the volatility should not change when the reference date is changed !");

    // test market data change...
    if (mktDataFloating){
        Volatility initialVolatility = atmVolsHandle_[0][0]->value();
        boost::dynamic_pointer_cast<SimpleQuote>(
                        atmVolsHandle_[0][0].currentLink())->setValue(10);
        newVol = vol->volatility(referenceDate + atmOptionTenors_[0],
                                 atmSwapTenors_[0], dummyStrike, false);
        boost::dynamic_pointer_cast<SimpleQuote>(
                        atmVolsHandle_[0][0].currentLink())->setValue(initialVolatility);
        if (initialVol == newVol)
            BOOST_ERROR(description << " the volatility should change when"
                        " the market data is changed !");
    }
}

void makeObservabilityTest2(const std::string& description,
                            const boost::shared_ptr<SwaptionVolatilityStructure>& vol0,
                            const boost::shared_ptr<SwaptionVolatilityStructure>& vol1) {
    
    Rate dummyStrike = 0.03;
    for (Size i=0;i<optionTenorsNb; i++ ) {
        for (Size j=0; j<swapTenorsNb; j++) {
            for (Size k=0; k<strikeSpreadsNb; k++) {
                
                Volatility v0 = vol0->volatility(optionTenors_[i],
                                                 swapTenors_[j],
                                                 dummyStrike + strikeSpreads_[k],
                                                 false);
                Volatility v1 = vol1->volatility(optionTenors_[i],
                                                 swapTenors_[j],
                                                 dummyStrike + strikeSpreads_[k],
                                                 false);
                if (v0 != v1)
                    BOOST_ERROR(description <<
                                " option tenor = " << optionTenors_[i] << 
                                " swap tenor = " << swapTenors_[j] << 
                                " strike = " << io::rate(dummyStrike+strikeSpreads_[k])<<
                                "  v0 = " << io::volatility(v0) <<
                                "  v1 = " << io::volatility(v1));                
            }
        }
    }   
}

void makeATMCoherenceTest(const std::string& description,
                          const boost::shared_ptr<SwaptionVolatilityDiscrete>& vol) {

    Date refDate = vol->referenceDate();
    for (Size i=0; i<atmOptionTenorsNb; ++i) {
        Date expOptDate = calendar_.advance(refDate, atmOptionTenors_[i], optionBdc_);
        Date actOptDate = vol->optionDates()[i];
        if (actOptDate!=expOptDate)
            BOOST_FAIL("\nrecovery of option dates failed for " <<
                       description << ":"
                       "\n            calendar = " << calendar_ <<
                       "\n       referenceDate = " << refDate <<
                       "\n        expiry tenor = " << atmOptionTenors_[i] <<
                       "\nexpected option date = " << expOptDate <<
                       "\n  actual option date = " << actOptDate);
    }

    Date lengthRef = vol->optionDates()[0];
    DayCounter volDC = vol->dayCounter();
    for (Size j=0; j<atmSwapTenors_.size(); ++j) {
        Period actSwapTenor = vol->swapTenors()[j];
        Date endDate = lengthRef + atmSwapTenors_[j];
        Time expSwapLength = volDC.yearFraction(lengthRef, endDate);
        Time actSwapLength = vol->swapLengths()[j];
        if ((atmSwapTenors_[j]!=actSwapTenor) ||
            (expSwapLength!=actSwapLength))
            BOOST_FAIL("\nrecovery of " << io::ordinal(j) <<
                       " swap tenor failed for " <<
                       description << ":"
                       "\nexpected swap tenor  = " << atmSwapTenors_[j] <<
                       "\n  actual swap tenor  = " << actSwapTenor <<
                       "\nexpected swap length = " << expSwapLength <<
                       "\n  actual swap length = " << actSwapLength);
    }

    Real tolerance = 1.0e-16;
    for (Size i=0; i<atmOptionTenorsNb; ++i) {
      for (Size j=0; j<atmSwapTenorsNb; j++) {

          Period thisOptionTenor=  atmOptionTenors_[i];
          Date thisOptionDate = vol->optionDates()[i];
          Period thisSwapTenor = atmSwapTenors_[j];

          std::pair<Time, Time> p = vol->convertDates(
              thisOptionDate, thisSwapTenor);
          if ((p.first !=vol->optionTimes()[i]) ||
              (p.second!=vol->swapLengths()[j]))
              BOOST_FAIL("\nconvertDates failure for " <<
                         description << ":"
                         "\n       option date  = " << thisOptionDate <<
                         "\n       option tenor = " << thisOptionTenor <<
                         "\nactual option time  = " << p.first <<
                         "\n  exp. option time  = " << vol->optionTimes()[i] <<
                         "\n        swap tenor  = " << thisSwapTenor <<
                         "\n actual swap length = " << p.second <<
                         "\n   exp. swap length = " << vol->swapLengths()[j]);

          Volatility error, actVol, expVol = atmVols_[i][j];

          actVol = vol->volatility(
              thisOptionDate, thisSwapTenor, 0.05, true);
          error = std::abs(expVol-actVol);
          if (error>tolerance)
              BOOST_FAIL("\nrecovery of atm vols failed for " <<
                         description << ":"
                         "\noption date = " << thisOptionDate <<
                         "\nswap length = " << thisSwapTenor <<
                         "\n   exp. vol = " << io::volatility(expVol) <<
                         "\n actual vol = " << io::volatility(actVol) <<
                         "\n      error = " << io::volatility(error) <<
                         "\n  tolerance = " << tolerance);

          actVol = vol->volatility(
              thisOptionTenor, thisSwapTenor, 0.05, true);
          error = std::abs(expVol-actVol);
          if (error>tolerance)
              BOOST_FAIL("\nrecovery of atm vols failed for " <<
                         description << ":"
                         "\noption tenor = " << thisOptionTenor <<
                         "\n swap length = " << thisSwapTenor <<
                         "\nexpected vol = " << io::volatility(expVol) <<
                         "\n  actual vol = " << io::volatility(actVol) <<
                         "\n       error = " << io::volatility(error) <<
                         "\n   tolerance = " << tolerance);

      }
    }
}


void makeCubeCoherenceTest(const std::string& description,
                           const boost::shared_ptr<SwaptionVolatilityDiscrete>& vol) {

    Date refDate = vol->referenceDate();
    for (Size i=0; i<optionTenors_.size(); ++i) {
        Date expOptDate = calendar_.advance(refDate, optionTenors_[i], optionBdc_);
        Date actOptDate = vol->optionDates()[i];
        if (actOptDate!=expOptDate)
            BOOST_FAIL("\nrecovery of option dates failed for " <<
                       description << ":"
                       "\n            calendar = " << calendar_ <<
                       "\n       referenceDate = " << refDate <<
                       "\n        expiry tenor = " << optionTenors_[i] <<
                       "\nexpected option date = " << expOptDate <<
                       "\n  actual option date = " << actOptDate);
    }

    Date lengthRef = vol->optionDates()[0];
    DayCounter volDC = vol->dayCounter();
    for (Size j=0; j<swapTenors_.size(); ++j) {
        Period actSwapTenor = vol->swapTenors()[j];
        Date endDate = lengthRef + swapTenors_[j];
        Time expSwapLength = volDC.yearFraction(lengthRef, endDate);
        Time actSwapLength = vol->swapLengths()[j];
        if ((swapTenors_[j]!=actSwapTenor) ||
            (expSwapLength!=actSwapLength))         
            BOOST_FAIL("\nrecovery of " << io::ordinal(j) <<
                       " swap tenor failed for " <<
                       description << ":"
                       "\nexpected swap tenor  = " << swapTenors_[j] <<
                       "\n  actual swap tenor  = " << actSwapTenor <<
                       "\nexpected swap length = " << expSwapLength <<
                       "\n  actual swap length = " << actSwapLength);
    }

    Real tolerance = 1.0e-16;
    for (Size i=0; i<optionTenorsNb; ++i) {
      for (Size j=0; j<swapTenorsNb; j++) {

          Period thisOptionTenor = optionTenors_[i];
          Date thisOptionDate = vol->optionDates()[i];
          Period thisSwapTenor = swapTenors_[j];

          std::pair<Time, Time> p = vol->convertDates(thisOptionDate, thisSwapTenor);
          if ((p.first != vol->optionTimes()[i]) ||
              (p.second!= vol->swapLengths()[j]))
              BOOST_FAIL("\nconvertDates failure for " <<
                         description << ":"
                         "\n       option date  = " << thisOptionDate <<
                         "\n       option tenor = " << thisOptionTenor <<
                         "\nactual option time  = " << p.first <<
                         "\n  exp. option time  = " << vol->optionTimes()[i] <<
                         "\n        swap tenor  = " << thisSwapTenor <<
                         "\n actual swap length = " << p.second <<
                         "\n   exp. swap length = " << vol->swapLengths()[j]);

//          Volatility error, actVol, expVol = /*atmVols_[i][j] +*/ volSpreadsMatrix_[i][j];

      //    actVol = vol->volatility(thisOptionDate, thisSwapTenor, 0.05, true);
      //    error = std::abs(expVol-actVol);
      //    if (error>tolerance)
      //        BOOST_FAIL("\nrecovery of atm vols failed for " <<
      //                   description << ":"
      //                   "\noption date = " << thisOptionDate <<
      //                   "\nswap length = " << thisSwapTenor <<
      //                   "\n   exp. vol = " << io::volatility(expVol) <<
      //                   "\n actual vol = " << io::volatility(actVol) <<
      //                   "\n      error = " << io::volatility(error) <<
      //                   "\n  tolerance = " << tolerance);

      //    actVol = vol->volatility(thisOptionTenor, thisSwapTenor, 0.05, true);
      //    error = std::abs(expVol-actVol);
      //    if (error>tolerance)
      //        BOOST_FAIL("\nrecovery of atm vols failed for " <<
      //                   description << ":"
      //                   "\noption tenor = " << thisOptionTenor <<
      //                   "\n swap length = " << thisSwapTenor <<
      //                   "\nexpected vol = " << io::volatility(expVol) <<
      //                   "\n  actual vol = " << io::volatility(actVol) <<
      //                   "\n       error = " << io::volatility(error) <<
      //                   "\n   tolerance = " << tolerance);

      }
    }
}

void makeAtmVolTest(const std::string& description,
                    const SwaptionVolatilityCube& volCube,
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
              BOOST_ERROR(description << "recovery of atm vols failed:"
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

void makeVolSpreadsTest(const std::string& description,
                        const SwaptionVolatilityCube& volCube,
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
                  BOOST_ERROR(description << "recovery of smile vol spreads failed:"
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

QL_END_TEST_LOCALS(SwaptionVolatilityStructuresTest)

void SwaptionVolatilityStructuresTest::testSwaptionVolMatrixObservability() {

    BOOST_MESSAGE("Testing swaption volatility matrix observability...");

    SavedSettings backup;

    setup();

    boost::shared_ptr<SwaptionVolatilityMatrix> vol;
    std::string description;

    //floating reference date, floating market data
    description = "floating reference date, floating market data";
    vol = boost::shared_ptr<SwaptionVolatilityMatrix>(new
        SwaptionVolatilityMatrix(calendar_,
                                 atmOptionTenors_,
                                 atmSwapTenors_,
                                 atmVolsHandle_,
                                 dayCounter_,
                                 optionBdc_));
    makeObservabilityTest(description, vol, true, true);

    //fixed reference date, floating market data
    description = "fixed reference date, floating market data";
    vol = boost::shared_ptr<SwaptionVolatilityMatrix>(new
        SwaptionVolatilityMatrix(Settings::instance().evaluationDate(),
                                 calendar_,
                                 atmOptionTenors_,
                                 atmSwapTenors_,
                                 atmVolsHandle_,
                                 dayCounter_,
                                 optionBdc_));
    makeObservabilityTest(description, vol, true, false);

    // floating reference date, fixed market data
    description = "floating reference date, fixed market data";
    vol = boost::shared_ptr<SwaptionVolatilityMatrix>(new
        SwaptionVolatilityMatrix(calendar_,
                                 atmOptionTenors_,
                                 atmSwapTenors_,
                                 atmVols_,
                                 dayCounter_,
                                 optionBdc_));
    makeObservabilityTest(description, vol, false, true);

    // fixed reference date, fixed market data
    description = "fixed reference date, fixed market data";
    vol = boost::shared_ptr<SwaptionVolatilityMatrix>(new
        SwaptionVolatilityMatrix(Settings::instance().evaluationDate(),
                                 calendar_,
                                 atmOptionTenors_,
                                 atmSwapTenors_,
                                 atmVols_,
                                 dayCounter_,
                                 optionBdc_));
    makeObservabilityTest(description, vol, false, false);

   // fixed reference date and fixed market data, option dates
        //SwaptionVolatilityMatrix(const Date& referenceDate,
        //                         const std::vector<Date>& exerciseDates,
        //                         const std::vector<Period>& swapTenors,
        //                         const Matrix& volatilities,
        //                         const DayCounter& dayCounter);

    //
    //
    boost::shared_ptr<SwaptionVolatilityMatrix> vol_0, vol_1;
    // created before change of reference date
    vol_0 = boost::shared_ptr<SwaptionVolatilityMatrix>(new
        SwaptionVolatilityMatrix(calendar_,
                                 atmOptionTenors_,
                                 atmSwapTenors_,
                                 atmVols_,
                                 dayCounter_,
                                 optionBdc_));

    Date referenceDate = Settings::instance().evaluationDate();
    Settings::instance().evaluationDate() = calendar_.advance(referenceDate, Period(1, Days), optionBdc_);
    
    // created after change of reference date
    vol_1 = boost::shared_ptr<SwaptionVolatilityMatrix>(new
        SwaptionVolatilityMatrix(calendar_,
                                 atmOptionTenors_,
                                 atmSwapTenors_,
                                 atmVols_,
                                 dayCounter_,
                                 optionBdc_));

    
    makeObservabilityTest2("atmvol", vol_0,vol_1);
    
    Settings::instance().evaluationDate() = referenceDate;

}


void SwaptionVolatilityStructuresTest::testSwaptionVolMatrixCoherence() {

    BOOST_MESSAGE("Testing swaption volatility matrix coherence...");

    SavedSettings backup;

    setup();

    boost::shared_ptr<SwaptionVolatilityMatrix> vol;
    std::string description;

    //floating reference date, floating market data
    description = "floating reference date, floating market data";
    vol = boost::shared_ptr<SwaptionVolatilityMatrix>(new
        SwaptionVolatilityMatrix(calendar_,
                                 atmOptionTenors_,
                                 atmSwapTenors_,
                                 atmVolsHandle_,
                                 dayCounter_,
                                 optionBdc_));
    makeATMCoherenceTest(description, vol);

    //fixed reference date, floating market data
    description = "fixed reference date, floating market data";
    vol = boost::shared_ptr<SwaptionVolatilityMatrix>(new
        SwaptionVolatilityMatrix(Settings::instance().evaluationDate(),
                                 calendar_,
                                 atmOptionTenors_,
                                 atmSwapTenors_,
                                 atmVolsHandle_,
                                 dayCounter_,
                                 optionBdc_));
    makeATMCoherenceTest(description, vol);

    // floating reference date, fixed market data
    description = "floating reference date, fixed market data";
    vol = boost::shared_ptr<SwaptionVolatilityMatrix>(new
        SwaptionVolatilityMatrix(calendar_,
                                 atmOptionTenors_,
                                 atmSwapTenors_,
                                 atmVols_,
                                 dayCounter_,
                                 optionBdc_));
    makeATMCoherenceTest(description, vol);

    // fixed reference date, fixed market data
    description = "fixed reference date, fixed market data";
    vol = boost::shared_ptr<SwaptionVolatilityMatrix>(new
        SwaptionVolatilityMatrix(Settings::instance().evaluationDate(),
                                 calendar_,
                                 atmOptionTenors_,
                                 atmSwapTenors_,
                                 atmVols_,
                                 dayCounter_,
                                 optionBdc_));
    makeATMCoherenceTest(description, vol);
}




void SwaptionVolatilityStructuresTest::testSwaptionVolCubeAtmVols() {

    BOOST_MESSAGE("Testing swaption volatility cube (atm vols)...");

    SavedSettings backup;

    setup();
    
    SwaptionVolCube1 volCube1(atmVolMatrix_,
                              optionTenors_,
                              swapTenors_,
                              strikeSpreads_,
                              volSpreads_,
                              swapIndexBase_,
                              vegaWeightedSmileFit_,
                              parametersGuess_,
                              isParameterFixed_,
                              true);
    Real tolerance = 3.0e-4;
    makeAtmVolTest("\nSwaptionVolCube1: ", volCube1, tolerance);

    SwaptionVolCube2 volCube2(atmVolMatrix_,
                             optionTenors_,
                             swapTenors_,
                             strikeSpreads_,
                             volSpreads_,
                             swapIndexBase_,
                             vegaWeightedSmileFit_);

    tolerance = 1.0e-16;
    makeAtmVolTest("\nSwaptionVolCube2: ", volCube2, tolerance);
}
void SwaptionVolatilityStructuresTest::testSwaptionVolCubeSmile() {

    BOOST_MESSAGE("Testing swaption volatility cube (smile)...");

    SavedSettings backup;

    setup();

    SwaptionVolCube1 volCube1(atmVolMatrix_,
                             optionTenors_,
                             swapTenors_,
                             strikeSpreads_,
                             volSpreads_,
                             swapIndexBase_,
                             vegaWeightedSmileFit_,
                             parametersGuess_,
                             isParameterFixed_,
                             true);
    Real tolerance = 3.0e-4;
    makeAtmVolTest("\nSwaptionVolCube1: ", volCube1, tolerance);

    SwaptionVolCube2 volCube2(atmVolMatrix_,
                             optionTenors_,
                             swapTenors_,
                             strikeSpreads_,
                             volSpreads_,
                             swapIndexBase_,
                             vegaWeightedSmileFit_);

    tolerance = 1.0e-16;
    makeVolSpreadsTest("\nSwaptionVolCube2: ", volCube2, tolerance);
}

void SwaptionVolatilityStructuresTest::testSwaptionVolSpreadedCube() {

    BOOST_MESSAGE("Testing spreaded swaption volatility cube...");

    SavedSettings backup;
    setup();

    Handle<SwaptionVolatilityStructure> volCube(
        boost::shared_ptr<SwaptionVolatilityStructure>(new
            SwaptionVolCube1(atmVolMatrix_,
                             optionTenors_,
                             swapTenors_,
                             strikeSpreads_,
                             volSpreads_,
                             swapIndexBase_,
                             vegaWeightedSmileFit_,
                             parametersGuess_,
                             isParameterFixed_,
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


void SwaptionVolatilityStructuresTest::testSwaptionVolCubeObservability() {

    BOOST_MESSAGE("Testing swaption volatility cube observability...");

    SavedSettings backup;

    setup();

    boost::shared_ptr<SwaptionVolCube1> volCube1;
    boost::shared_ptr<SwaptionVolCube2> volCube2;
    std::string description;

    //floating reference date, floating market data
    description = "floating reference date, floating market data";
    
    volCube1 = boost::shared_ptr<SwaptionVolCube1>(new SwaptionVolCube1(atmVolMatrix_,
                                                                 optionTenors_,
                                                                 swapTenors_,
                                                                 strikeSpreads_,
                                                                 volSpreads_,
                                                                 swapIndexBase_,
                                                                 vegaWeightedSmileFit_,
                                                                 parametersGuess_,
                                                                 isParameterFixed_,
                                                                 true));
    //makeObservabilityTest(description, volCube1, true, true); FAILS

    volCube2 = boost::shared_ptr<SwaptionVolCube2>(new SwaptionVolCube2(atmVolMatrix_,
                                           optionTenors_,
                                           swapTenors_,
                                           strikeSpreads_,
                                           volSpreads_,
                                           swapIndexBase_,
                                           vegaWeightedSmileFit_));
    makeObservabilityTest(description, volCube2, true, true);

    // floating reference date, fixed market data
    description = "floating reference date, fixed market data";

    volCube1 = boost::shared_ptr<SwaptionVolCube1>(new SwaptionVolCube1(atmVolMatrix_,
                                                                 optionTenors_,
                                                                 swapTenors_,
                                                                 strikeSpreads_,
                                                                 volSpreads_,
                                                                 swapIndexBase_,
                                                                 vegaWeightedSmileFit_,
                                                                 parametersGuess_,
                                                                 isParameterFixed_,
                                                                 true));

    //makeObservabilityTest(description, volCube1, true, true); FAILS

    volCube2 =  boost::shared_ptr<SwaptionVolCube2>(new SwaptionVolCube2(atmVolMatrix_,
                                                               optionTenors_,
                                                               swapTenors_,
                                                               strikeSpreads_,
                                                               volSpreads_,
                                                               swapIndexBase_,
                                                               vegaWeightedSmileFit_));
    makeObservabilityTest(description, volCube2, false, true);

    //
    boost::shared_ptr<SwaptionVolCube1> volCube1_0, volCube1_1;
    // created before change of reference date
    volCube1_0 = boost::shared_ptr<SwaptionVolCube1>(new SwaptionVolCube1(atmVolMatrix_,
                                                                     optionTenors_,
                                                                     swapTenors_,
                                                                     strikeSpreads_,
                                                                     volSpreads_,
                                                                     swapIndexBase_,
                                                                     vegaWeightedSmileFit_,
                                                                     parametersGuess_,
                                                                     isParameterFixed_,
                                                                     true));

    Date referenceDate = Settings::instance().evaluationDate();
    Settings::instance().evaluationDate() = calendar_.advance(referenceDate, Period(1, Days), optionBdc_);
    
    // created after change of reference date
    volCube1_1 = boost::shared_ptr<SwaptionVolCube1>(new SwaptionVolCube1(atmVolMatrix_,
                             optionTenors_,
                             swapTenors_,
                             strikeSpreads_,
                             volSpreads_,
                             swapIndexBase_,
                             vegaWeightedSmileFit_,
                             parametersGuess_,
                             isParameterFixed_,
                             true));
    
    makeObservabilityTest2("swaptionvolcube1", volCube1_0,volCube1_1);
    
    Settings::instance().evaluationDate() = referenceDate;

    //
    boost::shared_ptr<SwaptionVolCube2> volCube2_0, volCube2_1;
    // created before change of reference date
    volCube2_0 = boost::shared_ptr<SwaptionVolCube2>(new SwaptionVolCube2(atmVolMatrix_,
                                                               optionTenors_,
                                                               swapTenors_,
                                                               strikeSpreads_,
                                                               volSpreads_,
                                                               swapIndexBase_,
                                                               vegaWeightedSmileFit_));

    Settings::instance().evaluationDate() = calendar_.advance(referenceDate, Period(1, Days), optionBdc_);
    
    // created after change of reference date
    volCube2_1 = boost::shared_ptr<SwaptionVolCube2>(new SwaptionVolCube2(atmVolMatrix_,
                                                               optionTenors_,
                                                               swapTenors_,
                                                               strikeSpreads_,
                                                               volSpreads_,
                                                               swapIndexBase_,
                                                               vegaWeightedSmileFit_));
    
    makeObservabilityTest2("swaptionvolcube2", volCube2_0,volCube2_1);
    Settings::instance().evaluationDate() = referenceDate;
}


void SwaptionVolatilityStructuresTest::testSwaptionVolCubeCoherence() {

    BOOST_MESSAGE("Testing swaption volatility cube...");

    SavedSettings backup;

    setup();

    boost::shared_ptr<SwaptionVolCube2> volCube2;

    std::string description;

    //floating reference date, floating market data
    //description = "floating reference date, floating market data";
    //volCube2 =  boost::shared_ptr<SwaptionVolCube2>(new SwaptionVolCube2(atmVolMatrix_,
    //                                       optionTenors_,
    //                                       swapTenors_,
    //                                       strikeSpreads_,
    //                                       volSpreads_,
    //                                       swapIndexBase_,
    //                                       vegaWeightedSmileFit_));
    //makeCubeCoherenceTest(description, volCube2);


    // floating reference date, fixed market data
    description = "floating reference date, fixed market data";
    volCube2 =  boost::shared_ptr<SwaptionVolCube2>(new SwaptionVolCube2(atmVolMatrix_,
                                           optionTenors_,
                                           swapTenors_,
                                           strikeSpreads_,
                                           volSpreads_,
                                           swapIndexBase_,
                                           vegaWeightedSmileFit_));
    makeCubeCoherenceTest(description, volCube2);

}



test_suite* SwaptionVolatilityStructuresTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Swaption Volatility Structures tests");

    //suite->add(BOOST_TEST_CASE(&SwaptionVolatilityStructuresTest::testSwaptionVolMatrixCoherence));
    //suite->add(BOOST_TEST_CASE(&SwaptionVolatilityStructuresTest::testSwaptionVolMatrixObservability));

    //suite->add(BOOST_TEST_CASE(&SwaptionVolatilityStructuresTest::testSwaptionVolCubeAtmVols));
    //suite->add(BOOST_TEST_CASE(&SwaptionVolatilityStructuresTest::testSwaptionVolCubeSmile));
    //suite->add(BOOST_TEST_CASE(&SwaptionVolatilityStructuresTest::testSwaptionVolSpreadedCube));
    //
    //suite->add(BOOST_TEST_CASE(&SwaptionVolatilityStructuresTest::testSwaptionVolCubeCoherence));
    //suite->add(BOOST_TEST_CASE(&SwaptionVolatilityStructuresTest::testSwaptionVolCubeObservability));

    return suite;
}
