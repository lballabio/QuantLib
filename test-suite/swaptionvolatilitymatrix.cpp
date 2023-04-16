/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2008 Ferdinando Ametrano
 Copyright (C) 2006 François du Vignaud
 Copyright (C) 2007 Cristina Duminuco

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
#include <ql/indexes/swap/euriborswap.hpp>
#include <ql/instruments/makeswaption.hpp>
#include <ql/pricingengines/swaption/blackswaptionengine.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/math/comparison.hpp>
#include <string>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace swaption_volatility_matrix_test {

    struct CommonVars {
        // global data
        Date referenceDate;
        SwaptionMarketConventions conventions;
        AtmVolatility atm;
        RelinkableHandle<YieldTermStructure> termStructure;
        RelinkableHandle<SwaptionVolatilityStructure> atmVolMatrix;
        Real tolerance;

        // cleanup
        SavedSettings backup;

        // setup
        CommonVars() {
            conventions.setConventions();
            atm.setMarketData();
            Settings::instance().evaluationDate() =
                conventions.calendar.adjust(Date::todaysDate());
            atmVolMatrix = RelinkableHandle<SwaptionVolatilityStructure>(
                std::shared_ptr<SwaptionVolatilityStructure>(new
                    SwaptionVolatilityMatrix(conventions.calendar,
                                             conventions.optionBdc,
                                             atm.tenors.options,
                                             atm.tenors.swaps,
                                             atm.volsHandle,
                                             conventions.dayCounter)));
            termStructure.linkTo(
                std::shared_ptr<YieldTermStructure>(new
                    FlatForward(0, conventions.calendar,
                                0.05, Actual365Fixed())));
        }

        // utilities
        void makeObservabilityTest(
                const std::string& description,
                const std::shared_ptr<SwaptionVolatilityStructure>& vol,
                bool mktDataFloating,
                bool referenceDateFloating) {
            Rate dummyStrike = .02;
            Date referenceDate = Settings::instance().evaluationDate();
            Volatility initialVol = vol->volatility(
                        referenceDate + atm.tenors.options[0],
                        atm.tenors.swaps[0], dummyStrike, false);
            // testing evaluation date change ...
            Settings::instance().evaluationDate() =
                referenceDate - Period(1, Years);
            Volatility newVol =  vol->volatility(
                        referenceDate + atm.tenors.options[0],
                        atm.tenors.swaps[0], dummyStrike, false);
            Settings::instance().evaluationDate() = referenceDate;
            if (referenceDateFloating && (initialVol == newVol))
                BOOST_ERROR(description <<
                            " the volatility should change when the reference date is changed !");
            if (!referenceDateFloating && (initialVol != newVol))
                BOOST_ERROR(description <<
                            " the volatility should not change when the reference date is changed !");

            // test market data change...
            if (mktDataFloating){
                Volatility initialVolatility = atm.volsHandle[0][0]->value();
                std::dynamic_pointer_cast<SimpleQuote>(
                              atm.volsHandle[0][0].currentLink())->setValue(10);
                newVol = vol->volatility(
                    referenceDate + atm.tenors.options[0],
                    atm.tenors.swaps[0], dummyStrike, false);
                std::dynamic_pointer_cast<SimpleQuote>(
                    atm.volsHandle[0][0].currentLink())
                    ->setValue(initialVolatility);
                if (initialVol == newVol)
                    BOOST_ERROR(description << " the volatility should change when"
                                " the market data is changed !");
            }
        }

        void makeCoherenceTest(
                const std::string& description,
                const std::shared_ptr<SwaptionVolatilityDiscrete>& vol) {

            for (Size i=0; i<atm.tenors.options.size(); ++i) {
                Date optionDate =
                    vol->optionDateFromTenor(atm.tenors.options[i]);
                if (optionDate!=vol->optionDates()[i])
                    BOOST_FAIL(
                         "optionDateFromTenor failure for " <<
                         description << ":"
                         "\n       option tenor: " << atm.tenors.options[i] <<
                         "\nactual option date : " << optionDate <<
                         "\n  exp. option date : " << vol->optionDates()[i]);
                Time optionTime = vol->timeFromReference(optionDate);
                if (!close(optionTime,vol->optionTimes()[i]))
                    BOOST_FAIL(
                         "timeFromReference failure for " <<
                         description << ":"
                         "\n       option tenor: " << atm.tenors.options[i] <<
                         "\n       option date : " << optionDate <<
                         "\nactual option time : " << optionTime <<
                         "\n  exp. option time : " << vol->optionTimes()[i]);
            }

            std::shared_ptr<BlackSwaptionEngine> engine(new
                BlackSwaptionEngine(termStructure,
                                    Handle<SwaptionVolatilityStructure>(vol)));

            for (Size j=0; j<atm.tenors.swaps.size(); j++) {
                Time swapLength = vol->swapLength(atm.tenors.swaps[j]);
                if (!close(swapLength,years(atm.tenors.swaps[j])))
                    BOOST_FAIL("convertSwapTenor failure for " <<
                               description << ":"
                               "\n        swap tenor : " << atm.tenors.swaps[j] <<
                               "\n actual swap length: " << swapLength <<
                               "\n   exp. swap length: " << years(atm.tenors.swaps[j]));

                std::shared_ptr<SwapIndex> swapIndex(new
                    EuriborSwapIsdaFixA(atm.tenors.swaps[j], termStructure));

                for (Size i=0; i<atm.tenors.options.size(); ++i) {
                    Real error, tolerance = 1.0e-16;
                    Volatility actVol, expVol = atm.vols[i][j];

                    actVol = vol->volatility(atm.tenors.options[i],
                                             atm.tenors.swaps[j], 0.05, true);
                    error = std::abs(expVol-actVol);
                    if (error>tolerance)
                        BOOST_FAIL(
                              "recovery of atm vols failed for " <<
                              description << ":"
                              "\noption tenor = " << atm.tenors.options[i] <<
                              "\n swap length = " << atm.tenors.swaps[j] <<
                              "\nexpected vol = " << io::volatility(expVol) <<
                              "\n  actual vol = " << io::volatility(actVol) <<
                              "\n       error = " << io::volatility(error) <<
                              "\n   tolerance = " << tolerance);

                    Date optionDate =
                        vol->optionDateFromTenor(atm.tenors.options[i]);
                    actVol = vol->volatility(optionDate,
                                             atm.tenors.swaps[j], 0.05, true);
                    error = std::abs(expVol-actVol);
                    if (error>tolerance)
                        BOOST_FAIL(
                             "recovery of atm vols failed for " <<
                             description << ":"
                             "\noption tenor: " << atm.tenors.options[i] <<
                             "\noption date : " << optionDate <<
                             "\n  swap tenor: " << atm.tenors.swaps[j] <<
                             "\n   exp. vol: " << io::volatility(expVol) <<
                             "\n actual vol: " << io::volatility(actVol) <<
                             "\n      error: " << io::volatility(error) <<
                             "\n  tolerance: " << tolerance);

                    Time optionTime = vol->timeFromReference(optionDate);
                    actVol = vol->volatility(optionTime, swapLength,
                                             0.05, true);
                    error = std::abs(expVol-actVol);
                    if (error>tolerance)
                        BOOST_FAIL(
                             "recovery of atm vols failed for " <<
                             description << ":"
                             "\noption tenor: " << atm.tenors.options[i] <<
                             "\noption time : " << optionTime <<
                             "\n  swap tenor: " << atm.tenors.swaps[j] <<
                             "\n swap length: " << swapLength <<
                             "\n    exp. vol: " << io::volatility(expVol) <<
                             "\n  actual vol: " << io::volatility(actVol) <<
                             "\n       error: " << io::volatility(error) <<
                             "\n   tolerance: " << tolerance);

                    // ATM swaption
                    Swaption swaption =
                        MakeSwaption(swapIndex, atm.tenors.options[i])
                        .withPricingEngine(engine);

                    Date exerciseDate = swaption.exercise()->dates().front();
                    if (exerciseDate!=vol->optionDates()[i])
                        BOOST_FAIL(
                             "\noptionDateFromTenor mismatch for " <<
                             description << ":"
                             "\n      option tenor: " << atm.tenors.options[i] <<
                             "\nactual option date: " << exerciseDate <<
                             "\n  exp. option date: " << vol->optionDates()[i]);

                    Date start = swaption.underlyingSwap()->startDate();
                    Date end = swaption.underlyingSwap()->maturityDate();
                    Time swapLength2 = vol->swapLength(start, end);
                    if (!close(swapLength2,swapLength))
                        BOOST_FAIL("\nswapLength failure for " <<
                                   description << ":"
                                   "\n   exp. swap length: " << swapLength <<
                                   "\n actual swap length: " << swapLength2 <<
                                   "\n        swap tenor : " << atm.tenors.swaps[j] <<
                                   "\n  swap index tenor : " << swapIndex->tenor() <<
                                   "\n        option date: " << exerciseDate <<
                                   "\n         start date: " << start <<
                                   "\n      maturity date: " << end
                                   );

                    Real npv = swaption.NPV();
                    actVol = swaption.impliedVolatility(npv, termStructure,
                                                        expVol*0.98, 1e-6,
                                                        100, 10.0e-7, 4.0,
                                                        ShiftedLognormal, 0.0);
                    error = std::abs(expVol-actVol);
                    Real tolerance2 = 0.000001;
                    if (error>tolerance2)
                        BOOST_FAIL(
                             "recovery of atm vols through BlackSwaptionEngine failed for " <<
                             description << ":"
                             "\noption tenor: " << atm.tenors.options[i] <<
                             "\noption time : " << optionTime <<
                             "\n  swap tenor: " << atm.tenors.swaps[j] <<
                             "\n swap length: " << swapLength <<
                             "\n   exp. vol: " << io::volatility(expVol) <<
                             "\n actual vol: " << io::volatility(actVol) <<
                             "\n      error: " << io::volatility(error) <<
                             "\n  tolerance: " << tolerance2);
                }
            }
        }
    };

}


