/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Cristina Duminuco
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
#include "swaptionvolstructuresutilities.hpp"
#include "utilities.hpp"
#include <ql/utilities/dataformatters.hpp>
#include <string>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(SwaptionVolatilityMatrixTest)

// global data
Date referenceDate_;
SwaptionMarketConventions conventions_;
AtmVolatility atm_;
RelinkableHandle<YieldTermStructure> termStructure_;
RelinkableHandle<SwaptionVolatilityStructure> atmVolMatrix_;
Real tolerance_;

// utilities
void setup() {
    conventions_.setConventions();
    atm_.setMarketData();
    atmVolMatrix_ = RelinkableHandle<SwaptionVolatilityStructure>(
        boost::shared_ptr<SwaptionVolatilityStructure>(new
            SwaptionVolatilityMatrix(conventions_.calendar,
                                     atm_.tenors.options,
                                     atm_.tenors.swaps,
                                     atm_.volsHandle,
                                     conventions_.dayCounter,
                                     conventions_.optionBdc)));
}

void makeObservabilityTest(
                const std::string& description,
                const boost::shared_ptr<SwaptionVolatilityStructure>& vol,
                bool mktDataFloating,
                bool referenceDateFloating) {
    Rate dummyStrike = .02;
    Date referenceDate = Settings::instance().evaluationDate();
    Volatility initialVol = vol->volatility(
                    referenceDate + atm_.tenors.options[0],
                    atm_.tenors.swaps[0], dummyStrike, false);
    // testing evaluation date change ...
    Settings::instance().evaluationDate() = referenceDate - Period(1, Years);
    Volatility newVol =  vol->volatility(
                        referenceDate + atm_.tenors.options[0],
                        atm_.tenors.swaps[0], dummyStrike, false);
    Settings::instance().evaluationDate() = referenceDate;
    if (referenceDateFloating && (initialVol == newVol))
        BOOST_ERROR(description <<
        " the volatility should change when the reference date is changed !");
    if (!referenceDateFloating && (initialVol != newVol))
        BOOST_ERROR(description <<
        " the volatility should not change when the reference date is changed !");

    // test market data change...
    if (mktDataFloating){
        Volatility initialVolatility = atm_.volsHandle[0][0]->value();
        boost::dynamic_pointer_cast<SimpleQuote>(
                              atm_.volsHandle[0][0].currentLink())->setValue(10);
        newVol = vol->volatility(
                referenceDate + atm_.tenors.options[0],
                atm_.tenors.swaps[0], dummyStrike, false);
        boost::dynamic_pointer_cast<SimpleQuote>(
               atm_.volsHandle[0][0].currentLink())->setValue(initialVolatility);
        if (initialVol == newVol)
            BOOST_ERROR(description << " the volatility should change when"
                        " the market data is changed !");

    }
}

