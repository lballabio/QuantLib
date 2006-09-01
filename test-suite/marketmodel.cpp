/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Cristina Duminuco
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
#include <ql/MarketModels/Products/marketmodelcomposite.hpp>
#include <ql/MarketModels/accountingengine.hpp>
#include <ql/MarketModels/Evolvers/forwardratepcevolver.hpp>
#include <ql/MarketModels/Evolvers/forwardrateipcevolver.hpp>
#include <ql/MarketModels/Models/expcorrflatvol.hpp>
#include <ql/MarketModels/Models/expcorrabcdvol.hpp>
#include <ql/MarketModels/Models/calibratedmarketmodel.hpp>
#include <ql/ShortRateModels/LiborMarketModels/lmlinexpcorrmodel.hpp>
#include <ql/ShortRateModels/LiborMarketModels/lmextlinexpvolmodel.hpp>
#include <ql/MarketModels/BrownianGenerators/mtbrowniangenerator.hpp>
#include <ql/schedule.hpp>
#include <ql/Calendars/nullcalendar.hpp>
#include <ql/DayCounters/simpledaycounter.hpp>
#include <ql/PricingEngines/blackmodel.hpp>
#include <ql/Utilities/dataformatters.hpp>
#include <ql/Math/segmentintegral.hpp>
#include <ql/Math/functional.hpp>
#include <ql/Optimization/levenbergmarquardt.hpp>
#include <ql/Optimization/steepestdescent.hpp>

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
    dayCounter = SimpleDayCounter();

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
    Volatility mktVols[] = {
                                0.15541283,
                                0.18719678,
                                0.20890740,
                                0.22318179,
                                0.23212717,
                                0.23731450,
                                0.23988649,
                                0.24066384,
                                0.24023111,
                                0.23900189,
                                0.23726699,
                                0.23522952,
                                0.23303022,
                                0.23076564,
                                0.22850101,
                                0.22627951,
                                0.22412881,
                                0.22206569,
                                0.22009939
    };
    a = -0.0597;
    b =  0.1677;
    c =  0.5403;
    d =  0.1710;

    volatilities = std::vector<Volatility>(todaysForwards.size());
    for (Size i=0; i<LENGTH(mktVols); i++) {
        volatilities[i]= mktVols[i];
    }

    measureOffset_ = 5;

    seed = 42;

    paths = 32767; // 2^15-1
}


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


void testForwardsAndCaplets(const SequenceStatistics& stats,
                            const std::vector<Rate>& forwardStrikes,
                            const std::vector<Rate>& capletStrikes) {
    std::vector<Real> results = stats.mean();
    std::vector<Real> errors = stats.errorEstimate();
    std::vector<Real> stdDevs(todaysForwards.size());

    Size N = todaysForwards.size();
    std::vector<Rate> expectedForwards(N), expectedCaplets(N);
    std::vector<Real> forwardStdDevs(N), capletStdDev(N);
    Real minError = QL_MAX_REAL;
    Real maxError = QL_MIN_REAL;
    for (Size i=0; i<N; ++i) {
        expectedForwards[i] = (todaysForwards[i]-forwardStrikes[i])
            *accruals[i]*todaysDiscounts[i+1];
        forwardStdDevs[i] = (results[i]-expectedForwards[i])/errors[i];
        if (forwardStdDevs[i]>maxError)
            maxError = forwardStdDevs[i];
        else if (forwardStdDevs[i]<minError)
            minError = forwardStdDevs[i];

        Time expiry = rateTimes[i];
        expectedCaplets[i] =
            detail::blackFormula(todaysForwards[i]+displacements[i],
                                 capletStrikes[i]+displacements[i],
                                 volatilities[i]*std::sqrt(expiry), 1)
            *accruals[i]*todaysDiscounts[i+1];
        capletStdDev[i] = (results[i+N]-expectedCaplets[i])/errors[i+N];
        if (capletStdDev[i]>maxError)
            maxError = capletStdDev[i];
        else if (capletStdDev[i]<minError)
            minError = capletStdDev[i];
    }

    Real errorThreshold = 2.35;
    if (minError > 0.0 || maxError < 0.0 ||
        minError <-errorThreshold || maxError > errorThreshold) {
        for (Size i=0; i<N; ++i) {
            BOOST_MESSAGE(io::ordinal(i+1) << " forward: "
                          << io::rate(results[i])
                          << " +- " << io::rate(errors[i])
                          << "; expected: " << io::rate(expectedForwards[i])
                          << "; discrepancy = "
                          << forwardStdDevs[i]
                          << " standard errors");
        }
        for (Size i=0; i<N; ++i) {
            BOOST_MESSAGE(io::ordinal(i+1) << " caplet: "
                          << io::rate(results[i+N])
                          << " +- " << io::rate(errors[i+N])
                          << "; expected: " << io::rate(expectedCaplets[i])
                          << "; discrepancy = "
                          << (results[i+N]-expectedCaplets[i])/(errors[i+N] == 0.0 ? 1.0 : errors[i+N])
                          << " standard errors");
        }
        BOOST_ERROR("test failed");
    }
}


