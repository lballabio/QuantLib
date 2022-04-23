/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2008, 2010, 2018 Klaus Spanderen

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

 Example results: 1. i7 7820X@3.6GHz        :24192.2 mflops
                  2. i7 4702HQ@2.2GHz       : 6524.9 mflops
                  3. i7 870@2.93GHz         : 4759.2 mflops
                  4. Core2 Q9300@2.5Ghz     : 2272.6 mflops
                  5. Core2 Q6600@2.4Ghz     : 1984.0 mflops
                  6. i3 540@3.1Ghz          : 1755.3 mflops
                  7. Raspberry Pi4@1.5GHz   : 1704.2 mflops
                  8. Core2 Dual@2.0Ghz      :  835.9 mflops
                  9. Athlon 64 X2 4400+     :  824.2 mflops
                 10. Cortex-A57@2.0GHz      :  821.7 mflops
                 11. Core2 Dual@2.0Ghz      :  754.1 mflops
                 12. Pentium4 Dual@2.8Ghz   :  423.8 mflops
                 13. Raspberry Pi3@1.2GHz   :  309.2 mflops
                 14. Pentium4@3.0Ghz        :  266.3 mflops
                 15. PentiumIII@1.1Ghz      :  146.2 mflops
                 16. Alpha 2xEV68@833Mhz    :  184.6 mflops
                 17. Wii PowerPC 750@729MHz :   46.1 mflops
                 18. Raspberry Pi ARM@700Mhz:   28.3 mflops
                 19. MIPS R5000@150MHz      :   12.6 mflops
                 20. RISC-V on FPGA@25Mhz   :    2.4 mflops
                 21. Strong ARM@206Mhz      :    1.4 mflops

 Remarks: OS: Linux, static libs
  1. g++-6.3.0 -O3 -ffast-math -march=core-avx2
      Remark: 16 processes
  2. g++-4.8.1 -O3 -ffast-math -march=core-avx2
      Remark: eight processes
  3. gcc-4.6.3, -O3 -ffast-math -mfpmath=sse,387 -march=corei7
      Remark: eight processes
  4. icc-11.0,  -gcc-version=420 -fast -fp-model fast=2 -ipo-jobs2
      Remark: four processes
  5. icc-11.0,  -gcc-version=420 -fast -fp-model fast=2 -ipo-jobs2
      Remark: four processes
  6. gcc-4.4.5, -O3 -ffast-math -mfpmath=sse,387 -msse4.2 -march=core2
      Remark: four processes
  7. gcc-8.3.0, -O3 -ffast-math -mcpu=cortx-a8 -mfpu=neon-fp-armv8
      Remark: four processes
  8. icc-11.0,  -gcc-version=420 -fast -fp-model fast=2 -ipo-jobs2
      Remark: two processes
  9. icc-11.0,  -gcc-version=420 -xSSSE3 -O3 -ipo -no-prec-div -static
                -fp-model fast=2 -ipo-jobs2, Remark: two processes
 10. clang++-6.0.1 -O2, Remark: four processes
 11. gcc-4.2.1, -O3 -ffast-math -mfpmath=sse,387 -msse3 -funroll-all-loops
      Remark: two processes
 12. gcc-4.0.1, -O3 -march=pentium4 -ffast-math
      -mfpmath=sse,387 -msse2 -funroll-all-loops, Remark: two processes
 13. gcc-4.9.2  -O2, Remark: four processes
 14. gcc-4.0.1, -O3 -march=pentium4 -ffast-math
                -mfpmath=sse,387 -msse2 -funroll-all-loops
 15. gcc-4.1.1, -O3 -march=pentium3 -ffast-math
                -mfpmath=sse,387 -msse -funroll-all-loops
 16. gcc-3.3.5, -O3 -mcpu=e67 -funroll-all-loops, Remark: two processes
 17. gcc-4.9.2, -O2 -g on a Nintendo Wii
 18. gcc-4.6.3, -O3
 19. gcc-4-7-4, -O2 on a SGI Indy
 20. gcc-9.2,   -O2 on RISC-V softcore on an Artix7 100T FPGA
 21. gcc-3.4.3, -O2 -g on a Zaurus PDA

  This benchmark is derived from quantlibtestsuite.cpp. Please see the
  copyrights therein.
*/

#include <ql/types.hpp>
#include <ql/version.hpp>
#ifdef QL_USE_HEADER_ONLY_BOOST_TEST
#include <boost/test/included/unit_test.hpp>
#else
#include <boost/test/unit_test.hpp>
#endif
#include <iomanip>
#include <iostream>
#include <list>
#include <string>
#include <utility>
#include <chrono>

/* PAPI code
#include <stdio.h
#include <papi.h>
*/

