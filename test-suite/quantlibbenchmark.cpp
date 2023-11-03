/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2008, 2010, 2018, 2023 Klaus Spanderen

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
 performance in mflops. This benchmarks supports multiprocessing, e.g.

 Single process benchmark:
 ./quantlib-benchmark

 Benchmark with 16 processes:
 ./quantlib-benchmark --mp=16

 Benchmark with one process per core
 ./quantlib-benchmark --mp

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
                 22. SPARC v7@25MHz         :    0.78mflops

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
 22. gcc-7.5.0, -O2 on a Sun SPARCstation IPC, FPU: Weitek 3170

  This benchmark is derived from quantlibtestsuite.cpp. Please see the
  copyrights therein.
*/

#include <ql/types.hpp>
#include <ql/version.hpp>

#ifdef QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER
#include <boost/process.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#endif

#define BOOST_TEST_NO_MAIN 1
#include <boost/test/included/unit_test.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include <iomanip>
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <chrono>
#include <thread>

/* PAPI code
#include <stdio.h
#include <papi.h>
*/

/* Use BOOST_MSVC instead of _MSC_VER since some other vendors (Metrowerks,
   for example) also #define _MSC_VER
*/
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#  include <ql/auto_link.hpp>
#endif

#include "utilities.hpp"
#include "hestonmodel.hpp"
#include "interpolations.hpp"
#include "jumpdiffusion.hpp"
#include "marketmodel_smm.hpp"
#include "marketmodel_cms.hpp"
#include "lowdiscrepancysequences.hpp"
#include "quantooption.hpp"
#include "riskstats.hpp"
#include "shortratemodels.hpp"

namespace QuantLibTest {
    namespace AmericanOptionTest {
        struct testFdAmericanGreeks:
            public BOOST_AUTO_TEST_CASE_FIXTURE { void test_method(); };
    }

    namespace AsianOptionTest {
        struct testMCDiscreteArithmeticAveragePrice:
            public BOOST_AUTO_TEST_CASE_FIXTURE { void test_method(); };
    }

    namespace BarrierOptionTest {
        struct testBabsiriValues:
            public BOOST_AUTO_TEST_CASE_FIXTURE { void test_method(); };
    }

    namespace BasketOptionTest {
        struct testEuroTwoValues:
            public BOOST_AUTO_TEST_CASE_FIXTURE { void test_method(); };

        struct testTavellaValues:
            public BOOST_AUTO_TEST_CASE_FIXTURE { void test_method(); };

        struct testOddSamples:
            public BOOST_AUTO_TEST_CASE_FIXTURE { void test_method(); };
    }

    namespace BatesModelTest {
        struct testDAXCalibration:
            public BOOST_AUTO_TEST_CASE_FIXTURE { void test_method(); };
    }

    namespace ConvertibleBondTest {
        struct testBond:
            public BOOST_AUTO_TEST_CASE_FIXTURE { void test_method(); };
    }

    namespace DigitalOptionTest {
        struct testMCCashAtHit:
            public BOOST_AUTO_TEST_CASE_FIXTURE { void test_method(); };
    }

    namespace DividendOptionTest {
        struct testFdEuropeanGreeks:
            public BOOST_AUTO_TEST_CASE_FIXTURE { void test_method(); };

        struct testFdAmericanGreeks:
            public BOOST_AUTO_TEST_CASE_FIXTURE { void test_method(); };
    }

    namespace EuropeanOptionTest {
        struct testMcEngines:
            public BOOST_AUTO_TEST_CASE_FIXTURE { void test_method(); };

        struct testImpliedVol:
            public BOOST_AUTO_TEST_CASE_FIXTURE { void test_method(); };

        struct testFdEngines:
            public BOOST_AUTO_TEST_CASE_FIXTURE { void test_method(); };
    }

    namespace FdHestonTest {
        struct testFdmHestonAmerican:
            public BOOST_AUTO_TEST_CASE_FIXTURE { void test_method(); };
    }
}

namespace {

    class Benchmark {
      public:
        Benchmark(std::string name, std::function<void(void)> f, double mflop)
        : f_(std::move(f)), name_(std::move(name)), mflop_(mflop) {}

