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
 test cases. This benchmarks supports multiprocessing, e.g.

 Single process benchmark for testing:
 ./quantlib-benchmark --size=1 --nProc=1 

 Benchmark with 16 processes and the default size:
 ./quantlib-benchmark --nProc=16

 Benchmark with one worker process per hardware thread and the default size:
 ./quantlib-benchmark 

 This benchmark is derived from quantlibtestsuite.cpp. Please see the
 copyrights therein.
*/

#include <ql/types.hpp>
#include <ql/version.hpp>

#ifdef QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER
#include <boost/process.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#endif

#define BOOST_TEST_NO_MAIN 
#define BOOST_TEST_ALTERNATIVE_INIT_API 
#include <boost/test/included/unit_test.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/framework.hpp>

#include <iomanip>
#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <chrono>
#include <thread>



/* Use BOOST_MSVC instead of _MSC_VER since some other vendors (Metrowerks,
   for example) also #define _MSC_VER
*/
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#  include <ql/auto_link.hpp>
#endif

#include "utilities.hpp"




namespace {

    /**
     * A class representing an individual benchmark.  Each benchmark is one of the QuantLib
     * test-suite tests, run one or more times.  Boost unit test framework causes a dilemma:
     *
     *    * if we don't use boost::unit_test::framework::run to run the test, then all the 
     *       correcness checks are disabled.  We can't validate that the test passed.
     *    * if we do use boost::unit_test::framework::run, then we incur a very large overhead
     *       especially for short tests that are run many thousands of times.
     *
     * We deal with this by running each test exactly once using boost::unit_test::framework::run.
     * Failures are marked using a boost::unit_test::test_observer and cause immediate tear down
     * of the benchmark master process.  All subsequent runs of the test are done through a hack.
     * We copy the declarations of the BOOST_AUTO_TEST_CASE and friends macros in boost/test/unit_test_suite.hpp
     * to declare the symbols that Boost creates.  This allows us to call these symbols directly, 
     * by-passing the boost unit test framework completely.
     *
     * The overall benchmark is parallelised using Boost::IPC.  QuantLib is not thread safe, so any
     * kind of shared memory paralellism is ruled out.  The benchmark creates a large (fixed) amount of
     * work, distributes this between all the workers, and sees how quickly the workers can finish it all.
     * The overall metric is #tasks/s that the system can process.  The tasks are pre-set (these are the
     * tests from the test-suite), and the --size argument to the benchmark controls how many times the
     * entire set of tasks is executed. Once the machine is saturated with work the benchmark typically 
     * exhibits perfect weak scaling: doubling --size will double runtime and leave #tasks/s unchanged.
     * The #tasks/s will typically increase as the machine is given more work to do.
     *
     * The pre-set benchmark sizes are chosen to saturate even very large machines.
     */
    class Benchmark 
    {
        public:
            template<class CALLABLE>
                Benchmark(
                        std::string name,               // the test name, as known by boost::unit_test::test_unit
                        CALLABLE &&body,                // the "body" of the test we want to run
                        double cost                     // how expensive (runtime) this test is relative to others
                        )
                : name_(std::move(name)), test_(nullptr), cost_(cost), totalRuntime_(0), testBody_(std::forward<CALLABLE>(body)) {}

            Benchmark(const Benchmark& copy) = default;        
            Benchmark(Benchmark&& move) = default;        
            Benchmark& operator=(const Benchmark &other) = default;
            Benchmark& operator=(Benchmark &&other) = default;

            double getCost() const          { return cost_; }
            std::string getName() const     { return name_; }
            bool foundTestUnit() const      { return test_ != nullptr; }
            // Total runtime across multiple runs is manually accumulated into the class
            double& getTotalRuntime()       { return totalRuntime_; }
            const double& getTotalRuntime() const { return totalRuntime_; }
            void setTestUnit(const boost::unit_test::test_unit * unit) { test_ = unit; }


            // Run the underlying QuantLib test exactly once using the Boost test framework
            // This will check all results and will flag any errors that are found.  It is much
            // slower than running just the test body outside of the Boost framework
            double runValidation() const 
            {                      
                double time = -1.0;
                try {
                    auto startTime = std::chrono::steady_clock::now();  
                    boost::unit_test::framework::run(test_, false);
                    auto stopTime = std::chrono::steady_clock::now();
                    time = std::chrono::duration_cast<std::chrono::microseconds>(stopTime - startTime).count() * 1e-6;
                } 
                catch(const std::exception &e) {
                    std::cerr << "error: caught exception in benchmark " << getName() << "\n"
                        << "message: " << e.what() << "\n" << std::endl;                
                }
                catch(...) {
                    std::cerr << "error: caught unknown exception in benchmark " << getName() << std::endl;                
                }
                return time;
            }

