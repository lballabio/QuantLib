/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 StatPro Italia srl

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

#include "marketmodel.hpp"
#include "utilities.hpp"
#include <ql/MarketModels/Products/marketmodelforwards.hpp>
#include <ql/MarketModels/Products/marketmodelcaplets.hpp>
#include <ql/MarketModels/Products/marketmodelforwardsonestep.hpp>
#include <ql/MarketModels/Products/marketmodelcapletsonestep.hpp>
#include <ql/MarketModels/accountingengine.hpp>
#include <ql/MarketModels/Evolvers/forwardratepcevolver.hpp>
#include <ql/MarketModels/Evolvers/forwardrateipcevolver.hpp>
#include <ql/MarketModels/PseudoRoots/exponentialcorrelation.hpp>
#include <ql/MarketModels/PseudoRoots/abcdvolatility.hpp>
#include <ql/MarketModels/BrownianGenerators/mtbrowniangenerator.hpp>
#include <ql/schedule.hpp>
#include <ql/Calendars/nullcalendar.hpp>
#include <ql/DayCounters/actual365fixed.hpp>
#include <ql/PricingEngines/blackmodel.hpp>
#include <ql/Utilities/dataformatters.hpp>

#if defined(BOOST_MSVC)
#include <float.h>
//namespace { unsigned int u = _controlfp(_EM_INEXACT, _MCW_EM); }
#endif

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(MarketModelTest)

#define BEGIN(x) (x+0)
#define END(x) (x+LENGTH(x))

Date todaysDate;
Date endDate;
Array rateTimes, paymentTimes, accruals;
Calendar calendar;
DayCounter dayCounter;

Array todaysForwards, displacements, todaysDiscounts;
std::vector<Volatility> volatilities;
unsigned long seed;

Size paths;

void setup() {

    // times
    calendar = NullCalendar();
    todaysDate = Settings::instance().evaluationDate();
    endDate = todaysDate + 10*Years;
    Schedule dates(calendar, todaysDate, endDate,
                   Semiannual, Following);
    rateTimes = Array(dates.size()-1);
    paymentTimes = Array(rateTimes.size()-1);
    accruals = Array(rateTimes.size()-1);
    dayCounter = Actual365Fixed();

    for (Size i=1; i<dates.size(); ++i)
        rateTimes[i-1] = dayCounter.yearFraction(todaysDate, dates[i]);

    std::copy(rateTimes.begin()+1, rateTimes.end(), paymentTimes.begin());
    
    for (Size i=1; i<rateTimes.size(); ++i)
        accruals[i-1] = rateTimes[i] - rateTimes[i-1];

    // rates
    todaysForwards = Array(paymentTimes.size());
    displacements = Array(paymentTimes.size());
    for (Size i=0; i<todaysForwards.size(); ++i) {
        todaysForwards[i] = 0.03 + 0.0010*i;
        displacements[i] = 0.02 + 0.0005*i;
    }

    todaysDiscounts = Array(rateTimes.size());
    todaysDiscounts[0] = 0.95;
    for (Size i=1; i<rateTimes.size(); ++i)
        todaysDiscounts[i] = todaysDiscounts[i-1] / 
            (1.0+todaysForwards[i-1]*accruals[i-1]);

    // volatilities
    volatilities = std::vector<Volatility>(todaysForwards.size());
    for (Size i=0; i<volatilities.size(); ++i)
        volatilities[i] = 0.10 + 0.005*i;

    seed = 42;

    paths = 16383; // 2^14-1

}


QL_END_TEST_LOCALS(MarketModelTest)



const boost::shared_ptr<SequenceStatistics> simulate(
        const boost::shared_ptr<MarketModelEvolver>& evolver,
        const boost::shared_ptr<MarketModelProduct>& product,
        const EvolutionDescription& evolution,
        Size paths)
{
    Size initialNumeraire = evolution.numeraires().front();
    Real initialNumeraireValue = todaysDiscounts[initialNumeraire];

    AccountingEngine engine(evolver, product, evolution,
                            initialNumeraireValue);
    boost::shared_ptr<SequenceStatistics> stats(new
        SequenceStatistics(product->numberOfProducts()));
    engine.multiplePathValues(*stats, paths);
    return stats;
}