        std::function<void(void)> getTestCase() const {
            return f_;
        }
        double getMflop() const {
            return mflop_;
        }
        std::string getName() const {
            return name_;
        }
        void swap(Benchmark& other) {
            std::swap(f_, other.f_);
            std::swap(name_, other.name_);
            std::swap(mflop_, other.mflop_);
        }
      private:
        std::function<void(void)> f_;
        std::string name_;
        double mflop_; // total number of mega floating
                       // point operations (not per sec!)
    };

    std::vector<Benchmark> bm = {
        Benchmark("AmericanOption::FdAmericanGreeks", std::bind(&QuantLibTest::AmericanOptionTest::testFdAmericanGreeks::test_method, QuantLibTest::AmericanOptionTest::testFdAmericanGreeks()), 518.31),
        Benchmark("AsianOption::MCArithmeticAveragePrice", std::bind(&QuantLibTest::AsianOptionTest::testMCDiscreteArithmeticAveragePrice::test_method, QuantLibTest::AsianOptionTest::testMCDiscreteArithmeticAveragePrice()), 5186.13),
        Benchmark("BarrierOption::BabsiriValues", std::bind(&QuantLibTest::BarrierOptionTest::testBabsiriValues::test_method, QuantLibTest::BarrierOptionTest::testBabsiriValues()), 880.8),
        Benchmark("BasketOption::EuroTwoValues", std::bind(&QuantLibTest::BasketOptionTest::testEuroTwoValues::test_method, QuantLibTest::BasketOptionTest::testEuroTwoValues()), 340.04),
        Benchmark("BasketOption::EuroTwoValues", std::bind(&QuantLibTest::BasketOptionTest::testTavellaValues::test_method, QuantLibTest::BasketOptionTest::testTavellaValues()), 933.80),
        Benchmark("BasketOption::EuroTwoValues", std::bind(&QuantLibTest::BasketOptionTest::testOddSamples::test_method, QuantLibTest::BasketOptionTest::testOddSamples()), 642.46),
        Benchmark("BatesModel::DAXCalibration", std::bind(&QuantLibTest::BatesModelTest::testDAXCalibration::test_method, QuantLibTest::BatesModelTest::testDAXCalibration()), 1993.35),
        Benchmark("ConvertibleBondTest::testBond", std::bind(&QuantLibTest::ConvertibleBondTest::testBond::test_method, QuantLibTest::ConvertibleBondTest::testBond()), 159.85),
        Benchmark("DigitalOption::MCCashAtHit", std::bind(&QuantLibTest::DigitalOptionTest::testMCCashAtHit::test_method, QuantLibTest::DigitalOptionTest::testMCCashAtHit()), 995.87),
        Benchmark("DividendOption::FdEuropeanGreeks", std::bind(&QuantLibTest::DividendOptionTest::testFdEuropeanGreeks::test_method, QuantLibTest::DividendOptionTest::testFdEuropeanGreeks()), 949.52),
        Benchmark("DividendOption::FdAmericanGreeks", std::bind(&QuantLibTest::DividendOptionTest::testFdAmericanGreeks::test_method, QuantLibTest::DividendOptionTest::testFdAmericanGreeks()), 1113.74),
        Benchmark("EuropeanOption::FdMcEngines", std::bind(&QuantLibTest::EuropeanOptionTest::testMcEngines::test_method, QuantLibTest::EuropeanOptionTest::testMcEngines()), 1988.63),
        Benchmark("EuropeanOption::ImpliedVol", std::bind(&QuantLibTest::EuropeanOptionTest::testImpliedVol::test_method, QuantLibTest::EuropeanOptionTest::testImpliedVol()), 131.51),
        Benchmark("EuropeanOption::FdEngines", std::bind(&QuantLibTest::EuropeanOptionTest::testFdEngines::test_method, QuantLibTest::EuropeanOptionTest::testFdEngines()), 148.43),
        Benchmark("FdHestonTest::testFdmHestonAmerican", std::bind(&QuantLibTest::FdHestonTest::testFdmHestonAmerican::test_method, QuantLibTest::FdHestonTest::testFdmHestonAmerican()), 234.21),
        Benchmark("HestonModel::DAXCalibration", &HestonModelTest::testDAXCalibration, 555.19),
        Benchmark("InterpolationTest::testSabrInterpolation", &InterpolationTest::testSabrInterpolation, 2266.06),
        Benchmark("JumpDiffusion::Greeks", &JumpDiffusionTest::testGreeks, 433.77),
        Benchmark("MarketModelCmsTest::testCmSwapsSwaptions", &MarketModelCmsTest::testMultiStepCmSwapsAndSwaptions, 11497.73),
        Benchmark("MarketModelSmmTest::testMultiSmmSwaptions", &MarketModelSmmTest::testMultiStepCoterminalSwapsAndSwaptions, 11244.95),
        Benchmark("QuantoOption::ForwardGreeks", &QuantoOptionTest::testForwardGreeks, 90.98),
        Benchmark("RandomNumber::MersenneTwisterDescrepancy", &LowDiscrepancyTest::testMersenneTwisterDiscrepancy, 951.98),
        Benchmark("RiskStatistics::Results", &RiskStatisticsTest::testResults, 300.28),
        Benchmark("ShortRateModel::Swaps", &ShortRateModelTest::testSwaps, 454.73)
    };