/* Use BOOST_MSVC instead of _MSC_VER since some other vendors (Metrowerks,
   for example) also #define _MSC_VER
*/
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#  include <ql/auto_link.hpp>
#  define BOOST_LIB_NAME boost_unit_test_framework
#  include <boost/config/auto_link.hpp>
#  undef BOOST_LIB_NAME
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

    std::list<double> runTimes;

    /* PAPI code
    float real_time, proc_time, mflops;
    long_long lflop, flop=0;
    */

    class TimedCase {
      public:
        typedef void (*fct_ptr)();
        explicit TimedCase(fct_ptr f) : f_(f) {}

        void startMeasurement() const {
            /* PAPI code
               lflop = flop;
               PAPI_flops(&real_time, &proc_time, &flop, &mflops);
            */
        }

        void stopMeasurement() const {
            /* PAPI code
               PAPI_flops(&real_time, &proc_time, &flop, &mflops);
               printf("Real_time: %f Proc_time: %f Total mflop: %f\n",
               real_time, proc_time, (flop-lflop)/1e6);
            */
        }

        void operator()() const {
            startMeasurement();
            auto startTime = std::chrono::steady_clock::now();
            BOOST_CHECK(true); // to prevent no-assertion warning
            f_();
            auto stopTime = std::chrono::steady_clock::now();
            stopMeasurement();
            runTimes.push_back(std::chrono::duration_cast<std::chrono::microseconds>(stopTime - startTime).count() * 1e-6);
        }
      private:
        fct_ptr f_;
    };

    class Benchmark {
      public:
        typedef void (*fct_ptr)();
        Benchmark(std::string name, fct_ptr f, double mflop)
        : f_(f), name_(std::move(name)), mflop_(mflop) {}

        test_case* getTestCase() const {
            #if BOOST_VERSION >= 105900
            return boost::unit_test::make_test_case(f_, name_,
                                                    __FILE__, __LINE__);
            #else
            return boost::unit_test::make_test_case(
                       boost::unit_test::callback0<>(f_), name_);
            #endif
        }
        double getMflop() const {
            return mflop_;
        }
        std::string getName() const {
            return name_;
        }
      private:
        TimedCase f_;
        const std::string name_;
        const double mflop_; // total number of mega floating
                             // point operations (not per sec!)
    };

    std::list<Benchmark> bm;

    void printResults() {
        std::string header = "Benchmark Suite "
        "QuantLib " QL_VERSION;

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
            ++iterT;
            ++iterBM;
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
    ThreadKey sessionId() { return {}; }
}
#endif

test_suite* init_unit_test_suite(int, char*[]) {
    bm.emplace_back("AmericanOption::FdAmericanGreeks", &AmericanOptionTest::testFdAmericanGreeks,
                    518.31);
    bm.emplace_back("AsianOption::MCArithmeticAveragePrice",
                    &AsianOptionTest::testMCDiscreteArithmeticAveragePrice, 5186.13);
    bm.emplace_back("BarrierOption::BabsiriValues", &BarrierOptionTest::testBabsiriValues, 880.8);
    bm.emplace_back("BasketOption::EuroTwoValues", &BasketOptionTest::testEuroTwoValues, 340.04);
    bm.emplace_back("BasketOption::TavellaValues", &BasketOptionTest::testTavellaValues, 933.80);
    bm.emplace_back("BasketOption::OddSamples", &BasketOptionTest::testOddSamples, 642.46);
    bm.emplace_back("BatesModel::DAXCalibration", &BatesModelTest::testDAXCalibration, 1993.35);
    bm.emplace_back("ConvertibleBondTest::testBond", &ConvertibleBondTest::testBond, 159.85);
    bm.emplace_back("DigitalOption::MCCashAtHit", &DigitalOptionTest::testMCCashAtHit, 995.87);
    bm.emplace_back("DividendOption::FdEuropeanGreeks", &DividendOptionTest::testFdEuropeanGreeks,
                    949.52);
    bm.emplace_back("DividendOption::FdAmericanGreeks", &DividendOptionTest::testFdAmericanGreeks,
                    1113.74);
    bm.emplace_back("EuropeanOption::FdMcEngines", &EuropeanOptionTest::testMcEngines, 1988.63);
    bm.emplace_back("EuropeanOption::ImpliedVol", &EuropeanOptionTest::testImpliedVol, 131.51);
    bm.emplace_back("EuropeanOption::FdEngines", &EuropeanOptionTest::testFdEngines, 148.43);
    bm.emplace_back("FdHestonTest::testFdmHestonAmerican", &FdHestonTest::testFdmHestonAmerican,
                    234.21);
    bm.emplace_back("HestonModel::DAXCalibration", &HestonModelTest::testDAXCalibration, 555.19);
    bm.emplace_back("InterpolationTest::testSabrInterpolation",
                    &InterpolationTest::testSabrInterpolation, 2266.06);
    bm.emplace_back("JumpDiffusion::Greeks", &JumpDiffusionTest::testGreeks, 433.77);
    bm.emplace_back("MarketModelCmsTest::testCmSwapsSwaptions",
                    &MarketModelCmsTest::testMultiStepCmSwapsAndSwaptions, 11497.73);
    bm.emplace_back("MarketModelSmmTest::testMultiSmmSwaptions",
                    &MarketModelSmmTest::testMultiStepCoterminalSwapsAndSwaptions, 11244.95);
    bm.emplace_back("QuantoOption::ForwardGreeks", &QuantoOptionTest::testForwardGreeks, 90.98);
    bm.emplace_back("RandomNumber::MersenneTwisterDescrepancy",
                    &LowDiscrepancyTest::testMersenneTwisterDiscrepancy, 951.98);
    bm.emplace_back("RiskStatistics::Results", &RiskStatisticsTest::testResults, 300.28);
    bm.emplace_back("ShortRateModel::Swaps", &ShortRateModelTest::testSwaps, 454.73);

    auto* test = BOOST_TEST_SUITE("QuantLib benchmark suite");

    for (std::list<Benchmark>::const_iterator iter = bm.begin();
         iter != bm.end(); ++iter) {
        test->add(iter->getTestCase());
    }

    test->add(QUANTLIB_TEST_CASE(printResults));

    return test;
}