void testForwards(const SequenceStatistics& stats,
                  const Array& strikes)
{
    std::vector<Real> results = stats.mean();
    std::vector<Real> errors = stats.errorEstimate();
    std::vector<Real> stdDevs(todaysForwards.size());
    
    Array expected(todaysForwards.size());
    Real minError = QL_MAX_REAL;
    Real maxError = QL_MIN_REAL;
    for (Size i=0; i<expected.size(); ++i) {
        expected[i] = (todaysForwards[i]-strikes[i])
            *accruals[i]*todaysDiscounts[i+1];
        stdDevs[i] = (results[i]-expected[i])/errors[i];
        if (stdDevs[i]>maxError)
            maxError = stdDevs[i];
        else if (stdDevs[i]<minError)
            minError = stdDevs[i];
    }

    Real errorThreshold = 2.32;
    if (minError > 0.0 || maxError < 0.0 ||
        minError <-errorThreshold || maxError > errorThreshold) {
        for (Size i=0; i<results.size(); ++i) {
            BOOST_MESSAGE(io::ordinal(i+1) << " forward: "
                          << io::rate(results[i])
                          << " +- " << io::rate(errors[i])
                          << "; expected: " << io::rate(expected[i])
                          << "; discrepancy = "
                          << stdDevs[i]
                          << " standard errors");
        }
        BOOST_ERROR("test failed");
    }
}

void testCaplets(const SequenceStatistics& stats,
                 const Array& strikes)
{
    std::vector<Real> results = stats.mean();
    std::vector<Real> errors = stats.errorEstimate();
    std::vector<Real> stdDevs(todaysForwards.size());
    
    Array expected(todaysForwards.size());
    Real minError = QL_MAX_REAL;
    Real maxError = QL_MIN_REAL;
    for (Size i=0; i<expected.size(); ++i) {
        Time expiry = rateTimes[i];
        expected[i] =
            detail::blackFormula(todaysForwards[i]+displacements[i],
                                 strikes[i]+displacements[i],
                                 volatilities[i]*std::sqrt(expiry), 1)
            *accruals[i]*todaysDiscounts[i+1];
        stdDevs[i] = (results[i]-expected[i])/errors[i];
        if (stdDevs[i]>maxError)
            maxError = stdDevs[i];
        else if (stdDevs[i]<minError)
            minError = stdDevs[i];
    }

    Real errorThreshold = 1.98;
    if (minError > 0.0 || maxError < 0.0 ||
        minError <-errorThreshold || maxError > errorThreshold) {
        for (Size i=0; i<results.size(); ++i) {
            BOOST_MESSAGE(io::ordinal(i+1) << " caplet: "
                          << io::rate(results[i])
                          << " +- " << io::rate(errors[i])
                          << "; expected: " << io::rate(expected[i])
                          << "; discrepancy = "
                          << (results[i]-expected[i])/(errors[i] == 0.0 ? 1.0 : errors[i])
                          << " standard errors");
        }
        BOOST_ERROR("test failed");
    }
}

enum PseudoRootType { ExpCorr, AbcdVol };

std::string pseudoRootTypeToString(PseudoRootType type) {
    switch (type) {
      case ExpCorr:
          return "Exponential Correlation";
      case AbcdVol:
          return "abcd Volatility";
      default:
        QL_FAIL("unknown MarketModelEvolver type");
    }
}

boost::shared_ptr<PseudoRoot> makePseudoRoot(
    const EvolutionDescription& evolution,
    const Size numberOfFactors,
    PseudoRootType pseudoRootType)
{
    Real longTermCorrelation = 0.5;
    Real beta = 0.2;
    switch (pseudoRootType) {
        case ExpCorr:
            return boost::shared_ptr<PseudoRoot>(new
                ExponentialCorrelation(longTermCorrelation, beta,
                                       volatilities,
                                       evolution,
                                       numberOfFactors,
                                       todaysForwards,
                                       displacements));
        case AbcdVol:
            return boost::shared_ptr<PseudoRoot>(new
                AbcdVolatility(1.0, 0.0, 0.0, 1.0,
                               volatilities,
                               longTermCorrelation, beta,
                               evolution,
                               numberOfFactors,
                               todaysForwards,
                               displacements));
        default:
            QL_FAIL("unknown PseudoRoot type");
    }
}