            // Directly run the body of the underlying QuantLib test (multiple times) without using the Boost
            // test framework. This eliminates all the boost overhead, but also disables all results checking.
            double runBenchmark() const 
            {                      
                double time = -1.0;
                try {
                    auto startTime = std::chrono::steady_clock::now();  
                    testBody_();
                    auto stopTime = std::chrono::steady_clock::now();
                    time = std::chrono::duration_cast<std::chrono::microseconds>(stopTime - startTime).count() * 1e-6;
                } 
                catch(const std::exception &e) {
                    std::cerr << "Error: caught exception in benchmark " << getName() << "\n"
                        << "Message: " << e.what() << "\n" << std::endl;                
                }
                catch(...) {
                    std::cerr << "Error: caught unknown exception in benchmark " << getName() << std::endl;                
                }
                return time;
            }

        private:
            std::string name_;
            const boost::unit_test::test_unit * test_;
            double cost_; 
            double totalRuntime_;
            std::function<void(void)> testBody_;
    };


    /**
     * To determine programmatically whether a test has passed or not, Boost unit test framework requires
     * us to register a test observer class. This only gives the pass/fail status for the most recently
     * run test, not even the name of the test that was run.  Hence we need some additional 
     * plumbing to ensure that intra-test failures are not overridden by intra-test passes
     * (for a test that has multiple calls to BOOST_CHECK or BOOST_FAIL). 
     */
    struct BenchmarkResult : public boost::unit_test::test_observer
    {
        public: 
            BenchmarkResult() : passed_(true) {
                boost::unit_test::framework::register_observer(*this);
            }
            ~BenchmarkResult() {
                boost::unit_test::framework::deregister_observer(*this);
            }
            BenchmarkResult(const BenchmarkResult&) = delete;
            BenchmarkResult(BenchmarkResult&&) = delete;
            BenchmarkResult& operator=(const BenchmarkResult &) = delete;
            BenchmarkResult& operator=(BenchmarkResult &&) = delete;


            void assertion_result( boost::unit_test::assertion_result  ar ) override 
            {
                passed_ = passed_ && (ar == boost::unit_test::AR_PASSED);
            }
            bool pass() const { return passed_; }
            void reset() { passed_ = true; }

        private:
            bool passed_;
    };


    /**
     * This class takes a list of Benchmarks and attempts to find the corresponding 
     * test_units in the Boost test unit tree.  
     * */
    class TestUnitFinder : public boost::unit_test::test_tree_visitor
    {
        private:
            TestUnitFinder(std::vector<Benchmark> & bm) : bm_(bm) {}

            // Utility method needed for initialising the Boost test framework
            static bool init_unit_test_suite() { return true; }

        public:
            bool visit(const boost::unit_test::test_unit & tu) override
            {
                const std::string& thisTest = tu.full_name();
                // Try find this in the bm array.  We know every test name sill start with
                //   "QuantLibTests/"  which contains 14 characters
                for(auto &b : bm_ ) {
                    if( thisTest.find( b.getName(), 14) != std::string::npos ) {
                        // We have a match
                        b.setTestUnit( &tu );
                    }
                }
                // Continue visiting
                return true;
            }


            // Find the corresponding Boost test_unit for each Benchmark
            // If we can't find a test_unit, throw an exception
            static void findAllTests(char** argv, std::vector<Benchmark> &bm)
            {
                boost::unit_test::framework::init(TestUnitFinder::init_unit_test_suite, 1, argv);
                boost::unit_test_framework::framework::finalize_setup_phase();

                TestUnitFinder tuf(bm);
                boost::unit_test::traverse_test_tree(boost::unit_test_framework::framework::master_test_suite(), tuf, true);

                // Now check that we've found all test units
                for(const auto &b : bm)  {
                    if( !b.foundTestUnit() ) {
                        std::string msg = "Unable to find the Boost test unit for Benchmark '";
                        msg += b.getName();
                        msg += "'";
                        std::runtime_error err(msg);
                        throw err;
                    }
                }
            }

        private:
            std::vector<Benchmark> & bm_;
    };


    // The container holding all the benchmarks we will run
    std::vector<Benchmark> bm;



    /**
     * A clas to group and tidy up all the benchmark IO and boilerplate routines
     */
    struct BenchmarkSupport
    {
        // Verbosity level and a logging macro to help debugging
        static int verbose;   
#define LOG_MESSAGE(...)  if(BenchmarkSupport::verbose >= 3) { std::cout << __VA_ARGS__ << std::endl; }


        // The set of pre-defined benchmark sizes that we support
        static const std::vector< std::pair<std::string, unsigned int> > bmSizes;

        // Turn a command line '--size=<value>' string into a benchmark size
        static unsigned int parseBmSize(const std::string &size)
        {
            for(const auto & p : bmSizes) {
                if(p.first == size)
                    return p.second;
            }
            // OK - it's not a preset size, let's see if it's parsable as an integer
            try {
                unsigned int sz = std::stoul(size);
                return sz;
            } 
            catch(const std::exception &e) {
                // Unable to convert to integer.  Abort
                std::cerr << "Error: INVALID BENCHMARK RUN\n";
                std::cerr << "Invalid custom benchmark size specified, unable to convert to an integer\n";
                std::cerr << "Exception generated: " << e.what() << "\n";
                exit(1);
            }
        }