void SwaptionVolatilityMatrixTest::testSwaptionVolMatrixObservability() {

    BOOST_TEST_MESSAGE("Testing swaption volatility matrix observability...");

    using namespace swaption_volatility_matrix_test;

    CommonVars vars;

    std::shared_ptr<SwaptionVolatilityMatrix> vol;
    std::string description;

    //floating reference date, floating market data
    description = "floating reference date, floating market data";
    vol = std::make_shared<SwaptionVolatilityMatrix>(vars.conventions.calendar,
                                 vars.conventions.optionBdc,
                                 vars.atm.tenors.options,
                                 vars.atm.tenors.swaps,
                                 vars.atm.volsHandle,
                                 vars.conventions.dayCounter);
    vars.makeObservabilityTest(description, vol, true, true);

    //fixed reference date, floating market data
    description = "fixed reference date, floating market data";
    vol = std::make_shared<SwaptionVolatilityMatrix>(Settings::instance().evaluationDate(),
                                 vars.conventions.calendar,
                                 vars.conventions.optionBdc,
                                 vars.atm.tenors.options,
                                 vars.atm.tenors.swaps,
                                 vars.atm.volsHandle,
                                 vars.conventions.dayCounter);
    vars.makeObservabilityTest(description, vol, true, false);

    // floating reference date, fixed market data
    description = "floating reference date, fixed market data";
    vol = std::make_shared<SwaptionVolatilityMatrix>(vars.conventions.calendar,
                                 vars.conventions.optionBdc,
                                 vars.atm.tenors.options,
                                 vars.atm.tenors.swaps,
                                 vars.atm.volsHandle,
                                 vars.conventions.dayCounter);
    vars.makeObservabilityTest(description, vol, false, true);

    // fixed reference date, fixed market data
    description = "fixed reference date, fixed market data";
    vol = std::make_shared<SwaptionVolatilityMatrix>(Settings::instance().evaluationDate(),
                                 vars.conventions.calendar,
                                 vars.conventions.optionBdc,
                                 vars.atm.tenors.options,
                                 vars.atm.tenors.swaps,
                                 vars.atm.volsHandle,
                                 vars.conventions.dayCounter);
    vars.makeObservabilityTest(description, vol, false, false);

   // fixed reference date and fixed market data, option dates
        //SwaptionVolatilityMatrix(const Date& referenceDate,
        //                         const std::vector<Date>& exerciseDates,
        //                         const std::vector<Period>& swapTenors,
        //                         const Matrix& volatilities,
        //                         const DayCounter& dayCounter);
}


