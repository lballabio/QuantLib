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
#include <ql/MarketModels/Products/marketmodelcoinitialswaps.hpp>
#include <ql/MarketModels/Products/marketmodelcoterminalswaps.hpp>
#include <ql/MarketModels/accountingengine.hpp>
#include <ql/MarketModels/Evolvers/forwardratepcevolver.hpp>
#include <ql/MarketModels/Evolvers/forwardrateipcevolver.hpp>
#include <ql/MarketModels/PseudoRoots/expcorrflatvol.hpp>
#include <ql/MarketModels/PseudoRoots/expcorrabcdvol.hpp>
#include <ql/MarketModels/PseudoRoots/calibratedmarketmodel.hpp>
#include <ql/ShortRateModels/LiborMarketModels/lmlinexpcorrmodel.hpp>
#include <ql/ShortRateModels/LiborMarketModels/lmextlinexpvolmodel.hpp>
#include <ql/MarketModels/BrownianGenerators/mtbrowniangenerator.hpp>
#include <ql/schedule.hpp>
#include <ql/Calendars/nullcalendar.hpp>
#include <ql/DayCounters/actual365fixed.hpp>
#include <ql/PricingEngines/blackmodel.hpp>
#include <ql/Utilities/dataformatters.hpp>
#include <ql/MarketModels/PseudoRoots/abcd.hpp>


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
std::vector<Time> rateTimes, paymentTimes;
std::vector<Real> accruals;
Calendar calendar;
DayCounter dayCounter;

std::vector<Rate> todaysForwards, displacements;
std::vector<DiscountFactor> todaysDiscounts;
std::vector<Volatility> volatilities;
Real a, b, c, d;
Size measureOffset_;
unsigned long seed;

Size paths;