enum MeasureType { ProductSuggested, Terminal, MoneyMarket };

std::string measureTypeToString(MeasureType type) {
    switch (type) {
      case ProductSuggested:
          return "ProductSuggested";
      case Terminal:
          return "Terminal";
      case MoneyMarket:
          return "Money Market";
      default:
        QL_FAIL("unknown measure type");
    }
}

void setMeasure(EvolutionDescription& evolution,
                MeasureType measureType) {
    switch (measureType) {
      case ProductSuggested:
        break;
      case Terminal:
        evolution.setTerminalMeasure();
        break;
      case MoneyMarket:
        evolution.setMoneyMarketMeasure();
        break;
      default:
        QL_FAIL("unknown measure type");
    }
}


        
enum EvolverType { Ipc, Pc };

std::string evolverTypeToString(EvolverType type) {
    switch (type) {
      case Ipc:
          return "iterative predictor corrector";
      case Pc:
          return "predictor corrector";
      default:
        QL_FAIL("unknown MarketModelEvolver type");
    }
}

boost::shared_ptr<MarketModelEvolver> makeMarketModelEvolver(
    const boost::shared_ptr<PseudoRoot>& pseudoRoot,
    const EvolutionDescription& evolution,
    const BrownianGeneratorFactory& generatorFactory,
    EvolverType evolverType)
{
    switch (evolverType) {
        case Ipc:
            return boost::shared_ptr<MarketModelEvolver>(new
                ForwardRateIpcEvolver(pseudoRoot, evolution, generatorFactory));
        case Pc:
            return boost::shared_ptr<MarketModelEvolver>(new
                ForwardRatePcEvolver(pseudoRoot, evolution, generatorFactory));
        default:
            QL_FAIL("unknown MarketModelEvolver type");
    }
}

void MarketModelTest::testLongJumpForwards() {

    BOOST_MESSAGE("Repricing (long jump) forwards in a LIBOR market model...");

    QL_TEST_SETUP

    Array strikes = todaysForwards + 0.01;
    boost::shared_ptr<MarketModelProduct> product(new
        MarketModelForwards(rateTimes, accruals, paymentTimes, strikes));
    EvolutionDescription evolution = product->suggestedEvolution();

    for (Size n=0; n<1; n++) {
        MTBrownianGeneratorFactory generatorFactory(seed);
        BOOST_MESSAGE(n+1 << ". Random Sequence: MTBrownianGeneratorFactory");

        for (Size m=0; m<1; m++) {
            Size factors = (m==0 ? todaysForwards.size() : m);
            BOOST_MESSAGE(n+1 << "." << m+1 << "." << " Factors: " << factors);

            PseudoRootType pseudoRoots[] = { ExpCorr, AbcdVol };
            for (Size k=0; k<LENGTH(pseudoRoots); k++) {
                BOOST_MESSAGE(n+1 << "." << m+1 << "." << k+1 << "." << " PseudoRoot: " << pseudoRootTypeToString(pseudoRoots[k]));
                boost::shared_ptr<PseudoRoot> pseudoRoot = makePseudoRoot(evolution, factors, pseudoRoots[k]);

                MeasureType measures[] = { ProductSuggested, Terminal, MoneyMarket };
                for (Size j=0; j<LENGTH(measures); j++) {
                    BOOST_MESSAGE(n+1 << "." << m+1 << "." << k+1 << "." << j+1 << "." << " Measure: " << measureTypeToString(measures[j]));
                    setMeasure(evolution, measures[j]);

                    EvolverType evolvers[] = { Pc, Ipc };
                    boost::shared_ptr<MarketModelEvolver> evolver;
                    Size stop = evolution.isInTerminalMeasure() ? 0 : 1; 
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        BOOST_MESSAGE(n+1 << "." << m+1 << "." << k+1 << "." << j+1 << "." << i+1 << "." << " Evolver: " << evolverTypeToString(evolvers[i]));
                        evolver = makeMarketModelEvolver(pseudoRoot, evolution, generatorFactory, evolvers[i]);
                        boost::shared_ptr<SequenceStatistics> stats = simulate(evolver, product, evolution, paths);
                        testForwards(*stats, strikes);
                    }
                }
            }
        }
    }
}

