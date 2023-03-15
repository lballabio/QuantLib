/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2007 Ferdinando Ametrano
 Copyright (C) 2003, 2007, 2008 StatPro Italia srl

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

#include "extendedtrees.hpp"
#include "utilities.hpp"
#include <ql/time/daycounters/actual360.hpp>
#include <ql/instruments/europeanoption.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/binomialengine.hpp>
#include <ql/experimental/lattices/extendedbinomialtree.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <map>

using namespace QuantLib;
using namespace boost::unit_test_framework;

#undef REPORT_FAILURE
#define REPORT_FAILURE(greekName, payoff, exercise, s, q, r, today, \
                       v, expected, calculated, error, tolerance) \
    BOOST_ERROR(exerciseTypeToString(exercise) << " " \
               << payoff->optionType() << " option with " \
               << payoffTypeToString(payoff) << " payoff:\n" \
               << "    spot value:       " << s << "\n" \
               << "    strike:           " << payoff->strike() << "\n" \
               << "    dividend yield:   " << io::rate(q) << "\n" \
               << "    risk-free rate:   " << io::rate(r) << "\n" \
               << "    reference date:   " << today << "\n" \
               << "    maturity:         " << exercise->lastDate() << "\n" \
               << "    volatility:       " << io::volatility(v) << "\n\n" \
               << "    expected " << greekName << ":   " << expected << "\n" \
               << "    calculated " << greekName << ": " << calculated << "\n"\
               << "    error:            " << error << "\n" \
               << "    tolerance:        " << tolerance);

namespace extended_trees_test {

    // utilities

    enum EngineType { Analytic,
                      JR, CRR, EQP, TGEO, TIAN, LR, JOSHI };

    ext::shared_ptr<GeneralizedBlackScholesProcess>
    makeProcess(const ext::shared_ptr<Quote>& u,
                const ext::shared_ptr<YieldTermStructure>& q,
                const ext::shared_ptr<YieldTermStructure>& r,
                const ext::shared_ptr<BlackVolTermStructure>& vol) {
        return ext::make_shared<BlackScholesMertonProcess>(
           Handle<Quote>(u),
                                         Handle<YieldTermStructure>(q),
                                         Handle<YieldTermStructure>(r),
                                         Handle<BlackVolTermStructure>(vol));
    }

    ext::shared_ptr<VanillaOption>
    makeOption(const ext::shared_ptr<StrikedTypePayoff>& payoff,
               const ext::shared_ptr<Exercise>& exercise,
               const ext::shared_ptr<Quote>& u,
               const ext::shared_ptr<YieldTermStructure>& q,
               const ext::shared_ptr<YieldTermStructure>& r,
               const ext::shared_ptr<BlackVolTermStructure>& vol,
               EngineType engineType,
               Size binomialSteps) {

        ext::shared_ptr<GeneralizedBlackScholesProcess> stochProcess =
            makeProcess(u,q,r,vol);

        ext::shared_ptr<PricingEngine> engine;
        switch (engineType) {
          case Analytic:
            engine = ext::shared_ptr<PricingEngine>(
                                    new AnalyticEuropeanEngine(stochProcess));
            break;
          case JR:
            engine = ext::shared_ptr<PricingEngine>(
                new BinomialVanillaEngine<ExtendedJarrowRudd>(stochProcess,
                                                              binomialSteps));
            break;
          case CRR:
            engine = ext::shared_ptr<PricingEngine>(
                new BinomialVanillaEngine<ExtendedCoxRossRubinstein>(
                                                              stochProcess,
                                                              binomialSteps));
            break;
          case EQP:
            engine = ext::shared_ptr<PricingEngine>(
                new BinomialVanillaEngine<ExtendedAdditiveEQPBinomialTree>(
                                                              stochProcess,
                                                              binomialSteps));
            break;
          case TGEO:
            engine = ext::shared_ptr<PricingEngine>(
                new BinomialVanillaEngine<ExtendedTrigeorgis>(stochProcess,
                                                              binomialSteps));
            break;
          case TIAN:
            engine = ext::shared_ptr<PricingEngine>(
                new BinomialVanillaEngine<ExtendedTian>(stochProcess,
                                                        binomialSteps));
            break;
          case LR:
            engine = ext::shared_ptr<PricingEngine>(
                      new BinomialVanillaEngine<ExtendedLeisenReimer>(
                                                              stochProcess,
                                                              binomialSteps));
            break;
          case JOSHI:
            engine = ext::shared_ptr<PricingEngine>(
                new BinomialVanillaEngine<ExtendedJoshi4>(stochProcess,
                                                          binomialSteps));
            break;
          default:
            QL_FAIL("unknown engine type");
        }

        ext::shared_ptr<VanillaOption> option(
                                        new EuropeanOption(payoff, exercise));
        option->setPricingEngine(engine);
        return option;
    }

}

