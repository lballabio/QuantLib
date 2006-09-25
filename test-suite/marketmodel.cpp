/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Marco Bianchetti
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

#include <ql/MarketModels/Products/OneStep/onestepforwards.hpp>
#include <ql/MarketModels/Products/OneStep/onestepcaplets.hpp>

#include <ql/MarketModels/Products/MultiStep/multistepforwards.hpp>
#include <ql/MarketModels/Products/MultiStep/multistepcaplets.hpp>
#include <ql/MarketModels/Products/MultiStep/multistepcoinitialswaps.hpp>
#include <ql/MarketModels/Products/MultiStep/multistepcoterminalswaps.hpp>
#include <ql/MarketModels/Products/MultiStep/multistepswap.hpp>
#include <ql/MarketModels/Products/MultiStep/callspecifiedmultiproduct.hpp>
#include <ql/MarketModels/Products/MultiStep/multistepnothing.hpp>
#include <ql/MarketModels/Products/MultiStep/exerciseadapter.hpp>

#include <ql/MarketModels/Products/multiproductcomposite.hpp>

#include <ql/MarketModels/accountingengine.hpp>
#include <ql/MarketModels/swapbasissystem.hpp>
#include <ql/MarketModels/lsdatacollector.hpp>
#include <ql/MarketModels/marketmodeldiscounter.hpp>
#include <ql/MarketModels/utilities.hpp>
#include <ql/MarketModels/Evolvers/forwardratepcevolver.hpp>
#include <ql/MarketModels/Evolvers/forwardrateipcevolver.hpp>
#include <ql/MarketModels/ExerciseStrategies/swapratetrigger.hpp>
#include <ql/MarketModels/ExerciseStrategies/lsstrategy.hpp>
#include <ql/MarketModels/ExerciseValues/nothingexercisevalue.hpp>
#include <ql/MarketModels/Models/expcorrflatvol.hpp>
#include <ql/MarketModels/Models/expcorrabcdvol.hpp>
#include <ql/MarketModels/Models/calibratedmarketmodel.hpp>
#include <ql/MarketModels/BrownianGenerators/mtbrowniangenerator.hpp>
#include <ql/MonteCarlo/genericlsregression.hpp>
#include <ql/ShortRateModels/LiborMarketModels/lmlinexpcorrmodel.hpp>
#include <ql/ShortRateModels/LiborMarketModels/lmextlinexpvolmodel.hpp>
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

Date todaysDate, startDate, endDate;
std::vector<Time> rateTimes, paymentTimes;
std::vector<Real> accruals;
Calendar calendar;
DayCounter dayCounter;

std::vector<Rate> todaysForwards, displacements;
std::vector<DiscountFactor> todaysDiscounts;
std::vector<Volatility> volatilities, blackVols;
Real a, b, c, d;
Real longTermCorrelation, beta;
Size measureOffset_;
unsigned long seed_;

Size paths_, trainingPaths_;

bool printReport_ = false;

template <class T, class U>
boost::shared_ptr<T> clone(const boost::shared_ptr<U>& ptr) {
    boost::shared_ptr<T> p = boost::dynamic_pointer_cast<T>(ptr);
    return boost::shared_ptr<T>(new T(*p));
}

void setup() {

    // times
    calendar = NullCalendar();
    todaysDate = Settings::instance().evaluationDate();
    //startDate = todaysDate + 5*Years;
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
    blackVols = std::vector<Volatility>(todaysForwards.size());
    for (Size i=0; i<LENGTH(mktVols); i++) {
        volatilities[i]= todaysForwards[i]*mktVols[i]/
            (todaysForwards[i]+displacements[i]);
        blackVols[i]= mktVols[i];
    }

    longTermCorrelation = 0.5;
    beta = 0.2;

    measureOffset_ = 5;

    seed_ = 42;

    paths_ = 32767; // 2^15-1
    trainingPaths_ = 1023; // 2^10-1
}


const boost::shared_ptr<SequenceStatistics> simulate(
        const boost::shared_ptr<MarketModelEvolver>& evolver,
        const boost::shared_ptr<MarketModelMultiProduct>& product)
{
    Size initialNumeraire = evolver->numeraires().front();
    Real initialNumeraireValue = todaysDiscounts[initialNumeraire];

    AccountingEngine engine(evolver, product, initialNumeraireValue);
    boost::shared_ptr<SequenceStatistics> stats(new
        SequenceStatistics(product->numberOfProducts()));
    engine.multiplePathValues(*stats, paths_);
    return stats;
}


enum MarketModelType { ExponentialCorrelationFlatVolatility,
                       ExponentialCorrelationAbcdVolatility/*,
                       CalibratedMM*/
};