void MarketModelTest::testVeryLongJumpForwards() {

    BOOST_MESSAGE("Repricing (very long jump) forwards in a LIBOR market model...");

    QL_TEST_SETUP

    Array strikes = todaysForwards + 0.01;
    boost::shared_ptr<MarketModelProduct> product(new
        MarketModelForwardsOneStep(rateTimes, accruals, paymentTimes,strikes));
    EvolutionDescription evolution = product->suggestedEvolution();

    for (Size n=0; n<1; n++) {
        MTBrownianGeneratorFactory generatorFactory(seed);
        BOOST_MESSAGE(n+1 << ". Random Sequence: MTBrownianGeneratorFactory");

        for (Size m=0; m<1; m++) {
            Size factors = (m==0 ? todaysForwards.size() : m);
            BOOST_MESSAGE(n+1 << "." << m+1 << "." << " Factors: " << factors);

            PseudoRootType pseudoRoots[] = { ExpCorr, AbcdVol };
            for (Size k=0; k<LENGTH(pseudoRoots); k++) {
                BOOST_MESSAGE(n+1 << "." << m+1 << "." << k+1 << "." << " PseudoRoot: " << pseudoRootTypeToString(pseudoRoots[k]));
                boost::shared_ptr<PseudoRoot> pseudoRoot = makePseudoRoot(evolution, factors, pseudoRoots[k]);

                MeasureType measures[] = { Terminal, MoneyMarket };
                for (Size j=0; j<LENGTH(measures); j++) {
                    BOOST_MESSAGE(n+1 << "." << m+1 << "." << k+1 << "." << j+1 << "." << " Measure: " << measureTypeToString(measures[j]));
                    setMeasure(evolution, measures[j]);

                    EvolverType evolvers[] = { Pc, Ipc };
                    boost::shared_ptr<MarketModelEvolver> evolver;
                    Size stop = evolution.isInTerminalMeasure() ? 0 : 1; 
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        BOOST_MESSAGE(n+1 << "." << m+1 << "." << k+1 << "." << j+1 << "." << i+1 << "." << " Evolver: " << evolverTypeToString(evolvers[i]));
                        evolver = makeMarketModelEvolver(pseudoRoot, evolution, generatorFactory, evolvers[i]);
                        boost::shared_ptr<SequenceStatistics> stats = simulate(evolver, product, evolution, paths);
                        testForwards(*stats, strikes);
                    }
                }
            }
        }
    }
}

