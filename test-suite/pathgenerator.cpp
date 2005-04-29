/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "pathgenerator.hpp"
#include "utilities.hpp"
#include <ql/MonteCarlo/pathgenerator.hpp>
#include <ql/MonteCarlo/multipathgenerator.hpp>
#include <ql/RandomNumbers/rngtraits.hpp>
#include <ql/Processes/all.hpp>
#include <ql/DayCounters/actual360.hpp>
#include <ql/Utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(PathGeneratorTest)

void teardown() {
    Settings::instance().evaluationDate() = Date();
}

void testSingle(const boost::shared_ptr<StochasticProcess1D>& process,
                const std::string& tag, bool brownianBridge, Real expected) {
    typedef PseudoRandom::rsg_type rsg_type;
    typedef PathGenerator<rsg_type>::sample_type sample_type;

    BigNatural seed = 42;
    Time length = 10;
    Size timeSteps = 12;
    rsg_type rsg = PseudoRandom::make_sequence_generator(timeSteps, seed);
    PathGenerator<rsg_type> generator(process, length, timeSteps,
                                      rsg, brownianBridge);
    for (Size i=0; i<100; i++)
        generator.next();
    sample_type sample = generator.next();
    Real calculated = sample.value[sample.value.size()-1];
    if (std::fabs(calculated-expected) > 1.0e-12) {
        BOOST_FAIL("using " << tag << " process "
                   << (brownianBridge ? "with " : "without ")
                   << "brownian bridge:\n"
                   << std::setprecision(13)
                   << "    calculated: " << calculated << "\n"
                   << "    expected:   " << expected);
    }
}

void testMultiple(
        const std::vector<boost::shared_ptr<StochasticProcess1D> >& processes,
        const Matrix& correlation,
        const std::string& tag, Real expected[]) {
    typedef PseudoRandom::rsg_type rsg_type;
    typedef MultiPathGenerator<rsg_type>::sample_type sample_type;

    BigNatural seed = 42;
    Time length = 10;
    Size timeSteps = 12;
    Size assets = processes.size();
    rsg_type rsg = PseudoRandom::make_sequence_generator(timeSteps*assets,
                                                         seed);
    MultiPathGenerator<rsg_type> generator(processes, correlation,
                                           TimeGrid(length, timeSteps),
                                           rsg, false);
    for (Size i=0; i<100; i++)
        generator.next();
    sample_type sample = generator.next();
    for (Size j=0; j<assets; j++) {
        Real calculated = sample.value[j][sample.value.pathSize()-1];
        if (std::fabs(calculated-expected[j]) > 1.0e-10) {
            BOOST_ERROR("using " << tag << " process "
                       << "(" << io::ordinal(j+1) << " asset:)\n"
                       << std::setprecision(13)
                       << "    calculated: " << calculated << "\n"
                       << "    expected:   " << expected[j]);
        }
    }
}

QL_END_TEST_LOCALS(PathGeneratorTest)


void PathGeneratorTest::testPathGenerator() {

    BOOST_MESSAGE("Testing 1-D path generation against cached values...");

    QL_TEST_BEGIN

    Settings::instance().evaluationDate() = Date(26,April,2005);

    Handle<Quote> x0(boost::shared_ptr<Quote>(new SimpleQuote(100.0)));
    Handle<YieldTermStructure> r(flatRate(0.05, Actual360()));
    Handle<YieldTermStructure> q(flatRate(0.02, Actual360()));
    Handle<BlackVolTermStructure> sigma(flatVol(0.20, Actual360()));
    testSingle(boost::shared_ptr<StochasticProcess1D>(
                                       new BlackScholesProcess(x0,q,r,sigma)),
               "Black-Scholes", false, -0.05907199907827);
    testSingle(boost::shared_ptr<StochasticProcess1D>(
                                       new BlackScholesProcess(x0,q,r,sigma)),
               "Black-Scholes", true, 0.0465700264949);

    testSingle(boost::shared_ptr<StochasticProcess1D>(
                       new GeometricBrownianMotionProcess(100.0, 0.03, 0.20)),
               "geometric Brownian", false, -1.223200366087);
    /*testSingle(boost::shared_ptr<StochasticProcess1D>(
                       new GeometricBrownianMotionProcess(100.0, 0.03, 0.20)),
               "geometric Brownian", true, 0.7287922656694);*/

    testSingle(boost::shared_ptr<StochasticProcess1D>(
                                     new OrnsteinUhlenbeckProcess(0.1, 0.20)),
               "Ornstein-Uhlenbeck", false, 0.003963152890092);
    /*testSingle(boost::shared_ptr<StochasticProcess1D>(
                                     new OrnsteinUhlenbeckProcess(0.1, 0.20)),
               "Ornstein-Uhlenbeck", true, 0.09162840758277);*/

    testSingle(boost::shared_ptr<StochasticProcess1D>(
                                 new SquareRootProcess(0.1, 0.1, 0.20, 10.0)),
               "square-root", false, -0.2488203640255);
    /*testSingle(boost::shared_ptr<StochasticProcess1D>(
                                 new SquareRootProcess(0.1, 0.1, 0.20, 10.0)),
               "square-root", true, -0.09560382006972);*/

    QL_TEST_TEARDOWN
}