std::string marketModelTypeToString(MarketModelType type) {
    switch (type) {
      case ExponentialCorrelationFlatVolatility:
          return "Exp. Corr. Flat Vol.";
      case ExponentialCorrelationAbcdVolatility:
          return "Exp. Corr. Abcd Vol.";
      //case CalibratedMM:
      //    return "CalibratedMarketModel";
      default:
        QL_FAIL("unknown MarketModelEvolver type");
    }
}

boost::shared_ptr<MarketModel> makeMarketModel(
    const EvolutionDescription& evolution,
    Size numberOfFactors,
    MarketModelType marketModelType)
{
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
          return "ProductSuggested measure";
      case Terminal:
          return "Terminal measure";
      case MoneyMarket:
          return "Money Market measure";
      case MoneyMarketPlus:
          return "Money Market Plus measure";
      default:
        QL_FAIL("unknown measure type");
    }
}

std::vector<Size> makeMeasure(
                   const boost::shared_ptr<MarketModelMultiProduct>& product,
                   MeasureType measureType)
{
    std::vector<Size> result;
    EvolutionDescription evolution(product->evolution());
    switch (measureType) {
      case ProductSuggested:
        result = product->suggestedNumeraires();
        break;
      case Terminal:
        result = terminalMeasure(evolution);
        if (!isInTerminalMeasure(evolution, result)) {
            Array a(result.size());
            std::copy(result.begin(), result.end(), a.begin());
            BOOST_ERROR("\nfailure in verifying Terminal measure:\n" << a);
        }
        break;
      case MoneyMarket:
        result = moneyMarketMeasure(evolution);
        if (!isInMoneyMarketMeasure(evolution, result)) {
            Array a(result.size());
            std::copy(result.begin(), result.end(), a.begin());
            BOOST_ERROR("\nfailure in verifying MoneyMarket measure:\n" << a);
        }
        break;
      case MoneyMarketPlus:
        result = moneyMarketPlusMeasure(evolution, measureOffset_);
        if (!isInMoneyMarketPlusMeasure(evolution, result, measureOffset_)) {
            Array a(result.size());
            std::copy(result.begin(), result.end(), a.begin());
            BOOST_ERROR("\nfailure in verifying MoneyMarketPlus(" <<
                        measureOffset_ << ") measure:\n" << a);
        }
        break;
      default:
        QL_FAIL("unknown measure type");
    }
    checkCompatibility(evolution, result);
    if (printReport_) {
        Array num(result.size());
        std::copy(result.begin(), result.end(), num.begin());
        BOOST_MESSAGE("    " << num);
    }
    return result;
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
    const std::vector<Size>& numeraires,
    const BrownianGeneratorFactory& generatorFactory,
    EvolverType evolverType)
{
    switch (evolverType) {
        case Ipc:
            return boost::shared_ptr<MarketModelEvolver>(new
                ForwardRateIpcEvolver(marketModel, generatorFactory,
                                      numeraires));
        case Pc:
            return boost::shared_ptr<MarketModelEvolver>(new
                ForwardRatePcEvolver(marketModel, generatorFactory,
                                     numeraires));
        default:
            QL_FAIL("unknown MarketModelEvolver type");
    }
}


QL_END_TEST_LOCALS(MarketModelTest)

void checkForwardsAndCaplets(const SequenceStatistics& stats,
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
            blackFormula(Option::Call,
                         capletStrikes[i]+displacements[i],
                         todaysForwards[i]+displacements[i],
                         volatilities[i]*std::sqrt(expiry)) *
            accruals[i]*todaysDiscounts[i+1];
        capletStdDev[i] = (results[i+N]-expectedCaplets[i])/errors[i+N];
        if (capletStdDev[i]>maxError)
            maxError = capletStdDev[i];
        else if (capletStdDev[i]<minError)
            minError = capletStdDev[i];
    }

    Real errorThreshold = 2.50;
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


