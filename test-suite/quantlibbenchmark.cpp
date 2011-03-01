/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2008, 2010 Klaus Spanderen

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
 using the perfex library, http://user.it.uu.se/~mikpe/linux/perfctr
 and PAPI, http://icl.cs.utk.edu/papi

 Example results: 1. Core2 Q9300@2.5Ghz  :2272.6 mflops
                  2. Core2 Q6600@2.4Ghz  :1984.0 mflops
                  3. Core2 Dual@2.0Ghz   : 835.9 mflops
                  4. Athlon 64 X2 4400+  : 824.2 mflops
                  5. Core2 Dual@2.0Ghz   : 754.1 mflops
                  6. Pentium4 Dual@2.8Ghz: 423.8 mflops
                  7. Pentium4@3.0Ghz     : 266.3 mflops
                  8. PentiumIII@1.1Ghz   : 146.2 mflops
                  9. Alpha 2xEV68@833Mhz : 184.6 mflops
                 10. Strong ARM@206Mhz   :   1.4 mflops

 Remarks: OS: Linux, static libs
  1. icc-11.0,  -gcc-version=420 -fast -fp-model fast=2 -ipo-jobs2
      Remark: four processes
  2. icc-11.0,  -gcc-version=420 -fast -fp-model fast=2 -ipo-jobs2
      Remark: four processes
  3. icc-11.0,  -gcc-version=420 -fast -fp-model fast=2 -ipo-jobs2
      Remark: two processes
  4. icc-11.0,  -gcc-version=420 -xSSSE3 -O3 -ipo -no-prec-div -static
                -fp-model fast=2 -ipo-jobs2, Remark: two processes
  5. gcc-4.2.1, -O3 -ffast-math -mfpmath=sse,387 -msse3 -funroll-all-loops
      Remark: two processes
  6. gcc-4.0.1, -O3 -march=pentium4 -ffast-math
      -mfpmath=sse,387 -msse2 -funroll-all-loops, Remark: two processes
  7. gcc-4.0.1, -O3 -march=pentium4 -ffast-math
                -mfpmath=sse,387 -msse2 -funroll-all-loops
  8. gcc-4.1.1, -O3 -march=pentium3 -ffast-math
                -mfpmath=sse,387 -msse -funroll-all-loops
  9. gcc-3.3.5, -O3 -mcpu=e67 -funroll-all-loops, Remark: two processes
 10. gcc-3.4.3, -O2 -g on a Zaurus PDA

  This benchmark is derived from quantlibtestsuite.cpp. Please see the
  copyrights therein.
*/

#include <ql/types.hpp>
#include <ql/version.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/timer.hpp>
#include <iostream>
#include <iomanip>
#include <list>
#include <string>

/* PAPI code
#include <papi.h>
*/

/* Use BOOST_MSVC instead of _MSC_VER since some other vendors (Metrowerks,
   for example) also #define _MSC_VER
*/
#ifdef BOOST_MSVC
#  include <ql/auto_link.hpp>
#  define BOOST_LIB_NAME boost_unit_test_framework
#  include <boost/config/auto_link.hpp>
#  undef BOOST_LIB_NAME

/* uncomment the following lines to unmask floating-point exceptions.
   See http://www.wilmott.com/messageview.cfm?catid=10&threadid=9481
*/
//#  include <float.h>
//   namespace { unsigned int u = _controlfp(_EM_INEXACT, _MCW_EM); }

#endif
#include "utilities.hpp"

#include "americanoption.hpp"
#include "asianoptions.hpp"
#include "barrieroption.hpp"
#include "basketoption.hpp"
#include "batesmodel.hpp"
#include "convertiblebonds.hpp"
#include "digitaloption.hpp"
#include "dividendoption.hpp"
#include "europeanoption.hpp"
#include "fdheston.hpp"
#include "hestonmodel.hpp"
#include "interpolations.hpp"
#include "jumpdiffusion.hpp"
#include "marketmodel_smm.hpp"
#include "marketmodel_cms.hpp"
#include "lowdiscrepancysequences.hpp"
#include "quantooption.hpp"
#include "riskstats.hpp"
#include "shortratemodels.hpp"

using namespace boost::unit_test_framework;


namespace {

    class Benchmark {
      public:
        typedef void (*fct_ptr)();
        Benchmark(std::string name, fct_ptr f, double mflop)
        : f_(f), name_(name), mflop_(mflop) {
        }

        test_case* getTestCase() const {
            return QUANTLIB_TEST_CASE(f_);
        }
        double getMflop() const {
            return mflop_;
        }
        std::string getName() const {
            return name_;
        }
      private:
        fct_ptr f_;
        const std::string name_;
        const double mflop_; // total number of mega floating
                             // point operations (not per sec!)
    };

    boost::timer t;
    std::list<double> runTimes;
    std::list<Benchmark> bm;

    /* PAPI code
    float real_time, proc_time, mflops;
    long_long lflops, flop=0;
    */

    void startTimer() {
        t.restart();

        /* PAPI code
        lflop = flop;
        PAPI_flops(&real_time, &proc_time, &flop, &mflops);
        */
    }

    void stopTimer() {
        runTimes.push_back(t.elapsed());

        /* PAPI code
        PAPI_flops(&real_time, &proc_time, &flop, &mflops);
        printf("Real_time: %f Proc_time: %f Total mflop: %f\n",
               real_time, proc_time, (flop-lflop)/1e6);
        */
    }