    /* PAPI code
    float real_time, proc_time, mflops;
    long_long lflop, flop=0;
    */

    class TimedBenchmark {
      public:
        explicit TimedBenchmark(std::function<void(void)> f) : f_(std::move(f)) {}

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

        double operator()() const {
            startMeasurement();
            auto startTime = std::chrono::steady_clock::now();
            BOOST_CHECK(true); // to prevent no-assertion warning
            f_();
            auto stopTime = std::chrono::steady_clock::now();
            stopMeasurement();
            return std::chrono::duration_cast<std::chrono::microseconds>(
                 stopTime - startTime).count() * 1e-6;
        }
      private:
        std::function<void(void)> f_;
    };

    void printResults(
        unsigned nProc,
        std::vector<std::pair<Benchmark, double> >& runTimes) {

        const std::string header = "Benchmark Suite QuantLib "  QL_VERSION;

        std::cout << std::endl << std::string(58,'-') << std::endl;
        std::cout << header << std::endl;
        std::cout << std::string(58,'-') << std::endl << std::endl;

        std::sort(runTimes.begin(), runTimes.end(),
            [](const auto& a, const auto& b) {
                return a.first.getName() < b.first.getName();
            }
        );

        std::vector<std::tuple<Benchmark, int, double> > aggTimes;
        for (const auto& iter: runTimes) {
            if (aggTimes.empty()
                    || std::get<0>(aggTimes.back()).getName()
                        != iter.first.getName()) {
                aggTimes.emplace_back(iter.first, 1, iter.second);
            }
            else {
                ++std::get<1>(aggTimes.back());
                std::get<2>(aggTimes.back()) += iter.second;
            }
        }

        double sum=0;
        for (const auto& iterT: aggTimes) {
            const double mflopsPerSec
                = std::get<0>(iterT).getMflop() / std::get<2>(iterT)
                    * nProc * std::get<1>(iterT);

            std::cout << std::get<0>(iterT).getName()
                      << std::string(42-std::get<0>(iterT).getName().length(),' ')
                      << ":" << std::fixed << std::setw(8) << std::setprecision(1)
                      << mflopsPerSec
                      << " mflops" << std::endl;

            sum+=mflopsPerSec;
        }
        std::cout << std::string(58,'-') << std::endl
                  << "QuantLib Benchmark Index                  :"
                  << std::fixed << std::setw(8) << std::setprecision(1)
                  << sum/aggTimes.size()
                  << " mflops" << std::endl;
    }
#ifdef QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER
    int worker(const char* exe, const std::vector<std::string>& args) {
        return boost::process::system(exe, boost::process::args=args);
    }
#endif
}