void SwaptionVolatilityMatrixTest::testSwaptionVolMatrixCoherence() {

    BOOST_TEST_MESSAGE("Testing swaption volatility matrix...");

    using namespace swaption_volatility_matrix_test;

    CommonVars vars;

    std::shared_ptr<SwaptionVolatilityMatrix> vol;
    std::string description;

    //floating reference date, floating market data
    description = "floating reference date, floating market data";
    vol = std::make_shared<SwaptionVolatilityMatrix>(vars.conventions.calendar,
                                 vars.conventions.optionBdc,
                                 vars.atm.tenors.options,
                                 vars.atm.tenors.swaps,
                                 vars.atm.volsHandle,
                                 vars.conventions.dayCounter);
    vars.makeCoherenceTest(description, vol);

    //fixed reference date, floating market data
    description = "fixed reference date, floating market data";
    vol = std::make_shared<SwaptionVolatilityMatrix>(Settings::instance().evaluationDate(),
                                 vars.conventions.calendar,
                                 vars.conventions.optionBdc,
                                 vars.atm.tenors.options,
                                 vars.atm.tenors.swaps,
                                 vars.atm.volsHandle,
                                 vars.conventions.dayCounter);
    vars.makeCoherenceTest(description, vol);

    // floating reference date, fixed market data
    description = "floating reference date, fixed market data";
    vol = std::make_shared<SwaptionVolatilityMatrix>(vars.conventions.calendar,
                                 vars.conventions.optionBdc,
                                 vars.atm.tenors.options,
                                 vars.atm.tenors.swaps,
                                 vars.atm.volsHandle,
                                 vars.conventions.dayCounter);
    vars.makeCoherenceTest(description, vol);

    // fixed reference date, fixed market data
    description = "fixed reference date, fixed market data";
    vol = std::make_shared<SwaptionVolatilityMatrix>(Settings::instance().evaluationDate(),
                                 vars.conventions.calendar,
                                 vars.conventions.optionBdc,
                                 vars.atm.tenors.options,
                                 vars.atm.tenors.swaps,
                                 vars.atm.volsHandle,
                                 vars.conventions.dayCounter);
    vars.makeCoherenceTest(description, vol);
}

test_suite* SwaptionVolatilityMatrixTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Swaption Volatility Matrix tests");

    suite->add(QUANTLIB_TEST_CASE(
              &SwaptionVolatilityMatrixTest::testSwaptionVolMatrixCoherence));

    suite->add(QUANTLIB_TEST_CASE(
          &SwaptionVolatilityMatrixTest::testSwaptionVolMatrixObservability));

    return suite;
}
