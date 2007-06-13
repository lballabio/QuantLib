/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 François du Vignaud

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

#include "swaptionvolatilitymatrix.hpp"
#include "utilities.hpp"
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/termstructures/volatilities/swaption/swaptionvolmatrix.hpp>
#include <iostream>
#include <string>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(SwaptionVolatilityMatrixTest)

// global data

Calendar calendar_;
BusinessDayConvention bdc_;
Matrix vols_;
DayCounter dayCounter_;
std::vector<std::vector<Handle<Quote> > > volsHandles_;
std::vector<Period> optionTenors_;
std::vector<Period> swapTenors_;
Real tolerance_;

// utilities
void setup() {

    calendar_ = TARGET();
    bdc_ = Following;
    dayCounter_ = Actual365Fixed();

    optionTenors_.resize(6);
    optionTenors_[0] = Period(1, Months);
    optionTenors_[1] = Period(6, Months);
    optionTenors_[2] = Period(1, Years);
    optionTenors_[3] = Period(5, Years);
    optionTenors_[4] = Period(10, Years);
    optionTenors_[5] = Period(30, Years);

    swapTenors_.resize(4);
    swapTenors_[0] = Period(1, Years);
    swapTenors_[1] = Period(5, Years);
    swapTenors_[2] = Period(10, Years);
    swapTenors_[3] = Period(30, Years);

    vols_ = Matrix(optionTenors_.size(), swapTenors_.size());
    vols_[0][0]=0.1300; vols_[0][1]=0.1560; vols_[0][2]=0.1390; vols_[0][3]=0.1220;
    vols_[1][0]=0.1440; vols_[1][1]=0.1580; vols_[1][2]=0.1460; vols_[1][3]=0.1260;
    vols_[2][0]=0.1600; vols_[2][1]=0.1590; vols_[2][2]=0.1470; vols_[2][3]=0.1290;
    vols_[3][0]=0.1640; vols_[3][1]=0.1470; vols_[3][2]=0.1370; vols_[3][3]=0.1220;
    vols_[4][0]=0.1400; vols_[4][1]=0.1300; vols_[4][2]=0.1250; vols_[4][3]=0.1100;
    vols_[5][0]=0.1130; vols_[5][1]=0.1090; vols_[5][2]=0.1070; vols_[5][3]=0.0930;

    volsHandles_.resize(vols_.rows());
    for (Size i=0; i<vols_.rows(); i++){
        volsHandles_[i].resize(vols_.columns());
        for (Size j=0; j<vols_.columns(); j++)
            volsHandles_[i][j] = Handle<Quote>(boost::shared_ptr<Quote>(new
                SimpleQuote(vols_[i][j])));
    }
}

void makeObservabilityTest(
                      const std::string& description,
                      const boost::shared_ptr<SwaptionVolatilityMatrix>& vol,
                      bool mktDataFloating,
                      bool referenceDateFloating) {
    Rate dummyStrike = .02;
    Date referenceDate = Settings::instance().evaluationDate();
    Volatility initialVol = vol->volatility(
                    referenceDate + optionTenors_[0],
                    swapTenors_[0], dummyStrike, false);
    // testing evaluation date change ...
    Settings::instance().evaluationDate() = referenceDate - Period(1, Years);
    Volatility newVol =  vol->volatility(
                        referenceDate + optionTenors_[0],
                        swapTenors_[0], dummyStrike, false);
    Settings::instance().evaluationDate() = referenceDate;
    if (referenceDateFloating && (initialVol == newVol))
        BOOST_ERROR(description <<
        " the volatility should change when the reference date is changed !");
    if (!referenceDateFloating && (initialVol != newVol))
        BOOST_ERROR(description <<
        " the volatility should not change when the reference date is changed !");

    // test market data change...
    if (mktDataFloating){
        Volatility initialVolatility = volsHandles_[0][0]->value();
        boost::dynamic_pointer_cast<SimpleQuote>(
                              volsHandles_[0][0].currentLink())->setValue(10);
        newVol = vol->volatility(
                referenceDate + optionTenors_[0],
                swapTenors_[0], .02, false);
        boost::dynamic_pointer_cast<SimpleQuote>(
               volsHandles_[0][0].currentLink())->setValue(initialVolatility);
        if (initialVol == newVol)
            BOOST_ERROR(description << " the volatility should change when"
                        " the market data is changed !");

    }
}

