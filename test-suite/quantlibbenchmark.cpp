/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Klaus Spanderen

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


/*
 QuantLib Benchmark Suite

 Measures the performance of a preselected set of numerically intensive
 test cases. The overall QuantLib Benchmark Index is given by the average
 performance in mflops.

 The number of floating point operations of a given test case was measured
 using the perfex library, http://user.it.uu.se/~mikpe/linux/perfctr.

 Example results: 1. Pentium4 Dual@2.8Ghz: 423.8 mflops
                  2. Pentium4@3.0Ghz     : 266.3 mflops
                  3. PentiumIII@1.1Ghz   : 146.2 mflops
                  4. Alpha 2xEV68@833Mhz : 184.6 mflops
                  5. Strong ARM@206Mhz   :   1.4 mflops

 Remarks: OS: Linux, static libs
  1. gcc-4.0.1, -O3 -march=pentium4 -ffast-math
                -mfpmath=sse,387 -msse2 -funroll-all-loops
                slightly modified QL version, to enable multi threading.
  2. gcc-4.0.1, -O3 -march=pentium4 -ffast-math
                -mfpmath=sse,387 -msse2 -funroll-all-loops
  3. gcc-4.1.1, -O3 -march=pentium3 -ffast-math
                -mfpmath=sse,387 -msse -funroll-all-loops
  4. gcc-3.3.5, -O3 -mcpu=e67 -funroll-all-loops
                slightly modified QL version, to enable multi threading.
  5. gcc-3.4.3, -O2 -g on a Zaurus PDA

  This benchmark is derived from quantlibtestsuite.cpp. Please see the
  copyrights therein.
*/

#include <ql/types.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/timer.hpp>
#include <iostream>
#include <iomanip>
#include <list>
#include <string>

/* Use BOOST_MSVC instead of _MSC_VER since some other vendors (Metrowerks,
   for example) also #define _MSC_VER
*/
#ifdef BOOST_MSVC
#  define BOOST_LIB_NAME boost_unit_test_framework
#  include <boost/config/auto_link.hpp>
#  undef BOOST_LIB_NAME
#endif

#include "americanoption.hpp"
#include "asianoptions.hpp"
#include "barrieroption.hpp"
#include "basketoption.hpp"
#include "batesmodel.hpp"
#include "digitaloption.hpp"
#include "dividendoption.hpp"
#include "europeanoption.hpp"
#include "hestonmodel.hpp"
#include "jumpdiffusion.hpp"
#include "libormarketmodel.hpp"
#include "libormarketmodelprocess.hpp"
#include "lowdiscrepancysequences.hpp"
#include "quantooption.hpp"
#include "riskstats.hpp"
#include "shortratemodels.hpp"
#include "old_pricers.hpp"

using namespace boost::unit_test_framework;


namespace {

    class Benchmark {
      public:
        typedef void (*fct_ptr)();
        Benchmark(std::string name, fct_ptr f, double mflops)
        : f_(f), name_(name), mflops_(mflops) {
        }

        test_case* getTestCase() const {
            return BOOST_TEST_CASE(f_);
        }
        double getMflops() const {
            return mflops_;
        }
        std::string getName() const {
            return name_;
        }
      private:
        fct_ptr f_;
        const std::string name_;
        const double mflops_; // total number of mega floating
                              // point operations (not per sec!)
    };

    boost::timer t;
    std::list<double> runTimes;
    std::list<Benchmark> bm;

    void startTimer() {
        t.restart();
    }

    void stopTimer() {
        runTimes.push_back(t.elapsed());
    }

    void printResults() {
        std::string header = "Benchmark Suite "
        #ifdef BOOST_MSVC
        QL_LIB_NAME;
        #else
        "QuantLib " QL_VERSION;
        #endif

        std::cout << std::endl
                  << std::string(52,'-') << std::endl;
        std::cout << header << std::endl;
        std::cout << std::string(52,'-')
                  << std::endl << std::endl;

        double sum=0;
        std::list<double>::const_iterator iterT = runTimes.begin();
        std::list<Benchmark>::const_iterator iterBM = bm.begin();

        while (iterT != runTimes.end()) {
            const double mflopsPerSec = iterBM->getMflops()/(*iterT);
            std::cout << iterBM->getName()
                      << std::string(40-iterBM->getName().length(),' ') << ":"
                      << std::fixed << std::setw(6) << std::setprecision(1)
                      << mflopsPerSec
                      << " mflops" << std::endl;

            sum+=mflopsPerSec;
            iterT++;
            iterBM++;
        }
        std::cout << std::string(52,'-') << std::endl
                  << "QuantLib Benchmark Index                :"
                  << std::fixed << std::setw(6) << std::setprecision(1)
                  << sum/runTimes.size()
                  << " mflops" << std::endl;
    }
}