void checkCoinitialSwaps(const SequenceStatistics& stats,
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

void checkCoterminalSwaps(const SequenceStatistics& stats,
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

void checkCallableSwap(const SequenceStatistics& stats)
{
    Real payerNPV    = stats.mean()[0];
    Real receiverNPV = stats.mean()[1];
    Real bermudanNPV = stats.mean()[2];
    Real callableNPV = stats.mean()[3];
    Real tolerance = 6.0e-16;
    Real error = std::fabs(receiverNPV+payerNPV);
    if (error>tolerance)
        BOOST_ERROR("agreement between payer and receiver swap failed:"
                    "\n    payer swap:    " << payerNPV <<
                    "\n    receiver swap: " << receiverNPV <<
                    "\n    error:         " << error <<
                    "\n    tolerance:     " << tolerance);

    if (bermudanNPV<0.0)
        BOOST_ERROR("negative bermudan option value:"
                    "\n    bermudan:          " << bermudanNPV);

    if (callableNPV<receiverNPV)
        BOOST_ERROR("callable receiver less valuable than plain receiver:"
                    "\n    receiver swap:     " << receiverNPV <<
                    "\n    callable:          " << callableNPV);

    error = std::fabs(receiverNPV+bermudanNPV-callableNPV);
    if (error>tolerance)
        BOOST_ERROR("agreement between receiver+bermudan and callable failed:"
                    "\n    receiver swap:     " << receiverNPV <<
                    "\n    bermudan:          " << bermudanNPV <<
                    "\n    receiver+bermudan: " << receiverNPV+bermudanNPV <<
                    "\n    callable:          " << callableNPV <<
                    "\n    error:             " << error <<
                    "\n    tolerance:         " << tolerance);

    if (printReport_)
        BOOST_MESSAGE("    payer swap:        " << payerNPV <<
                    "\n    receiver swap:     " << receiverNPV <<
                    "\n    bermudan:          " << bermudanNPV <<
                    "\n    receiver+bermudan: " << receiverNPV+bermudanNPV <<
                    "\n    callable:          " << callableNPV);

}

void MarketModelTest::testOneStepForwardsAndCaplets() {

    BOOST_MESSAGE("Repricing one-step forwards and caplets "
                  "in a LIBOR market model...");

    QL_TEST_SETUP

    std::vector<Rate> forwardStrikes(todaysForwards.size());
    for (Size i=0; i<todaysForwards.size(); ++i)
        forwardStrikes[i] = todaysForwards[i] + 0.01;
    std::vector<Rate> capletStrikes = todaysForwards;

    boost::shared_ptr<MarketModelMultiProduct> forwards(new
        OneStepForwards(rateTimes, accruals, paymentTimes, forwardStrikes));
    boost::shared_ptr<MarketModelMultiProduct> caplets(new
        OneStepCaplets(rateTimes, accruals, paymentTimes, capletStrikes));

    boost::shared_ptr<MultiProductComposite> product(new
                                                    MultiProductComposite);
    product->add(forwards);
    product->add(caplets);
    product->finalize();

    EvolutionDescription evolution = product->evolution();

    // for one step MoneyMarketPlus==ProductSuggested
    // for one step Terminal==ProductSuggested
    MeasureType measures[] = { ProductSuggested,
                               MoneyMarket };
    for (Size k=0; k<LENGTH(measures); k++) {
        std::vector<Size> numeraires = makeMeasure(product, measures[k]);

        for (Size m=0; m<1; m++) { // one step always full factors
            Size factors = (m==0 ? todaysForwards.size() : m);

            MarketModelType marketModels[] = { //CalibratedMM,
                ExponentialCorrelationFlatVolatility,
                ExponentialCorrelationAbcdVolatility };
            for (Size j=0; j<LENGTH(marketModels); j++) {
                boost::shared_ptr<MarketModel> marketModel =
                    makeMarketModel(evolution, factors, marketModels[j]);

                for (Size n=0; n<1; n++) {
                    MTBrownianGeneratorFactory generatorFactory(seed_);

                    EvolverType evolvers[] = { Pc, Ipc };
                    boost::shared_ptr<MarketModelEvolver> evolver;
                    Size stop = isInTerminalMeasure(evolution, numeraires) ? 0 : 1;
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        evolver = makeMarketModelEvolver(marketModel,
                                                         numeraires,
                                                         generatorFactory,
                                                         evolvers[i]);
                        BOOST_MESSAGE(
                          "    " << measureTypeToString(measures[k]) <<
                          ", " << factors <<
                          (factors>1 ? " factors, " : " factor,") <<
                          marketModelTypeToString(marketModels[j]) <<
                          ", MT BGF" <<
                          ", " << evolverTypeToString(evolvers[i]));
                        boost::shared_ptr<SequenceStatistics> stats =
                            simulate(evolver, product);
                        checkForwardsAndCaplets(*stats,
                                                forwardStrikes,
                                                capletStrikes);
                    }
                }
            }
        }
    }
}


void MarketModelTest::testMultiStepForwardsAndCaplets() {

    BOOST_MESSAGE("Repricing multi-step forwards and caplets "
                  "in a LIBOR market model...");

    QL_TEST_SETUP

    std::vector<Rate> forwardStrikes(todaysForwards.size());
    for (Size i=0; i<todaysForwards.size(); ++i)
        forwardStrikes[i] = todaysForwards[i] + 0.01;
    std::vector<Rate> capletStrikes = todaysForwards;

    boost::shared_ptr<MarketModelMultiProduct> forwards(new
        MultiStepForwards(rateTimes, accruals, paymentTimes,forwardStrikes));

    boost::shared_ptr<MarketModelMultiProduct> caplets(new
        MultiStepCaplets(rateTimes, accruals, paymentTimes, capletStrikes));

    boost::shared_ptr<MultiProductComposite> product(new MultiProductComposite);
    product->add(forwards);
    product->add(caplets);
    product->finalize();

    EvolutionDescription evolution = product->evolution();

    MeasureType measures[] = { ProductSuggested,
                               MoneyMarketPlus,
                               MoneyMarket,
                               Terminal };
    for (Size k=0; k<LENGTH(measures); k++) {
        std::vector<Size> numeraires = makeMeasure(product, measures[k]);

        //for (Size m=0; m<todaysForwards.size(); m++) {
        for (Size m=0; m<1; m++) {
            Size factors = (m==0 ? todaysForwards.size() : m);

            MarketModelType marketModels[] = { //CalibratedMM,
                ExponentialCorrelationFlatVolatility,
                ExponentialCorrelationAbcdVolatility };
            for (Size j=0; j<LENGTH(marketModels); j++) {
                boost::shared_ptr<MarketModel> marketModel =
                    makeMarketModel(evolution, factors, marketModels[j]);

                for (Size n=0; n<1; n++) {
                    MTBrownianGeneratorFactory generatorFactory(seed_);

                    EvolverType evolvers[] = { Pc, Ipc };
                    boost::shared_ptr<MarketModelEvolver> evolver;
                    Size stop = isInTerminalMeasure(evolution, numeraires) ? 0 : 1;
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        evolver = makeMarketModelEvolver(marketModel,
                                                         numeraires,
                                                         generatorFactory,
                                                         evolvers[i]);
                        BOOST_MESSAGE(
                          "    " << measureTypeToString(measures[k]) <<
                          ", " << factors <<
                          (factors>1 ? " factors, " : " factor,") <<
                          marketModelTypeToString(marketModels[j]) <<
                          ", MT BGF" <<
                          ", " << evolverTypeToString(evolvers[i]));
                        boost::shared_ptr<SequenceStatistics> stats =
                            simulate(evolver, product);
                        checkForwardsAndCaplets(*stats,
                                                forwardStrikes,
                                                capletStrikes);
                    }
                }
            }
        }
    }
}