void testCoinitialSwaps(const SequenceStatistics& stats,
                  const Real fixedRate)
{
    std::vector<Real> results = stats.mean();
    std::vector<Real> errors = stats.errorEstimate();
    std::vector<Real> stdDevs(todaysForwards.size());

    std::vector<Rate> expected(todaysForwards.size());
    Real minError = QL_MAX_REAL;
    Real maxError = QL_MIN_REAL;
    Real tmp = 0.;
    for (Size i=0; i<expected.size(); ++i) {
        tmp += (todaysForwards[i]-fixedRate)
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
                  const Real fixedRate)
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
        tmp += (todaysForwards[N-i]-fixedRate)
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
                      ExponentialCorrelationAbcdVolatility/*,
                      CalibratedMM*/
};

std::string marketModelTypeToString(PseudoRootType type) {
    switch (type) {
      case ExponentialCorrelationFlatVolatility:
          return "Exponential Correlation Flat Volatility";
      case ExponentialCorrelationAbcdVolatility:
          return "Exponential Correlation Abcd Volatility";
      //case CalibratedMM:
      //    return "CalibratedMarketModel";
      default:
        QL_FAIL("unknown MarketModelEvolver type");
    }
}

boost::shared_ptr<MarketModel> makePseudoRoot(
    const EvolutionDescription& evolution,
    const Size numberOfFactors,
    PseudoRootType marketModelType)
{
    Real longTermCorrelation = 0.5;
    Real beta = 0.2;
    std::vector<Time> fixingTimes(evolution.rateTimes());
    fixingTimes.pop_back();
    boost::shared_ptr<LmVolatilityModel> volModel(new
        LmExtLinearExponentialVolModel(fixingTimes,0.5, 0.6, 0.1, 0.1));
    boost::shared_ptr<LmCorrelationModel> corrModel(
          new LmLinearExponentialCorrelationModel(evolution.numberOfRates(),
                                                  longTermCorrelation, beta));
    switch (marketModelType) {
        case ExponentialCorrelationFlatVolatility:
            return boost::shared_ptr<MarketModel>(new
                ExpCorrFlatVol(longTermCorrelation, beta,
                               volatilities,
                               evolution,
                               numberOfFactors,
                               todaysForwards,
                               displacements));
        case ExponentialCorrelationAbcdVolatility:
            return boost::shared_ptr<MarketModel>(new
                ExpCorrAbcdVol(0.0,0.0,1.0,1.0,
                               volatilities,
                               longTermCorrelation, beta,
                               evolution,
                               numberOfFactors,
                               todaysForwards,
                               displacements));
        //case CalibratedMM:
        //    return boost::shared_ptr<MarketModel>(new
        //        CalibratedMarketModel(volModel, corrModel,
        //                              evolution,
        //                              numberOfFactors,
        //                              todaysForwards,
        //                              displacements));
        default:
            QL_FAIL("unknown MarketModel type");
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
    const boost::shared_ptr<MarketModel>& marketModel,
    const EvolutionDescription& evolution,
    const BrownianGeneratorFactory& generatorFactory,
    EvolverType evolverType)
{
    switch (evolverType) {
        case Ipc:
            return boost::shared_ptr<MarketModelEvolver>(
                              new ForwardRateIpcEvolver(marketModel, evolution,
                                                        generatorFactory));
        case Pc:
            return boost::shared_ptr<MarketModelEvolver>(
                               new ForwardRatePcEvolver(marketModel, evolution,
                                                        generatorFactory));
        default:
            QL_FAIL("unknown MarketModelEvolver type");
    }
}

QL_END_TEST_LOCALS(MarketModelTest)



void MarketModelTest::testLongJumpForwardsAndCaplets() {

    BOOST_MESSAGE("Repricing (long jump) forwards and caplets "
                  "in a LIBOR market model...");

    QL_TEST_SETUP

    std::vector<Rate> forwardStrikes(todaysForwards.size());
    for (Size i=0; i<todaysForwards.size(); ++i)
        forwardStrikes[i] = todaysForwards[i] + 0.01;
    std::vector<Rate> capletStrikes = todaysForwards;

    boost::shared_ptr<MarketModelProduct> forwards(
                       new MarketModelForwards(rateTimes, accruals,
                                               paymentTimes, forwardStrikes));

    boost::shared_ptr<MarketModelProduct> caplets(
                        new MarketModelCaplets(rateTimes, accruals,
                                               paymentTimes, capletStrikes));

    boost::shared_ptr<MarketModelComposite> product(new MarketModelComposite);
    product->add(forwards);
    product->add(caplets);
    product->finalize();

    EvolutionDescription evolution = product->suggestedEvolution();

    for (Size n=0; n<1; n++) {
        MTBrownianGeneratorFactory generatorFactory(seed);
        BOOST_MESSAGE("\n\t" << n << ". Random Sequence: MTBrownianGeneratorFactory");

        for (Size m=0; m<1; m++) {
            Size factors = (m==0 ? todaysForwards.size() : m);
            BOOST_MESSAGE("\n\t" << n << "." << m << "." << " Factors: " << factors);

            PseudoRootType marketModels[] = { //CalibratedMM,
                ExponentialCorrelationFlatVolatility,
                ExponentialCorrelationAbcdVolatility };
            for (Size k=0; k<LENGTH(marketModels); k++) {
                BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << " MarketModel: " << marketModelTypeToString(marketModels[k]));
                boost::shared_ptr<MarketModel> marketModel =
                    makePseudoRoot(evolution, factors, marketModels[k]);

                MeasureType measures[] = { ProductSuggested,
                                           Terminal,
                                           MoneyMarket,
                                           MoneyMarketPlus };
                for (Size j=0; j<LENGTH(measures); j++) {
                    BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << j << "." << " Measure: " << measureTypeToString(measures[j]));
                    setMeasure(evolution, measures[j]);

                    EvolverType evolvers[] = { Pc, Ipc };
                    boost::shared_ptr<MarketModelEvolver> evolver;
                    Size stop = evolution.isInTerminalMeasure() ? 0 : 1;
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << j << "." << i << "." << " Evolver: " << evolverTypeToString(evolvers[i]));
                        evolver = makeMarketModelEvolver(marketModel,
                                                         evolution,
                                                         generatorFactory,
                                                         evolvers[i]);
                        boost::shared_ptr<SequenceStatistics> stats =
                            simulate(evolver, product, evolution, paths);
                        testForwardsAndCaplets(*stats,
                                               forwardStrikes,
                                               capletStrikes);
                    }
                }
            }
        }
    }
}

