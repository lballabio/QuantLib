
/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/qldefines.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/timer.hpp>

/* Use BOOST_MSVC instead of _MSC_VER since some other vendors (Metrowerks,
   for example) also #define _MSC_VER

*/
#ifdef BOOST_MSVC
#  define BOOST_LIB_DIAGNOSTIC
#  define BOOST_LIB_NAME boost_unit_test_framework
#  include <boost/config/auto_link.hpp>
#  undef BOOST_LIB_NAME
#  undef BOOST_LIB_DIAGNOSTIC
#endif

#include "americanoption.hpp"
#include "asianoptions.hpp"
#include "barrieroption.hpp"
#include "basketoption.hpp"
#include "bonds.hpp"
#include "calendars.hpp"
#include "capfloor.hpp"
#include "cliquetoption.hpp"
#include "compoundforward.hpp"
#include "covariance.hpp"
#include "dates.hpp"
#include "daycounters.hpp"
#include "digitaloption.hpp"
#include "distributions.hpp"
#include "dividendeuropeanoption.hpp"
#include "europeanoption.hpp"
#include "exchangerate.hpp"
#include "factorial.hpp"
#include "forwardoption.hpp"
#include "instruments.hpp"
#include "integrals.hpp"
#include "interestrates.hpp"
#include "interpolations.hpp"
#include "jumpdiffusion.hpp"
#include "lowdiscrepancysequences.hpp"
#include "matrices.hpp"
#include "mersennetwister.hpp"
#include "money.hpp"
#include "operators.hpp"
#include "piecewiseflatforward.hpp"
#include "quantooption.hpp"
#include "quotes.hpp"
#include "riskstats.hpp"
#include "rngtraits.hpp"
#include "rounding.hpp"
#include "solvers.hpp"
#include "stats.hpp"
#include "swap.hpp"
#include "swaption.hpp"
#include "termstructures.hpp"
// to be deprecated
#include "old_pricers.hpp"

#include <iostream>

using namespace boost::unit_test_framework;

namespace {

    boost::timer t;

    void startTimer() { t.restart(); }
    void stopTimer() {
        std::cout << " \nTests completed in "
                  << t.elapsed() << " s\n " << std::endl;
    }

}

test_suite* init_unit_test_suite(int, char* []) {

    std::string header = "Testing "
                         #ifdef BOOST_MSVC
                         QL_LIB_NAME
                         #else
                         "QuantLib " QL_VERSION
                         #endif
                         #ifdef QL_DISABLE_DEPRECATED
                         " (deprecated code disabled)"
                         #endif
                         ;
    std::string rule = std::string(header.length(),'=');

    BOOST_MESSAGE(rule);
    BOOST_MESSAGE(header);
    BOOST_MESSAGE(rule);
    test_suite* test = BOOST_TEST_SUITE("QuantLib test suite");

    test->add(BOOST_TEST_CASE(startTimer));

    test->add(AmericanOptionTest::suite());
    test->add(AsianOptionTest::suite());
    test->add(BarrierOptionTest::suite());
    test->add(BasketOptionTest::suite());
    test->add(BondTest::suite());
    test->add(CalendarTest::suite());
    test->add(CapFloorTest::suite());
    test->add(CliquetOptionTest::suite());
    test->add(CompoundForwardTest::suite());
    test->add(CovarianceTest::suite());
    test->add(DateTest::suite());
    test->add(DayCounterTest::suite());
    test->add(DigitalOptionTest::suite());
    test->add(DistributionTest::suite());
    test->add(DividendEuropeanOptionTest::suite());
    test->add(EuropeanOptionTest::suite());
    test->add(ExchangeRateTest::suite());
    test->add(FactorialTest::suite());
    test->add(ForwardOptionTest::suite());
    test->add(InstrumentTest::suite());
    test->add(IntegralTest::suite());
    test->add(InterestRateTest::suite());
    test->add(InterpolationTest::suite());
    test->add(JumpDiffusionTest::suite());
    test->add(LowDiscrepancyTest::suite());
    test->add(MatricesTest::suite());
    test->add(MersenneTwisterTest::suite());
    test->add(MoneyTest::suite());
    test->add(OperatorTest::suite());
    test->add(PiecewiseFlatForwardTest::suite());
    test->add(QuantoOptionTest::suite());
    test->add(QuoteTest::suite());
    test->add(RiskStatisticsTest::suite());
    test->add(RngTraitsTest::suite());
    test->add(RoundingTest::suite());
    test->add(Solver1DTest::suite());
    test->add(StatisticsTest::suite());
    test->add(SwapTest::suite());
    test->add(SwaptionTest::suite());
    test->add(TermStructureTest::suite());

    // tests for deprecated (or generally old-style) classes
    test->add(OldPricerTest::suite());

    test->add(BOOST_TEST_CASE(stopTimer));

    return test;
}