namespace {

    void testEngineConsistency(extended_trees_test::EngineType engine,
                               Size binomialSteps,
                               std::map<std::string,Real> tolerance) {

        using namespace extended_trees_test;

        std::map<std::string,Real> calculated, expected;

        // test options
        Option::Type types[] = { Option::Call, Option::Put };
        Real strikes[] = { 75.0, 100.0, 125.0 };
        Integer lengths[] = { 1 };

        // test data
        Real underlyings[] = { 100.0 };
        Rate qRates[] = { 0.00, 0.05 };
        Rate rRates[] = { 0.01, 0.05, 0.15 };
        Volatility vols[] = { 0.11, 0.50, 1.20 };

        DayCounter dc = Actual360();
        Date today = Date::todaysDate();

        ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
        ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
        ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today,vol,dc);
        ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
        ext::shared_ptr<YieldTermStructure> qTS = flatRate(today,qRate,dc);
        ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
        ext::shared_ptr<YieldTermStructure> rTS = flatRate(today,rRate,dc);

        for (auto& type : types) {
            for (Real strike : strikes) {
                for (int length : lengths) {
                    Date exDate = today + length * 360;
                    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));
                    ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, strike));
                    // reference option
                    ext::shared_ptr<VanillaOption> refOption =
                        makeOption(payoff, exercise, spot, qTS, rTS, volTS, Analytic, Null<Size>());
                    // option to check
                    ext::shared_ptr<VanillaOption> option =
                        makeOption(payoff, exercise, spot, qTS, rTS, volTS, engine, binomialSteps);

                    for (Real u : underlyings) {
                        for (Real m : qRates) {
                            for (Real n : rRates) {
                                for (Real v : vols) {
                                    Rate q = m, r = n;
                                    spot->setValue(u);
                                    qRate->setValue(q);
                                    rRate->setValue(r);
                                    vol->setValue(v);

                                    expected.clear();
                                    calculated.clear();

                                    expected["value"] = refOption->NPV();
                                    calculated["value"] = option->NPV();

                                    if (option->NPV() > spot->value() * 1.0e-5) {
                                        expected["delta"] = refOption->delta();
                                        expected["gamma"] = refOption->gamma();
                                        expected["theta"] = refOption->theta();
                                        calculated["delta"] = option->delta();
                                        calculated["gamma"] = option->gamma();
                                        calculated["theta"] = option->theta();
                                    }
                                    std::map<std::string, Real>::iterator it;
                                    for (it = calculated.begin(); it != calculated.end(); ++it) {
                                        std::string greek = it->first;
                                        Real expct = expected[greek], calcl = calculated[greek],
                                             tol = tolerance[greek];
                                        Real error = relativeError(expct, calcl, u);
                                        if (error > tol) {
                                            REPORT_FAILURE(greek, payoff, exercise, u, q, r, today,
                                                           v, expct, calcl, error, tol);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

}


void ExtendedTreesTest::testJRBinomialEngines() {

    BOOST_TEST_MESSAGE("Testing time-dependent JR binomial European engines "
                       "against analytic results...");

    using namespace extended_trees_test;

    SavedSettings backup;

    EngineType engine = JR;
    Size steps = 251;
    std::map<std::string,Real> relativeTol;
    relativeTol["value"] = 0.002;
    relativeTol["delta"] = 1.0e-3;
    relativeTol["gamma"] = 1.0e-4;
    relativeTol["theta"] = 0.03;
    testEngineConsistency(engine, steps, relativeTol);
}

void ExtendedTreesTest::testCRRBinomialEngines() {

    BOOST_TEST_MESSAGE("Testing time-dependent CRR binomial European engines "
                       "against analytic results...");

    using namespace extended_trees_test;

    SavedSettings backup;

    EngineType engine = CRR;
    Size steps = 501;
    std::map<std::string,Real> relativeTol;
    relativeTol["value"] = 0.02;
    relativeTol["delta"] = 1.0e-3;
    relativeTol["gamma"] = 1.0e-4;
    relativeTol["theta"] = 0.03;
    testEngineConsistency(engine, steps, relativeTol);
}

void ExtendedTreesTest::testEQPBinomialEngines() {

    BOOST_TEST_MESSAGE("Testing time-dependent EQP binomial European engines "
                       "against analytic results...");

    using namespace extended_trees_test;

    SavedSettings backup;

    EngineType engine = EQP;
    Size steps = 501;
    std::map<std::string,Real> relativeTol;
    relativeTol["value"] = 0.02;
    relativeTol["delta"] = 1.0e-3;
    relativeTol["gamma"] = 1.0e-4;
    relativeTol["theta"] = 0.03;
    testEngineConsistency(engine, steps, relativeTol);
}

void ExtendedTreesTest::testTGEOBinomialEngines() {

    BOOST_TEST_MESSAGE("Testing time-dependent TGEO binomial European engines "
                       "against analytic results...");

    using namespace extended_trees_test;

    SavedSettings backup;

    EngineType engine = TGEO;
    Size steps = 251;
    std::map<std::string,Real> relativeTol;
    relativeTol["value"] = 0.002;
    relativeTol["delta"] = 1.0e-3;
    relativeTol["gamma"] = 1.0e-4;
    relativeTol["theta"] = 0.03;
    testEngineConsistency(engine, steps, relativeTol);
}

void ExtendedTreesTest::testTIANBinomialEngines() {

    BOOST_TEST_MESSAGE("Testing time-dependent TIAN binomial European engines "
                       "against analytic results...");

    using namespace extended_trees_test;

    SavedSettings backup;

    EngineType engine = TIAN;
    Size steps = 251;
    std::map<std::string,Real> relativeTol;
    relativeTol["value"] = 0.002;
    relativeTol["delta"] = 1.0e-3;
    relativeTol["gamma"] = 1.0e-4;
    relativeTol["theta"] = 0.03;
    testEngineConsistency(engine, steps, relativeTol);
}

void ExtendedTreesTest::testLRBinomialEngines() {

    BOOST_TEST_MESSAGE("Testing time-dependent LR binomial European engines "
                       "against analytic results...");

    using namespace extended_trees_test;

    SavedSettings backup;

    EngineType engine = LR;
    Size steps = 251;
    std::map<std::string,Real> relativeTol;
    relativeTol["value"] = 1.0e-6;
    relativeTol["delta"] = 1.0e-3;
    relativeTol["gamma"] = 1.0e-4;
    relativeTol["theta"] = 0.03;
    testEngineConsistency(engine, steps, relativeTol);
}

void ExtendedTreesTest::testJOSHIBinomialEngines() {

    BOOST_TEST_MESSAGE("Testing time-dependent Joshi binomial European engines "
                       "against analytic results...");

    using namespace extended_trees_test;

    SavedSettings backup;

    EngineType engine = JOSHI;
    Size steps = 251;
    std::map<std::string,Real> relativeTol;
    relativeTol["value"] = 1.0e-7;
    relativeTol["delta"] = 1.0e-3;
    relativeTol["gamma"] = 1.0e-4;
    relativeTol["theta"] = 0.03;
    testEngineConsistency(engine, steps, relativeTol);
}

test_suite* ExtendedTreesTest::suite() {
    auto* suite = BOOST_TEST_SUITE("European option extended trees tests");

    suite->add(QUANTLIB_TEST_CASE(&ExtendedTreesTest::testJRBinomialEngines));
    suite->add(QUANTLIB_TEST_CASE(&ExtendedTreesTest::testCRRBinomialEngines));
    suite->add(QUANTLIB_TEST_CASE(&ExtendedTreesTest::testEQPBinomialEngines));
    suite->add(QUANTLIB_TEST_CASE(&ExtendedTreesTest::testTGEOBinomialEngines));
    suite->add(QUANTLIB_TEST_CASE(&ExtendedTreesTest::testTIANBinomialEngines));
    suite->add(QUANTLIB_TEST_CASE(&ExtendedTreesTest::testLRBinomialEngines));
    suite->add(QUANTLIB_TEST_CASE(
                               &ExtendedTreesTest::testJOSHIBinomialEngines));

    return suite;
}