void MarketModelTest::testMultiStepCoinitialSwaps() {

    BOOST_MESSAGE("Repricing multi-step coinitial swaps "
                  "in a LIBOR market model...");

    QL_TEST_SETUP

    Real fixedRate = 0.04;

    boost::shared_ptr<MarketModelMultiProduct> product(new
        MultiStepCoinitialSwaps(rateTimes, accruals, accruals,
                                paymentTimes, fixedRate));
    EvolutionDescription evolution = product->evolution();

    MeasureType measures[] = { ProductSuggested,
                                MoneyMarketPlus,
                                MoneyMarket,
                                Terminal };
    for (Size k=0; k<LENGTH(measures); k++) {
        std::vector<Size> numeraires = makeMeasure(product, measures[k]);

        //for (Size m=0; m<todaysForwards.size(); m++) {
        for (Size m=0; m<1; m++) {
            Size factors = (m==0 ? todaysForwards.size() : m);

            MarketModelType marketModels[] = { //CalibratedMM,
                ExponentialCorrelationFlatVolatility,
                ExponentialCorrelationAbcdVolatility };
            for (Size j=0; j<LENGTH(marketModels); j++) {
                boost::shared_ptr<MarketModel> marketModel =
                    makeMarketModel(evolution, factors, marketModels[j]);

                for (Size n=0; n<1; n++) {
                    MTBrownianGeneratorFactory generatorFactory(seed_);

                    EvolverType evolvers[] = { Pc, Ipc };
                    boost::shared_ptr<MarketModelEvolver> evolver;
                    Size stop = isInTerminalMeasure(evolution, numeraires) ? 0 : 1;
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        evolver = makeMarketModelEvolver(marketModel, numeraires, generatorFactory, evolvers[i]);
                        BOOST_MESSAGE("    " << measureTypeToString(measures[k]) <<
                                      ", " << factors <<
                                      (factors>1 ? " factors, " : " factor,") <<
                                      marketModelTypeToString(marketModels[j]) <<
                                      ", MT BGF" <<
                                      ", " << evolverTypeToString(evolvers[i]));
                        boost::shared_ptr<SequenceStatistics> stats = simulate(evolver, product);
                        checkCoinitialSwaps(*stats, fixedRate);
                    }
                }
            }
        }
    }
}