void makeCoherenceTest(
                      const std::string& description,
                      const boost::shared_ptr<SwaptionVolatilityMatrix>& vol) {


    Date refDate = vol->referenceDate();
    for (Size i=0; i<optionTenors_.size(); i++) {
        Date expOptDate = calendar_.advance(refDate, optionTenors_[i], bdc_);
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
    for (Size j=0; j<swapTenors_.size(); j++) {
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
    for (Size i=0; i<optionTenors_.size(); i++) {
      for (Size j=0; j<swapTenors_.size(); j++) {

          Period thisOptionTenor=  optionTenors_[i];
          Date thisOptionDate = vol->optionDates()[i];
          Period thisSwapTenor = swapTenors_[j];

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



          Volatility error, actVol, expVol = vols_[i][j];

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

QL_END_TEST_LOCALS(SwaptionVolatilityMatrixTest)

void SwaptionVolatilityMatrixTest::testSwaptionVolMatrixObservability() {

    BOOST_MESSAGE("Testing swaption volatility matrix observability...");

    SavedSettings backup;

    setup();

    boost::shared_ptr<SwaptionVolatilityMatrix> vol;
    std::string description;

    //floating reference date, floating market data
    description = "floating reference date, floating market data";
    vol = boost::shared_ptr<SwaptionVolatilityMatrix>(new
        SwaptionVolatilityMatrix(calendar_,
                                 optionTenors_,
                                 swapTenors_,
                                 volsHandles_,
                                 dayCounter_,
                                 bdc_));
    makeObservabilityTest(description, vol, true, true);

    //fixed reference date, floating market data
    description = "fixed reference date, floating market data";
    vol = boost::shared_ptr<SwaptionVolatilityMatrix>(new
        SwaptionVolatilityMatrix(Settings::instance().evaluationDate(),
                                 calendar_,
                                 optionTenors_,
                                 swapTenors_,
                                 volsHandles_,
                                 dayCounter_,
                                 bdc_));
    makeObservabilityTest(description, vol, true, false);

    // floating reference date, fixed market data
    description = "floating reference date, fixed market data";
    vol = boost::shared_ptr<SwaptionVolatilityMatrix>(new
        SwaptionVolatilityMatrix(calendar_,
                                 optionTenors_,
                                 swapTenors_,
                                 vols_,
                                 dayCounter_,
                                 bdc_));
    makeObservabilityTest(description, vol, false, true);

    // fixed reference date, fixed market data
    description = "fixed reference date, fixed market data";
    vol = boost::shared_ptr<SwaptionVolatilityMatrix>(new
        SwaptionVolatilityMatrix(Settings::instance().evaluationDate(),
                                 calendar_,
                                 optionTenors_,
                                 swapTenors_,
                                 vols_,
                                 dayCounter_,
                                  bdc_));
    makeObservabilityTest(description, vol, false, false);

   // fixed reference date and fixed market data, option dates
        //SwaptionVolatilityMatrix(const Date& referenceDate,
        //                         const std::vector<Date>& exerciseDates,
        //                         const std::vector<Period>& swapTenors,
        //                         const Matrix& volatilities,
        //                         const DayCounter& dayCounter);
}


void SwaptionVolatilityMatrixTest::testSwaptionVolMatrixCoherence() {

    BOOST_MESSAGE("Testing swaption volatility matrix...");

    SavedSettings backup;

    setup();

    boost::shared_ptr<SwaptionVolatilityMatrix> vol;
    std::string description;

    //floating reference date, floating market data
    description = "floating reference date, floating market data";
    vol = boost::shared_ptr<SwaptionVolatilityMatrix>(new
        SwaptionVolatilityMatrix(calendar_,
                                 optionTenors_, swapTenors_,
                                 volsHandles_,
                                 dayCounter_, bdc_));
    makeCoherenceTest(description, vol);

    //fixed reference date, floating market data
    description = "fixed reference date, floating market data";
    vol = boost::shared_ptr<SwaptionVolatilityMatrix>(new
        SwaptionVolatilityMatrix(Settings::instance().evaluationDate(),
                                 calendar_,
                                 optionTenors_,
                                 swapTenors_,
                                 volsHandles_,
                                 dayCounter_,
                                 bdc_));
    makeCoherenceTest(description, vol);

    // floating reference date, fixed market data
    description = "floating reference date, fixed market data";
    vol = boost::shared_ptr<SwaptionVolatilityMatrix>(new
        SwaptionVolatilityMatrix(calendar_,
                                 optionTenors_,
                                 swapTenors_,
                                 vols_,
                                 dayCounter_,
                                 bdc_));
    makeCoherenceTest(description, vol);

    // fixed reference date, fixed market data
    description = "fixed reference date, fixed market data";
    vol = boost::shared_ptr<SwaptionVolatilityMatrix>(new
        SwaptionVolatilityMatrix(Settings::instance().evaluationDate(),
                                 calendar_,
                                 optionTenors_,
                                 swapTenors_,
                                 vols_,
                                 dayCounter_,
                                  bdc_));
    makeCoherenceTest(description, vol);
}




test_suite* SwaptionVolatilityMatrixTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Swaption Volatility Matrix tests");

    suite->add(BOOST_TEST_CASE(&SwaptionVolatilityMatrixTest::testSwaptionVolMatrixCoherence));

    suite->add(BOOST_TEST_CASE(&SwaptionVolatilityMatrixTest::testSwaptionVolMatrixObservability));

    return suite;
}