        // Turn a benchmark size into a string for printing
        static std::string bmSizeAsString(unsigned int size)
        {
            for(const auto& p : bmSizes) {
                if(p.second == size)
                    return p.first;
            }
            // Not a preset size
            return "Custom (" + std::to_string(size) + ")";
        }


        static void printGreeting(const std::string &size, unsigned nProc)
        {
            std::cout << std::endl;
            std::cout << std::string(84,'-') << "\n";
            std::cout << "Benchmark Suite QuantLib "  QL_VERSION << "\n";
            std::cout << "\n";
            std::cout << "Benchmark size='" << size << "' on " << nProc << " processes\n";
            std::cout << std::string(84,'-') << "\n";
            std::cout << std::endl;        
        }

        // If a test fails, notify the user and terminate the benchmark
        static void terminateBenchmark()
        {
            std::cerr << "\033[0m\nError: INVALID BENCHMARK RUN.\n"
                <<  "One or more tests failed, please see the log for details" << std::endl ;
            // Tear down the master process, which kills all child threads/processes
            exit(1);
        }


        static void printResults(
                unsigned nSize,                         // the size of the benchmark
                double masterLifetime,                  // lifetime of the master process
                std::vector<double> workerLifetimes     // lifetimes of all the worker processes
                ) 
        {
            std::cout     << "\033[0m\n";
            std::cout     << "Benchmark Size        = " << BenchmarkSupport::bmSizeAsString(nSize) << std::endl;
            std::cout     << "System Throughput     = " << (double(nSize) * bm.size() ) / masterLifetime << " tasks/s" << std::endl;
            std::cout     << "Benchmark Runtime     = " << masterLifetime<< "s" << std::endl;

            if(verbose >=1 ) 
            {
                const size_t nProc = workerLifetimes.size();
                std::cout << "Num. Worker Processes = " << nProc << std::endl;            

                // Work out tail effect.  We define "tail effect" as the ratio of the average (geomean) 
                // tail lifetime, to the lifetime of the master process.  The cutoff for defining 
                // the "tail" is arbitrary.  A ratio of 1 means no tail effect  (tail lifetime is same
                // as lifetime of master process), a ratio near 0 means tail finished significantly 
                // before master process
                std::sort(workerLifetimes.begin(), workerLifetimes.end());        
                const double thresh = 0.1;
                int tail = (int)std::ceil(thresh * nProc);
                double tailGeomean = 1.0;
                for(int i=0; i<tail; i++) {
                    tailGeomean *= workerLifetimes[i];
                }
                tailGeomean = std::pow(tailGeomean, 1.0/tail);
                const double tailEffect = tailGeomean / masterLifetime;

                std::cout << "Tail Effect Ratio     = " << tailEffect << std::endl;
                std::cout << "                      =  Geomean( Shortest " << tail << " worker lifetimes )" << std::endl;
                std::cout << "                      --------------------------------------------------------" << std::endl;
                std::cout << "                                    Lifetime( Master process )" << std::endl;
                std::cout << std::endl;
            }

            std::cout << std::string(84,'-') << std::endl;

            if(verbose >= 2) {            
                std::cout << "                       Total Runtime spent in each test " << std::endl;
                std::cout << std::string(84,'-') << std::endl;

                // Compute max test name length
                size_t len = 0;
                for (const auto & b : bm) { len = std::max(len, b.getName().length() ); }

                for (const auto& b: bm) {
                    std::cout << b.getName()
                        << std::string(len+2 - b.getName().length(),' ')
                        << ": " << b.getTotalRuntime()  << "s" << std::endl;
                }
                std::cout << std::string(84,'-') << std::endl;
            }
            std::cout << std::endl; 
        }


#ifdef QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER
        // The entry point for the std::thread's that will be the workers
        static int worker(const char * exe, const std::vector<std::string>& args) {        
            return boost::process::system(exe, boost::process::args=args);
        }
#endif

        // A helper class to push benchmark objects into the benchmark container 
        // before main() starts.  Every time the constructor is called, a test is added.
        struct AddBenchmark {
            template<class CALLABLE>
                AddBenchmark(std::vector<Benchmark> &bm, CALLABLE && test_body, const char* name, double cost) {
                    bm.push_back( Benchmark(name, std::move(test_body), cost) );
                }
        };
    };
    int BenchmarkSupport::verbose = 0;
    const std::vector< std::pair<std::string, unsigned int> > BenchmarkSupport::bmSizes = {
            {"XXS",  60},
            {"XS",   120},
            {"S",    240},
            {"M",    480},
            {"L",    960}
        };