void MarketModelTest::testMultiStepCoterminalSwaps() {

    BOOST_MESSAGE("Repricing multi-step coterminal swaps "
                  "in a LIBOR market model...");

    QL_TEST_SETUP

    Real fixedRate = 0.04;

    boost::shared_ptr<MarketModelMultiProduct> product(new
        MultiStepCoterminalSwaps(rateTimes, accruals, accruals, paymentTimes, fixedRate));
    EvolutionDescription evolution = product->evolution();

    MeasureType measures[] = { ProductSuggested,
                               MoneyMarketPlus,
                               MoneyMarket,
                               Terminal };
    for (Size k=0; k<LENGTH(measures); k++) {
        std::vector<Size> numeraires = makeMeasure(product, measures[k]);

        //for (Size m=0; m<todaysForwards.size(); m++) {
        for (Size m=0; m<1; m++) {
            Size factors = (m==0 ? todaysForwards.size() : m);

            MarketModelType marketModels[] = { //CalibratedMM,
                ExponentialCorrelationFlatVolatility,
                ExponentialCorrelationAbcdVolatility };
            for (Size j=0; j<LENGTH(marketModels); j++) {
                boost::shared_ptr<MarketModel> marketModel =
                    makeMarketModel(evolution, factors, marketModels[j]);

                for (Size n=0; n<1; n++) {
                    MTBrownianGeneratorFactory generatorFactory(seed_);

                    EvolverType evolvers[] = { Pc, Ipc };
                    boost::shared_ptr<MarketModelEvolver> evolver;
                    Size stop = isInTerminalMeasure(evolution, numeraires) ? 0 : 1;
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        evolver = makeMarketModelEvolver(marketModel, numeraires, generatorFactory, evolvers[i]);
                        BOOST_MESSAGE("    " << measureTypeToString(measures[k]) <<
                                      ", " << factors <<
                                      (factors>1 ? " factors, " : " factor,") <<
                                      marketModelTypeToString(marketModels[j]) <<
                                      ", MT BGF" <<
                                      ", " << evolverTypeToString(evolvers[i]));
                        boost::shared_ptr<SequenceStatistics> stats = simulate(evolver, product);
                        checkCoterminalSwaps(*stats, fixedRate);
                    }
                }
            }
        }
    }
}

void MarketModelTest::testAbcdVolatilityIntegration() {

    BOOST_MESSAGE("Testing Abcd-volatility integration... ");

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

    BOOST_MESSAGE("Testing different implementations of Abcd-volatility...");

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
    Real a = 0.0597;                      // --> d
    Real b = 0.1677;                      // --> a
    Real c = 0.5403;                      // --> b
    Real d = 0.1710;                      // --> c

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

    BOOST_MESSAGE("Testing Abcd-volatility fit ...");

    QL_TEST_SETUP

    Abcd instVol;
    Real a0 = instVol.a();
    Real b0 = instVol.b();
    Real c0 = instVol.c();
    Real d0 = instVol.d();
    Real error0 = instVol.error(blackVols, rateTimes.begin());

    instVol.capletCalibration(blackVols, rateTimes.begin());
    Real a1 = instVol.a();
    Real b1 = instVol.b();
    Real c1 = instVol.c();
    Real d1 = instVol.d();
    Real error1 = instVol.error(blackVols, rateTimes.begin());

    if (error1>=error0)
        BOOST_FAIL("Parameters:" <<
            "\na:     " << a0 << " ---> " << a1 <<
            "\nb:     " << b0 << " ---> " << b1 <<
            "\nc:     " << c0 << " ---> " << c1 <<
            "\nd:     " << d0 << " ---> " << d1 <<
            "\nerror: " << error0 << " ---> " << error1);

    std::vector<Real> k = instVol.k(blackVols, rateTimes.begin());
    Real tol = 2.0e-4;
    for (Size i=0; i<blackVols.size(); i++) {
        if (std::abs(k[i]-1.0)>tol) {
            Real modelVol = instVol.volatility(0.0, rateTimes[i], rateTimes[i]);
            BOOST_FAIL("\nFixing Time = " << rateTimes[i] <<
                       " MktVol = " << io::rate(blackVols[i]) <<
                       " ModVol = " << io::rate(modelVol) <<
                       " k=" << k[i] << " error=" << std::abs(k[i]-1.0) <<
                       " tol=" << tol);
        }
    }

}

