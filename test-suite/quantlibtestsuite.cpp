
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

/* auto_link was not available in Boost version 1.30.x.  Therefore, we
   require Boost 1.31 under Visual C++ and Borland (for the test
   suite, not the whole library! 1.30.x is fine for the latter) while
   we just skip the inclusion for Unixen since it's not needed.

   Use BOOST_MSVC instead of _MSC_VER since some other vendors (Metrowerks,
   for example) also #define _MSC_VER

*/
//#if (defined(BOOST_MSVC) || defined(__BORLANDC__))
#ifdef BOOST_MSVC
#  if BOOST_VERSION < 103100
     #error Boost 1.31 or later is required for the test suite.
#  endif
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
#include "calendars.hpp"
#include "capfloor.hpp"
#include "compoundforward.hpp"
#include "covariance.hpp"
#include "dates.hpp"
#include "daycounters.hpp"
#include "digitaloption.hpp"
#include "distributions.hpp"
#include "europeanoption.hpp"
#include "factorial.hpp"
#include "instruments.hpp"
#include "integrals.hpp"
#include "interpolations.hpp"
#include "jumpdiffusion.hpp"
#include "lowdiscrepancysequences.hpp"
#include "matrices.hpp"
#include "mersennetwister.hpp"
#include "operators.hpp"
#include "piecewiseflatforward.hpp"
#include "quotes.hpp"
#include "riskstats.hpp"
#include "solvers.hpp"
#include "stats.hpp"
#include "swap.hpp"
#include "swaption.hpp"
#include "termstructures.hpp"
// to be deprecated
#include "old_pricers.hpp"

using namespace boost::unit_test_framework;


test_suite* init_unit_test_suite(int, char* []) {

    std::string header = "Testing QuantLib " QL_VERSION;
    std::string rule = std::string(header.length(),'=');

    BOOST_MESSAGE(rule);
    BOOST_MESSAGE(header);
    BOOST_MESSAGE(rule);
    test_suite* test = BOOST_TEST_SUITE("QuantLib test suite");
    test->add(AmericanOptionTest::suite());
    test->add(AsianOptionTest::suite());
    test->add(BarrierOptionTest::suite());
    test->add(BasketOptionTest::suite());
    test->add(CalendarTest::suite());
    test->add(CapFloorTest::suite());
    test->add(CompoundForwardTest::suite());
    test->add(CovarianceTest::suite());
    test->add(DateTest::suite());
    test->add(DayCounterTest::suite());
    test->add(DigitalOptionTest::suite());
    test->add(DistributionTest::suite());
    test->add(EuropeanOptionTest::suite());
    test->add(FactorialTest::suite());
    test->add(InstrumentTest::suite());
    test->add(IntegralTest::suite());
    test->add(InterpolationTest::suite());
    test->add(JumpDiffusionTest::suite());
    test->add(LowDiscrepancyTest::suite());
    test->add(MatricesTest::suite());
    test->add(MersenneTwisterTest::suite());
    test->add(OperatorTest::suite());
    test->add(PiecewiseFlatForwardTest::suite());
    test->add(QuoteTest::suite());
    test->add(RiskStatisticsTest::suite());
    test->add(Solver1DTest::suite());
    test->add(StatisticsTest::suite());
    test->add(SwapTest::suite());
    test->add(SwaptionTest::suite());
    test->add(TermStructureTest::suite());
    // to be deprecated
    test->add(OldPricerTest::suite());
    return test;
}