void MarketModelTest::testLongJumpCaplets() {

    BOOST_MESSAGE("Repricing (long jump) caplets in a LIBOR market model...");

    QL_TEST_SETUP

    Array strikes = todaysForwards + 0.01;
    boost::shared_ptr<MarketModelProduct> product(
         new MarketModelCaplets(rateTimes, accruals, paymentTimes, strikes));
    EvolutionDescription evolution = product->suggestedEvolution();

    for (Size n=0; n<1; n++) {
        MTBrownianGeneratorFactory generatorFactory(seed);
        BOOST_MESSAGE(n+1 << ". Random Sequence: MTBrownianGeneratorFactory");

        for (Size m=0; m<1; m++) {
            Size factors = (m==0 ? todaysForwards.size() : m);
            BOOST_MESSAGE(n+1 << "." << m+1 << "." << " Factors: " << factors);

            PseudoRootType pseudoRoots[] = { ExpCorr, AbcdVol };
            for (Size k=0; k<LENGTH(pseudoRoots); k++) {
                BOOST_MESSAGE(n+1 << "." << m+1 << "." << k+1 << "." << " PseudoRoot: " << pseudoRootTypeToString(pseudoRoots[k]));
                boost::shared_ptr<PseudoRoot> pseudoRoot = makePseudoRoot(evolution, factors, pseudoRoots[k]);

                MeasureType measures[] = { Terminal, MoneyMarket };
                for (Size j=0; j<LENGTH(measures); j++) {
                    BOOST_MESSAGE(n+1 << "." << m+1 << "." << k+1 << "." << j+1 << "." << " Measure: " << measureTypeToString(measures[j]));
                    setMeasure(evolution, measures[j]);

                    EvolverType evolvers[] = { Pc, Ipc };
                    boost::shared_ptr<MarketModelEvolver> evolver;
                    Size stop = evolution.isInTerminalMeasure() ? 0 : 1; 
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        BOOST_MESSAGE(n+1 << "." << m+1 << "." << k+1 << "." << j+1 << "." << i+1 << "." << " Evolver: " << evolverTypeToString(evolvers[i]));
                        evolver = makeMarketModelEvolver(pseudoRoot, evolution, generatorFactory, evolvers[i]);
                        boost::shared_ptr<SequenceStatistics> stats = simulate(evolver, product, evolution, paths);
                        testCaplets(*stats, strikes);
                    }
                }
            }
        }
    }
}


void MarketModelTest::testVeryLongJumpCaplets() {

    BOOST_MESSAGE("Repricing (very long jump) caplets in a LIBOR market model...");

    QL_TEST_SETUP

    Array strikes = todaysForwards + 0.01;
    boost::shared_ptr<MarketModelProduct> product(
         new MarketModelCapletsOneStep(rateTimes, accruals,
                                       paymentTimes, strikes));
    EvolutionDescription evolution = product->suggestedEvolution();

    for (Size n=0; n<1; n++) {
        MTBrownianGeneratorFactory generatorFactory(seed);
        BOOST_MESSAGE(n+1 << ". Random Sequence: MTBrownianGeneratorFactory");

        for (Size m=0; m<1; m++) {
            Size factors = (m==0 ? todaysForwards.size() : m);
            BOOST_MESSAGE(n+1 << "." << m+1 << "." << " Factors: " << factors);

            PseudoRootType pseudoRoots[] = { ExpCorr, AbcdVol };
            for (Size k=0; k<LENGTH(pseudoRoots); k++) {
                BOOST_MESSAGE(n+1 << "." << m+1 << "." << k+1 << "." << " PseudoRoot: " << pseudoRootTypeToString(pseudoRoots[k]));
                boost::shared_ptr<PseudoRoot> pseudoRoot = makePseudoRoot(evolution, factors, pseudoRoots[k]);

                MeasureType measures[] = { Terminal, MoneyMarket };
                for (Size j=0; j<LENGTH(measures); j++) {
                    BOOST_MESSAGE(n+1 << "." << m+1 << "." << k+1 << "." << j+1 << "." << " Measure: " << measureTypeToString(measures[j]));
                    setMeasure(evolution, measures[j]);

                    EvolverType evolvers[] = { Pc, Ipc };
                    boost::shared_ptr<MarketModelEvolver> evolver;
                    Size stop = evolution.isInTerminalMeasure() ? 0 : 1; 
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        BOOST_MESSAGE(n+1 << "." << m+1 << "." << k+1 << "." << j+1 << "." << i+1 << "." << " Evolver: " << evolverTypeToString(evolvers[i]));
                        evolver = makeMarketModelEvolver(pseudoRoot, evolution, generatorFactory, evolvers[i]);
                        boost::shared_ptr<SequenceStatistics> stats = simulate(evolver, product, evolution, paths);
                        testCaplets(*stats, strikes);
                    }
                }
            }
        }
    }
}


test_suite* MarketModelTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Market-model tests");
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testLongJumpForwards));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testVeryLongJumpForwards));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testLongJumpCaplets));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testVeryLongJumpCaplets));
    return suite;
}