void MarketModelTest::testVeryLongJumpForwardsAndCaplets() {

    BOOST_MESSAGE("Repricing (very long jump) forwards and caplets "
                  "in a LIBOR market model...");

    QL_TEST_SETUP

    std::vector<Rate> forwardStrikes(todaysForwards.size());
    for (Size i=0; i<todaysForwards.size(); ++i)
        forwardStrikes[i] = todaysForwards[i] + 0.01;
    std::vector<Rate> capletStrikes = todaysForwards;

    boost::shared_ptr<MarketModelProduct> forwards(
                new MarketModelForwardsOneStep(rateTimes, accruals,
                                               paymentTimes, forwardStrikes));
    boost::shared_ptr<MarketModelProduct> caplets(
                 new MarketModelCapletsOneStep(rateTimes, accruals,
                                               paymentTimes, capletStrikes));

    boost::shared_ptr<MarketModelComposite> product(new MarketModelComposite);
    product->add(forwards);
    product->add(caplets);
    product->finalize();



    EvolutionDescription evolution = product->suggestedEvolution();

    for (Size n=0; n<1; n++) {
        MTBrownianGeneratorFactory generatorFactory(seed);
        BOOST_MESSAGE("\n\t" << n << ". Random Sequence: MTBrownianGeneratorFactory");

        for (Size m=0; m<1; m++) {
            Size factors = (m==0 ? todaysForwards.size() : m);
            BOOST_MESSAGE("\n\t" << n << "." << m << "." << " Factors: " << factors);

            PseudoRootType marketModels[] = { //CalibratedMM,
                ExponentialCorrelationFlatVolatility,
                ExponentialCorrelationAbcdVolatility };
            for (Size k=0; k<LENGTH(marketModels); k++) {
                BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << " MarketModel: " << marketModelTypeToString(marketModels[k]));
                boost::shared_ptr<MarketModel> marketModel =
                    makePseudoRoot(evolution, factors, marketModels[k]);

                MeasureType measures[] = { Terminal,
                                           MoneyMarket,
                                           MoneyMarketPlus };
                for (Size j=0; j<LENGTH(measures); j++) {
                    BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << j << "." << " Measure: " << measureTypeToString(measures[j]));
                    setMeasure(evolution, measures[j]);

                    EvolverType evolvers[] = { Pc, Ipc };
                    boost::shared_ptr<MarketModelEvolver> evolver;
                    Size stop = evolution.isInTerminalMeasure() ? 0 : 1;
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << j << "." << i << "." << " Evolver: " << evolverTypeToString(evolvers[i]));
                        evolver = makeMarketModelEvolver(marketModel,
                                                         evolution,
                                                         generatorFactory,
                                                         evolvers[i]);
                        boost::shared_ptr<SequenceStatistics> stats =
                            simulate(evolver, product, evolution, paths);
                        testForwardsAndCaplets(*stats,
                                               forwardStrikes,
                                               capletStrikes);
                    }
                }
            }
        }
    }
}