    // The messages sent from workers to master across boost IPC queues
    struct IPCResultMsg
    {
        unsigned bmId;              // the benchcmark that was run
        unsigned threadId;          // the ID of the worker who ran it
        double time;                // the runtime
    };

    // The messages sent from master to workers across boost IPC queues
    struct IPCInstructionMsg
    {
        unsigned j = 0;             // the benchmark to run
        bool validate = false;      // whether to run in validation mode or not
    };



}  // END anonymous namespace


// These are pulled from boost/unit_test/unit_test_suite.hpp.  We declare the
// bodies of the tests so that we can run them more efficiently.
#define QL_BENCHMARK_DECLARE(test_fixture, test_name, num_iters, cost)   \
    namespace QuantLibTests {                                        \
        namespace test_fixture {                                         \
            struct test_name : public BOOST_AUTO_TEST_CASE_FIXTURE {     \
                void test_method();                                      \
            };                                                           \
        }}                                                               \
        \
        namespace {             \
            /* Declare unique global variable and push benchmark into bm */ \
            BenchmarkSupport::AddBenchmark test_fixture##_##test_name( \
                    bm, \
                    [] { QuantLibTests::test_fixture::test_name thetest; for(int i=0; i<num_iters; i++) thetest.test_method(); }, \
#test_fixture "/" #test_name, cost);                                             \
        }


// Set of all tests we will run.  The integer is the number of times the test is run, and
// the value at the end is a relative runtime cost of each benchmark compared with the others.
// Exact values are not needed, we just need to know what is "expensive" and what is "cheap" 
// in terms of runtime.

// Equity & FX
QL_BENCHMARK_DECLARE(AmericanOptionTests, testFdAmericanGreeks, 1, 0.5);
QL_BENCHMARK_DECLARE(AmericanOptionTests, testFdValues, 20, 3.0);
QL_BENCHMARK_DECLARE(AmericanOptionTests, testCallPutParity, 100, 1.0);
QL_BENCHMARK_DECLARE(AmericanOptionTests, testQdEngineStandardExample, 400, 0.5);
QL_BENCHMARK_DECLARE(EuropeanOptionTests, testImpliedVol, 1, 0.5);
QL_BENCHMARK_DECLARE(EuropeanOptionTests, testMcEngines, 1, 1.0);
QL_BENCHMARK_DECLARE(EuropeanOptionTests, testLocalVolatility, 3, 2.0);
QL_BENCHMARK_DECLARE(BatesModelTests, testDAXCalibration, 1, 0.5);
QL_BENCHMARK_DECLARE(BatesModelTests, testAnalyticVsMCPricing, 1, 1.0);
QL_BENCHMARK_DECLARE(BatesModelTests, testAnalyticAndMcVsJumpDiffusion, 5, 1.0);
QL_BENCHMARK_DECLARE(HestonModelTests, testDAXCalibration, 1, 0.5);
QL_BENCHMARK_DECLARE(HestonModelTests, testFdBarrierVsCached, 1, 3.0);
QL_BENCHMARK_DECLARE(HestonModelTests, testFdAmerican, 1, 1.0);
QL_BENCHMARK_DECLARE(HestonModelTests, testLocalVolFromHestonModel, 10, 1.0);
QL_BENCHMARK_DECLARE(FdHestonTests, testFdmHestonAmerican, 10, 1.0);
QL_BENCHMARK_DECLARE(FdHestonTests, testAmericanCallPutParity, 15, 1.5);
QL_BENCHMARK_DECLARE(FdHestonTests, testFdmHestonBarrierVsBlackScholes, 1, 2.0);
QL_BENCHMARK_DECLARE(HestonSLVModelTests, testMonteCarloCalibration, 1, 3.0);
QL_BENCHMARK_DECLARE(HestonSLVModelTests, testHestonFokkerPlanckFwdEquation, 1, 5.0);
QL_BENCHMARK_DECLARE(HestonSLVModelTests, testBarrierPricingViaHestonLocalVol, 1, 1.0);
QL_BENCHMARK_DECLARE(MCLongstaffSchwartzEngineTests, testAmericanOption, 1, 2.0);
QL_BENCHMARK_DECLARE(VarianceGammaTests, testVarianceGamma, 1, 0.1);
QL_BENCHMARK_DECLARE(ConvertibleBondTests, testBond, 100, 2.0);
QL_BENCHMARK_DECLARE(AndreasenHugeVolatilityInterplTests, testArbitrageFree, 1, 1.0);
QL_BENCHMARK_DECLARE(AndreasenHugeVolatilityInterplTests, testAndreasenHugeCallPut, 1, 1.0);
QL_BENCHMARK_DECLARE(AndreasenHugeVolatilityInterplTests, testAndreasenHugeCall, 1, 1.0);
QL_BENCHMARK_DECLARE(AndreasenHugeVolatilityInterplTests, testAndreasenHugePut, 1, 1.0);
QL_BENCHMARK_DECLARE(AndreasenHugeVolatilityInterplTests, testFlatVolCalibration, 1, 1.0);
QL_BENCHMARK_DECLARE(AndreasenHugeVolatilityInterplTests, testTimeDependentInterestRates, 1, 1.0);
QL_BENCHMARK_DECLARE(AndreasenHugeVolatilityInterplTests, testPiecewiseConstantInterpolation, 1, 1.0);
QL_BENCHMARK_DECLARE(AndreasenHugeVolatilityInterplTests, testLinearInterpolation, 1, 1.0);