#if defined(QL_ENABLE_SESSIONS)
namespace QuantLib {
    Integer sessionId() { return 0; }
}
#endif

test_suite* init_unit_test_suite(int, char*[]) {

    bm.push_back(Benchmark("AmericanOption::FdAmericanGreeks",
        &AmericanOptionTest::testFdAmericanGreeks, 518.4));
    bm.push_back(Benchmark("AmericanOption::FdShoutGreeks",
        &AmericanOptionTest::testFdShoutGreeks, 546.0));
    bm.push_back(Benchmark("AsianOption::MCArithmeticAveragePrice",
        &AsianOptionTest::testMCDiscreteArithmeticAveragePrice, 4301.1));
    bm.push_back(Benchmark("BarrierOption::BabsiriValues",
        &BarrierOptionTest::testBabsiriValues, 944.5));
    bm.push_back(Benchmark("BasketOption::EuroTwoValues",
        &BasketOptionTest::testEuroTwoValues, 392.9));
    bm.push_back(Benchmark("BasketOption::TavellaValues",
        &BasketOptionTest::testTavellaValues, 520.2));
    bm.push_back(Benchmark("BasketOption::OddSamples",
        &BasketOptionTest::testOddSamples, 749.1));
    bm.push_back(Benchmark("BatesModel::DAXCalibration",
        &BatesModelTest::testDAXCalibration, 2527.7));
    bm.push_back(Benchmark("DigitalOption::MCCashAtHit",
        &DigitalOptionTest::testMCCashAtHit,1033.5));
    bm.push_back(Benchmark("DividendOption::FdEuropeanValues",
        &DividendOptionTest::testFdEuropeanValues, 992.0));
    bm.push_back(Benchmark("DividendOption::FdEuropeanGreeks",
        &DividendOptionTest::testFdEuropeanGreeks, 949.6));
    bm.push_back(Benchmark("DividendOption::FdAmericanGreeks",
        &DividendOptionTest::testFdAmericanGreeks, 1113.8));
    bm.push_back(Benchmark("EuropeanOption::FdMcEngines",
        &EuropeanOptionTest::testMcEngines, 2285.9));
    bm.push_back(Benchmark("EuropeanOption::ImpliedVol",
        &EuropeanOptionTest::testImpliedVol, 137.5));
    bm.push_back(Benchmark("EuropeanOption::FdEngines",
        &EuropeanOptionTest::testFdEngines, 148.52));
    bm.push_back(Benchmark("EuropeanOption::PriceCurve",
        &EuropeanOptionTest::testPriceCurve, 414.8));
    bm.push_back(Benchmark("HestonModel::DAXCalibration",
        &HestonModelTest::testDAXCalibration, 730.7));
    bm.push_back(Benchmark("HestonModel::McVsCached",
        &HestonModelTest::testMcVsCached, 1447.9));
    bm.push_back(Benchmark("JumpDiffusion::Greeks",
        &JumpDiffusionTest::testGreeks, 77.3));
    bm.push_back(Benchmark("LiborMarketModel::SwaptionPricing",
        &LiborMarketModelTest::testSwaptionPricing, 4608.9));
    bm.push_back(Benchmark("LiborMarketModel::Calibration",
        &LiborMarketModelTest::testCalibration, 832.54));
    bm.push_back(Benchmark("LiborMarketModelProcess::CapletPricing",
        &LiborMarketModelProcessTest::testMonteCarloCapletPricing, 4608.9));
    bm.push_back(Benchmark("OldPricer::McMultiFactorPricers",
        &OldPricerTest::testMcMultiFactorPricers, 443.4));
    bm.push_back(Benchmark("QuantoOption::ForwardGreeks",
        &QuantoOptionTest::testForwardGreeks, 89.1));
    bm.push_back(Benchmark("RandomNumber::MersenneTwisterDescrepancy",
        &LowDiscrepancyTest::testMersenneTwisterDiscrepancy, 952.1));
    bm.push_back(Benchmark("RiskStatistics::Results",
        &RiskStatisticsTest::testResults, 300.4));
    bm.push_back(Benchmark("ShortRateModel::Swaps",
        &ShortRateModelTest::testSwaps, 450.7));

    test_suite* test = BOOST_TEST_SUITE("QuantLib benchmark suite");

    for (std::list<Benchmark>::const_iterator iter = bm.begin();
         iter != bm.end(); ++iter) {
        test->add(BOOST_TEST_CASE(startTimer));
        test->add(iter->getTestCase());
        test->add(BOOST_TEST_CASE(stopTimer));
    }

    test->add(BOOST_TEST_CASE(printResults));

    return test;
}

