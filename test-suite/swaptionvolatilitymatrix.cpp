/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2006, 2008 Ferdinando Ametrano
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
#include <ql/indexes/swap/euriborswapfixa.hpp>
#include <ql/instruments/makeswaption.hpp>
#include <ql/pricingengines/swaption/blackswaptionengine.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <string>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    // TODO: use CommonVars
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
        termStructure_.linkTo(
            boost::shared_ptr<YieldTermStructure>(new
                FlatForward(0, conventions_.calendar, 0.05, Actual365Fixed())));
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
        Settings::instance().evaluationDate() =
            referenceDate - Period(1, Years);
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

        for (Size i=0; i<atm_.tenors.options.size(); ++i) {
            Date optionDate = vol->optionDateFromTenor(atm_.tenors.options[i]);
            if (optionDate!=vol->optionDates()[i])
                BOOST_FAIL("\noptionDateFromTenor failure for " <<
                           description << ":"
                           "\n       option tenor: " << atm_.tenors.options[i] <<
                           "\nactual option date : " << optionDate <<
                           "\n  exp. option date : " << vol->optionDates()[i]);
            Time optionTime = vol->timeFromReference(optionDate);
            if (optionTime!=vol->optionTimes()[i])
                BOOST_FAIL("\ntimeFromReference failure for " <<
                           description << ":"
                           "\n       option tenor: " << atm_.tenors.options[i] <<
                           "\n       option date : " << optionDate <<
                           "\nactual option time : " << optionTime <<
                           "\n  exp. option time : " << vol->optionTimes()[i]);
        }

        boost::shared_ptr<BlackSwaptionEngine> engine(new
            BlackSwaptionEngine(termStructure_,
                                Handle<SwaptionVolatilityStructure>(vol)));

        for (Size j=0; j<atm_.tenors.swaps.size(); j++) {
          Time swapLength = vol->swapLength(atm_.tenors.swaps[j]);
          if (swapLength!=years(atm_.tenors.swaps[j]))
              BOOST_FAIL("\nconvertSwapTenor failure for " <<
                         description << ":"
                         "\n        swap tenor : " << atm_.tenors.swaps[j] <<
                         "\n actual swap length: " << swapLength <<
                         "\n   exp. swap length: " << years(atm_.tenors.swaps[j]));

          boost::shared_ptr<SwapIndex> swapIndex(new
              EuriborSwapFixA(atm_.tenors.swaps[j], termStructure_));

          for (Size i=0; i<atm_.tenors.options.size(); ++i) {
              Real error, tolerance = 1.0e-16;
              Volatility actVol, expVol = atm_.vols[i][j];

              actVol = vol->volatility(atm_.tenors.options[i], atm_.tenors.swaps[j], 0.05, true);
              error = std::abs(expVol-actVol);
              if (error>tolerance)
                  BOOST_FAIL("\nrecovery of atm vols failed for " <<
                             description << ":"
                             "\noption tenor = " << atm_.tenors.options[i] <<
                             "\n swap length = " << atm_.tenors.swaps[j] <<
                             "\nexpected vol = " << io::volatility(expVol) <<
                             "\n  actual vol = " << io::volatility(actVol) <<
                             "\n       error = " << io::volatility(error) <<
                             "\n   tolerance = " << tolerance);

              Date optionDate = vol->optionDateFromTenor(atm_.tenors.options[i]);
              actVol = vol->volatility(optionDate, atm_.tenors.swaps[j], 0.05, true);
              error = std::abs(expVol-actVol);
              if (error>tolerance)
                  BOOST_FAIL("\nrecovery of atm vols failed for " <<
                             description << ":"
                             "\noption tenor: " << atm_.tenors.options[i] <<
                             "\noption date : " << optionDate <<
                             "\n  swap tenor: " << atm_.tenors.swaps[j] <<
                             "\n   exp. vol: " << io::volatility(expVol) <<
                             "\n actual vol: " << io::volatility(actVol) <<
                             "\n      error: " << io::volatility(error) <<
                             "\n  tolerance: " << tolerance);

              Time optionTime = vol->timeFromReference(optionDate);
              actVol = vol->volatility(optionTime, swapLength, 0.05, true);
              error = std::abs(expVol-actVol);
              if (error>tolerance)
                  BOOST_FAIL("\nrecovery of atm vols failed for " <<
                             description << ":"
                             "\noption tenor: " << atm_.tenors.options[i] <<
                             "\noption time : " << optionTime <<
                             "\n  swap tenor: " << atm_.tenors.swaps[j] <<
                             "\n swap length: " << swapLength <<
                             "\n   exp. vol: " << io::volatility(expVol) <<
                             "\n actual vol: " << io::volatility(actVol) <<
                             "\n      error: " << io::volatility(error) <<
                             "\n  tolerance: " << tolerance);

              // ATM swaption
              Swaption swaption =
                  MakeSwaption(swapIndex, atm_.tenors.options[i])
                  .withPricingEngine(engine);

              Date exerciseDate = swaption.exercise()->dates().front();
              if (exerciseDate!=vol->optionDates()[i])
                  BOOST_FAIL("\noptionDateFromTenor mismatch for " <<
                             description << ":"
                             "\n       option tenor: " << atm_.tenors.options[i] <<
                             "\nactual option date : " << exerciseDate <<
                             "\n  exp. option date : " << vol->optionDates()[i]);

              Date start = swaption.underlyingSwap()->startDate();
              Date end = swaption.underlyingSwap()->maturityDate();
              Time swapLength2 = vol->swapLength(start, end);
              if (swapLength2!=swapLength)
                  BOOST_FAIL("\nswapLength failure for " <<
                             description << ":"
                             "\n        swap tenor : " << atm_.tenors.swaps[j] <<
                             "\n actual swap length: " << swapLength2 <<
                             "\n   exp. swap length: " << swapLength);

              Real npv = swaption.NPV();
              actVol = swaption.impliedVolatility(npv, termStructure_, expVol*0.98, 1e-6);
              error = std::abs(expVol-actVol);
              Real tolerance2 = 0.000001;
              if (error>tolerance2)
                  BOOST_FAIL("\nrecovery of atm vols through BlackSwaptionEngine failed for " <<
                             description << ":"
                             "\noption tenor: " << atm_.tenors.options[i] <<
                             "\noption time : " << optionTime <<
                             "\n  swap tenor: " << atm_.tenors.swaps[j] <<
                             "\n swap length: " << swapLength <<
                             "\n   exp. vol: " << io::volatility(expVol) <<
                             "\n actual vol: " << io::volatility(actVol) <<
                             "\n      error: " << io::volatility(error) <<
                             "\n  tolerance: " << tolerance2);
          }
        }
    }

}


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