    void printResults() {
        std::string header = "Benchmark Suite "
        #ifdef BOOST_MSVC
        QL_LIB_NAME;
        #else
        "QuantLib " QL_VERSION;
        #endif

        std::cout << std::endl
                  << std::string(56,'-') << std::endl;
        std::cout << header << std::endl;
        std::cout << std::string(56,'-')
                  << std::endl << std::endl;

        double sum=0;
        std::list<double>::const_iterator iterT = runTimes.begin();
        std::list<Benchmark>::const_iterator iterBM = bm.begin();

        while (iterT != runTimes.end()) {
            const double mflopsPerSec = iterBM->getMflop()/(*iterT);
            std::cout << iterBM->getName()
                      << std::string(42-iterBM->getName().length(),' ') << ":"
                      << std::fixed << std::setw(6) << std::setprecision(1)
                      << mflopsPerSec
                      << " mflops" << std::endl;

            sum+=mflopsPerSec;
            iterT++;
            iterBM++;
        }
        std::cout << std::string(56,'-') << std::endl
                  << "QuantLib Benchmark Index                  :"
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
        &AmericanOptionTest::testFdAmericanGreeks, 518.31));
    bm.push_back(Benchmark("AmericanOption::FdShoutGreeks",
        &AmericanOptionTest::testFdShoutGreeks, 546.58));
    bm.push_back(Benchmark("AsianOption::MCArithmeticAveragePrice",
        &AsianOptionTest::testMCDiscreteArithmeticAveragePrice, 5186.13));
    bm.push_back(Benchmark("BarrierOption::BabsiriValues",
        &BarrierOptionTest::testBabsiriValues, 880.8));
    bm.push_back(Benchmark("BasketOption::EuroTwoValues",
        &BasketOptionTest::testEuroTwoValues, 340.04));
    bm.push_back(Benchmark("BasketOption::TavellaValues",
        &BasketOptionTest::testTavellaValues, 933.80));
    bm.push_back(Benchmark("BasketOption::OddSamples",
        &BasketOptionTest::testOddSamples, 642.46));
    bm.push_back(Benchmark("BatesModel::DAXCalibration",
        &BatesModelTest::testDAXCalibration, 1993.35));
    bm.push_back(Benchmark("ConvertibleBondTest::testBond",
        &ConvertibleBondTest::testBond, 159.85));
    bm.push_back(Benchmark("DigitalOption::MCCashAtHit",
        &DigitalOptionTest::testMCCashAtHit,995.87));
    bm.push_back(Benchmark("DividendOption::FdEuropeanValues",
        &DividendOptionTest::testFdEuropeanValues, 992.12));
    bm.push_back(Benchmark("DividendOption::FdEuropeanGreeks",
        &DividendOptionTest::testFdEuropeanGreeks, 949.52));
    bm.push_back(Benchmark("DividendOption::FdAmericanGreeks",
        &DividendOptionTest::testFdAmericanGreeks, 1113.74));
    bm.push_back(Benchmark("EuropeanOption::FdMcEngines",
        &EuropeanOptionTest::testMcEngines, 1988.63));
    bm.push_back(Benchmark("EuropeanOption::ImpliedVol",
        &EuropeanOptionTest::testImpliedVol, 131.51));
    bm.push_back(Benchmark("EuropeanOption::FdEngines",
        &EuropeanOptionTest::testFdEngines, 148.43));
    bm.push_back(Benchmark("EuropeanOption::PriceCurve",
        &EuropeanOptionTest::testPriceCurve, 414.76));
    bm.push_back(Benchmark("FdHestonTest::testFdmHestonAmerican",
        &FdHestonTest::testFdmHestonAmerican, 234.21));
    bm.push_back(Benchmark("HestonModel::DAXCalibration",
        &HestonModelTest::testDAXCalibration, 555.19));
    bm.push_back(Benchmark("HestonModel::McVsCached",
        &HestonModelTest::testMcVsCached, 150.04));
    bm.push_back(Benchmark("InterpolationTest::testSabrInterpolation",
        &InterpolationTest::testSabrInterpolation, 2266.06));
    bm.push_back(Benchmark("JumpDiffusion::Greeks",
        &JumpDiffusionTest::testGreeks, 433.77));
    bm.push_back(Benchmark("MarketModelCmsTest::testCmSwapsSwaptions",
        &MarketModelCmsTest::testMultiStepCmSwapsAndSwaptions,
        11497.73));
    bm.push_back(Benchmark("MarketModelSmmTest::testMultiSmmSwaptions",
        &MarketModelSmmTest::testMultiStepCoterminalSwapsAndSwaptions,
        11244.95));
    bm.push_back(Benchmark("QuantoOption::ForwardGreeks",
        &QuantoOptionTest::testForwardGreeks, 90.98));
    bm.push_back(Benchmark("RandomNumber::MersenneTwisterDescrepancy",
        &LowDiscrepancyTest::testMersenneTwisterDiscrepancy, 951.98));
    bm.push_back(Benchmark("RiskStatistics::Results",
        &RiskStatisticsTest::testResults, 300.28));
    bm.push_back(Benchmark("ShortRateModel::Swaps",
        &ShortRateModelTest::testSwaps, 454.73));

    test_suite* test = BOOST_TEST_SUITE("QuantLib benchmark suite");

    for (std::list<Benchmark>::const_iterator iter = bm.begin();
         iter != bm.end(); ++iter) {
        test->add(QUANTLIB_TEST_CASE(startTimer));
        test->add(iter->getTestCase());
        test->add(QUANTLIB_TEST_CASE(stopTimer));
    }

    test->add(QUANTLIB_TEST_CASE(printResults));

    return test;
}