// Interest Rates
QL_BENCHMARK_DECLARE(ShortRateModelTests, testSwaps, 30, 3.0);
QL_BENCHMARK_DECLARE(ShortRateModelTests, testCachedHullWhite2, 500, 1.0);
QL_BENCHMARK_DECLARE(ShortRateModelTests, testCachedHullWhiteFixedReversion, 1000, 1.0);
QL_BENCHMARK_DECLARE(MarketModelCmsTests, testMultiStepCmSwapsAndSwaptions, 1, 11.0);
QL_BENCHMARK_DECLARE(MarketModelSmmTests, testMultiStepCoterminalSwapsAndSwaptions, 1, 9.0);
QL_BENCHMARK_DECLARE(BermudanSwaptionTests, testCachedG2Values, 1, 2.0);
QL_BENCHMARK_DECLARE(BermudanSwaptionTests, testCachedValues, 100, 3.0);
QL_BENCHMARK_DECLARE(LiborMarketModelTests, testSwaptionPricing, 1, 1.0);
QL_BENCHMARK_DECLARE(LiborMarketModelTests, testCalibration, 1, 5.0);
QL_BENCHMARK_DECLARE(PiecewiseYieldCurveTests, testConvexMonotoneForwardConsistency, 10, 2.0);
QL_BENCHMARK_DECLARE(PiecewiseYieldCurveTests, testFlatForwardConsistency, 50, 3.0);
QL_BENCHMARK_DECLARE(PiecewiseYieldCurveTests, testGlobalBootstrap, 20, 2.0);
QL_BENCHMARK_DECLARE(OvernightIndexedSwapTests, testBootstrapWithArithmeticAverage, 10, 5.0);
QL_BENCHMARK_DECLARE(OvernightIndexedSwapTests, testBaseBootstrap, 10, 3.0);
QL_BENCHMARK_DECLARE(OvernightIndexedSwapTests, testBootstrapRegression, 10, 1.0);
QL_BENCHMARK_DECLARE(MarkovFunctionalTests, testCalibrationTwoInstrumentSets, 1, 3.0);
QL_BENCHMARK_DECLARE(MarkovFunctionalTests, testCalibrationOneInstrumentSet, 1, 4.0);
QL_BENCHMARK_DECLARE(MarkovFunctionalTests, testVanillaEngines, 1, 7.0);
QL_BENCHMARK_DECLARE(MarkovFunctionalTests, testBermudanSwaption, 3, 1.0);
QL_BENCHMARK_DECLARE(SwaptionVolatilityCubeTests, testSpreadedCube, 20, 1.0);
QL_BENCHMARK_DECLARE(SwaptionVolatilityCubeTests, testSabrNormalVolatility, 1, 1.0);
QL_BENCHMARK_DECLARE(SwaptionVolatilityCubeTests, testSabrVols, 30, 1.0);
QL_BENCHMARK_DECLARE(ZabrTests, testConsistency, 1, 10.0);
QL_BENCHMARK_DECLARE(CmsSpreadTests, testCouponPricing, 1, 1.0);
QL_BENCHMARK_DECLARE(CmsTests, testCmsSwap, 20, 2.0);
QL_BENCHMARK_DECLARE(CmsTests, testParity, 30, 2.0);
QL_BENCHMARK_DECLARE(InterestRateTests, testConversions, 10000, 0.1);

// Credit Derivatives
QL_BENCHMARK_DECLARE(NthToDefaultTests, testGauss, 2, 14.0);
QL_BENCHMARK_DECLARE(CreditDefaultSwapTests, testImpliedHazardRate, 1000, 1.0);
QL_BENCHMARK_DECLARE(CreditDefaultSwapTests, testCachedMarketValue, 1000, 0.1);
QL_BENCHMARK_DECLARE(CreditDefaultSwapTests, testIsdaEngine, 200, 2.0);
QL_BENCHMARK_DECLARE(SquareRootCLVModelTests, testSquareRootCLVMappingFunction, 20, 0.5);
QL_BENCHMARK_DECLARE(SquareRootCLVModelTests, testSquareRootCLVVanillaPricing, 200, 0.5);

// Energy
QL_BENCHMARK_DECLARE(SwingOptionTests, testExtOUJumpSwingOption, 1, 3.0);
QL_BENCHMARK_DECLARE(SwingOptionTests, testExtOUJumpVanillaEngine, 1, 3.0);
QL_BENCHMARK_DECLARE(SwingOptionTests, testFdBSSwingOption, 20, 1.0);
QL_BENCHMARK_DECLARE(VppTests, testVPPPricing, 1, 5.0);
QL_BENCHMARK_DECLARE(VppTests, testKlugeExtOUSpreadOption, 1, 1.0);

