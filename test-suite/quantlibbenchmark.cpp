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
 using PAPI, http://icl.cs.utk.edu/papi

 Example results can be found at https://openbenchmarking.org/test/pts/quantlib

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


/* initialize PAPI on Linux
  sudo sysctl -w kernel.perf_event_paranoid=0
  export PAPI_EVENTS="PAPI_TOT_INS,PAPI_FP_OPS,PAPI_FP_INS"
  export PAPI_REPORT=1
*/
//#include <papi.h>



/* Use BOOST_MSVC instead of _MSC_VER since some other vendors (Metrowerks,
   for example) also #define _MSC_VER
*/
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#  include <ql/auto_link.hpp>
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


namespace {

    class Benchmark {
      public:
        typedef void (*fct_ptr)();
        Benchmark(std::string name, fct_ptr f, double mflop)
        : f_(f), name_(std::move(name)), mflop_(mflop) {}

        fct_ptr getTestCase() const {
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
        fct_ptr f_;
        std::string name_;
        double mflop_; // total number of mega floating
                       // point operations (not per sec!)
    };

    std::vector<Benchmark> bm = {
        Benchmark("AmericanOption::FdAmericanGreeks", &AmericanOptionTest::testFdAmericanGreeks, 518.31),
        Benchmark("AsianOption::MCArithmeticAveragePrice", &AsianOptionTest::testMCDiscreteArithmeticAveragePrice, 5186.13),
        Benchmark("BarrierOption::BabsiriValues", &BarrierOptionTest::testBabsiriValues, 880.8),
        Benchmark("BasketOption::EuroTwoValues", &BasketOptionTest::testEuroTwoValues, 340.04),
        Benchmark("BasketOption::TavellaValues", &BasketOptionTest::testTavellaValues, 933.80),
        Benchmark("BasketOption::OddSamples", &BasketOptionTest::testOddSamples, 642.46),
        Benchmark("BatesModel::DAXCalibration", &BatesModelTest::testDAXCalibration, 1993.35),
        Benchmark("ConvertibleBondTest::testBond", &ConvertibleBondTest::testBond, 159.85),
        Benchmark("DigitalOption::MCCashAtHit", &DigitalOptionTest::testMCCashAtHit, 995.87),
        Benchmark("DividendOption::FdEuropeanGreeks", &DividendOptionTest::testFdEuropeanGreeks, 949.52),
        Benchmark("DividendOption::FdAmericanGreeks", &DividendOptionTest::testFdAmericanGreeks, 1113.74),
        Benchmark("EuropeanOption::FdMcEngines", &EuropeanOptionTest::testMcEngines, 1988.63),
        Benchmark("EuropeanOption::ImpliedVol", &EuropeanOptionTest::testImpliedVol, 131.51),
        Benchmark("EuropeanOption::FdEngines", &EuropeanOptionTest::testFdEngines, 148.43),
        Benchmark("FdHestonTest::testFdmHestonAmerican", &FdHestonTest::testFdmHestonAmerican, 234.21),
        Benchmark("HestonModel::DAXCalibration", &HestonModelTest::testDAXCalibration, 555.19),
        Benchmark("InterpolationTest::testSabrInterpolation", &InterpolationTest::testSabrInterpolation, 295.63),
        Benchmark("JumpDiffusion::Greeks", &JumpDiffusionTest::testGreeks, 433.77),
        Benchmark("MarketModelCmsTest::testCmSwapsSwaptions", &MarketModelCmsTest::testMultiStepCmSwapsAndSwaptions, 11497.73),
        Benchmark("MarketModelSmmTest::testMultiSmmSwaptions", &MarketModelSmmTest::testMultiStepCoterminalSwapsAndSwaptions, 11244.95),
        Benchmark("QuantoOption::ForwardGreeks", &QuantoOptionTest::testForwardGreeks, 90.98),
        Benchmark("RandomNumber::MersenneTwisterDescrepancy", &LowDiscrepancyTest::testMersenneTwisterDiscrepancy, 951.98),
        Benchmark("RiskStatistics::Results", &RiskStatisticsTest::testResults, 300.28),
        Benchmark("ShortRateModel::Swaps", &ShortRateModelTest::testSwaps, 454.73)
    };

    class TimedBenchmark {
      public:
        typedef void (*fct_ptr)();
        explicit TimedBenchmark(fct_ptr f, const std::string name) : f_(f), name_(name) {}

        void startMeasurement() const {
            //QL_REQUIRE(PAPI_hl_region_begin(name_.c_str()) == PAPI_OK,
            //    "could not initialize PAPI");
        }

        void stopMeasurement() const {
            //QL_REQUIRE(PAPI_hl_region_end(name_.c_str()) == PAPI_OK,
            //    "could not stop PAPI");
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
        fct_ptr f_;
        const std::string name_;
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
                    iter, TimedBenchmark(iter.getTestCase(), iter.getName())());
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