void setup() {

    // times
    calendar = NullCalendar();
    todaysDate = Settings::instance().evaluationDate();
    endDate = todaysDate + 10*Years;
    Schedule dates(calendar, todaysDate, endDate,
                   Semiannual, Following);
    rateTimes = std::vector<Time>(dates.size()-1);
    paymentTimes = std::vector<Time>(rateTimes.size()-1);
    accruals = std::vector<Real>(rateTimes.size()-1);
    dayCounter = Actual365Fixed();

    for (Size i=1; i<dates.size(); ++i)
        rateTimes[i-1] = dayCounter.yearFraction(todaysDate, dates[i]);

    std::copy(rateTimes.begin()+1, rateTimes.end(), paymentTimes.begin());
    
    for (Size i=1; i<rateTimes.size(); ++i)
        accruals[i-1] = rateTimes[i] - rateTimes[i-1];

    // rates
    
    todaysForwards = std::vector<Rate>(paymentTimes.size());
    displacements = std::vector<Rate>(paymentTimes.size());
    for (Size i=0; i<todaysForwards.size(); ++i) {
        todaysForwards[i] = 0.03 + 0.0010*i;
        displacements[i] = 0.02 + 0.0005*i;
    }

    todaysDiscounts = std::vector<DiscountFactor>(rateTimes.size());
    todaysDiscounts[0] = 0.95;
    for (Size i=1; i<rateTimes.size(); ++i)
        todaysDiscounts[i] = todaysDiscounts[i-1] / 
            (1.0+todaysForwards[i-1]*accruals[i-1]);

    // volatilities
    Volatility mktVols[] = {    26.723105,
                                29.155879,
                                30.764028,
                                31.728647,
                                32.255761,
                                32.468845,
                                32.474319,
                                32.341540,
                                32.116407,
                                31.840758,
                                31.529932,
                                31.208719,
                                30.881461,
                                30.564436,
                                30.251958,
                                29.957099,
                                29.671729,
                                29.405887,
                                29.150905};

    volatilities = std::vector<Volatility>(todaysForwards.size());
    for (Size i=0; i<volatilities.size(); ++i)
        volatilities[i] = mktVols[i]/100.;

    a=-0.0597 + 0.06;
    b=0.1677;
    c=0.5403;
    d=0.1710 + 0.06;

    measureOffset_ = 5;

    seed = 42;

    paths = 16383; // 2^14-1
    //paths = 32767; // 2^15-1

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
                  const std::vector<Rate>& strikes)
{
    std::vector<Real> results = stats.mean();
    std::vector<Real> errors = stats.errorEstimate();
    std::vector<Real> stdDevs(todaysForwards.size());
    
    std::vector<Rate> expected(todaysForwards.size());
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
                 const std::vector<Rate>& strikes)
{
    std::vector<Real> results = stats.mean();
    std::vector<Real> errors = stats.errorEstimate();
    std::vector<Real> stdDevs(todaysForwards.size());
    
    std::vector<Rate> expected(todaysForwards.size());
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

void testCoinitialSwaps(const SequenceStatistics& stats,
                  const Real swapRate)
{
    std::vector<Real> results = stats.mean();
    std::vector<Real> errors = stats.errorEstimate();
    std::vector<Real> stdDevs(todaysForwards.size());
    
    std::vector<Rate> expected(todaysForwards.size());
    Real minError = QL_MAX_REAL;
    Real maxError = QL_MIN_REAL;
    Real tmp = 0.;
    for (Size i=0; i<expected.size(); ++i) {
        tmp += (todaysForwards[i]-swapRate)
            *accruals[i]*todaysDiscounts[i+1];
        expected[i] = tmp;
        stdDevs[i] = (results[i]-expected[i])/errors[i];
        if (stdDevs[i]>maxError)
            maxError = stdDevs[i];
        else if (stdDevs[i]<minError)
            minError = stdDevs[i];
    }

    Real errorThreshold = 2.32;
    if (minError <-errorThreshold || maxError > errorThreshold) {
        for (Size i=0; i<results.size(); ++i) {
            BOOST_MESSAGE(io::ordinal(i+1) << " coinitial swap: "
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
void testCoterminalSwaps(const SequenceStatistics& stats,
                  const Real swapRate)
{
    std::vector<Real> results = stats.mean();
    std::vector<Real> errors = stats.errorEstimate();
    std::vector<Real> stdDevs(todaysForwards.size());
    
    std::vector<Rate> expected(todaysForwards.size());
    Real minError = QL_MAX_REAL;
    Real maxError = QL_MIN_REAL;
    Real tmp = 0.;
    Size N = expected.size();
    for (Size i=1; i<=N; ++i) {
        tmp += (todaysForwards[N-i]-swapRate)
            *accruals[N-i]*todaysDiscounts[N-i+1];
        expected[N-i] = tmp;
        stdDevs[N-i] = (results[N-i]-expected[N-i])/errors[N-i];
        if (stdDevs[N-i]>maxError)
            maxError = stdDevs[N-i];
        else if (stdDevs[N-i]<minError)
            minError = stdDevs[N-i];
    }

    Real errorThreshold = 2.32;
    if (minError <-errorThreshold || maxError > errorThreshold) {
        for (Size i=0; i<results.size(); ++i) {
            BOOST_MESSAGE(io::ordinal(i+1) << " coterminal swap: "
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
enum PseudoRootType { ExponentialCorrelationFlatVolatility,
                      ExponentialCorrelationAbcdVolatility,
                      CalibratedMM
};

std::string pseudoRootTypeToString(PseudoRootType type) {
    switch (type) {
      case ExponentialCorrelationFlatVolatility:
          return "Exponential Correlation Flat Volatility";
      case ExponentialCorrelationAbcdVolatility:
          return "Exponential Correlation Abcd Volatility";
      case CalibratedMM:
          return "CalibratedMarketModel";
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
    std::vector<Time> fixingTimes(evolution.rateTimes());
    fixingTimes.pop_back();
    boost::shared_ptr<LmVolatilityModel> volModel(new
        LmExtLinearExponentialVolModel(fixingTimes, b, c, d, a));
    boost::shared_ptr<LmCorrelationModel> corrModel(new
        LmLinearExponentialCorrelationModel(evolution.numberOfRates(), longTermCorrelation, beta));
    std::vector<Real> ks(volatilities.size(),1.0);
    switch (pseudoRootType) {
        case ExponentialCorrelationFlatVolatility:
            return boost::shared_ptr<PseudoRoot>(new
                ExpCorrFlatVol(longTermCorrelation, beta,
                               volatilities,
                               evolution,
                               numberOfFactors,
                               todaysForwards,
                               displacements));
        case ExponentialCorrelationAbcdVolatility:
            return boost::shared_ptr<PseudoRoot>(new
                ExpCorrAbcdVol(a, b, c, d,
                               ks,
                               longTermCorrelation, beta,
                               evolution,
                               numberOfFactors,
                               todaysForwards,
                               displacements));
        case CalibratedMM:
            return boost::shared_ptr<PseudoRoot>(new
                CalibratedMarketModel(volModel, corrModel,
                                      evolution,
                                      numberOfFactors,
                                      todaysForwards,
                                      displacements));
        default:
            QL_FAIL("unknown PseudoRoot type");
    }
}

enum MeasureType { ProductSuggested, Terminal, MoneyMarket, MoneyMarketPlus };

std::string measureTypeToString(MeasureType type) {
    switch (type) {
      case ProductSuggested:
          return "ProductSuggested";
      case Terminal:
          return "Terminal";
      case MoneyMarket:
          return "Money Market";
      case MoneyMarketPlus:
          return "Money Market Plus";
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
      case MoneyMarketPlus:
        evolution.setMoneyMarketMeasurePlus(measureOffset_);
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

    std::vector<Rate> strikes(todaysForwards.size());
    for (Size i=0; i<todaysForwards.size(); ++i)
        strikes[i] = todaysForwards[i] + 0.01;

    boost::shared_ptr<MarketModelProduct> product(new
        MarketModelForwards(rateTimes, accruals, paymentTimes, strikes));
    EvolutionDescription evolution = product->suggestedEvolution();

    for (Size n=0; n<1; n++) {
        MTBrownianGeneratorFactory generatorFactory(seed);
        BOOST_MESSAGE("\n\t" << n << ". Random Sequence: MTBrownianGeneratorFactory");

        for (Size m=0; m<1; m++) {
            Size factors = (m==0 ? todaysForwards.size() : m);
            BOOST_MESSAGE("\n\t" << n << "." << m << "." << " Factors: " << factors);


            PseudoRootType pseudoRoots[] = { //CalibratedMM,
                ExponentialCorrelationFlatVolatility, ExponentialCorrelationAbcdVolatility };
            for (Size k=0; k<LENGTH(pseudoRoots); k++) {
                BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << " PseudoRoot: " << pseudoRootTypeToString(pseudoRoots[k]));
                boost::shared_ptr<PseudoRoot> pseudoRoot = makePseudoRoot(evolution, factors, pseudoRoots[k]);

                MeasureType measures[] = { ProductSuggested, Terminal, MoneyMarket, MoneyMarketPlus };
                for (Size j=0; j<LENGTH(measures); j++) {
                    BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << j << "." << " Measure: " << measureTypeToString(measures[j]));
                    setMeasure(evolution, measures[j]);

                    EvolverType evolvers[] = { Pc, Ipc };
                    boost::shared_ptr<MarketModelEvolver> evolver;
                    Size stop = evolution.isInTerminalMeasure() ? 0 : 1; 
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << j << "." << i << "." << " Evolver: " << evolverTypeToString(evolvers[i]));
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

    std::vector<Rate> strikes(todaysForwards.size());
    for (Size i=0; i<todaysForwards.size(); ++i)
        strikes[i] = todaysForwards[i] + 0.01;

    boost::shared_ptr<MarketModelProduct> product(new
        MarketModelForwardsOneStep(rateTimes, accruals, paymentTimes,strikes));
    EvolutionDescription evolution = product->suggestedEvolution();

    for (Size n=0; n<1; n++) {
        MTBrownianGeneratorFactory generatorFactory(seed);
        BOOST_MESSAGE("\n\t" << n << ". Random Sequence: MTBrownianGeneratorFactory");

        for (Size m=0; m<1; m++) {
            Size factors = (m==0 ? todaysForwards.size() : m);
            BOOST_MESSAGE("\n\t" << n << "." << m << "." << " Factors: " << factors);

            PseudoRootType pseudoRoots[] = { //CalibratedMM,
                ExponentialCorrelationFlatVolatility, ExponentialCorrelationAbcdVolatility };
            for (Size k=0; k<LENGTH(pseudoRoots); k++) {
                BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << " PseudoRoot: " << pseudoRootTypeToString(pseudoRoots[k]));
                boost::shared_ptr<PseudoRoot> pseudoRoot = makePseudoRoot(evolution, factors, pseudoRoots[k]);

                MeasureType measures[] = { Terminal, MoneyMarket, MoneyMarketPlus };
                for (Size j=0; j<LENGTH(measures); j++) {
                    BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << j << "." << " Measure: " << measureTypeToString(measures[j]));
                    setMeasure(evolution, measures[j]);

                    EvolverType evolvers[] = { Pc, Ipc };
                    boost::shared_ptr<MarketModelEvolver> evolver;
                    Size stop = evolution.isInTerminalMeasure() ? 0 : 1; 
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << j << "." << i << "." << " Evolver: " << evolverTypeToString(evolvers[i]));
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

    std::vector<Rate> strikes = todaysForwards;

    boost::shared_ptr<MarketModelProduct> product(
         new MarketModelCaplets(rateTimes, accruals, paymentTimes, strikes));
    EvolutionDescription evolution = product->suggestedEvolution();

    for (Size n=0; n<1; n++) {
        MTBrownianGeneratorFactory generatorFactory(seed);
        BOOST_MESSAGE("\n\t" << n << ". Random Sequence: MTBrownianGeneratorFactory");

        for (Size m=0; m<1; m++) {
            Size factors = (m==0 ? todaysForwards.size() : m);
            BOOST_MESSAGE("\n\t" << n << "." << m << "." << " Factors: " << factors);

            PseudoRootType pseudoRoots[] = { //CalibratedMM,
                ExponentialCorrelationFlatVolatility, ExponentialCorrelationAbcdVolatility };
            for (Size k=0; k<LENGTH(pseudoRoots); k++) {
                BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << " PseudoRoot: " << pseudoRootTypeToString(pseudoRoots[k]));
                boost::shared_ptr<PseudoRoot> pseudoRoot = makePseudoRoot(evolution, factors, pseudoRoots[k]);

                MeasureType measures[] = { Terminal, MoneyMarket, MoneyMarketPlus };
                for (Size j=0; j<LENGTH(measures); j++) {
                    BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << j << "." << " Measure: " << measureTypeToString(measures[j]));
                    setMeasure(evolution, measures[j]);

                    EvolverType evolvers[] = { Pc, Ipc };
                    boost::shared_ptr<MarketModelEvolver> evolver;
                    Size stop = evolution.isInTerminalMeasure() ? 0 : 1; 
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << j << "." << i << "." << " Evolver: " << evolverTypeToString(evolvers[i]));
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

    std::vector<Rate> strikes = todaysForwards;

    boost::shared_ptr<MarketModelProduct> product(
         new MarketModelCapletsOneStep(rateTimes, accruals,
                                       paymentTimes, strikes));
    EvolutionDescription evolution = product->suggestedEvolution();

    for (Size n=0; n<1; n++) {
        MTBrownianGeneratorFactory generatorFactory(seed);
        BOOST_MESSAGE("\n\t" << n << ". Random Sequence: MTBrownianGeneratorFactory");

        for (Size m=0; m<1; m++) {
            Size factors = (m==0 ? todaysForwards.size() : m);
            BOOST_MESSAGE("\n\t" << n << "." << m << "." << " Factors: " << factors);

            PseudoRootType pseudoRoots[] = { //CalibratedMM,
                ExponentialCorrelationFlatVolatility, ExponentialCorrelationAbcdVolatility };
            for (Size k=0; k<LENGTH(pseudoRoots); k++) {
                BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << " PseudoRoot: " << pseudoRootTypeToString(pseudoRoots[k]));
                boost::shared_ptr<PseudoRoot> pseudoRoot = makePseudoRoot(evolution, factors, pseudoRoots[k]);

                MeasureType measures[] = { Terminal, MoneyMarket, MoneyMarketPlus };
                for (Size j=0; j<LENGTH(measures); j++) {
                    BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << j << "." << " Measure: " << measureTypeToString(measures[j]));
                    setMeasure(evolution, measures[j]);

                    EvolverType evolvers[] = { Pc, Ipc };
                    boost::shared_ptr<MarketModelEvolver> evolver;
                    Size stop = evolution.isInTerminalMeasure() ? 0 : 1; 
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << j << "." << i << "." << " Evolver: " << evolverTypeToString(evolvers[i]));
                        evolver = makeMarketModelEvolver(pseudoRoot, evolution, generatorFactory, evolvers[i]);
                        boost::shared_ptr<SequenceStatistics> stats = simulate(evolver, product, evolution, paths);
                        testCaplets(*stats, strikes);
                    }
                }
            }
        }
    }
}


void MarketModelTest::testLongJumpCoinitialSwaps() {

    BOOST_MESSAGE("Repricing (long jump) coinitial swaps in a LIBOR market model...");

    QL_TEST_SETUP

    Real swapRate = 0.04;

    boost::shared_ptr<MarketModelProduct> product(new
        MarketModelCoinitialSwaps(rateTimes, accruals, accruals, paymentTimes, swapRate));
    EvolutionDescription evolution = product->suggestedEvolution();

    for (Size n=0; n<1; n++) {
        MTBrownianGeneratorFactory generatorFactory(seed);
        BOOST_MESSAGE("\n\t" << n << ". Random Sequence: MTBrownianGeneratorFactory");

        for (Size m=0; m<1; m++) {
            Size factors = (m==0 ? todaysForwards.size() : m);
            BOOST_MESSAGE("\n\t" << n << "." << m << "." << " Factors: " << factors);

            PseudoRootType pseudoRoots[] = { //CalibratedMM,
                ExponentialCorrelationFlatVolatility, ExponentialCorrelationAbcdVolatility };
            for (Size k=0; k<LENGTH(pseudoRoots); k++) {
                BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << " PseudoRoot: " << pseudoRootTypeToString(pseudoRoots[k]));
                boost::shared_ptr<PseudoRoot> pseudoRoot = makePseudoRoot(evolution, factors, pseudoRoots[k]);

                MeasureType measures[] = { ProductSuggested, Terminal, MoneyMarket, MoneyMarketPlus };
                for (Size j=0; j<LENGTH(measures); j++) {
                    BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << j << "." << " Measure: " << measureTypeToString(measures[j]));
                    setMeasure(evolution, measures[j]);

                    EvolverType evolvers[] = { Pc, Ipc };
                    boost::shared_ptr<MarketModelEvolver> evolver;
                    Size stop = evolution.isInTerminalMeasure() ? 0 : 1; 
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << j << "." << i << "." << " Evolver: " << evolverTypeToString(evolvers[i]));
                        evolver = makeMarketModelEvolver(pseudoRoot, evolution, generatorFactory, evolvers[i]);
                        boost::shared_ptr<SequenceStatistics> stats = simulate(evolver, product, evolution, paths);
                        testCoinitialSwaps(*stats, swapRate);
                    }
                }
            }
        }
    }
}

void MarketModelTest::testLongJumpCoterminalSwaps() {

    BOOST_MESSAGE("Repricing (long jump) coterminal swaps in a LIBOR market model...");

    QL_TEST_SETUP

    Real swapRate = 0.04;

    boost::shared_ptr<MarketModelProduct> product(new
        MarketModelCoterminalSwaps(rateTimes, accruals, accruals, paymentTimes, swapRate));
    EvolutionDescription evolution = product->suggestedEvolution();

    for (Size n=0; n<1; n++) {
        MTBrownianGeneratorFactory generatorFactory(seed);
        BOOST_MESSAGE("\n\t" << n << ". Random Sequence: MTBrownianGeneratorFactory");

        for (Size m=0; m<1; m++) {
            Size factors = (m==0 ? todaysForwards.size() : m);
            BOOST_MESSAGE("\n\t" << n << "." << m << "." << " Factors: " << factors);

            PseudoRootType pseudoRoots[] = { //CalibratedMM,
                ExponentialCorrelationFlatVolatility, ExponentialCorrelationAbcdVolatility };
            for (Size k=0; k<LENGTH(pseudoRoots); k++) {
                BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << " PseudoRoot: " << pseudoRootTypeToString(pseudoRoots[k]));
                boost::shared_ptr<PseudoRoot> pseudoRoot = makePseudoRoot(evolution, factors, pseudoRoots[k]);

                MeasureType measures[] = { ProductSuggested, Terminal, MoneyMarket, MoneyMarketPlus };
                for (Size j=0; j<LENGTH(measures); j++) {
                    BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << j << "." << " Measure: " << measureTypeToString(measures[j]));
                    setMeasure(evolution, measures[j]);

                    EvolverType evolvers[] = { Pc, Ipc };
                    boost::shared_ptr<MarketModelEvolver> evolver;
                    Size stop = evolution.isInTerminalMeasure() ? 0 : 1; 
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << j << "." << i << "." << " Evolver: " << evolverTypeToString(evolvers[i]));
                        evolver = makeMarketModelEvolver(pseudoRoot, evolution, generatorFactory, evolvers[i]);
                        boost::shared_ptr<SequenceStatistics> stats = simulate(evolver, product, evolution, paths);
                        testCoterminalSwaps(*stats, swapRate);
                    }
                }
            }
        }
    }
}


/////


//void MarketModelTest::testFitAbcdVolatility() {
//
//
//    BOOST_MESSAGE("Testing Abcd volatility ....");
//
//    QL_TEST_SETUP
//
//    //Time T = 2;
//    //// set-up the model
//    //// Parameters following Rebonato
//    //Real a_ = 0.4;              // --> d 
//    //Real b_ = 0.1;              // --> a 
//    //Real c_ = 0.6;              // --> b
//    //Real d_ = 0.5;              // --> c 
//    //// instantaneous volatility
//    //boost::shared_ptr<LmVolatilityModel> volaModel(
//    //                new LmExtLinearExponentialVolModel(rateTimes,b_,c_,d_,a_));
//
//    //boost::shared_ptr<Abcd> volaModelMine(new Abcd(a_,b_,c_,d_));
//    //Real varKlaus = volaModel->integratedVariance(1,2,T);
//    //Real varMine = volaModelMine->primitive(T, 1, 1.5041095890410958);
//
//
//    Real MktFwd[] = {3.415553126, 3.717360278,
//                    3.726857521, 3.723640878,
//                    3.739183376, 3.774429588,
//                    3.816047345, 3.853840187,
//                    3.893627619, 3.931578284,
//                    3.985502557, 4.026108099,
//                    4.087548052, 4.130912684,
//                    4.179982923, 4.223786546,
//                    4.264038801, 4.307456679,
//                    4.336609257, 4.378736736 };
//    
//    //Volatility MktVols[] = { 10.63, 11.59,
//    //                        13.06, 15.48,
//    //                        16.69, 17.66,
//    //                        16.91, 16.78,
//    //                        16.70, 16.51,
//    //                        16.35, 16.18,
//    //                        15.98, 15.78,
//    //                        15.54, 15.26,
//    //                        14.99, 14.74,
//    //                        14.53, 14.34 };
//
//
//    std::vector<Rate> strikes;
//    std::vector<Real> variances;
//    for (Size i=0; i<LENGTH(MktFwd); i++) {
//        strikes.push_back(MktFwd[i]/100.);
//        variances.push_back(volatilities[i]*volatilities[i]*rateTimes[i]);
//    }
//
//    Size nRates = todaysForwards.size();
//
//    boost::shared_ptr<Abcd> instVol(new Abcd(a, b, c, d));
//
//    std::vector<Real> modelVariances(nRates);
//    std::vector<Real> modelVols(nRates);
//    std::vector<Real> ks(nRates);
//    //std::vector<Real> blackNPV(nRates);
//    //std::vector<Real> modelNPV(nRates);
//
//    for (Size i=0; i<nRates; i++) {
//        Time expiry = rateTimes[i];
//        modelVariances[i] = instVol->variance(expiry);
//        modelVols[i] = std::sqrt(modelVariances[i]/expiry);
//        ks[i] = volatilities[i]/modelVols[i];
//        // no dispacement
//        //blackNPV[i] = detail::blackFormula(todaysForwards[i],strikes[i],std::sqrt(variances[i]),1)
//        //            * accruals[i] * todaysDiscounts[i+1];
//        //modelNPV[i] = detail::blackFormula(todaysForwards[i],strikes[i],std::sqrt(modelVariances[i]),1)
//        //            * accruals[i] * todaysDiscounts[i+1];
//        BOOST_MESSAGE(i << " MktVol="    << io::rate(volatilities[i])      <<
//                           " ModVol="    << io::rate(modelVols[i]) <<
//                           " k=" << ks[i]);
//
//    } 
//        
//}


test_suite* MarketModelTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Market-model tests");
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testVeryLongJumpForwards));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testLongJumpForwards));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testVeryLongJumpCaplets));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testLongJumpCaplets));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testLongJumpCoinitialSwaps));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testLongJumpCoterminalSwaps));
    //suite->add(BOOST_TEST_CASE(&MarketModelTest::testFitAbcdVolatility));   
    return suite;
}