// Math
QL_BENCHMARK_DECLARE(RiskStatisticsTests, testResults, 4, 0.5);
QL_BENCHMARK_DECLARE(LowDiscrepancyTests, testMersenneTwisterDiscrepancy, 2, 0.5);
QL_BENCHMARK_DECLARE(LinearLeastSquaresRegressionTests, testMultiDimRegression, 20, 2.0);
QL_BENCHMARK_DECLARE(StatisticsTests, testIncrementalStatistics, 20, 0.5);
QL_BENCHMARK_DECLARE(FunctionsTests, testFactorial, 1000, 0.1);
QL_BENCHMARK_DECLARE(FunctionsTests, testGammaFunction, 1000, 0.5);
QL_BENCHMARK_DECLARE(FunctionsTests, testGammaValues, 100000, 0.5);
QL_BENCHMARK_DECLARE(FunctionsTests, testModifiedBesselFunctions, 10000, 0.5);
QL_BENCHMARK_DECLARE(FunctionsTests, testWeightedModifiedBesselFunctions, 20, 0.5);
QL_BENCHMARK_DECLARE(LowDiscrepancyTests, testHalton, 80, 1.0);
QL_BENCHMARK_DECLARE(GaussianQuadraturesTests, testNonCentralChiSquared, 4000, 0.5);
QL_BENCHMARK_DECLARE(GaussianQuadraturesTests, testNonCentralChiSquaredSumOfNodes, 8000, 0.5);
QL_BENCHMARK_DECLARE(GaussianQuadraturesTests, testMomentBasedGaussianPolynomial, 100000, 0.5);
QL_BENCHMARK_DECLARE(RoundingTests, testCeiling, 100000, 0.1);
QL_BENCHMARK_DECLARE(RoundingTests, testUp, 100000, 0.1);
QL_BENCHMARK_DECLARE(RoundingTests, testFloor, 100000, 0.1);
QL_BENCHMARK_DECLARE(RoundingTests, testDown, 100000, 0.1);
QL_BENCHMARK_DECLARE(RoundingTests, testClosest, 100000, 0.1);