void PathGeneratorTest::testMultiPathGenerator() {

    BOOST_MESSAGE("Testing n-D path generation against cached values...");

    QL_TEST_BEGIN

    Settings::instance().evaluationDate() = Date(26,April,2005);

    Handle<Quote> x0(boost::shared_ptr<Quote>(new SimpleQuote(100.0)));
    Handle<YieldTermStructure> r(flatRate(0.05, Actual360()));
    Handle<YieldTermStructure> q(flatRate(0.02, Actual360()));
    Handle<BlackVolTermStructure> sigma(flatVol(0.20, Actual360()));

    Matrix correlation(3,3);
    correlation[0][0] = 1.0; correlation[0][1] = 0.9; correlation[0][2] = 0.7;
    correlation[1][0] = 0.9; correlation[1][1] = 1.0; correlation[1][2] = 0.4;
    correlation[2][0] = 0.7; correlation[2][1] = 0.4; correlation[2][2] = 1.0;

    std::vector<boost::shared_ptr<StochasticProcess1D> > processes(3);
    processes[0] = boost::shared_ptr<StochasticProcess1D>(
                                       new BlackScholesProcess(x0,q,r,sigma));
    processes[1] = boost::shared_ptr<StochasticProcess1D>(
                                       new BlackScholesProcess(x0,q,r,sigma));
    processes[2] = boost::shared_ptr<StochasticProcess1D>(
                                       new BlackScholesProcess(x0,q,r,sigma));
    Real result1[] = {
        0.1428138446652,
        0.2092744336113,
        0.04057136028457 };
    testMultiple(processes, correlation, "Black-Scholes", result1);

    processes[0] = boost::shared_ptr<StochasticProcess1D>(
                       new GeometricBrownianMotionProcess(100.0, 0.03, 0.20));
    processes[1] = boost::shared_ptr<StochasticProcess1D>(
                       new GeometricBrownianMotionProcess(100.0, 0.03, 0.20));
    processes[2] = boost::shared_ptr<StochasticProcess1D>(
                       new GeometricBrownianMotionProcess(100.0, 0.03, 0.20));
    Real result2[] = {
        24.04649099339,
        43.72875185354,
        6.44889193814 };
    testMultiple(processes, correlation, "geometric Brownian", result2);

    processes[0] = boost::shared_ptr<StochasticProcess1D>(
                                     new OrnsteinUhlenbeckProcess(0.1, 0.20));
    processes[1] = boost::shared_ptr<StochasticProcess1D>(
                                     new OrnsteinUhlenbeckProcess(0.1, 0.20));
    processes[2] = boost::shared_ptr<StochasticProcess1D>(
                                     new OrnsteinUhlenbeckProcess(0.1, 0.20));
    Real result3[] = {
        0.1145529276342,
        0.1609358646784,
        0.03128651475229 };
    testMultiple(processes, correlation, "Ornstein-Uhlenbeck", result3);

    processes[0] = boost::shared_ptr<StochasticProcess1D>(
                                 new SquareRootProcess(0.1, 0.1, 0.20, 10.0));
    processes[1] = boost::shared_ptr<StochasticProcess1D>(
                                 new SquareRootProcess(0.1, 0.1, 0.20, 10.0));
    processes[2] = boost::shared_ptr<StochasticProcess1D>(
                                 new SquareRootProcess(0.1, 0.1, 0.20, 10.0));
    Real result4[] = {
        -0.07385744304226,
        0.04484225895393,
        -0.2484464926277 };
    testMultiple(processes, correlation, "square-root", result4);

    QL_TEST_TEARDOWN
}


test_suite* PathGeneratorTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Path generation tests");
    suite->add(BOOST_TEST_CASE(&PathGeneratorTest::testPathGenerator));
    suite->add(BOOST_TEST_CASE(&PathGeneratorTest::testMultiPathGenerator));
    return suite;
}