void MarketModelTest::testLongJumpCoinitialSwaps() {

    BOOST_MESSAGE("Repricing (long jump) coinitial swaps in a LIBOR market model...");

    QL_TEST_SETUP

    Real fixedRate = 0.04;

    boost::shared_ptr<MarketModelProduct> product(new
        MarketModelCoinitialSwaps(rateTimes, accruals, accruals, paymentTimes, fixedRate));
    EvolutionDescription evolution = product->suggestedEvolution();

    for (Size n=0; n<1; n++) {
        MTBrownianGeneratorFactory generatorFactory(seed);
        BOOST_MESSAGE("\n\t" << n << ". Random Sequence: MTBrownianGeneratorFactory");

        for (Size m=0; m<1; m++) {
            Size factors = (m==0 ? todaysForwards.size() : m);
            BOOST_MESSAGE("\n\t" << n << "." << m << "." << " Factors: " << factors);

            PseudoRootType marketModels[] = { //CalibratedMM,
                ExponentialCorrelationFlatVolatility, ExponentialCorrelationAbcdVolatility };
            for (Size k=0; k<LENGTH(marketModels); k++) {
                BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << " MarketModel: " << marketModelTypeToString(marketModels[k]));
                boost::shared_ptr<MarketModel> marketModel = makePseudoRoot(evolution, factors, marketModels[k]);

                MeasureType measures[] = { ProductSuggested, Terminal, MoneyMarket, MoneyMarketPlus };
                for (Size j=0; j<LENGTH(measures); j++) {
                    BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << j << "." << " Measure: " << measureTypeToString(measures[j]));
                    setMeasure(evolution, measures[j]);

                    EvolverType evolvers[] = { Pc, Ipc };
                    boost::shared_ptr<MarketModelEvolver> evolver;
                    Size stop = evolution.isInTerminalMeasure() ? 0 : 1;
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << j << "." << i << "." << " Evolver: " << evolverTypeToString(evolvers[i]));
                        evolver = makeMarketModelEvolver(marketModel, evolution, generatorFactory, evolvers[i]);
                        boost::shared_ptr<SequenceStatistics> stats = simulate(evolver, product, evolution, paths);
                        testCoinitialSwaps(*stats, fixedRate);
                    }
                }
            }
        }
    }
}

