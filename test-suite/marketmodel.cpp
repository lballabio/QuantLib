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
#include <ql/MarketModels/Products/MultiStep/multistepcoterminalswaptions.hpp>
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
#include <ql/MarketModels/BrownianGenerators/mtbrowniangenerator.hpp>
#include <ql/MonteCarlo/genericlsregression.hpp>
#include <ql/ShortRateModels/LiborMarketModels/lmlinexpcorrmodel.hpp>
#include <ql/ShortRateModels/LiborMarketModels/lmextlinexpvolmodel.hpp>
#include <ql/schedule.hpp>
#include <ql/Calendars/nullcalendar.hpp>
#include <ql/DayCounters/simpledaycounter.hpp>
#include <ql/PricingEngines/blackformula.hpp>
#include <ql/Utilities/dataformatters.hpp>
#include <ql/Math/segmentintegral.hpp>
#include <ql/Math/convergencestatistics.hpp>
#include <ql/Math/functional.hpp>
#include <ql/Optimization/levenbergmarquardt.hpp>
#include <ql/Optimization/steepestdescent.hpp>
#include <sstream>

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
std::vector<Rate> todaysForwards, todaysCoterminalSwapRates;
std::vector<Real> coterminalAnnuity;
std::vector<Spread> displacements;
std::vector<DiscountFactor> todaysDiscounts;
std::vector<Volatility> volatilities, blackVols;
std::vector<Volatility> swaptionsVolatilities, swaptionsBlackVols;
Real a, b, c, d;
Real longTermCorrelation, beta;
Size measureOffset_;
unsigned long seed_;
Size paths_, trainingPaths_;
bool printReport_ = false;

void setup() {

    // Times
    calendar = NullCalendar();
    todaysDate = Settings::instance().evaluationDate();
    //startDate = todaysDate + 5*Years;
    endDate = todaysDate + 10*Years;
    Schedule dates(todaysDate, endDate, Period(Semiannual),
                   calendar, Following, Following, true, false);
    rateTimes = std::vector<Time>(dates.size()-1);
    paymentTimes = std::vector<Time>(rateTimes.size()-1);
    accruals = std::vector<Real>(rateTimes.size()-1);
    dayCounter = SimpleDayCounter();
    for (Size i=1; i<dates.size(); ++i)
        rateTimes[i-1] = dayCounter.yearFraction(todaysDate, dates[i]);
    std::copy(rateTimes.begin()+1, rateTimes.end(), paymentTimes.begin());
    for (Size i=1; i<rateTimes.size(); ++i)
        accruals[i-1] = rateTimes[i] - rateTimes[i-1];

    // Rates & displacements
    todaysForwards = std::vector<Rate>(paymentTimes.size());
    displacements = std::vector<Spread>(paymentTimes.size());
    for (Size i=0; i<todaysForwards.size(); ++i) {
        todaysForwards[i] = 0.03 + 0.0010*i;
        displacements[i] = 0.02 + 0.0005*i;
    }

    // Discounts
    todaysDiscounts = std::vector<DiscountFactor>(rateTimes.size());
    todaysDiscounts[0] = 0.95;
    for (Size i=1; i<rateTimes.size(); ++i)
        todaysDiscounts[i] = todaysDiscounts[i-1] /
            (1.0+todaysForwards[i-1]*accruals[i-1]);

    // Coterminal swap rates & annuities
    Size N = todaysForwards.size();
    todaysCoterminalSwapRates = std::vector<Rate>(N);
    coterminalAnnuity = std::vector<Real>(N);
    Real floatingLeg = 0.0;
    for (Size i=1; i<=N; ++i) {
        if (i==1) {
            coterminalAnnuity[N-1] = accruals[N-1]*todaysDiscounts[N];
        } else {
            coterminalAnnuity[N-i] = coterminalAnnuity[N-i+1] +
                                     accruals[N-i]*todaysDiscounts[N-i+1];
        }
        floatingLeg = todaysDiscounts[N-i]-todaysDiscounts[N];
        todaysCoterminalSwapRates[N-i] = floatingLeg/coterminalAnnuity[N-i];
    }

    // Cap/Floor Volatilities
    Volatility mktVols[] = {0.15541283,
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
        volatilities[i] = todaysForwards[i]*mktVols[i]/
                         (todaysForwards[i]+displacements[i]);
        blackVols[i]= mktVols[i];
    }

    // Cap/Floor Correlation
    longTermCorrelation = 0.5;
    beta = 0.2;
    measureOffset_ = 5;

    // Monte Carlo
    seed_ = 42;
    paths_ = 32767; // 2^15-1
    trainingPaths_ = 1023; // 2^10-1
}