int main(int argc, char* argv[] ) 
{
    const std::string clientModeStr = "--client_mode=true";
    bool clientMode = false;

    // Default number of worker processes to use
#if defined(QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER)
    unsigned nProc = std::thread::hardware_concurrency();
#else
    unsigned nProc = 1;
#endif

    // By default, run the smallest size we have.
    std::string defaultSize = "3";
    std::string size = defaultSize;

    // A threadId is useful for debugging, but has no other purpose
    unsigned threadId = 0;




    ////  Argument handling  //////////////////////////
    for (int i=1; i<argc; ++i) {
        std::string arg = argv[i];
        std::vector<std::string> tok;
        boost::split(tok, arg, boost::is_any_of("="));

        if (tok[0] == "--nProc") {
            QL_REQUIRE(tok.size() == 2, "Must provide a number of worker processes");
            try {
                nProc = boost::numeric_cast<unsigned>(std::stoul(tok[1]));
            } catch(const std::exception &e) {
                std::cerr << "Invalid argument to 'nProc', not a positive integer" << std::endl;
                std::cerr << "Exception generated: " << e.what() << "\n";
                exit(1);
            }
        }
        else if (tok[0] == "--threadId") {
            QL_REQUIRE(tok.size() == 2, "Must provide a threadId");
            try {
                threadId = boost::numeric_cast<unsigned>(std::stoul(tok[1]));                
            } catch(const std::exception &e) {
                std::cerr << "Invalid argument to 'threadId', not a positive integer. This is an internal error, please contact the developers" << std::endl;
                std::cerr << "Exception generated: " << e.what() << "\n";
                exit(1);
            }
        }
        else if (tok[0] == "--verbose") {
            QL_REQUIRE(tok.size() == 2, "Must provide a value for verbose");
            try {
                BenchmarkSupport::verbose = boost::numeric_cast<unsigned>(std::stoul(tok[1]));
            } catch(const std::exception &e) {
                std::cerr << "Invalid argument to 'verbose', not a positive integer" << std::endl;
                std::cerr << "Exception generated: " << e.what() << "\n";
                exit(1);
            }
            QL_REQUIRE(BenchmarkSupport::verbose>=0 && BenchmarkSupport::verbose <= 3, "Value for verbose must be 0, 1, 2 or 3");
        }
        else if (tok[0] == "--size") {
            QL_REQUIRE(tok.size() == 2,
                    "benchmark size is not given");
            size = tok[1];
        }
        else if (arg == "-h" || arg == "--help" || arg == "-?") {
            std::cout
                << "\n'quantlib-benchmark' is QuantLib " QL_VERSION " CPU performance benchmark\n"
                << "\n"
                << "You are strongly encouraged to run 'ulimit -n unlimited' before running this benchmark\n"
                << "on Linux systems.  It uses Boost::IPC for parallelism, and a large number of file descriptors\n"
                << "are needed to run this benchmark with a large number of worker processes.\n"
                << "\n"
                << "By default the benchmark uses a tiny size as a quick check that\n"
                << "everything works.  To benchmark large systems a size of 'S' or larger\n"
                << "should be used.\n"
                << "\n"
                << "Usage: ./quantlib-benchmark [OPTION] ...\n"
                << "\n"
                << "with the following options:"
                << "\n"
#ifdef QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER
                << "--nProc[=NN]       \t parallel execution with NN worker processes.\n"
                << "                   \t Default value is nProc=" << nProc << "\n"
                << "\n"
#endif
                << "--size=<";
            for(const auto &p : BenchmarkSupport::bmSizes) {
                std::cout << p.first << "|";
            }
            std::cout  << "NN> \n"
                << "                   \t the size of the benchmark (how many times each \n"
                << "                   \t task is run), where 'NN' can be any positive integer.\n"
                << "                   \t Default vaue is size=" << defaultSize << "\n"
                << "\n"
                << "--verbose=<0|1|2|3>\t controls verbosity of output, default value is verbose=" << BenchmarkSupport::verbose << "\n"
                << "\n"
                << "-?, --help         \t display this help and exit"
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

    const unsigned int nSize = BenchmarkSupport::parseBmSize(size);
    std::vector<double> workerLifetimes;

    ////////  Finished argument processing, start benchmark code   //////////////////////////////////////////////

    try {

        // Ensure we find the Boost test_unit for each benchmark
        TestUnitFinder::findAllTests(argv, bm);

        // To alleviate tail effects, we sort the bechmarks so that the most expensive ones are first.
        // These will be the first to be dispatched to the OS scheduler
        std::sort(bm.begin(), bm.end(),
                [](const auto& a, const auto& b) { return a.getCost() > b.getCost(); });


        BenchmarkResult bmResult;
        if( !clientMode) 
            BenchmarkSupport::printGreeting(size, nProc);



        if (nProc == 1 && !clientMode) {        
            // Sequential benchmark, useful for debugging
            auto startTime = std::chrono::steady_clock::now();
            for (unsigned i=0; i < nSize; ++i) {
                for(unsigned int j=0; j<bm.size(); j++) {
                    double time;
                    // First run the validation for each benchmark
                    if(i == 0) {
                        bmResult.reset();
                        time = bm[j].runValidation();
                        if( !bmResult.pass() ) {
                            BenchmarkSupport::terminateBenchmark();
                        }
                    }
                    else {
                        time = bm[j].runBenchmark();
                    }
                    bm[j].getTotalRuntime() += time;
                    LOG_MESSAGE("MASTER  :  completed benchmarkId=" << j << ", time=" << time);              
                }
            }
            auto stopTime = std::chrono::steady_clock::now();            
            double masterLifetime = std::chrono::duration_cast<std::chrono::microseconds>(stopTime - startTime).count() * 1e-6;
            workerLifetimes.push_back(masterLifetime);        
            BenchmarkSupport::printResults(nSize, masterLifetime, workerLifetimes);
        }
        else {

#if defined(QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER)

            using namespace boost::interprocess;

            message_queue::size_type recvd_size;
            unsigned int priority=0;
            const unsigned int terminateId=-1;
            const char* const testUnitIdQueueName = "test_unit_queue";
            const char* const testResultQueueName = "test_result_queue";

            if (!clientMode) {     

                // Boost IPC message queue setup
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
                        nSize*bm.size(), sizeof(IPCInstructionMsg)
                        );
                message_queue rq(                
                        open_or_create, testResultQueueName,                 
                        std::max(16u, nProc),                 
                        sizeof(IPCResultMsg)
                        );


                // Start timer for the benchmark
                auto startTime = std::chrono::steady_clock::now();

                // Create the thread group and start each worker process, giving it a unique threadId (useful for debugging)
                std::vector<std::thread> threadGroup;            
                {
                    std::string thread("--threadId="), verb("--verbose=");
                    verb += std::to_string(BenchmarkSupport::verbose);
                    std::vector<std::string> workerArgs = {clientModeStr, thread, verb};            
                    for (unsigned i = 0; i < nProc; ++i) {
                        LOG_MESSAGE("MASTER    : creating worker threadId=" << i+1);         
                        workerArgs[1] = thread + std::to_string(i+1);                                  
                        threadGroup.emplace_back([&,workerArgs]() { BenchmarkSupport::worker(argv[0], workerArgs); });                
                    }
                }

                IPCInstructionMsg msg;
                IPCResultMsg r;
                // Fire off all the benchmarks
                for (unsigned j=0; j < bm.size(); ++j) {
                    // Enqueue nSize copies of each task to even out load balance
                    for (unsigned i=0; i < nSize; ++i) {
                        // Do validation for the first run of each benchmark
                        msg = {j, (i==0)};
                        // Will be non-blocking send since send buffer is big enough 
                        LOG_MESSAGE("MASTER    : sending benchmarkId=" << msg.j << " with validation=" << msg.validate);                   
                        mq.send(&msg, sizeof(IPCInstructionMsg), 0);
                    }
                }
                // Receive all results from workers
                for (unsigned i=0; i < nSize*bm.size(); ++i) {                
                    rq.receive(&r, sizeof(IPCResultMsg), recvd_size, priority);
                    LOG_MESSAGE("MASTER     : received result : threadId=" << r.threadId << ", benchmarkId=" << r.bmId 
                            << ", time=" << r.time << " : " << nSize*bm.size()-1-i << " results pending");    
                    if(r.time < 0) {
                        // A benchmark test has failed
                        BenchmarkSupport::terminateBenchmark();
                    }               
                    bm[r.bmId].getTotalRuntime() += r.time;                             
                }


                // Send terminate signal to all workers
                for (unsigned i=0; i < nProc; ++i) {
                    LOG_MESSAGE("MASTER    : sending TERMINATE signal");                   
                    msg = {terminateId, false};
                    mq.send(&msg, sizeof(IPCInstructionMsg), 0);
                }
                // Receive worker lifetimes
                for (unsigned i=0; i < nProc; ++i) {                
                    rq.receive(&r, sizeof(IPCResultMsg), recvd_size, priority);
                    LOG_MESSAGE("MASTER    : received worker lifetime : threadId=" << r.threadId << ", time=" << r.time << " : " << nProc-1-i << " lifetimes pending");
                    workerLifetimes.push_back(r.time);
                }


                // Synchronize with and exit all threads
                for (auto& thread: threadGroup) {
                    thread.join();
                }

                auto stopTime = std::chrono::steady_clock::now();            
                double masterLifetime = std::chrono::duration_cast<std::chrono::microseconds>(stopTime - startTime).count() * 1e-6;
                BenchmarkSupport::printResults(nSize, masterLifetime, workerLifetimes);


            }
            else {
                // We are a worker process - open Boost IPC queues
                message_queue mq(open_only, testUnitIdQueueName);
                message_queue rq(open_only, testResultQueueName);

                // Record start of this process's lifetime.  We keep tack of lifetimes
                // in order to monitor tail effects
                auto startTime = std::chrono::steady_clock::now();
                // If this worker has nothing to do, we still want a non-zero lifetime
                auto stopTime = std::chrono::steady_clock::now();;

                for(;;) {
                    IPCInstructionMsg id;
                    mq.receive(&id, sizeof(IPCInstructionMsg), recvd_size, priority);                

                    if(id.j == terminateId) {
                        // Worker process being told to terminate.  Report our lifetime.  
                        // Lifetime is how long it took until we completed our final task                    
                        double workerLifetime = std::chrono::duration_cast<std::chrono::microseconds>(stopTime - startTime).count() * 1e-6;
                        IPCResultMsg r {terminateId, threadId, workerLifetime};
                        LOG_MESSAGE("WORKER-" << std::setw(3) << threadId << ": received TERMINATE signal, sending lifetime=" << r.time);
                        rq.send(&r, sizeof(IPCResultMsg), 0);
                        break;
                    }
                    else {
                        LOG_MESSAGE("WORKER-" << std::setw(3) << threadId << ": received benchmarkId=" << id.j << ", validation=" << id.validate << ".  Starting execution ...");                    
                        double time;
                        if( id.validate ) {
                            bmResult.reset();
                            time = bm[id.j].runValidation();
                            time = (bmResult.pass() ? time : -1.0);
                        }
                        else {
                            time = bm[id.j].runBenchmark();
                        }
                        IPCResultMsg r {id.j, threadId, time};
                        // We record the timestamp after each task is complete
                        // We use this to define worker lifetime
                        stopTime = std::chrono::steady_clock::now();
                        LOG_MESSAGE("WORKER-" << std::setw(3) << threadId << ": sending result benchmarkId=" << id.j << ", time=" << r.time);
                        rq.send(&r, sizeof(IPCResultMsg), 0);
                    }                              
                }
                LOG_MESSAGE("WORKER-" << std::setw(3) << threadId << ": exiting");
            }

#else
            std::cout << "Please compile QuantLib with option 'QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER'"
                " to run the benchmarks in parallel" << std::endl;
#endif
        }

    } catch(const std::exception &e) {
        if( !clientMode )
            std::cerr << "MASTER process caught an exception:\n" << e.what() << std::endl;
        else
            std::cerr << "WORKER-" << std::setw(3) << threadId << " caught an exception:\n" << e.what() << std::endl;
    }

    return 0;
}