void MarketModelTest::testLongJumpCoterminalSwaps() {

    BOOST_MESSAGE("Repricing (long jump) coterminal swaps in a LIBOR market model...");

    QL_TEST_SETUP

    Real fixedRate = 0.04;

    boost::shared_ptr<MarketModelProduct> product(new
        MarketModelCoterminalSwaps(rateTimes, accruals, accruals, paymentTimes, fixedRate));
    EvolutionDescription evolution = product->suggestedEvolution();

    for (Size n=0; n<1; n++) {
        MTBrownianGeneratorFactory generatorFactory(seed);
        BOOST_MESSAGE("\n\t" << n << ". Random Sequence: MTBrownianGeneratorFactory");

        for (Size m=0; m<1; m++) {
            Size factors = (m==0 ? todaysForwards.size() : m);
            BOOST_MESSAGE("\n\t" << n << "." << m << "." << " Factors: " << factors);

            PseudoRootType marketModels[] = { //CalibratedMM,
                ExponentialCorrelationFlatVolatility, ExponentialCorrelationAbcdVolatility };
            for (Size k=0; k<LENGTH(marketModels); k++) {
                BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << " MarketModel: " << marketModelTypeToString(marketModels[k]));
                boost::shared_ptr<MarketModel> marketModel = makePseudoRoot(evolution, factors, marketModels[k]);

                MeasureType measures[] = { ProductSuggested, Terminal, MoneyMarket, MoneyMarketPlus };
                for (Size j=0; j<LENGTH(measures); j++) {
                    BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << j << "." << " Measure: " << measureTypeToString(measures[j]));
                    setMeasure(evolution, measures[j]);

                    EvolverType evolvers[] = { Pc, Ipc };
                    boost::shared_ptr<MarketModelEvolver> evolver;
                    Size stop = evolution.isInTerminalMeasure() ? 0 : 1;
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        BOOST_MESSAGE("\n\t" << n << "." << m << "." << k << "." << j << "." << i << "." << " Evolver: " << evolverTypeToString(evolvers[i]));
                        evolver = makeMarketModelEvolver(marketModel, evolution, generatorFactory, evolvers[i]);
                        boost::shared_ptr<SequenceStatistics> stats = simulate(evolver, product, evolution, paths);
                        testCoterminalSwaps(*stats, fixedRate);
                    }
                }
            }
        }
    }
}

void MarketModelTest::testAbcdVolatilityIntegration() {

    BOOST_MESSAGE("Testing AbcdVolatilityIntegration ... ");

    QL_TEST_SETUP

    Real a = -0.0597;
    Real b =  0.1677;
    Real c =  0.5403;
    Real d =  0.1710;

    const Size N = 10;
    const Real precision = 1e-04;

    boost::shared_ptr<Abcd> instVol(new Abcd(a,b,c,d));
    SegmentIntegral SI(20000);
    for (Size i=0; i<N; i++) {
        Time T1 = 0.5*(1+i);     // expiry of forward 1: after T1 AbcdVol = 0
        for (Size k=0; k<N-i; k++) {
            Time T2 = 0.5*(1+k); // expiry of forward 2: after T2 AbcdVol = 0
            Time tMin = std::min(T1,T2);
            Time tMax = std::max(T1,T2);
            //Integration
            for(Size j=0; j<N; j++) {
                Real xMin = 0.5*j;
                for (Size l=0; l<N-j; l++) {
                    Real xMax = xMin + 0.5*l;
                    AbcdSquared abcd2(a,b,c,d,T1,T2);
                    Real numerical = SI(abcd2,xMin,xMax);
                    Real analytical = instVol->covariance(xMin,xMax,T1,T2);
                    if (std::abs(analytical-numerical)>precision) {
                        BOOST_MESSAGE("     T1=" << T1 << "," <<
                                   "T2=" << T2 << ",\t\t" <<
                                   "xMin=" << xMin << "," <<
                                   "xMax=" << xMax << ",\t\t" <<
                                   "analytical: " << analytical << ",\t" <<
                                   "numerical:   " << numerical);
                    }
                    if (T1==T2) {
                        Real variance = instVol->variance(xMin,xMax,T1);
                        if (std::abs(analytical-variance)>1e-14) {
                            BOOST_FAIL("     T1=" << T1 << "," <<
                                       "T2=" << T2 << ",\t\t" <<
                                       "xMin=" << xMin << "," <<
                                       "xMax=" << xMax << ",\t\t" <<
                                       "variance: " << variance << ",\t" <<
                                       "analytical: " << analytical);
                        }
                    }
                }
            }
        }
    }
}