void makeCoherenceTest(
                const std::string& description,
                const boost::shared_ptr<SwaptionVolatilityDiscrete>& vol) {

    Date refDate = vol->referenceDate();
    for (Size i=0; i<atm_.tenors.options.size(); ++i) {
        Date expOptDate = conventions_.calendar.advance(refDate, atm_.tenors.options[i], conventions_.optionBdc);
        Date actOptDate = vol->optionDates()[i];
        if (actOptDate!=expOptDate)
            BOOST_FAIL("\nrecovery of option dates failed for " <<
                       description << ":"
                       "\n            calendar = " << conventions_.calendar <<
                       "\n       referenceDate = " << refDate <<
                       "\n        expiry tenor = " << atm_.tenors.options[i] <<
                       "\nexpected option date = " << expOptDate <<
                       "\n  actual option date = " << actOptDate);
    }

    Date lengthRef = vol->referenceDate();
    DayCounter volDC = vol->dayCounter();
    for (Size j=0; j<atm_.tenors.swaps.size(); ++j) {
        Period actSwapTenor = vol->swapTenors()[j];
        Date endDate = lengthRef + atm_.tenors.swaps[j];
        Time expSwapLength = volDC.yearFraction(lengthRef, endDate);
        Time actSwapLength = vol->swapLengths()[j];
        if ((atm_.tenors.swaps[j]!=actSwapTenor) ||
            (expSwapLength!=actSwapLength))
            BOOST_FAIL("\nrecovery of " << io::ordinal(j) <<
                       " swap tenor failed for " <<
                       description << ":"
                       "\nexpected swap tenor  = " << atm_.tenors.swaps[j] <<
                       "\n  actual swap tenor  = " << actSwapTenor <<
                       "\nexpected swap length = " << expSwapLength <<
                       "\n  actual swap length = " << actSwapLength);
    }

    Real tolerance = 1.0e-16;
    for (Size j=0; j<atm_.tenors.swaps.size(); j++) {
      Period thisSwapTenor = atm_.tenors.swaps[j];
      Time swapLength = vol->convertSwapTenor(thisSwapTenor);
      if (swapLength!=vol->swapLengths()[j])
          BOOST_FAIL("\nconvertSwapTenor failure for " <<
                     description << ":"
                     "\n        swap tenor : " << thisSwapTenor <<
                     "\n actual swap length: " << swapLength <<
                     "\n   exp. swap length: " << vol->swapLengths()[j]);
      for (Size i=0; i<atm_.tenors.options.size(); ++i) {
          Period thisOptionTenor =  atm_.tenors.options[i];
          Date optionDate = vol->optionDateFromTenor(thisOptionTenor);
          if (optionDate!=vol->optionDates()[i])
              BOOST_FAIL("\noptionDateFromTenor failure for " <<
                         description << ":"
                         "\n       option tenor: " << thisOptionTenor <<
                         "\nactual option date : " << optionDate <<
                         "\n  exp. option date : " << vol->optionTimes()[i]);

          Time optionTime = vol->timeFromReference(optionDate);
          if (optionTime!=vol->optionTimes()[i])
              BOOST_FAIL("\ntimeFromReference failure for " <<
                         description << ":"
                         "\n       option tenor: " << thisOptionTenor <<
                         "\n       option date : " << optionDate <<
                         "\nactual option time : " << optionTime <<
                         "\n  exp. option time : " << vol->optionTimes()[i]);

          Volatility error, actVol, expVol = atm_.vols[i][j];

          actVol = vol->volatility(optionTime, swapLength, 0.05, true);
          error = std::abs(expVol-actVol);
          if (error>tolerance)
              BOOST_FAIL("\nrecovery of atm vols failed for " <<
                         description << ":"
                         "\noption tenor: " << thisOptionTenor <<
                         "\noption time : " << optionTime <<
                         "\n  swap tenor: " << thisSwapTenor <<
                         "\n swap length: " << swapLength <<
                         "\n   exp. vol: " << io::volatility(expVol) <<
                         "\n actual vol: " << io::volatility(actVol) <<
                         "\n      error: " << io::volatility(error) <<
                         "\n  tolerance: " << tolerance);

          actVol = vol->volatility(optionDate, thisSwapTenor, 0.05, true);
          error = std::abs(expVol-actVol);
          if (error>tolerance)
              BOOST_FAIL("\nrecovery of atm vols failed for " <<
                         description << ":"
                         "\noption tenor: " << thisOptionTenor <<
                         "\noption date : " << optionDate <<
                         "\n  swap tenor: " << thisSwapTenor <<
                         "\n   exp. vol: " << io::volatility(expVol) <<
                         "\n actual vol: " << io::volatility(actVol) <<
                         "\n      error: " << io::volatility(error) <<
                         "\n  tolerance: " << tolerance);

          actVol = vol->volatility(thisOptionTenor, thisSwapTenor, 0.05, true);
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
        SwaptionVolatilityMatrix(conventions_.calendar,
                                 atm_.tenors.options,
                                 atm_.tenors.swaps,
                                 atm_.volsHandle,
                                 conventions_.dayCounter,
                                 conventions_.optionBdc));
    makeObservabilityTest(description, vol, true, true);

    //fixed reference date, floating market data
    description = "fixed reference date, floating market data";
    vol = boost::shared_ptr<SwaptionVolatilityMatrix>(new
        SwaptionVolatilityMatrix(Settings::instance().evaluationDate(),
                                 conventions_.calendar,
                                 atm_.tenors.options,
                                 atm_.tenors.swaps,
                                 atm_.volsHandle,
                                 conventions_.dayCounter,
                                 conventions_.optionBdc));
    makeObservabilityTest(description, vol, true, false);

    // floating reference date, fixed market data
    description = "floating reference date, fixed market data";
    vol = boost::shared_ptr<SwaptionVolatilityMatrix>(new
        SwaptionVolatilityMatrix(conventions_.calendar,
                                 atm_.tenors.options,
                                 atm_.tenors.swaps,
                                 atm_.volsHandle,
                                 conventions_.dayCounter,
                                 conventions_.optionBdc));
    makeObservabilityTest(description, vol, false, true);

    // fixed reference date, fixed market data
    description = "fixed reference date, fixed market data";
    vol = boost::shared_ptr<SwaptionVolatilityMatrix>(new
        SwaptionVolatilityMatrix(Settings::instance().evaluationDate(),
                                 conventions_.calendar,
                                 atm_.tenors.options,
                                 atm_.tenors.swaps,
                                 atm_.volsHandle,
                                 conventions_.dayCounter,
                                 conventions_.optionBdc));
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
        SwaptionVolatilityMatrix(conventions_.calendar,
                                 atm_.tenors.options,
                                 atm_.tenors.swaps,
                                 atm_.volsHandle,
                                 conventions_.dayCounter,
                                 conventions_.optionBdc));
    makeCoherenceTest(description, vol);

    //fixed reference date, floating market data
    description = "fixed reference date, floating market data";
    vol = boost::shared_ptr<SwaptionVolatilityMatrix>(new
        SwaptionVolatilityMatrix(Settings::instance().evaluationDate(),
                                 conventions_.calendar,
                                 atm_.tenors.options,
                                 atm_.tenors.swaps,
                                 atm_.volsHandle,
                                 conventions_.dayCounter,
                                 conventions_.optionBdc));
    makeCoherenceTest(description, vol);

    // floating reference date, fixed market data
    description = "floating reference date, fixed market data";
    vol = boost::shared_ptr<SwaptionVolatilityMatrix>(new
        SwaptionVolatilityMatrix(conventions_.calendar,
                                 atm_.tenors.options,
                                 atm_.tenors.swaps,
                                 atm_.volsHandle,
                                 conventions_.dayCounter,
                                 conventions_.optionBdc));
    makeCoherenceTest(description, vol);

    // fixed reference date, fixed market data
    description = "fixed reference date, fixed market data";
    vol = boost::shared_ptr<SwaptionVolatilityMatrix>(new
        SwaptionVolatilityMatrix(Settings::instance().evaluationDate(),
                                 conventions_.calendar,
                                 atm_.tenors.options,
                                 atm_.tenors.swaps,
                                 atm_.volsHandle,
                                 conventions_.dayCounter,
                                 conventions_.optionBdc));
    makeCoherenceTest(description, vol);
}

test_suite* SwaptionVolatilityMatrixTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Swaption Volatility Matrix tests");

    suite->add(BOOST_TEST_CASE(&SwaptionVolatilityMatrixTest::testSwaptionVolMatrixCoherence));

    suite->add(BOOST_TEST_CASE(&SwaptionVolatilityMatrixTest::testSwaptionVolMatrixObservability));

    return suite;
}