void MarketModelTest::testDriftCalculator() {

    BOOST_MESSAGE("Testing drift calculation ...");
    QL_TEST_SETUP

    // full factor equivalence between compute() and computeReduced()
    Size factors = todaysForwards.size();

    std::vector<Time> evolutionTimes(rateTimes.size()-1);
    std::copy(rateTimes.begin(), rateTimes.end()-1, evolutionTimes.begin());
    EvolutionDescription evolution(rateTimes,evolutionTimes);   // default numeraires

    std::vector<Real> rateTaus = evolution.rateTaus();
    std::vector<Size> numeraires = moneyMarketPlusMeasure(evolution, measureOffset_);
    std::vector<Size> alive = evolution.firstAliveRate();
    Size numberOfSteps = evolutionTimes.size();
    
    std::vector<Real> drifts(numberOfSteps), driftsReduced(numberOfSteps);
    
    MarketModelType marketModels[] = {ExponentialCorrelationFlatVolatility,
                                      ExponentialCorrelationAbcdVolatility};
    
    for (Size k=0; k<LENGTH(marketModels); ++k) {   // cycle over market models
        boost::shared_ptr<MarketModel> marketModel =
            makeMarketModel(evolution, factors, marketModels[k]);

        std::vector<Rate> displacements = marketModel->displacements();

        for (Size j=0; j<numberOfSteps; ++j) {              // cycle over steps
            const Matrix& A = marketModel->pseudoRoot(j);
            //BOOST_MESSAGE(io::ordinal(j) << " pseudoroot:\n" << A);
            // add cycle over numeraires
            DriftCalculator driftcalculator(A, displacements, rateTaus,
                                            numeraires[j], alive[j]);
            driftcalculator.compute(todaysForwards, drifts);
            driftcalculator.computeReduced(todaysForwards, factors, driftsReduced);
            for (Size i=0; i<drifts.size(); ++i) {     // write resulting drifts
                Real tolerance = 1.0e-16;
                Real error = std::abs(driftsReduced[i]-drifts[i]);
                if (error>tolerance)
                    BOOST_ERROR("MarketModel: " <<
                                marketModelTypeToString(marketModels[k])
                                << ", " << io::ordinal(j) << " step, "
                                << ", " << io::ordinal(i) << " drift, "
                                << "\ndrift        =" << drifts[i]
                                << "\ndriftReduced =" << driftsReduced[i] 
                                << "\n       error =" << error
                                << "\n   tolerance =" << tolerance);
            }
        }
    }
}

void MarketModelTest::testCallableSwap1() {

    BOOST_MESSAGE("Pricing callable swap with naif exercise strategy in a LIBOR market model...");

    QL_TEST_SETUP

    Real fixedRate = 0.04;

    // 0. a payer swap
    boost::shared_ptr<MarketModelMultiProduct> payerSwap(new
        MultiStepSwap(rateTimes, accruals, accruals, paymentTimes,
                      fixedRate, true));

    // 1. the equivalent receiver swap
    boost::shared_ptr<MarketModelMultiProduct> receiverSwap(new
        MultiStepSwap(rateTimes, accruals, accruals, paymentTimes,
                      fixedRate, false));

    //exercise schedule
    std::vector<Rate> exerciseTimes(rateTimes);
    exerciseTimes.pop_back();
    //std::vector<Rate> exerciseTimes;
    //for (Size i=2; i<rateTimes.size()-1; i+=2)
    //    exerciseTimes.push_back(rateTimes[i]);

    // naif exercise strategy
    std::vector<Rate> swapTriggers(exerciseTimes.size(), fixedRate);
    boost::shared_ptr<ExerciseStrategy<CurveState> > exerciseStrategy(new
        SwapRateTrigger(rateTimes, swapTriggers, exerciseTimes));

    // 2. bermudan swaption to enter into the payer swap
    boost::shared_ptr<MarketModelMultiProduct> nothing(new
        MultiStepNothing(payerSwap->evolution()));
    boost::shared_ptr<MarketModelMultiProduct> bermudanProduct(new
        CallSpecifiedMultiProduct(nothing,
                                  clone<SwapRateTrigger>(exerciseStrategy),
                                  clone<MultiStepSwap>(payerSwap)));

    // 3. callable receiver swap
    boost::shared_ptr<MarketModelMultiProduct> callableProduct(new
        CallSpecifiedMultiProduct(clone<MultiStepSwap>(receiverSwap),
                                  clone<SwapRateTrigger>(exerciseStrategy)));

    // evolve all 4 products togheter
    boost::shared_ptr<MultiProductComposite> allProducts(new
        MultiProductComposite);
    allProducts->add(payerSwap);
    allProducts->add(receiverSwap);
    allProducts->add(bermudanProduct);
    allProducts->add(callableProduct);
    allProducts->finalize();

    EvolutionDescription evolution = callableProduct->evolution();

    MeasureType measures[] = { ProductSuggested,
                               MoneyMarketPlus,
                               MoneyMarket,
                               Terminal };
    for (Size k=0; k<LENGTH(measures); k++) {
        std::vector<Size> numeraires = makeMeasure(allProducts, measures[k]);

        //for (Size m=0; m<todaysForwards.size(); m++) {
        for (Size m=0; m<1; m++) {
            Size factors = (m==0 ? todaysForwards.size() : m);

            MarketModelType marketModels[] = {
                // CalibratedMM,
                ExponentialCorrelationFlatVolatility,
                ExponentialCorrelationAbcdVolatility };
            for (Size j=0; j<LENGTH(marketModels); j++) {
                boost::shared_ptr<MarketModel> marketModel =
                    makeMarketModel(evolution, factors, marketModels[j]);

                for (Size n=0; n<1; n++) {
                    MTBrownianGeneratorFactory generatorFactory(seed_);

                    EvolverType evolvers[] = { Pc, Ipc };
                    boost::shared_ptr<MarketModelEvolver> evolver;
                    Size stop = isInTerminalMeasure(evolution, numeraires) ? 0 : 1;
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        evolver  = makeMarketModelEvolver(marketModel, numeraires, generatorFactory, evolvers[i]);
                        BOOST_MESSAGE("    " << measureTypeToString(measures[k]) <<
                                      ", " << factors <<
                                      (factors>1 ? " factors, " : " factor,") <<
                                      marketModelTypeToString(marketModels[j]) <<
                                      ", MT BGF" <<
                                      ", " << evolverTypeToString(evolvers[i]));
                        boost::shared_ptr<SequenceStatistics> stats = simulate(evolver, allProducts);
                        checkCallableSwap(*stats);
                    }
                }
            }
        }
    }
}