int main(int argc, char* argv[] ) {
    const std::string clientModeStr = "--client_mode=true";
    bool clientMode = false;

    unsigned nProc = 1;
    std::vector<std::pair<Benchmark, double> > runTimes;

    for (int i=1; i<argc; ++i) {
        std::string arg = argv[i];
        std::vector<std::string> tok;
        boost::split(tok, arg, boost::is_any_of("="));

        if (tok[0] == "--mp") {
            nProc = (tok.size() == 2)
                ? boost::numeric_cast<unsigned>(std::stoul(tok[1]))
                : std::thread::hardware_concurrency();
        }
        else if (arg == "--help" || arg == "-?") {
            std::cout
                << "'quantlib-benchmark' is QuantLib " QL_VERSION " CPU performance benchmark"
                << std::endl << std::endl
                << "Usage: ./quantlib-benchmark [OPTION]..."
                << std::endl << std::endl
                << "with the following options:"
                << std::endl
#ifdef QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER
                << "--mp[=PROCESSES] \t parallel execution with PROCESSES processes"
                << std::endl
#endif
                << "-?, --help \t\t display this help and exit"
                << std::endl;
            return 0;
        }
        else if (arg == clientModeStr)  {
            clientMode = true;
        }
        else {
            std::cout << "quantlib-benchmark: unrecognized option '" << arg << "'."
                << std::endl
                << "Try 'quantlib-benchmark --help' for more information."
                << std::endl;
            return 0;
        }
    }

    if (nProc == 1 && !clientMode) {
        std::for_each(bm.begin(), bm.end(),
            [&runTimes](const Benchmark& iter) {
                runTimes.emplace_back(
                    iter, TimedBenchmark(iter.getTestCase())());
        });
        printResults(nProc, runTimes);
    }
    else {
#ifdef QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER
        using namespace boost::interprocess;

        typedef std::pair<unsigned, double> result_type;

        message_queue::size_type recvd_size;
        unsigned priority, terminateId=-1;

        const char* const testUnitIdQueueName = "test_unit_queue";
        const char* const testResultQueueName = "test_result_queue";

        if (!clientMode) {
            message_queue::remove(testUnitIdQueueName);
            message_queue::remove(testResultQueueName);
            struct queue_remove {
                explicit queue_remove(const char* name) : name_(name) { }
                ~queue_remove() { message_queue::remove(name_); }

            private:
                const char* const name_;
            } remover1(testUnitIdQueueName),remover2(testResultQueueName);

            message_queue mq(
                open_or_create, testUnitIdQueueName,
                nProc*bm.size(), sizeof(unsigned)
            );
            message_queue rq(
                open_or_create, testResultQueueName, 16, sizeof(result_type));

            const std::vector<std::string> workerArgs(1, clientModeStr);
            std::vector<std::thread> threadGroup;
            for (unsigned i = 0; i < nProc; ++i) {
                threadGroup.emplace_back([&]() { worker(argv[0], workerArgs); });
            }

            for (unsigned i=0; i < nProc; ++i)
                for (unsigned j=0; j < bm.size(); ++j)
                    mq.send(&j, sizeof(unsigned), 0);

            result_type r;
            for (unsigned i = 0; i < nProc*bm.size(); ++i) {
                rq.receive(&r, sizeof(result_type), recvd_size, priority);
                runTimes.push_back(std::make_pair(bm[r.first], r.second));
            }
            for (unsigned i=0; i < nProc; ++i) {
                mq.send(&terminateId, sizeof(unsigned), 0);
            }
            for (auto& thread: threadGroup) {
                thread.join();
            }
            printResults(nProc, runTimes);
        }
        else {
            message_queue mq(open_only, testUnitIdQueueName);
            message_queue rq(open_only, testResultQueueName);

            unsigned id=0;
            mq.receive(&id, sizeof(unsigned), recvd_size, priority);

            while (id != terminateId) {
                result_type a(id, TimedBenchmark(bm[id].getTestCase())());
                rq.send(&a, sizeof(result_type), 0);

                mq.receive(&id, sizeof(unsigned), recvd_size, priority);
            }
        }
#else
        std::cout << "Please compile QuantLib with option 'QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER'"
                " to run the benchmarks in parallel" << std::endl;
#endif
    }

    return 0;
}