void MarketModelTest::testAbcdVolatilityCompare() {

    BOOST_MESSAGE("Testing AbcdVolatility different implementations ...");

    QL_TEST_SETUP

    /*
        Given the instantaneous volatilities related to forward expiring at
        rateTimes[i1] and at rateTimes[i2], the methods:
        - LmExtLinearExponentialVolModel::integratedVariance(i1,i2,T)
        - Abcd::covariance(T)
        return the same result only if T < min(rateTimes[i1],rateTimes[i2]).
    */

    // Parameters following Rebonato  --> Parameters following Brigo-Mercurio
    // used in Abcd class                 used in LmExtLinearExponentialVolModel class
    Real a = 0.0597;              // --> d
    Real b = 0.1677;              // --> a
    Real c = 0.5403;              // --> b
    Real d = 0.1710;              // --> c


    Size i1; // index of forward 1
    Size i2; // index of forward 2

    boost::shared_ptr<LmVolatilityModel> lmAbcd(
                    new LmExtLinearExponentialVolModel(rateTimes,b,c,d,a));
    boost::shared_ptr<Abcd> abcd(new Abcd(a,b,c,d));
    for (i1=0; i1<rateTimes.size(); i1++ ) {
        for (i2=0; i2<rateTimes.size(); i2++ ) {
            Time T = 0.;
            do {
                Real lmCovariance = lmAbcd->integratedVariance(i1,i2,T);
                Real abcdCovariance = abcd->covariance(0,T,rateTimes[i1],rateTimes[i2]);
                if(std::abs(lmCovariance-abcdCovariance)>1e-10) {
                    BOOST_FAIL(" T1="   << rateTimes[i1] << ","     <<
                                  "T2="   << rateTimes[i2] << ",\t\t" <<
                                  "xMin=" << 0  << ","     <<
                                  "xMax=" << T  << ",\t\t" <<
                                  "abcd: " << abcdCovariance << ",\t" <<
                                  "lm: "   << lmCovariance);
                }
                T += 0.5;
            } while (T<std::min(rateTimes[i1],rateTimes[i2])) ;
        }
    }
}

void MarketModelTest::testAbcdVolatilityFit() {

    BOOST_MESSAGE("Testing AbcdVolatility fit ...");

    QL_TEST_SETUP

    Abcd instVol;
    Real a0 = instVol.a();
    Real b0 = instVol.b();
    Real c0 = instVol.c();
    Real d0 = instVol.d();
    Real error0 = instVol.error(volatilities, rateTimes.begin());

    instVol.calibrate(volatilities, rateTimes.begin());
    Real a1 = instVol.a();
    Real b1 = instVol.b();
    Real c1 = instVol.c();
    Real d1 = instVol.d();
    Real error1 = instVol.error(volatilities, rateTimes.begin());

    if (error1>=error0)
        BOOST_FAIL("Parameters:" <<
            "\na:     " << a0 << " ---> " << a1 <<
            "\nb:     " << b0 << " ---> " << b1 <<
            "\nc:     " << c0 << " ---> " << c1 <<
            "\nd:     " << d0 << " ---> " << d1 <<
            "\nerror: " << error0 << " ---> " << error1);

    std::vector<Real> k = instVol.k(volatilities, rateTimes.begin());
    Real tol = 2.0e-4;
    for (Size i=0; i<volatilities.size(); i++) {
        if (std::abs(k[i]-1.0)>tol) {
            Real modelVol = instVol.volatility(0.0, rateTimes[i], rateTimes[i]);
            BOOST_FAIL("\nFixing Time = " << rateTimes[i] <<
                       " MktVol = " << io::rate(volatilities[i]) <<
                       " ModVol = " << io::rate(modelVol) <<
                       " k=" << k[i] << " error=" << std::abs(k[i]-1.0) <<
                       " tol=" << tol);
        }
    }

}

test_suite* MarketModelTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Market-model tests");
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testAbcdVolatilityIntegration));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testAbcdVolatilityCompare));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testAbcdVolatilityFit));
    suite->add(
           BOOST_TEST_CASE(&MarketModelTest::testLongJumpForwardsAndCaplets));
    suite->add(
       BOOST_TEST_CASE(&MarketModelTest::testVeryLongJumpForwardsAndCaplets));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testLongJumpCoinitialSwaps));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testLongJumpCoterminalSwaps));
    return suite;
}