void MarketModelTest::testCallableSwap2() {

    BOOST_MESSAGE("Pricing callable swap with Longstaff-Schwartz exercise strategy in a LIBOR market model...");

    QL_TEST_SETUP

    Real fixedRate = 0.04;

    // 0. a payer swap
    boost::shared_ptr<MultiStepSwap> payerSwap(new
        MultiStepSwap(rateTimes, accruals, accruals, paymentTimes,
                      fixedRate, true));

    // 1. the equivalent receiver swap
    boost::shared_ptr<MultiStepSwap> receiverSwap(new
        MultiStepSwap(rateTimes, accruals, accruals, paymentTimes,
                      fixedRate, false));

    // exercise schedule
    std::vector<Rate> exerciseTimes(rateTimes);
    exerciseTimes.pop_back();
    //std::vector<Rate> exerciseTimes;
    //for (Size i=2; i<rateTimes.size()-1; i+=2)
    //    exerciseTimes.push_back(rateTimes[i]);

    // calculate all evolution times
    std::vector<std::vector<Time> > allTimes;
    allTimes.push_back(rateTimes);
    allTimes.back().pop_back();
    allTimes.push_back(exerciseTimes);
    std::vector<Time> evolutionTimes;
    std::vector<std::vector<bool> > dummy;
    mergeTimes(allTimes, evolutionTimes, dummy);

    EvolutionDescription evolution(rateTimes, evolutionTimes);

    // Longstaff-Schwartz exercise strategy
    std::vector<std::vector<LSNodeData> > collectedData;
    std::vector<std::vector<Real> > basisCoefficients;
    boost::shared_ptr<MarketModelExerciseValue> control(new
        NothingExerciseValue(rateTimes));
    boost::shared_ptr<MarketModelExerciseValue> control2(new
        NothingExerciseValue(rateTimes));
    boost::shared_ptr<MarketModelBasisSystem> basisSystem(new
        SwapBasisSystem(rateTimes,exerciseTimes));
    boost::shared_ptr<MarketModelBasisSystem> basisSystem2(new
        SwapBasisSystem(rateTimes,exerciseTimes));
    boost::shared_ptr<MarketModelExerciseValue> nullRebate(new
        NothingExerciseValue(rateTimes));
    boost::shared_ptr<MarketModelExerciseValue> nullRebate2(new
        NothingExerciseValue(rateTimes));


    // 2. bermudan swaption to enter into the payer swap
    boost::shared_ptr<MarketModelMultiProduct> nothing(new
        MultiStepNothing(evolution));

    // 3. callable receiver swap
    boost::shared_ptr<MarketModelExerciseValue> nullRebate3(new
        NothingExerciseValue(rateTimes));
    boost::shared_ptr<ExerciseAdapter> adaptedExercise(new
        ExerciseAdapter(nullRebate3));

    MeasureType measures[] = { ProductSuggested,
                               MoneyMarketPlus,
                               MoneyMarket,
                               Terminal };
    for (Size k=0; k<LENGTH(measures); k++) {
        std::vector<Size> numeraires = makeMeasure(receiverSwap, measures[k]);

        //for (Size m=0; m<todaysForwards.size(); m++) {
        for (Size m=0; m<1; m++) {
            Size factors = (m==0 ? todaysForwards.size() : m);

            MarketModelType marketModels[] = {
                // CalibratedMM,
                ExponentialCorrelationFlatVolatility,
                ExponentialCorrelationAbcdVolatility
            };
            for (Size j=0; j<LENGTH(marketModels); j++) {
                boost::shared_ptr<MarketModel> marketModel =
                    makeMarketModel(evolution, factors, marketModels[j]);

                for (Size n=0; n<1; n++) {
                    MTBrownianGeneratorFactory generatorFactory(seed_);

                    EvolverType evolvers[] = { Pc, Ipc };
                    boost::shared_ptr<MarketModelEvolver> evolver;
                    Size stop = isInTerminalMeasure(evolution, numeraires) ? 0 : 1;
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        evolver  = makeMarketModelEvolver(marketModel, numeraires, generatorFactory, evolvers[i]);

                        // calculate the exercise strategy
                        collectLongstaffSchwartzData(*evolver,
                            *receiverSwap, *basisSystem, *nullRebate,
                            *control, trainingPaths_, collectedData);
                        genericLongstaffSchwartzRegression(collectedData,
                            basisCoefficients);
                        boost::shared_ptr<LongstaffSchwartzExerciseStrategy>
                            exerciseStrategy(new
                                LongstaffSchwartzExerciseStrategy(
                                    basisSystem, basisCoefficients,
                                    evolution, numeraires,
                                    nullRebate, control));
                        boost::shared_ptr<LongstaffSchwartzExerciseStrategy>
                            exerciseStrategy2(new
                                LongstaffSchwartzExerciseStrategy(
                                    basisSystem2, basisCoefficients,
                                    evolution, numeraires,
                                    nullRebate2, control2));

                        // 2. bermudan swaption to enter into the payer swap
                        boost::shared_ptr<MarketModelMultiProduct> bermudanProduct(new
                            CallSpecifiedMultiProduct(nothing,
                                                      exerciseStrategy2,
                                                      clone<MultiStepSwap>(payerSwap)));

                        // 3. callable receiver swap
                        boost::shared_ptr<MarketModelMultiProduct> callableProduct(new
                            CallSpecifiedMultiProduct(
                                       clone<MultiStepSwap>(receiverSwap),
                                       exerciseStrategy,
                                       adaptedExercise));

                        // evolve all 4 products togheter
                        boost::shared_ptr<MultiProductComposite> allProducts(new
                            MultiProductComposite);
                        allProducts->add(payerSwap);
                        allProducts->add(receiverSwap);
                        allProducts->add(bermudanProduct);
                        allProducts->add(callableProduct);
                        allProducts->finalize();

                        BOOST_MESSAGE("    " << measureTypeToString(measures[k]) <<
                                    ", " << factors <<
                                    (factors>1 ? " factors, " : " factor,") <<
                                    marketModelTypeToString(marketModels[j]) <<
                                    ", MT BGF" <<
                                    ", " << evolverTypeToString(evolvers[i]));
                        boost::shared_ptr<SequenceStatistics> stats = simulate(evolver, allProducts);
                        checkCallableSwap(*stats);
                    }
                }
            }
        }
    }
}



test_suite* MarketModelTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Market-model tests");

    suite->add(BOOST_TEST_CASE(
                           &MarketModelTest::testAbcdVolatilityIntegration));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testAbcdVolatilityCompare));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testAbcdVolatilityFit));

    suite->add(BOOST_TEST_CASE(&MarketModelTest::testDriftCalculator));

    suite->add(BOOST_TEST_CASE(
                      &MarketModelTest::testOneStepForwardsAndCaplets));
    suite->add(BOOST_TEST_CASE(
                          &MarketModelTest::testMultiStepForwardsAndCaplets));
    suite->add(BOOST_TEST_CASE(
                              &MarketModelTest::testMultiStepCoinitialSwaps));
    suite->add(BOOST_TEST_CASE(
                              &MarketModelTest::testMultiStepCoterminalSwaps));

    suite->add(BOOST_TEST_CASE(&MarketModelTest::testCallableSwap1));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testCallableSwap2));
    return suite;
}