const boost::shared_ptr<SequenceStatistics> simulate(
        const boost::shared_ptr<MarketModelEvolver>& evolver,
        const MarketModelMultiProduct& product)
{
    Size initialNumeraire = evolver->numeraires().front();
    Real initialNumeraireValue = todaysDiscounts[initialNumeraire];

    AccountingEngine engine(evolver, product, initialNumeraireValue);
    boost::shared_ptr<SequenceStatistics> stats(new
        SequenceStatistics(product.numberOfProducts()));
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
                                        MarketModelType marketModelType) {

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

std::vector<Size> makeMeasure(const MarketModelMultiProduct& product,
                              MeasureType measureType)
{
    std::vector<Size> result;
    EvolutionDescription evolution(product.evolution());
    switch (measureType) {
      case ProductSuggested:
        result = product.suggestedNumeraires();
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


void checkForwardsAndCaplets(const SequenceStatistics& stats,
                             const std::vector<Rate>& forwardStrikes,
                             const std::vector<Rate>& capletStrikes,
                             const std::string& config) {
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
        BOOST_MESSAGE(config);
        Size i;
        for (i=0; i<N; ++i) {
            BOOST_MESSAGE(io::ordinal(i+1) << " forward: "
                          << io::rate(results[i])
                          << " +- " << io::rate(errors[i])
                          << "; expected: " << io::rate(expectedForwards[i])
                          << "; discrepancy = "
                          << forwardStdDevs[i]
                          << " standard errors");
        }
        for (i=0; i<N; ++i) {
            BOOST_MESSAGE(
                    io::ordinal(i+1) << " caplet: "
                    << io::rate(results[i+N])
                    << " +- " << io::rate(errors[i+N])
                    << "; expected: " << io::rate(expectedCaplets[i])
                    << "; discrepancy = "
                    << (results[i+N]-expectedCaplets[i])/(errors[i+N] == 0.0 ?
                                                          1.0 : errors[i+N])
                    << " standard errors");
        }
        BOOST_ERROR("test failed");
    }
}


void checkCoinitialSwaps(const SequenceStatistics& stats,
                         const Real fixedRate,
                         const std::string& config) {
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
        BOOST_MESSAGE(config);
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
                          const Real fixedRate,
                          const std::string& config) {
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
        BOOST_MESSAGE(config);
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

void checkCoterminalSwaptions(const SequenceStatistics& stats,
                              const std::vector<Rate>& strikes,
                              const std::string& config) {
    std::vector<Real> results = stats.mean();
    std::vector<Real> errors = stats.errorEstimate();
    Size N = todaysForwards.size();
    std::vector<Rate> expectedSwaptions(N);
    std::vector<Real> stdDevSwaptions(N);
    Real minError = QL_MAX_REAL;
    Real maxError = QL_MIN_REAL;
    for (Size i=1; i<=N; ++i) {
        Time expiry = rateTimes[N-i];
        expectedSwaptions[N-i] =
            blackFormula(Option::Call,
                         strikes[N-i]+displacements[N-i],
                         todaysCoterminalSwapRates[N-i]+displacements[N-i],
                         swaptionsVolatilities[N-i]*std::sqrt(expiry)) *
            coterminalAnnuity[N-i];
        stdDevSwaptions[N-i] = (results[N-i]-expectedSwaptions[N-i])/errors[N-i];
        if (stdDevSwaptions[N-i]>maxError) maxError = stdDevSwaptions[N-i];
        else if (stdDevSwaptions[i]<minError) minError = stdDevSwaptions[N-i];
    }
    Real errorThreshold = 2.50;
    if (minError > 0.0 || maxError < 0.0 ||
        minError < -errorThreshold || maxError > errorThreshold) {
        BOOST_MESSAGE(config);
        for (Size i=1; i<=N; ++i) {
            BOOST_MESSAGE(
                    io::ordinal(i) << " Swaption: "
                    << io::rate(results[N-i])
                    << " +- " << io::rate(errors[N-i])
                    << "; expected: " << io::rate(expectedSwaptions[N-i])
                    << "; discrepancy = "
                    << (results[N-i]-expectedSwaptions[N-i])/(errors[N-i] == 0.0 ?
                                                          1.0 : errors[N-i])
                    << " standard errors");
        }
        BOOST_ERROR("test failed");
    }
}

void checkCallableSwap(const SequenceStatistics& stats,
                       const std::string& config) {
    Real payerNPV    = stats.mean()[0];
    Real receiverNPV = stats.mean()[1];
    Real bermudanNPV = stats.mean()[2];
    Real callableNPV = stats.mean()[3];
    Real tolerance = 1.0e-15;
    Real swapError = std::fabs(receiverNPV+payerNPV);
    Real callableError = std::fabs(receiverNPV+bermudanNPV-callableNPV);

    if (swapError>tolerance || bermudanNPV<0.0 ||
        callableNPV<receiverNPV || callableError>tolerance)
        BOOST_MESSAGE(config);  // detailed error info below
    if (swapError>tolerance)
        BOOST_ERROR("agreement between payer and receiver swap failed:"
                    "\n    payer swap:    " << payerNPV <<
                    "\n    receiver swap: " << receiverNPV <<
                    "\n    error:         " << swapError <<
                    "\n    tolerance:     " << tolerance);
    if (bermudanNPV<0.0)
        BOOST_ERROR("negative bermudan option value:"
                    "\n    bermudan:          " << bermudanNPV);
    if (callableNPV<receiverNPV)
        BOOST_ERROR("callable receiver less valuable than plain receiver:"
                    "\n    receiver swap:     " << receiverNPV <<
                    "\n    callable:          " << callableNPV);
    if (callableError>tolerance)
        BOOST_ERROR("agreement between receiver+bermudan and callable failed:"
                    "\n    receiver swap:     " << receiverNPV <<
                    "\n    bermudan:          " << bermudanNPV <<
                    "\n    receiver+bermudan: " << receiverNPV+bermudanNPV <<
                    "\n    callable:          " << callableNPV <<
                    "\n    error:             " << callableError <<
                    "\n    tolerance:         " << tolerance);
    if (printReport_) {
        BOOST_MESSAGE("    payer swap:        " << payerNPV <<
                    "\n    receiver swap:     " << receiverNPV <<
                    "\n    bermudan:          " << bermudanNPV <<
                    "\n    receiver+bermudan: " << receiverNPV+bermudanNPV <<
                    "\n    callable:          " << callableNPV);
    }
}

QL_END_TEST_LOCALS(MarketModelTest)


void MarketModelTest::testOneStepForwardsAndCaplets() {

    BOOST_MESSAGE("Repricing one-step forwards and caplets "
                  "in a LIBOR market model...");

    QL_TEST_SETUP

    std::vector<Rate> forwardStrikes(todaysForwards.size());
    for (Size i=0; i<todaysForwards.size(); ++i)
        forwardStrikes[i] = todaysForwards[i] + 0.01;
    std::vector<Rate> capletStrikes = todaysForwards;

    OneStepForwards forwards(rateTimes, accruals,
                             paymentTimes, forwardStrikes);
    OneStepCaplets caplets(rateTimes, accruals,
                           paymentTimes, capletStrikes);

    MultiProductComposite product;
    product.add(forwards);
    product.add(caplets);
    product.finalize();

    EvolutionDescription evolution = product.evolution();

    MeasureType measures[] = { Terminal,
                               MoneyMarket };
    for (Size k=0; k<LENGTH(measures); k++) {
        std::vector<Size> numeraires = makeMeasure(product, measures[k]);

        for (Size m=0; m<1; ++m) { // one step always full factors
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
                    Size stop =
                        isInTerminalMeasure(evolution, numeraires) ? 0 : 1;
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        evolver = makeMarketModelEvolver(marketModel,
                                                         numeraires,
                                                         generatorFactory,
                                                         evolvers[i]);
                        std::ostringstream config;
                        config << measureTypeToString(measures[k]) << ", "
                               << factors
                               << (factors>1 ? " factors, " : " factor,")
                               << marketModelTypeToString(marketModels[j])
                               << ", MT BGF" << ", "
                               << evolverTypeToString(evolvers[i]);
                        if (printReport_) BOOST_MESSAGE("    " << config.str());
                        boost::shared_ptr<SequenceStatistics> stats =
                            simulate(evolver, product);
                        checkForwardsAndCaplets(*stats,
                                                forwardStrikes,
                                                capletStrikes,
                                                config.str());
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

    MultiStepForwards forwards(rateTimes, accruals,
                               paymentTimes, forwardStrikes);
    MultiStepCaplets caplets(rateTimes, accruals,
                             paymentTimes, capletStrikes);

    MultiProductComposite product;
    product.add(forwards);
    product.add(caplets);
    product.finalize();

    EvolutionDescription evolution = product.evolution();

    MeasureType measures[] = { MoneyMarketPlus,
                               MoneyMarket,
                               Terminal };
    for (Size k=0; k<LENGTH(measures); k++) {
        std::vector<Size> numeraires = makeMeasure(product, measures[k]);

        //for (Size m=0; m<todaysForwards.size(); m+=4) {
        for (Size m=0; m<1; ++m) {
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
                    Size stop =
                        isInTerminalMeasure(evolution, numeraires) ? 0 : 1;
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        evolver = makeMarketModelEvolver(marketModel,
                                                         numeraires,
                                                         generatorFactory,
                                                         evolvers[i]);
                        std::ostringstream config;
                        config << measureTypeToString(measures[k]) << ", "
                               << factors
                               << (factors>1 ? " factors, " : " factor,")
                               << marketModelTypeToString(marketModels[j])
                               << ", MT BGF" << ", "
                               << evolverTypeToString(evolvers[i]);
                        if (printReport_) BOOST_MESSAGE("    " << config.str());
                        boost::shared_ptr<SequenceStatistics> stats =
                            simulate(evolver, product);
                        checkForwardsAndCaplets(*stats,
                                                forwardStrikes,
                                                capletStrikes,
                                                config.str());
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

    MultiStepCoinitialSwaps product(rateTimes, accruals, accruals,
                                    paymentTimes, fixedRate);
    EvolutionDescription evolution = product.evolution();

    MeasureType measures[] = { ProductSuggested,
                               MoneyMarketPlus,
                               MoneyMarket,
                               Terminal };
    for (Size k=0; k<LENGTH(measures); k++) {
        std::vector<Size> numeraires = makeMeasure(product, measures[k]);

        //for (Size m=0; m<todaysForwards.size(); m+=4) {
        for (Size m=0; m<1; ++m) {
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
                    Size stop =
                        isInTerminalMeasure(evolution, numeraires) ? 0 : 1;
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        evolver = makeMarketModelEvolver(marketModel,
                                                         numeraires,
                                                         generatorFactory,
                                                         evolvers[i]);
                        std::ostringstream config;
                        config << measureTypeToString(measures[k]) << ", "
                               << factors
                               << (factors>1 ? " factors, " : " factor,")
                               << marketModelTypeToString(marketModels[j])
                               << ", MT BGF" << ", "
                               << evolverTypeToString(evolvers[i]);
                        if (printReport_) BOOST_MESSAGE("    " << config.str());
                        boost::shared_ptr<SequenceStatistics> stats =
                            simulate(evolver, product);
                        checkCoinitialSwaps(*stats, fixedRate, config.str());
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

    MultiStepCoterminalSwaps product(rateTimes, accruals, accruals,
                                     paymentTimes, fixedRate);
    EvolutionDescription evolution = product.evolution();

    MeasureType measures[] = { ProductSuggested,
                               MoneyMarketPlus,
                               MoneyMarket,
                               Terminal };
    for (Size k=0; k<LENGTH(measures); k++) {
        std::vector<Size> numeraires = makeMeasure(product, measures[k]);

        //for (Size m=0; m<todaysForwards.size(); m+=4) {
        for (Size m=0; m<1; ++m) {
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
                    Size stop =
                        isInTerminalMeasure(evolution, numeraires) ? 0 : 1;
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        evolver = makeMarketModelEvolver(marketModel,
                                                         numeraires,
                                                         generatorFactory,
                                                         evolvers[i]);
                        std::ostringstream config;
                        config << measureTypeToString(measures[k]) << ", "
                               << factors
                               << (factors>1 ? " factors, " : " factor,")
                               << marketModelTypeToString(marketModels[j])
                               << ", MT BGF" << ", "
                               << evolverTypeToString(evolvers[i]);
                        if (printReport_) BOOST_MESSAGE("    " << config.str());
                        boost::shared_ptr<SequenceStatistics> stats =
                            simulate(evolver, product);
                        checkCoterminalSwaps(*stats, fixedRate, config.str());
                    }
                }
            }
        }
    }
}

void MarketModelTest::testMultiStepCoterminalSwaptions() {

    BOOST_MESSAGE("Repricing multi-step coterminal swaptions "
                  "in a LIBOR market model...");

    QL_TEST_SETUP

    std::vector<Rate> swaptionsStrikes = todaysForwards;
    MultiStepCoterminalSwaptions swaptions(rateTimes, accruals,
                                           paymentTimes, swaptionsStrikes);
    MultiProductComposite product;
    product.add(swaptions);
    product.finalize();
    EvolutionDescription evolution = product.evolution();
    MeasureType measures[] = {ProductSuggested,MoneyMarketPlus,MoneyMarket,
                              Terminal};
    MarketModelType marketModels[] = {ExponentialCorrelationFlatVolatility,
                                      ExponentialCorrelationAbcdVolatility };
    EvolverType evolvers[] = {Pc, Ipc};
    for (Size k=0; k<LENGTH(measures); k++) {
        std::vector<Size> numeraires = makeMeasure(product, measures[k]);
        for (Size m=0; m<1; ++m) {
            Size factors = (m==0 ? todaysForwards.size() : m);
            for (Size j=0; j<LENGTH(marketModels); j++) {
                boost::shared_ptr<MarketModel> marketModel =
                    makeMarketModel(evolution, factors, marketModels[j]);
                for (Size n=0; n<1; n++) {
                    MTBrownianGeneratorFactory generatorFactory(seed_);
                    boost::shared_ptr<MarketModelEvolver> evolver;
                    Size stop =
                        isInTerminalMeasure(evolution, numeraires) ? 0 : 1;
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        evolver = makeMarketModelEvolver(marketModel,
                                                         numeraires,
                                                         generatorFactory,
                                                         evolvers[i]);
                        std::ostringstream config;
                        config << measureTypeToString(measures[k]) << ", "
                               << factors
                               << (factors>1 ? " factors, " : " factor,")
                               << marketModelTypeToString(marketModels[j])
                               << ", MT BGF" << ", "
                               << evolverTypeToString(evolvers[i]);
                        if (printReport_) BOOST_MESSAGE("    " << config.str());
                        boost::shared_ptr<SequenceStatistics> stats =
                            simulate(evolver, product);
                        checkCoterminalSwaptions(*stats, swaptionsStrikes,
                                                 config.str());
                    }
                }
            }
        }
    }
}

void MarketModelTest::testCallableSwap1() {

    BOOST_MESSAGE("Pricing callable swap with naive exercise strategy "
                  "in a LIBOR market model...");

    QL_TEST_SETUP

    Real fixedRate = 0.04;

    // 0. a payer swap
    MultiStepSwap payerSwap(rateTimes, accruals, accruals, paymentTimes,
                            fixedRate, true);

    // 1. the equivalent receiver swap
    MultiStepSwap receiverSwap(rateTimes, accruals, accruals, paymentTimes,
                               fixedRate, false);

    //exercise schedule
    std::vector<Rate> exerciseTimes(rateTimes);
    exerciseTimes.pop_back();
    //std::vector<Rate> exerciseTimes;
    //for (Size i=2; i<rateTimes.size()-1; i+=2)
    //    exerciseTimes.push_back(rateTimes[i]);

    // naif exercise strategy
    std::vector<Rate> swapTriggers(exerciseTimes.size(), fixedRate);
    SwapRateTrigger exerciseStrategy(rateTimes, swapTriggers, exerciseTimes);

    // 2. bermudan swaption to enter into the payer swap
    MultiStepNothing nothing(payerSwap.evolution());
    CallSpecifiedMultiProduct bermudanProduct(nothing, exerciseStrategy,
                                              payerSwap);

    // 3. callable receiver swap
    CallSpecifiedMultiProduct callableProduct(receiverSwap, exerciseStrategy);

    // evolve all 4 products togheter
    MultiProductComposite allProducts;
    allProducts.add(payerSwap);
    allProducts.add(receiverSwap);
    allProducts.add(bermudanProduct);
    allProducts.add(callableProduct);
    allProducts.finalize();

    EvolutionDescription evolution = callableProduct.evolution();

    MeasureType measures[] = { ProductSuggested,
                               MoneyMarketPlus,
                               MoneyMarket,
                               Terminal };
    for (Size k=0; k<LENGTH(measures); k++) {
        std::vector<Size> numeraires = makeMeasure(allProducts, measures[k]);

        //for (Size m=0; m<todaysForwards.size(); m+=4) {
        for (Size m=0; m<1; ++m) {
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
                    Size stop =
                        isInTerminalMeasure(evolution, numeraires) ? 0 : 1;
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        evolver = makeMarketModelEvolver(marketModel,
                                                         numeraires,
                                                         generatorFactory,
                                                         evolvers[i]);
                        std::ostringstream config;
                        config << measureTypeToString(measures[k]) << ", "
                               << factors
                               << (factors>1 ? " factors, " : " factor,")
                               << marketModelTypeToString(marketModels[j])
                               << ", MT BGF" << ", "
                               << evolverTypeToString(evolvers[i]);
                        if (printReport_) BOOST_MESSAGE("    " << config.str());
                        boost::shared_ptr<SequenceStatistics> stats =
                            simulate(evolver, allProducts);
                        checkCallableSwap(*stats, config.str());
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
    MultiStepSwap payerSwap(rateTimes, accruals, accruals, paymentTimes,
                            fixedRate, true);

    // 1. the equivalent receiver swap
    MultiStepSwap receiverSwap(rateTimes, accruals, accruals, paymentTimes,
                               fixedRate, false);

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
    NothingExerciseValue control(rateTimes);
    SwapBasisSystem basisSystem(rateTimes,exerciseTimes);
    NothingExerciseValue nullRebate(rateTimes);


    // 2. bermudan swaption to enter into the payer swap
    MultiStepNothing nothing(evolution);

    // 3. callable receiver swap
    ExerciseAdapter adaptedExercise(nullRebate);

    MeasureType measures[] = { ProductSuggested,
                               MoneyMarketPlus,
                               MoneyMarket,
                               Terminal };
    for (Size k=0; k<LENGTH(measures); k++) {
        std::vector<Size> numeraires = makeMeasure(receiverSwap, measures[k]);

        //for (Size m=0; m<todaysForwards.size(); m+=4) {
        for (Size m=0; m<1; ++m) {
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
                    Size stop =
                        isInTerminalMeasure(evolution, numeraires) ? 0 : 1;
                    for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                        evolver = makeMarketModelEvolver(marketModel,
                                                         numeraires,
                                                         generatorFactory,
                                                         evolvers[i]);

                        // calculate the exercise strategy
                        collectLongstaffSchwartzData(*evolver,
                            receiverSwap, basisSystem, nullRebate,
                            control, trainingPaths_, collectedData);
                        genericLongstaffSchwartzRegression(collectedData,
                            basisCoefficients);
                        LongstaffSchwartzExerciseStrategy exerciseStrategy(
                                               basisSystem, basisCoefficients,
                                               evolution, numeraires,
                                               nullRebate, control);

                        // 2. bermudan swaption to enter into the payer swap
                        CallSpecifiedMultiProduct bermudanProduct(
                                        nothing, exerciseStrategy, payerSwap);

                        // 3. callable receiver swap
                        CallSpecifiedMultiProduct callableProduct(
                             receiverSwap, exerciseStrategy, adaptedExercise);

                        // evolve all 4 products togheter
                        MultiProductComposite allProducts;
                        allProducts.add(payerSwap);
                        allProducts.add(receiverSwap);
                        allProducts.add(bermudanProduct);
                        allProducts.add(callableProduct);
                        allProducts.finalize();

                        std::ostringstream config;
                        config << measureTypeToString(measures[k]) << ", "
                               << factors
                               << (factors>1 ? " factors, " : " factor,")
                               << marketModelTypeToString(marketModels[j])
                               << ", MT BGF" << ", "
                               << evolverTypeToString(evolvers[i]);
                        if (printReport_) BOOST_MESSAGE("    " << config.str());
                        boost::shared_ptr<SequenceStatistics> stats =
                            simulate(evolver, allProducts);
                        checkCallableSwap(*stats, config.str());
                    }
                }
            }
        }
    }
}

//--------------------- Volatility tests ---------------------

void MarketModelTest::testAbcdVolatilityIntegration() {

    BOOST_MESSAGE("Testing Abcd-volatility integration...");

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

    // Parameters following Rebonato / Parameters following Brigo-Mercurio
    // used in Abcd class              used in LmExtLinearExponentialVolModel
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
                Real abcdCovariance =
                    abcd->covariance(0,T,rateTimes[i1],rateTimes[i2]);
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

    BOOST_MESSAGE("Testing Abcd-volatility fit...");

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
    Real tol = 3.0e-4;
    for (Size i=0; i<blackVols.size(); i++) {
        if (std::abs(k[i]-1.0)>tol) {
            Real modelVol =
                instVol.volatility(0.0, rateTimes[i], rateTimes[i]);
            BOOST_FAIL("\n   Fixing Time = " << rateTimes[i] <<
                       "\n   MktVol = " << io::rate(blackVols[i]) <<
                       "\n   ModVol = " << io::rate(modelVol) <<
                       "\n   k=" << k[i] << " error=" << std::abs(k[i]-1.0) <<
                       "\n   tol=" << tol);
        }
    }

}

//--------------------- Other tests ---------------------

void MarketModelTest::testDriftCalculator() {

    // Test full factor drift equivalence between compute() and
    // computeReduced()

    BOOST_MESSAGE("Testing drift calculation...");
    QL_TEST_SETUP

    Real tolerance = 1.0e-16;
    Size factors = todaysForwards.size();
    std::vector<Time> evolutionTimes(rateTimes.size()-1);
    std::copy(rateTimes.begin(), rateTimes.end()-1, evolutionTimes.begin());
    EvolutionDescription evolution(rateTimes,evolutionTimes);
    std::vector<Real> rateTaus = evolution.rateTaus();
    std::vector<Size> numeraires = moneyMarketPlusMeasure(evolution,
                                                          measureOffset_);
    std::vector<Size> alive = evolution.firstAliveRate();
    Size numberOfSteps = evolutionTimes.size();
    std::vector<Real> drifts(numberOfSteps), driftsReduced(numberOfSteps);
    MarketModelType marketModels[] = {ExponentialCorrelationFlatVolatility,
                                      ExponentialCorrelationAbcdVolatility};
    for (Size k=0; k<LENGTH(marketModels); ++k) {   // loop over market models
        boost::shared_ptr<MarketModel> marketModel =
            makeMarketModel(evolution, factors, marketModels[k]);
        std::vector<Rate> displacements = marketModel->displacements();
        for (Size j=0; j<numberOfSteps; ++j) {     // loop over steps
            const Matrix& A = marketModel->pseudoRoot(j);
            //BOOST_MESSAGE(io::ordinal(j) << " pseudoroot:\n" << A);
            Size inf = std::max(0,static_cast<Integer>(alive[j]));
            for (Size h=inf; h<numeraires.size(); ++h) {     // loop over numeraires
                DriftCalculator driftcalculator(A, displacements, rateTaus,
                                                numeraires[h], alive[j]);
                driftcalculator.computePlain(todaysForwards, drifts);
                driftcalculator.computeReduced(todaysForwards,
                                               driftsReduced);
                for (Size i=0; i<drifts.size(); ++i) {
                    Real error = std::abs(driftsReduced[i]-drifts[i]);
                    if (error>tolerance)
                            BOOST_ERROR("MarketModel: " <<
                                    marketModelTypeToString(marketModels[k])
                                    << ", " << io::ordinal(j) << " step, "
                                    << ", " << io::ordinal(h) << " numeraire, "
                                    << ", " << io::ordinal(i) << " drift, "
                                    << "\ndrift        =" << drifts[i]
                                    << "\ndriftReduced =" << driftsReduced[i]
                                    << "\n       error =" << error
                                    << "\n   tolerance =" << tolerance);
                }
            }
        }
    }
}

void MarketModelTest::testIsInSubset() {

    // Performance test for isInSubset function (temporary)

    BOOST_MESSAGE("Testing isInSubset ...");
    QL_TEST_SETUP

    Size dim = 100;
    std::vector<Time> set, subset;
    for (Size i=0; i<dim; i++) set.push_back(i*1.0);
    for (Size i=0; i<dim; i++) subset.push_back(dim+i*1.0);
    std::vector<bool> result = isInSubset(set, subset);
    // uncomment here to visually check results
    //for (Size i=0; i<dim; i++) {
    //    BOOST_MESSAGE("\n" << io::ordinal(i) << ":" <<
    //                " set[" << i << "] =  " << set[i] <<
    //                ", subset[" << i << "] =  " << subset[i] <<
    //                ", result[" << i << "] =  " << result[i]);
    //}
}

// --- Call the desired tests
test_suite* MarketModelTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Market-model tests");
    suite->add(BOOST_TEST_CASE(
        &MarketModelTest::testOneStepForwardsAndCaplets));
    suite->add(BOOST_TEST_CASE(
        &MarketModelTest::testMultiStepForwardsAndCaplets));
    suite->add(BOOST_TEST_CASE(
        &MarketModelTest::testMultiStepCoinitialSwaps));
    suite->add(BOOST_TEST_CASE(
        &MarketModelTest::testMultiStepCoterminalSwaps));
    //suite->add(BOOST_TEST_CASE(
    //    &MarketModelTest::testMultiStepCoterminalSwaptions));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testCallableSwap1));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testCallableSwap2));
    suite->add(BOOST_TEST_CASE(
        &MarketModelTest::testAbcdVolatilityIntegration));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testAbcdVolatilityCompare));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testAbcdVolatilityFit));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testDriftCalculator));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testIsInSubset));
    return suite;
}
