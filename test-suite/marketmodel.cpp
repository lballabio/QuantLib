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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "marketmodel.hpp"
#include "utilities.hpp"
#include <ql/models/marketmodels/accountingengine.hpp>
#include <ql/models/marketmodels/browniangenerators/mtbrowniangenerator.hpp>
#include <ql/models/marketmodels/browniangenerators/sobolbrowniangenerator.hpp>
#include <ql/models/marketmodels/callability/collectnodedata.hpp>
#include <ql/models/marketmodels/callability/lsstrategy.hpp>
#include <ql/models/marketmodels/callability/nothingexercisevalue.hpp>
#include <ql/models/marketmodels/callability/parametricexerciseadapter.hpp>
#include <ql/models/marketmodels/callability/swapbasissystem.hpp>
#include <ql/models/marketmodels/callability/swapratetrigger.hpp>
#include <ql/models/marketmodels/callability/triggeredswapexercise.hpp>
#include <ql/models/marketmodels/callability/upperboundengine.hpp>
#include <ql/models/marketmodels/curvestates/lmmcurvestate.hpp>
#include <ql/models/marketmodels/driftcomputation/lmmdriftcalculator.hpp>
#include <ql/models/marketmodels/evolvers/lognormalfwdrateeuler.hpp>
#include <ql/models/marketmodels/evolvers/lognormalfwdrateeulerconstrained.hpp>
#include <ql/models/marketmodels/evolvers/lognormalfwdrateipc.hpp>
#include <ql/models/marketmodels/evolvers/lognormalfwdratepc.hpp>
#include <ql/models/marketmodels/evolvers/normalfwdratepc.hpp>
#include <ql/models/marketmodels/discounter.hpp>
#include <ql/models/marketmodels/models/abcdvol.hpp>
#include <ql/models/marketmodels/models/flatvol.hpp>
#include <ql/models/marketmodels/correlations/expcorrelations.hpp>
#include <ql/models/marketmodels/correlations/timehomogeneousforwardcorrelation.hpp>
#include <ql/models/marketmodels/products/multiproductcomposite.hpp>
#include <ql/models/marketmodels/products/multistep/callspecifiedmultiproduct.hpp>
#include <ql/models/marketmodels/products/multistep/exerciseadapter.hpp>
#include <ql/models/marketmodels/products/multistep/multistepcoinitialswaps.hpp>
#include <ql/models/marketmodels/products/multistep/multistepcoterminalswaps.hpp>
#include <ql/models/marketmodels/products/multistep/multistepcoterminalswaptions.hpp>
#include <ql/models/marketmodels/products/multistep/multistepforwards.hpp>
#include <ql/models/marketmodels/products/multistep/multistepnothing.hpp>
#include <ql/models/marketmodels/products/multistep/multistepoptionlets.hpp>
#include <ql/models/marketmodels/products/multistep/multistepswap.hpp>
#include <ql/models/marketmodels/products/onestep/onestepforwards.hpp>
#include <ql/models/marketmodels/products/onestep/onestepoptionlets.hpp>
#include <ql/models/marketmodels/proxygreekengine.hpp>
#include <ql/models/marketmodels/swapforwardmappings.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <ql/methods/montecarlo/genericlsregression.hpp>
#include <ql/legacy/libormarketmodels/lmlinexpcorrmodel.hpp>
#include <ql/legacy/libormarketmodels/lmextlinexpvolmodel.hpp>
#include <ql/time/schedule.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/daycounters/simpledaycounter.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/math/integrals/segmentintegral.hpp>
#include <ql/math/statistics/convergencestatistics.hpp>
#include <ql/termstructures/volatilities/abcd.hpp>
#include <ql/termstructures/volatilities/abcdcalibration.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/optimization/simplex.hpp>
#include <ql/quotes/simplequote.hpp>
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
Spread displacement;
std::vector<DiscountFactor> todaysDiscounts;
std::vector<Volatility> volatilities, blackVols, normalVols;
std::vector<Volatility> swaptionsVolatilities, swaptionsBlackVols;
Real a, b, c, d;
Real longTermCorrelation, beta;
Size measureOffset_;
unsigned long seed_;
Size paths_, trainingPaths_;
bool printReport_ = false;


// a simple structure to store some data which will be used during tests
struct SubProductExpectedValues {
    SubProductExpectedValues(const std::string& descr):
                                description(descr), testBias(false) {}
    std::string description;
    std::vector<Real> values;
    bool testBias;
    Real errorThreshold;
};

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

    // Rates & displacement
    todaysForwards = std::vector<Rate>(paymentTimes.size());
    displacement = 0.0;
    for (Size i=0; i<todaysForwards.size(); ++i)
        todaysForwards[i] = 0.03 + 0.0010*i;

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
    normalVols = std::vector<Volatility>(todaysForwards.size());
    for (Size i=0; i<LENGTH(mktVols); i++) {
        volatilities[i] = todaysForwards[i]*mktVols[i]/
                         (todaysForwards[i]+displacement);
        blackVols[i]= mktVols[i];
        normalVols[i]= mktVols[i]*todaysForwards[i];
    }

    // Swaption volatility quick fix
    swaptionsVolatilities = volatilities;

    // Cap/Floor Correlation
    longTermCorrelation = 0.5;
    beta = 0.2;
    measureOffset_ = 5;

    // Monte Carlo
    seed_ = 42;

#ifdef _DEBUG
    paths_ = 127;
    trainingPaths_ = 31;
#else
    paths_ = 32767; //262144-1; //; // 2^15-1
    trainingPaths_ = 8191; // 2^13-1
#endif
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
                                        bool logNormal,
                                        const EvolutionDescription& evolution,
                                        Size numberOfFactors,
                                        MarketModelType marketModelType,
                                        Spread forwardBump = 0.0,
                                        Volatility volBump = 0.0) {

    std::vector<Time> fixingTimes(evolution.rateTimes());
    fixingTimes.pop_back();
    boost::shared_ptr<LmVolatilityModel> volModel(new
        LmExtLinearExponentialVolModel(fixingTimes,0.5, 0.6, 0.1, 0.1));
    boost::shared_ptr<LmCorrelationModel> corrModel(
          new LmLinearExponentialCorrelationModel(evolution.numberOfRates(),
                                                  longTermCorrelation, beta));

    std::vector<Rate> bumpedForwards(todaysForwards.size());
    std::transform(todaysForwards.begin(), todaysForwards.end(),
                   bumpedForwards.begin(),
                   std::bind1st(std::plus<Rate>(), forwardBump));

    std::vector<Volatility> bumpedVols(volatilities.size());
    if (logNormal)
        std::transform(volatilities.begin(), volatilities.end(),
                       bumpedVols.begin(),
                       std::bind1st(std::plus<Rate>(), volBump));
    else
        std::transform(normalVols.begin(), normalVols.end(),
                       bumpedVols.begin(),
                       std::bind1st(std::plus<Rate>(), volBump));

    Matrix correlations = exponentialCorrelations(evolution.rateTimes(),
                                                  longTermCorrelation,
                                                  beta);
    boost::shared_ptr<PiecewiseConstantCorrelation> corr(new
        TimeHomogeneousForwardCorrelation(correlations,
                                          evolution.rateTimes()));
    switch (marketModelType) {
        case ExponentialCorrelationFlatVolatility:
            return boost::shared_ptr<MarketModel>(new
                FlatVol(bumpedVols,
                               corr,
                               evolution,
                               numberOfFactors,
                               bumpedForwards,
                               std::vector<Spread>(bumpedForwards.size(), displacement)));
        case ExponentialCorrelationAbcdVolatility:
            return boost::shared_ptr<MarketModel>(new
                AbcdVol(0.0,0.0,1.0,1.0,
                               bumpedVols,
                               corr,
                               evolution,
                               numberOfFactors,
                               bumpedForwards,
                               std::vector<Spread>(bumpedForwards.size(), displacement)));
        //case CalibratedMM:
        //    return boost::shared_ptr<MarketModel>(new
        //        CalibratedMarketModel(volModel, corrModel,
        //                              evolution,
        //                              numberOfFactors,
        //                              bumpedForwards,
        //                              displacement));
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
        BOOST_MESSAGE("    " << measureTypeToString(measureType) << ": " << num);
    }
    return result;
}

enum EvolverType { Ipc, Pc, NormalPc};

std::string evolverTypeToString(EvolverType type) {
    switch (type) {
      case Ipc:
          return "iterative predictor corrector";
      case Pc:
          return "predictor corrector";
      case NormalPc:
          return "predictor corrector for normal case";
      default:
        QL_FAIL("unknown MarketModelEvolver type");
    }
}
boost::shared_ptr<MarketModelEvolver> makeMarketModelEvolver(
    const boost::shared_ptr<MarketModel>& marketModel,
    const std::vector<Size>& numeraires,
    const BrownianGeneratorFactory& generatorFactory,
    EvolverType evolverType,
    Size initialStep = 0)
{
    switch (evolverType) {
        case Ipc:
            return boost::shared_ptr<MarketModelEvolver>(new
                LogNormalFwdRateIpc(marketModel, generatorFactory,
                                      numeraires, initialStep));
        case Pc:
            return boost::shared_ptr<MarketModelEvolver>(new
                LogNormalFwdRatePc(marketModel, generatorFactory,
                                     numeraires, initialStep));
        case NormalPc:
            return boost::shared_ptr<MarketModelEvolver>(new
                NormalFwdRatePc(marketModel, generatorFactory,
                                     numeraires, initialStep));
        default:
            QL_FAIL("unknown MarketModelEvolver type");
    }
}

void checkMultiProductCompositeResults (const SequenceStatistics& stats,
    const std::vector<SubProductExpectedValues>& subProductExpectedValues,
    const std::string& config) {

    std::vector<Real> results = stats.mean();
    std::vector<Real> errors = stats.errorEstimate();

    // size check
    Size nbOfResults = 0;
    for (Size i=0; i<subProductExpectedValues.size(); ++i) {
        for (Size j=0; j<subProductExpectedValues[i].values.size(); ++j)
            ++nbOfResults;
    }

    if (nbOfResults != results.size())
        BOOST_ERROR("mismatch between the size of the result and the \
                                               number of results");
    Size currentResultIndex = 0;

    std::vector<Real> stdDevs;
    std::vector<SubProductExpectedValues>::const_iterator subProductExpectedValue;
    for (subProductExpectedValue = subProductExpectedValues.begin();
         subProductExpectedValue != subProductExpectedValues.end();
         ++subProductExpectedValue) {
        Real minError = QL_MAX_REAL;
        Real maxError = QL_MIN_REAL;
        Real errorThreshold = subProductExpectedValue->errorThreshold;
        for (Size j=0; j<subProductExpectedValue->values.size(); ++j) {
            Real stdDev =
                (results[currentResultIndex]-subProductExpectedValue->values[j])
                /errors[currentResultIndex];
            stdDevs.push_back(stdDev);
            maxError = std::max(maxError, stdDev);
            minError = std::min(minError, stdDev);
            ++currentResultIndex;
        }
        bool isBiased = minError > 0.0 || maxError < 0.0;
        if (printReport_
            || subProductExpectedValue->testBias && isBiased
            || std::max(-minError, maxError) > errorThreshold) {
            BOOST_MESSAGE(config);
            currentResultIndex = 0;
            for (Size j=0; j<subProductExpectedValue->values.size(); ++j) {
                BOOST_MESSAGE(io::ordinal(j+1)
                    << " "  << subProductExpectedValue->description
                    << ": " << io::rate(results[currentResultIndex])
                    << "\t" << io::rate(subProductExpectedValue->values[j])
                    << "\t" << io::rate(errors[currentResultIndex])
                    << "; discrepancy = "
                    << stdDevs[currentResultIndex]
                << "\n");
                ++currentResultIndex;
            }
            BOOST_ERROR("test failed");
        }
    }


}


void checkForwardsAndOptionlets(const SequenceStatistics& stats,
                                const std::vector<Rate>& forwardStrikes,
                                const std::vector<boost::shared_ptr<StrikedTypePayoff> >& displacedPayoffs,
                                const std::string& config) {

    std::vector<Real> results = stats.mean();
    std::vector<Real> errors = stats.errorEstimate();
    std::vector<Real> stdDevs(todaysForwards.size());

    Size N = todaysForwards.size();
    std::vector<Rate> expectedForwards(N), expectedCaplets(N);
    std::vector<Real> forwardStdDevs(N), capletStdDev(N);
    Real minError = QL_MAX_REAL;
    Real maxError = QL_MIN_REAL;
    // forwards check
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
            BlackCalculator(displacedPayoffs[i],
                            todaysForwards[i]+displacement,
                            volatilities[i]*std::sqrt(expiry),
                            todaysDiscounts[i+1]*accruals[i]).value();
        capletStdDev[i] = (results[i+N]-expectedCaplets[i])/errors[i+N];
        if (capletStdDev[i]>maxError)
            maxError = capletStdDev[i];
        else if (capletStdDev[i]<minError)
            minError = capletStdDev[i];
    }

    Real errorThreshold = 2.50;
    if ( printReport_ || minError > 0.0 || maxError < 0.0 ||
        minError <-errorThreshold || maxError > errorThreshold) {
        BOOST_MESSAGE(config);
        Size i;
        for (i=0; i<N; ++i) {
            BOOST_MESSAGE(io::ordinal(i+1) << " forward: "
                          << io::rate(results[i])
                          << "\t" << io::rate(expectedForwards[i])
                          << "\t" << io::rate(errors[i])
                          << "; discrepancy = "
                          << forwardStdDevs[i]
                          << "\n");
        }
        for (i=0; i<N; ++i) {
            BOOST_MESSAGE(
                    io::ordinal(i+1) << "\t"
                    << io::rate(results[i+N])
                    << " +- " << io::rate(errors[i+N])
                    << "\t" << io::rate(expectedCaplets[i])
                    << "\t" << io::rate(errors[i+N])
                    << "; discrepancy = "
                    << (results[i+N]-expectedCaplets[i])/(errors[i+N] == 0.0 ?
                                                          1.0 : errors[i+N])
                    << "\n");
        }
        BOOST_ERROR("test failed");
    }
}




void checkNormalForwardsAndOptionlets(const SequenceStatistics& stats,
                                const std::vector<Rate>& forwardStrikes,
                                const std::vector<boost::shared_ptr<PlainVanillaPayoff> >& displacedPayoffs,
                                const std::string& config) {

    std::vector<Real> results = stats.mean();
    std::vector<Real> errors = stats.errorEstimate();
    std::vector<Real> stdDevs(todaysForwards.size());

    Size N = todaysForwards.size();
    std::vector<Rate> expectedForwards(N), expectedCaplets(N);
    std::vector<Real> forwardStdDevs(N), capletStdDev(N);
    Real minError = QL_MAX_REAL;
    Real maxError = QL_MIN_REAL;
    // forwards check
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
            bachelierBlackFormula(displacedPayoffs[i],
                                  todaysForwards[i]+displacement,
                                  normalVols[i]*std::sqrt(expiry),
                                  todaysDiscounts[i+1]*accruals[i]);
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



void checkCallableSwap(const SequenceStatistics& stats,
                       const std::string& config) {
    Real payerNPV    = stats.mean()[0];
    Real receiverNPV = stats.mean()[1];
    Real bermudanNPV = stats.mean()[2];
    Real callableNPV = stats.mean()[3];
    Real tolerance = 1.1e-15;
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
        BOOST_MESSAGE(std::setprecision(2) <<
                      "    payer swap:        " << io::rate(payerNPV) << " +/- " << io::rate(stats.errorEstimate()[0]) <<
                    "\n    receiver swap:     " << io::rate(receiverNPV) << " +/- " << io::rate(stats.errorEstimate()[1]) <<
                    "\n    bermudan:          " << io::rate(bermudanNPV) << " +/- " << io::rate(stats.errorEstimate()[2]) <<
                    "\n    receiver+bermudan: " << io::rate(receiverNPV+bermudanNPV) <<
                    "\n    callable:          " << io::rate(callableNPV) << " +/- " << io::rate(stats.errorEstimate()[3]));
    }
}

QL_END_TEST_LOCALS(MarketModelTest)


void MarketModelTest::testOneStepForwardsAndOptionlets() {

    BOOST_MESSAGE("Testing exact repricing of "
                  "one-step forwards and optionlets "
                  "in a lognormal forward rate market model...");

    setup();

    std::vector<Rate> forwardStrikes(todaysForwards.size());
    std::vector<boost::shared_ptr<Payoff> > optionletPayoffs(todaysForwards.size());
    std::vector<boost::shared_ptr<StrikedTypePayoff> >
        displacedPayoffs(todaysForwards.size());
    for (Size i=0; i<todaysForwards.size(); ++i) {
        forwardStrikes[i] = todaysForwards[i] + 0.01;
        optionletPayoffs[i] = boost::shared_ptr<Payoff>(new
            PlainVanillaPayoff(Option::Call, todaysForwards[i]));
        displacedPayoffs[i] = boost::shared_ptr<StrikedTypePayoff>(new
            PlainVanillaPayoff(Option::Call, todaysForwards[i]+displacement));
    }

    OneStepForwards forwards(rateTimes, accruals,
                             paymentTimes, forwardStrikes);
    OneStepOptionlets optionlets(rateTimes, accruals,
                                 paymentTimes, optionletPayoffs);

    MultiProductComposite product;
    product.add(forwards);
    product.add(optionlets);
    product.finalize();

    EvolutionDescription evolution = product.evolution();

    MarketModelType marketModels[] = {
        // CalibratedMM,
        ExponentialCorrelationFlatVolatility,
        ExponentialCorrelationAbcdVolatility };
    for (Size j=0; j<LENGTH(marketModels); j++) {

        // one step must be always full factors
        Size testedFactors[] = { todaysForwards.size()};
        for (Size m=0; m<LENGTH(testedFactors); ++m) {
            Size factors = testedFactors[m];

            // for one step product ProductSuggested is equal to Terminal
            // for one step product MoneyMarketPlus is equal to Terminal
            MeasureType measures[] = { MoneyMarket,
                                       Terminal };
            for (Size k=0; k<LENGTH(measures); k++) {
                std::vector<Size> numeraires = makeMeasure(product, measures[k]);

                bool logNormal = true;
                boost::shared_ptr<MarketModel> marketModel =
                    makeMarketModel(logNormal, evolution, factors, marketModels[j]);

                EvolverType evolvers[] = { Pc, Ipc };
                boost::shared_ptr<MarketModelEvolver> evolver;
                Size stop =
                    isInTerminalMeasure(evolution, numeraires) ? 0 : 1;
                for (Size i=0; i<LENGTH(evolvers)-stop; i++) {

                    for (Size n=0; n<1; n++) {
                        MTBrownianGeneratorFactory generatorFactory(seed_);
                        //SobolBrownianGeneratorFactory generatorFactory(
                        //    SobolBrownianGenerator::Diagonal, seed_);

                        evolver = makeMarketModelEvolver(marketModel,
                                                         numeraires,
                                                         generatorFactory,
                                                         evolvers[i]);
                        std::ostringstream config;
                        config <<
                            marketModelTypeToString(marketModels[j]) << ", " <<
                            factors << (factors>1 ? (factors==todaysForwards.size() ? " (full) factors, " : " factors, ") : " factor,") <<
                            measureTypeToString(measures[k]) << ", " <<
                            evolverTypeToString(evolvers[i]) << ", " <<
                            "MT BGF";
                        if (printReport_)
                            BOOST_MESSAGE("    " << config.str());

                        boost::shared_ptr<SequenceStatistics> stats =
                            simulate(evolver, product);
                        checkForwardsAndOptionlets(*stats,
                                                   forwardStrikes,
                                                   displacedPayoffs,
                                                   config.str());
                    }
                }
            }
        }
    }
}


void MarketModelTest::testOneStepNormalForwardsAndOptionlets() {

    BOOST_MESSAGE("Testing exact repricing of "
                  "one-step forwards and optionlets "
                  "in a normal forward rate market model...");

    setup();

    std::vector<Rate> forwardStrikes(todaysForwards.size());
    std::vector<boost::shared_ptr<Payoff> > optionletPayoffs(todaysForwards.size());
    std::vector<boost::shared_ptr<PlainVanillaPayoff> >
        displacedPayoffs(todaysForwards.size());
    for (Size i=0; i<todaysForwards.size(); ++i) {
        forwardStrikes[i] = todaysForwards[i] + 0.01;
        optionletPayoffs[i] = boost::shared_ptr<Payoff>(new
            PlainVanillaPayoff(Option::Call, todaysForwards[i]));
        displacedPayoffs[i] = boost::shared_ptr<PlainVanillaPayoff>(new
            PlainVanillaPayoff(Option::Call, todaysForwards[i]+displacement));
    }

    OneStepForwards forwards(rateTimes, accruals,
                             paymentTimes, forwardStrikes);
    OneStepOptionlets optionlets(rateTimes, accruals,
                                 paymentTimes, optionletPayoffs);

    MultiProductComposite product;
    product.add(forwards);
    product.add(optionlets);
    product.finalize();

    EvolutionDescription evolution = product.evolution();

    MarketModelType marketModels[] = {
        // CalibratedMM,
        ExponentialCorrelationFlatVolatility,
        ExponentialCorrelationAbcdVolatility };
    for (Size j=0; j<LENGTH(marketModels); j++) {

        // one step must be always full factors
        Size testedFactors[] = { todaysForwards.size()};
        for (Size m=0; m<LENGTH(testedFactors); ++m) {
            Size factors = testedFactors[m];

            // for one step product ProductSuggested is equal to Terminal
            // for one step product MoneyMarketPlus is equal to Terminal
            MeasureType measures[] = { MoneyMarket,
                                       Terminal };
            for (Size k=0; k<LENGTH(measures); k++) {
                std::vector<Size> numeraires = makeMeasure(product, measures[k]);

                bool logNormal = false;
                boost::shared_ptr<MarketModel> marketModel =
                    makeMarketModel(logNormal, evolution, factors, marketModels[j]);

                EvolverType evolvers[] = { NormalPc};
                boost::shared_ptr<MarketModelEvolver> evolver;
                Size stop =
                    isInTerminalMeasure(evolution, numeraires) ? 0 : 1;
                for (Size i=0; i<LENGTH(evolvers)-stop; i++) {

                    for (Size n=0; n<1; n++) {
                        MTBrownianGeneratorFactory generatorFactory(seed_);
                        //SobolBrownianGeneratorFactory generatorFactory(
                        //    SobolBrownianGenerator::Diagonal, seed_);

                        evolver = makeMarketModelEvolver(marketModel,
                                                         numeraires,
                                                         generatorFactory,
                                                         evolvers[i]);
                        std::ostringstream config;
                        config <<
                            marketModelTypeToString(marketModels[j]) << ", " <<
                            factors << (factors>1 ? (factors==todaysForwards.size() ? " (full) factors, " : " factors, ") : " factor,") <<
                            measureTypeToString(measures[k]) << ", " <<
                            evolverTypeToString(evolvers[i]) << ", " <<
                            "MT BGF";
                        if (printReport_)
                            BOOST_MESSAGE("    " << config.str());

                        boost::shared_ptr<SequenceStatistics> stats =
                            simulate(evolver, product);
                        checkNormalForwardsAndOptionlets(*stats,
                                                         forwardStrikes,
                                                         displacedPayoffs,
                                                         config.str());
                    }
                }
            }
        }
    }
}

void testMultiProductComposite(const MarketModelMultiProduct& product,
    const std::vector<SubProductExpectedValues>& subProductExpectedValues,
    const std::string& testDescription) {

    BOOST_MESSAGE("Testing exact repricing of "
                    << testDescription
                    << "in a lognormal forward rate market model...");

    setup();

    EvolutionDescription evolution = product.evolution();

    MarketModelType marketModels[] = {
        // CalibratedMM,
        ExponentialCorrelationFlatVolatility,
        ExponentialCorrelationAbcdVolatility };
    for (Size j=0; j<LENGTH(marketModels); j++) {

        Size testedFactors[] = { 4, 8,
                                 todaysForwards.size()};
        for (Size m=0; m<LENGTH(testedFactors); ++m) {
            Size factors = testedFactors[m];

            // Composite's ProductSuggested is the Terminal one
            MeasureType measures[] = { // ProductSuggested,
                                        Terminal,
                                        MoneyMarketPlus,
                                        MoneyMarket};
        for (Size k=0; k<LENGTH(measures); k++) {
                std::vector<Size> numeraires = makeMeasure(product, measures[k]);

                bool logNormal = true;
                boost::shared_ptr<MarketModel> marketModel =
                    makeMarketModel(logNormal, evolution, factors, marketModels[j]);


                EvolverType evolvers[] = { Pc, Ipc };
                boost::shared_ptr<MarketModelEvolver> evolver;
                Size stop =
                    isInTerminalMeasure(evolution, numeraires) ? 0 : 1;
                for (Size i=0; i<LENGTH(evolvers)-stop; i++) {

                    for (Size n=0; n<1; n++) {
                        //MTBrownianGeneratorFactory generatorFactory(seed_);
                        SobolBrownianGeneratorFactory generatorFactory(
                            SobolBrownianGenerator::Diagonal, seed_);

                        evolver = makeMarketModelEvolver(marketModel,
                                                         numeraires,
                                                         generatorFactory,
                                                         evolvers[i]);
                        std::ostringstream config;
                        config <<
                            marketModelTypeToString(marketModels[j]) << ", " <<
                            factors << (factors>1 ? (factors==todaysForwards.size() ? " (full) factors, " : " factors, ") : " factor,") <<
                            measureTypeToString(measures[k]) << ", " <<
                            evolverTypeToString(evolvers[i]) << ", " <<
                            "MT BGF";
                        if (printReport_)
                            BOOST_MESSAGE("    " << config.str());

                        boost::shared_ptr<SequenceStatistics> stats =
                            simulate(evolver, product);
                        checkMultiProductCompositeResults(*stats,
                                                      subProductExpectedValues,
                                                      config.str());
                    }
                }
            }
        }
    }
}

void addForwards(MultiProductComposite& product,
        std::vector<SubProductExpectedValues>& subProductExpectedValues) {

    // create forwards and add them to the product...
    std::vector<Rate> forwardStrikes(todaysForwards.size());

    for (Size i=0; i<todaysForwards.size(); ++i)
        forwardStrikes[i] = todaysForwards[i] + 0.01;

    MultiStepForwards forwards(rateTimes, accruals,
                               paymentTimes, forwardStrikes);
    product.add(forwards);

    // computing and storing expected values
    subProductExpectedValues.push_back(SubProductExpectedValues("Forward"));
    subProductExpectedValues.back().errorThreshold = 2.50;
    for (Size i=0; i<todaysForwards.size(); ++i) {
        subProductExpectedValues.back().values.push_back(
            (todaysForwards[i]-forwardStrikes[i])
            *accruals[i]*todaysDiscounts[i+1]);
    }
}

void addOptionLets(MultiProductComposite& product,
                  std::vector<SubProductExpectedValues>& subProductExpectedValues) {

    // create the products...
    std::vector<boost::shared_ptr<Payoff> > optionletPayoffs(todaysForwards.size());
    std::vector<boost::shared_ptr<StrikedTypePayoff> >
        displacedPayoffs(todaysForwards.size());

    for (Size i=0; i<todaysForwards.size(); ++i) {
        optionletPayoffs[i] = boost::shared_ptr<Payoff>(new
            PlainVanillaPayoff(Option::Call, todaysForwards[i]));
            //CashOrNothingPayoff(Option::Call, todaysForwards[i], 0.01));
        displacedPayoffs[i] = boost::shared_ptr<StrikedTypePayoff>(new
            PlainVanillaPayoff(Option::Call, todaysForwards[i]+displacement));
            //CashOrNothingPayoff(Option::Call, todaysForwards[i]+displacement, 0.01));
    }

    MultiStepOptionlets optionlets(rateTimes, accruals,
                                   paymentTimes, optionletPayoffs);
    product.add(optionlets);

    // computing and storing expected values
    subProductExpectedValues.push_back(SubProductExpectedValues("Caplet"));
    subProductExpectedValues.back().errorThreshold = 2.50;
    for (Size i=0; i<todaysForwards.size(); ++i) {
        subProductExpectedValues.back().values.push_back(
            BlackCalculator(displacedPayoffs[i],
                            todaysForwards[i]+displacement,
                            volatilities[i]*std::sqrt(rateTimes[i]),
                            todaysDiscounts[i+1]*accruals[i]).value());
    }
}


void addCoinitialSwaps(MultiProductComposite& product,
                   std::vector<SubProductExpectedValues>& subProductExpectedValues) {

    // create the products...
    Real fixedRate = 0.04;
    MultiStepCoinitialSwaps multiStepCoinitialSwaps(rateTimes, accruals, accruals,
                                    paymentTimes, fixedRate);
    product.add(multiStepCoinitialSwaps);
    // computing and storing expected values
    subProductExpectedValues.push_back(SubProductExpectedValues("coinitial swap"));
    subProductExpectedValues.back().testBias = false;
    subProductExpectedValues.back().errorThreshold = 2.32;
    Real coinitialSwapValue = 0;
    for (Size i=0; i<todaysForwards.size(); ++i) {
        coinitialSwapValue += (todaysForwards[i]-fixedRate)
            *accruals[i]*todaysDiscounts[i+1];
        subProductExpectedValues.back().values.push_back(coinitialSwapValue);
    }
}

void addCoterminalSwapsAndSwaptions(MultiProductComposite& product,
    std::vector<SubProductExpectedValues>& subProductExpectedValues) {
    Real fixedRate = 0.04;
    MultiStepCoterminalSwaps swaps(rateTimes, accruals, accruals,
                                     paymentTimes, fixedRate);

    std::vector<boost::shared_ptr<StrikedTypePayoff> > payoffs(todaysForwards.size());
    for (Size i = 0; i < payoffs.size(); ++i)
        payoffs[i] = boost::shared_ptr<StrikedTypePayoff>(new
            PlainVanillaPayoff(Option::Call, todaysForwards[i]));

    MultiStepCoterminalSwaptions swaptions(rateTimes,
                                           rateTimes, payoffs);
    product.add(swaps);
    product.add(swaptions);

    subProductExpectedValues.push_back(SubProductExpectedValues("coterminal swap"));
    subProductExpectedValues.back().testBias = false;
    subProductExpectedValues.back().errorThreshold = 2.32;
    LMMCurveState curveState(rateTimes);  // not the best way to detect errors in LMMCurveState...
    curveState.setOnForwardRates(todaysForwards);
    std::vector<Rate> atmRates = curveState.coterminalSwapRates();
    for (Size i=0; i<todaysForwards.size(); ++i) {
        Real expectedNPV = curveState.coterminalSwapAnnuity(0, i) * (atmRates[i]-fixedRate) *
            todaysDiscounts.front();
        subProductExpectedValues.back().values.push_back(expectedNPV);
    }
    // we clone the prooduct to be able to finalize it and call evolution function member on it
    MultiProductComposite productClone = product;
    productClone.finalize();
    subProductExpectedValues.push_back(SubProductExpectedValues("coterminal swaption"));
    subProductExpectedValues.back().testBias = false;
    subProductExpectedValues.back().errorThreshold = 2.32;
    const Spread displacement = 0;
    Matrix jacobian =
        SwapForwardMappings::coterminalSwapZedMatrix(
        curveState, displacement);
    bool logNormal = true;

    EvolutionDescription evolution = productClone.evolution();
    Size factors = todaysForwards.size();
    MarketModelType marketModelType = ExponentialCorrelationFlatVolatility;
    boost::shared_ptr<MarketModel> marketModel =
                    makeMarketModel(logNormal, evolution, factors, marketModelType);
    for (Size i=0; i<todaysForwards.size(); ++i) {
        const Matrix& forwardsCovariance = marketModel->totalCovariance(i);
        Matrix cotSwapsCovariance =
            jacobian * forwardsCovariance * transpose(jacobian);
        boost::shared_ptr<PlainVanillaPayoff> payoff(
            new PlainVanillaPayoff(Option::Call, todaysForwards[i]+displacement));

        Real expectedSwaption =
            BlackCalculator(payoff,
                            todaysCoterminalSwapRates[i]+displacement,
                            std::sqrt(cotSwapsCovariance[i][i]),
                            curveState.coterminalSwapAnnuity(0,i) *
                                todaysDiscounts[0]).value();
        subProductExpectedValues.back().values.push_back(expectedSwaption);
    }
}


void MarketModelTest::testAllMultiStepProducts() {
    std::string testDescription = "all multiStepProducts ";

    setup();

    MultiProductComposite product;
    std::vector<SubProductExpectedValues> subProductExpectedValues;
    addForwards(product, subProductExpectedValues);
    addOptionLets(product, subProductExpectedValues);
    addCoinitialSwaps(product, subProductExpectedValues);
    addCoterminalSwapsAndSwaptions(product, subProductExpectedValues);
    product.finalize();
    testMultiProductComposite(product, subProductExpectedValues,
                              testDescription);
}

void MarketModelTest::testCallableSwapNaif() {

    BOOST_MESSAGE("Pricing callable swap with naif exercise strategy in a LIBOR market model...");

    setup();

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
    SwapRateTrigger naifStrategy(rateTimes, swapTriggers, exerciseTimes);

    // Longstaff-Schwartz exercise strategy
    std::vector<std::vector<NodeData> > collectedData;
    std::vector<std::vector<Real> > basisCoefficients;
    NothingExerciseValue control(rateTimes);
    SwapBasisSystem basisSystem(rateTimes,exerciseTimes);
    NothingExerciseValue nullRebate(rateTimes);

    CallSpecifiedMultiProduct dummyProduct =
        CallSpecifiedMultiProduct(receiverSwap, naifStrategy,
                                  ExerciseAdapter(nullRebate));

    EvolutionDescription evolution = dummyProduct.evolution();

    MarketModelType marketModels[] = {
        // CalibratedMM,
        ExponentialCorrelationFlatVolatility,
        ExponentialCorrelationAbcdVolatility };
    for (Size j=0; j<LENGTH(marketModels); j++) {

        Size testedFactors[] = { 4, // 8,
                                 todaysForwards.size()};
        for (Size m=0; m<LENGTH(testedFactors); ++m) {
            Size factors = testedFactors[m];

            // Composite's ProductSuggested is the Terminal one
            MeasureType measures[] = { // ProductSuggested,
                                       MoneyMarketPlus
                                       // MoneyMarket,
                                       // Terminal
            };
            for (Size k=0; k<LENGTH(measures); k++) {
                std::vector<Size> numeraires = makeMeasure(dummyProduct, measures[k]);

                bool logNormal = true;
                boost::shared_ptr<MarketModel> marketModel =
                    makeMarketModel(logNormal, evolution, factors, marketModels[j]);


                EvolverType evolvers[] = { Pc, Ipc };
                boost::shared_ptr<MarketModelEvolver> evolver;
                Size stop =
                    isInTerminalMeasure(evolution, numeraires) ? 0 : 1;
                for (Size i=0; i<LENGTH(evolvers)-stop; i++) {

                    for (Size n=0; n<1; n++) {
                        //MTBrownianGeneratorFactory generatorFactory(seed_);
                        SobolBrownianGeneratorFactory generatorFactory(
                            SobolBrownianGenerator::Diagonal, seed_);

                        evolver = makeMarketModelEvolver(marketModel,
                                                         numeraires,
                                                         generatorFactory,
                                                         evolvers[i]);
                        std::ostringstream config;
                        config <<
                            marketModelTypeToString(marketModels[j]) << ", " <<
                            factors << (factors>1 ? (factors==todaysForwards.size() ? " (full) factors, " : " factors, ") : " factor,") <<
                            measureTypeToString(measures[k]) << ", " <<
                            evolverTypeToString(evolvers[i]) << ", " <<
                            "MT BGF";
                        if (printReport_)
                            BOOST_MESSAGE("    " << config.str());

                        // use the naif strategy

                        // 2. bermudan swaption to enter into the payer swap
                        CallSpecifiedMultiProduct bermudanProduct =
                            CallSpecifiedMultiProduct(
                                                  MultiStepNothing(evolution),
                                                  naifStrategy, payerSwap);

                        // 3. callable receiver swap
                        CallSpecifiedMultiProduct callableProduct =
                            CallSpecifiedMultiProduct(
                                                   receiverSwap, naifStrategy,
                                                   ExerciseAdapter(nullRebate));

                        // lower bound: evolve all 4 products togheter
                        MultiProductComposite allProducts;
                        allProducts.add(payerSwap);
                        allProducts.add(receiverSwap);
                        allProducts.add(bermudanProduct);
                        allProducts.add(callableProduct);
                        allProducts.finalize();

                        boost::shared_ptr<SequenceStatistics> stats =
                            simulate(evolver, allProducts);
                        checkCallableSwap(*stats, config.str());


                        // upper bound

                        //MTBrownianGeneratorFactory uFactory(seed_+142);
                        SobolBrownianGeneratorFactory uFactory(
                            SobolBrownianGenerator::Diagonal, seed_+142);
                        evolver = makeMarketModelEvolver(marketModel,
                                                         numeraires,
                                                         uFactory,
                                                         evolvers[i]);

                        std::vector<boost::shared_ptr<MarketModelEvolver> >
                            innerEvolvers;

                        std::vector<bool> isExerciseTime =
                            isInSubset(evolution.evolutionTimes(),
                                       naifStrategy.exerciseTimes());
                        for (Size s=0; s < isExerciseTime.size(); ++s) {
                            if (isExerciseTime[s]) {
                                MTBrownianGeneratorFactory iFactory(seed_+s);
                                boost::shared_ptr<MarketModelEvolver> e =
                                    makeMarketModelEvolver(marketModel,
                                                           numeraires,
                                                           iFactory,
                                                           evolvers[i],
                                                           s);
                                innerEvolvers.push_back(e);
                            }
                        }

                        Size initialNumeraire = evolver->numeraires().front();
                        Real initialNumeraireValue =
                            todaysDiscounts[initialNumeraire];

                        UpperBoundEngine uEngine(evolver, innerEvolvers,
                                                 receiverSwap, nullRebate,
                                                 receiverSwap, nullRebate,
                                                 naifStrategy,
                                                 initialNumeraireValue);
                        Statistics uStats;
                        uEngine.multiplePathValues(uStats,255,256);
                        Real delta = uStats.mean();
                        Real deltaError = uStats.errorEstimate();
                        if (printReport_)
                            BOOST_MESSAGE("    upper bound delta: " << io::rate(delta) << " +- " << io::rate(deltaError));

                    }
                }
            }
        }
    }
}

void MarketModelTest::testCallableSwapLS() {

    BOOST_MESSAGE("Pricing callable swap with Longstaff-Schwartz exercise strategy in a LIBOR market model...");

    setup();

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
    SwapRateTrigger naifStrategy(rateTimes, swapTriggers, exerciseTimes);

    // Longstaff-Schwartz exercise strategy
    std::vector<std::vector<NodeData> > collectedData;
    std::vector<std::vector<Real> > basisCoefficients;
    NothingExerciseValue control(rateTimes);
    SwapBasisSystem basisSystem(rateTimes,exerciseTimes);
    NothingExerciseValue nullRebate(rateTimes);

    CallSpecifiedMultiProduct dummyProduct =
        CallSpecifiedMultiProduct(receiverSwap, naifStrategy,
                                  ExerciseAdapter(nullRebate));

    EvolutionDescription evolution = dummyProduct.evolution();

    MarketModelType marketModels[] = {
        // CalibratedMM,
        ExponentialCorrelationFlatVolatility,
        ExponentialCorrelationAbcdVolatility };
    for (Size j=0; j<LENGTH(marketModels); j++) {

        Size testedFactors[] = { 4, // 8,
                                 todaysForwards.size()};
        for (Size m=0; m<LENGTH(testedFactors); ++m) {
            Size factors = testedFactors[m];

            // Composite's ProductSuggested is the Terminal one
            MeasureType measures[] = { // ProductSuggested,
                                       // MoneyMarketPlus,
                                       MoneyMarket
                                       //Terminal
            };
            for (Size k=0; k<LENGTH(measures); k++) {
                std::vector<Size> numeraires = makeMeasure(dummyProduct, measures[k]);

                bool logNormal = true;
                boost::shared_ptr<MarketModel> marketModel =
                    makeMarketModel(logNormal, evolution, factors, marketModels[j]);


                EvolverType evolvers[] = { Pc, Ipc };
                boost::shared_ptr<MarketModelEvolver> evolver;
                Size stop =
                    isInTerminalMeasure(evolution, numeraires) ? 0 : 1;
                for (Size i=0; i<LENGTH(evolvers)-stop; i++) {

                    for (Size n=0; n<1; n++) {
                        //MTBrownianGeneratorFactory generatorFactory(seed_);
                        SobolBrownianGeneratorFactory generatorFactory(
                            SobolBrownianGenerator::Diagonal, seed_);

                        evolver = makeMarketModelEvolver(marketModel,
                                                         numeraires,
                                                         generatorFactory,
                                                         evolvers[i]);
                        std::ostringstream config;
                        config <<
                            marketModelTypeToString(marketModels[j]) << ", " <<
                            factors << (factors>1 ? (factors==todaysForwards.size() ? " (full) factors, " : " factors, ") : " factor,") <<
                            measureTypeToString(measures[k]) << ", " <<
                            evolverTypeToString(evolvers[i]) << ", " <<
                            "MT BGF";
                        if (printReport_)
                            BOOST_MESSAGE("    " << config.str());

                        // calculate the exercise strategy
                        collectNodeData(*evolver,
                            receiverSwap, basisSystem, nullRebate,
                            control, trainingPaths_, collectedData);
                        genericLongstaffSchwartzRegression(collectedData,
                            basisCoefficients);
                        LongstaffSchwartzExerciseStrategy exerciseStrategy(
                                               basisSystem, basisCoefficients,
                                               evolution, numeraires,
                                               nullRebate, control);

                        // 2. bermudan swaption to enter into the payer swap
                        CallSpecifiedMultiProduct bermudanProduct =
                            CallSpecifiedMultiProduct(
                                                  MultiStepNothing(evolution),
                                                  exerciseStrategy, payerSwap);

                        // 3. callable receiver swap
                        CallSpecifiedMultiProduct callableProduct =
                            CallSpecifiedMultiProduct(
                                               receiverSwap, exerciseStrategy,
                                               ExerciseAdapter(nullRebate));

                        // lower bound: evolve all 4 products togheter
                        MultiProductComposite allProducts;
                        allProducts.add(payerSwap);
                        allProducts.add(receiverSwap);
                        allProducts.add(bermudanProduct);
                        allProducts.add(callableProduct);
                        allProducts.finalize();

                        boost::shared_ptr<SequenceStatistics> stats =
                            simulate(evolver, allProducts);
                        checkCallableSwap(*stats, config.str());


                        // upper bound

                        //MTBrownianGeneratorFactory uFactory(seed_+142);
                        SobolBrownianGeneratorFactory uFactory(
                            SobolBrownianGenerator::Diagonal, seed_+142);
                        evolver = makeMarketModelEvolver(marketModel,
                                                         numeraires,
                                                         uFactory,
                                                         evolvers[i]);

                        std::vector<boost::shared_ptr<MarketModelEvolver> >
                            innerEvolvers;

                        std::vector<bool> isExerciseTime =
                            isInSubset(evolution.evolutionTimes(),
                                       exerciseStrategy.exerciseTimes());
                        for (Size s=0; s < isExerciseTime.size(); ++s) {
                            if (isExerciseTime[s]) {
                                MTBrownianGeneratorFactory iFactory(seed_+s);
                                boost::shared_ptr<MarketModelEvolver> e =
                                    makeMarketModelEvolver(marketModel,
                                                           numeraires,
                                                           iFactory,
                                                           evolvers[i],
                                                           s);
                                innerEvolvers.push_back(e);
                            }
                        }

                        Size initialNumeraire = evolver->numeraires().front();
                        Real initialNumeraireValue =
                            todaysDiscounts[initialNumeraire];

                        UpperBoundEngine uEngine(evolver, innerEvolvers,
                                                 receiverSwap, nullRebate,
                                                 receiverSwap, nullRebate,
                                                 exerciseStrategy,
                                                 initialNumeraireValue);
                        Statistics uStats;
                        uEngine.multiplePathValues(uStats,255,256);
                        Real delta = uStats.mean();
                        Real deltaError = uStats.errorEstimate();
                        if (printReport_)
                            BOOST_MESSAGE("    upper bound delta: " << io::rate(delta) << " +- " << io::rate(deltaError));

                    }
                }
            }
        }
    }
}

void MarketModelTest::testCallableSwapAnderson() {

    BOOST_MESSAGE("Pricing callable swap with Anderson exercise strategy in a LIBOR market model...");

    setup();

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
    SwapRateTrigger naifStrategy(rateTimes, swapTriggers, exerciseTimes);

    // Anderson exercise strategy
    std::vector<std::vector<NodeData> > collectedData;
    std::vector<std::vector<Real> > parameters;
    NothingExerciseValue control(rateTimes);
    NothingExerciseValue nullRebate(rateTimes);
    TriggeredSwapExercise parametricForm(rateTimes, exerciseTimes,
        std::vector<Time>(exerciseTimes.size(),fixedRate));

    CallSpecifiedMultiProduct dummyProduct =
        CallSpecifiedMultiProduct(receiverSwap, naifStrategy,
                                  ExerciseAdapter(nullRebate));

    EvolutionDescription evolution = dummyProduct.evolution();

    MarketModelType marketModels[] = {
        // CalibratedMM,
        ExponentialCorrelationFlatVolatility,
        ExponentialCorrelationAbcdVolatility };
    for (Size j=0; j<LENGTH(marketModels); j++) {

        Size testedFactors[] = { 4, 8,
                                 todaysForwards.size()};
        for (Size m=0; m<LENGTH(testedFactors); ++m) {
            Size factors = testedFactors[m];

            // Composite's ProductSuggested is the Terminal one
            MeasureType measures[] = { // ProductSuggested,
                                       // MoneyMarketPlus,
                                       // MoneyMarket,
                                       Terminal
            };
            for (Size k=0; k<LENGTH(measures); k++) {
                std::vector<Size> numeraires = makeMeasure(dummyProduct, measures[k]);
                bool logNormal = true;
                boost::shared_ptr<MarketModel> marketModel =
                    makeMarketModel(logNormal, evolution, factors, marketModels[j]);
                EvolverType evolvers[] = { Pc, Ipc };
                boost::shared_ptr<MarketModelEvolver> evolver;
                Size stop =
                    isInTerminalMeasure(evolution, numeraires) ? 0 : 1;
                for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
                    for (Size n=0; n<1; n++) {
                        //MTBrownianGeneratorFactory generatorFactory(seed_);
                        SobolBrownianGeneratorFactory generatorFactory(
                            SobolBrownianGenerator::Diagonal, seed_);
                        evolver = makeMarketModelEvolver(marketModel,
                                                         numeraires,
                                                         generatorFactory,
                                                         evolvers[i]);
                        std::ostringstream config;
                        config <<
                            marketModelTypeToString(marketModels[j]) << ", " <<
                            factors << (factors>1 ? (factors==todaysForwards.size() ? " (full) factors, " : " factors, ") : " factor,") <<
                            measureTypeToString(measures[k]) << ", " <<
                            evolverTypeToString(evolvers[i]) << ", " <<
                            "MT BGF";
                        if (printReport_)
                            BOOST_MESSAGE("    " << config.str());
                        // 1. calculate the exercise strategy
                        collectNodeData(*evolver,
                            receiverSwap, parametricForm, nullRebate,
                            control, trainingPaths_, collectedData);
                        Simplex om(0.01);
                        EndCriteria ec(1000, 100, 1e-8, 1e-16, 1e-8);
                        Size initialNumeraire = evolver->numeraires().front();
                        Real initialNumeraireValue = todaysDiscounts[initialNumeraire];
                        Real firstPassValue = genericEarlyExerciseOptimization(
                            collectedData, parametricForm, parameters, ec, om) *
                            initialNumeraireValue;
                        if (printReport_)
                            BOOST_MESSAGE("    initial estimate:  " << io::rate(firstPassValue));
                        ParametricExerciseAdapter exerciseStrategy(parametricForm, parameters);

                        // 2. bermudan swaption to enter into the payer swap
                        CallSpecifiedMultiProduct bermudanProduct =
                            CallSpecifiedMultiProduct(
                                                  MultiStepNothing(evolution),
                                                  exerciseStrategy, payerSwap);

                        // 3. callable receiver swap
                        CallSpecifiedMultiProduct callableProduct =
                            CallSpecifiedMultiProduct(
                                               receiverSwap, exerciseStrategy,
                                               ExerciseAdapter(nullRebate));
                        // lower bound: evolve all 4 products togheter
                        MultiProductComposite allProducts;
                        allProducts.add(payerSwap);
                        allProducts.add(receiverSwap);
                        allProducts.add(bermudanProduct);
                        allProducts.add(callableProduct);
                        allProducts.finalize();
                        boost::shared_ptr<SequenceStatistics> stats =
                            simulate(evolver, allProducts);
                        checkCallableSwap(*stats, config.str());

                        // upper bound
                        //MTBrownianGeneratorFactory uFactory(seed_+142);
                        SobolBrownianGeneratorFactory uFactory(
                            SobolBrownianGenerator::Diagonal, seed_+142);
                        evolver = makeMarketModelEvolver(marketModel,
                                                         numeraires,
                                                         uFactory,
                                                         evolvers[i]);
                        std::vector<boost::shared_ptr<MarketModelEvolver> >
                            innerEvolvers;
                        std::vector<bool> isExerciseTime =
                            isInSubset(evolution.evolutionTimes(),
                                       exerciseStrategy.exerciseTimes());
                        for (Size s=0; s < isExerciseTime.size(); ++s) {
                            if (isExerciseTime[s]) {
                                MTBrownianGeneratorFactory iFactory(seed_+s);
                                boost::shared_ptr<MarketModelEvolver> e =
                                    makeMarketModelEvolver(marketModel,
                                                           numeraires,
                                                           iFactory,
                                                           evolvers[i],
                                                           s);
                                innerEvolvers.push_back(e);
                            }
                        }
                        UpperBoundEngine uEngine(evolver, innerEvolvers,
                                                 receiverSwap, nullRebate,
                                                 receiverSwap, nullRebate,
                                                 exerciseStrategy,
                                                 initialNumeraireValue);
                        Statistics uStats;
                        uEngine.multiplePathValues(uStats,255,256);
                        Real delta = uStats.mean();
                        Real deltaError = uStats.errorEstimate();
                        if (printReport_)
                            BOOST_MESSAGE("    upper bound delta: " << io::rate(delta) << " +- " << io::rate(deltaError));

                    }
                }
            }
        }
    }
}



void MarketModelTest::testGreeks() {

    BOOST_MESSAGE("Testing caplets greeks in a lognormal forward rate market model...");

    setup();

    std::vector<boost::shared_ptr<Payoff> > payoffs(todaysForwards.size());
    std::vector<boost::shared_ptr<StrikedTypePayoff> >
        displacedPayoffs(todaysForwards.size());
    for (Size i=0; i<todaysForwards.size(); ++i) {
        payoffs[i] = boost::shared_ptr<Payoff>(new
            //PlainVanillaPayoff(Option::Call, todaysForwards[i]));
            CashOrNothingPayoff(Option::Call, todaysForwards[i], 0.01));
        displacedPayoffs[i] = boost::shared_ptr<StrikedTypePayoff>(new
            //PlainVanillaPayoff(Option::Call, todaysForwards[i]+displacement));
            CashOrNothingPayoff(Option::Call, todaysForwards[i]+displacement, 0.01));
    }

    MultiStepOptionlets product(rateTimes, accruals,
                                paymentTimes, payoffs);

    EvolutionDescription evolution = product.evolution();

    MarketModelType marketModels[] = {
        // CalibratedMM,
        // ExponentialCorrelationFlatVolatility,
        ExponentialCorrelationAbcdVolatility };
    for (Size j=0; j<LENGTH(marketModels); j++) {

        Size testedFactors[] = { 4, 8, todaysForwards.size() };
        for (Size m=0; m<LENGTH(testedFactors); ++m) {
            Size factors = testedFactors[m];

            MeasureType measures[] = { //MoneyMarketPlus,
                                       MoneyMarket//,
                                       //Terminal
            };
            for (Size k=0; k<LENGTH(measures); k++) {
                std::vector<Size> numeraires = makeMeasure(product, measures[k]);

                for (Size n=0; n<1; n++) {
                    //MTBrownianGeneratorFactory generatorFactory(seed_);
                    SobolBrownianGeneratorFactory generatorFactory(
                                           SobolBrownianGenerator::Diagonal,
                                           seed_);

                    bool logNormal = true;
                    boost::shared_ptr<MarketModel> marketModel =
                        makeMarketModel(logNormal, evolution, factors,
                                        marketModels[j]);

                    boost::shared_ptr<MarketModelEvolver> evolver(new
                                 LogNormalFwdRateEuler(marketModel,
                                                         generatorFactory,
                                                         numeraires));
                    SequenceStatistics stats(product.numberOfProducts());


                    std::vector<Size> startIndexOfConstraint;
                    std::vector<Size> endIndexOfConstraint;

                    for (Size i=0; i<evolution.evolutionTimes().size(); ++i) {
                        startIndexOfConstraint.push_back(i);
                        endIndexOfConstraint.push_back(i+1);
                    }


                    std::vector<
                        std::vector<boost::shared_ptr<ConstrainedEvolver> > >
                        constrainedEvolvers;
                    std::vector<std::vector<std::vector<Real> > > diffWeights;
                    std::vector<std::vector<SequenceStatistics> > greekStats;

                    std::vector<boost::shared_ptr<ConstrainedEvolver> >
                        deltaGammaEvolvers;
                    std::vector<std::vector<Real> > deltaGammaWeights(
                                                     2, std::vector<Real>(3));
                    std::vector<SequenceStatistics> deltaGammaStats(2,stats);


                    Spread forwardBump = 1.0e-6;
                    marketModel =
                        makeMarketModel(logNormal, evolution, factors,
                                        marketModels[j], -forwardBump);
                    deltaGammaEvolvers.push_back(
                        boost::shared_ptr<ConstrainedEvolver>(new
                            LogNormalFwdRateEulerConstrained(marketModel,
                                                        generatorFactory,
                                                        numeraires)));
                    deltaGammaEvolvers.back()->setConstraintType(
                        startIndexOfConstraint, endIndexOfConstraint);
                    marketModel =
                        makeMarketModel(logNormal, evolution, factors,
                                        marketModels[j], forwardBump);
                    deltaGammaEvolvers.push_back(
                        boost::shared_ptr<ConstrainedEvolver>(new
                            LogNormalFwdRateEulerConstrained(marketModel,
                                                        generatorFactory,
                                                        numeraires)));
                    deltaGammaEvolvers.back()->setConstraintType(
                        startIndexOfConstraint, endIndexOfConstraint);

                    deltaGammaWeights[0][0] = 0.0;
                    deltaGammaWeights[0][1] = -1.0/(2.0*forwardBump);
                    deltaGammaWeights[0][2] = 1.0/(2.0*forwardBump);

                    deltaGammaWeights[1][0] = -2.0/(forwardBump*forwardBump);
                    deltaGammaWeights[1][1] = 1.0/(forwardBump*forwardBump);
                    deltaGammaWeights[1][2] = 1.0/(forwardBump*forwardBump);


                    std::vector<boost::shared_ptr<ConstrainedEvolver> >
                        vegaEvolvers;
                    std::vector<std::vector<Real> > vegaWeights(
                                                     1, std::vector<Real>(3));
                    std::vector<SequenceStatistics> vegaStats(1,stats);

                    Volatility volBump = 1.0e-4;
                    marketModel =
                        makeMarketModel(logNormal, evolution, factors,
                                        marketModels[j], 0.0, -volBump);
                    vegaEvolvers.push_back(
                        boost::shared_ptr<ConstrainedEvolver>(new
                            LogNormalFwdRateEulerConstrained(marketModel,
                                                        generatorFactory,
                                                        numeraires)));
                    vegaEvolvers.back()->setConstraintType(
                        startIndexOfConstraint, endIndexOfConstraint);
                    marketModel =
                        makeMarketModel(logNormal, evolution, factors,
                                        marketModels[j], 0.0, volBump);
                    vegaEvolvers.push_back(
                        boost::shared_ptr<ConstrainedEvolver>(new
                            LogNormalFwdRateEulerConstrained(marketModel,
                                                        generatorFactory,
                                                        numeraires)));
                    vegaEvolvers.back()->setConstraintType(
                        startIndexOfConstraint, endIndexOfConstraint);

                    vegaWeights[0][0] = 0.0;
                    vegaWeights[0][1] = -1.0/(2.0*volBump);
                    vegaWeights[0][2] = 1.0/(2.0*volBump);



                    constrainedEvolvers.push_back(deltaGammaEvolvers);
                    diffWeights.push_back(deltaGammaWeights);
                    greekStats.push_back(deltaGammaStats);

                    constrainedEvolvers.push_back(vegaEvolvers);
                    diffWeights.push_back(vegaWeights);
                    greekStats.push_back(vegaStats);

                    std::ostringstream config;
                    config <<
                        marketModelTypeToString(marketModels[j]) << ", " <<
                        factors << (factors>1 ? (factors==todaysForwards.size() ? " (full) factors, " : " factors, ") : " factor,") <<
                        measureTypeToString(measures[k]) << ", " <<
                        "MT BGF";
                    if (printReport_)
                        BOOST_MESSAGE("    " << config.str());

                    Size initialNumeraire = evolver->numeraires().front();
                    Real initialNumeraireValue =
                        todaysDiscounts[initialNumeraire];

                    ProxyGreekEngine engine(evolver,
                                            constrainedEvolvers, diffWeights,
                                            startIndexOfConstraint,
                                            endIndexOfConstraint,
                                            product,
                                            initialNumeraireValue);

                    engine.multiplePathValues(stats, greekStats, paths_);

                    std::vector<Real> values = stats.mean();
                    std::vector<Real> errors = stats.errorEstimate();
                    std::vector<Real> deltas = greekStats[0][0].mean();
                    std::vector<Real> deltaErrors = greekStats[0][0].errorEstimate();
                    std::vector<Real> gammas = greekStats[0][1].mean();
                    std::vector<Real> gammaErrors = greekStats[0][1].errorEstimate();
                    std::vector<Real> vegas = greekStats[1][0].mean();
                    std::vector<Real> vegaErrors = greekStats[1][0].errorEstimate();

                    std::vector<DiscountFactor> discPlus(todaysForwards.size()+1, todaysDiscounts[0]);
                    std::vector<DiscountFactor> discMinus(todaysForwards.size()+1, todaysDiscounts[0]);
                    std::vector<Rate> fwdPlus(todaysForwards.size());
                    std::vector<Rate> fwdMinus(todaysForwards.size());
                    std::vector<Rate> pricePlus(todaysForwards.size());
                    std::vector<Rate> price0(todaysForwards.size());
                    std::vector<Rate> priceMinus(todaysForwards.size());
                    for (Size i=0; i<todaysForwards.size(); ++i) {
                        Time tau = rateTimes[i+1]-rateTimes[i];
                        fwdPlus[i]=todaysForwards[i]+forwardBump;
                        fwdMinus[i]=todaysForwards[i]-forwardBump;
                        discPlus[i+1]=discPlus[i]/(1.0+fwdPlus[i]*tau);
                        discMinus[i+1]=discMinus[i]/(1.0+fwdMinus[i]*tau);
                        pricePlus[i]=BlackCalculator(displacedPayoffs[i], fwdPlus[i],
                                                     volatilities[i]*sqrt(rateTimes[i]),
                                                     discPlus[i+1]*tau).value();
                        price0[i]=BlackCalculator(displacedPayoffs[i], todaysForwards[i],
                                                  volatilities[i]*sqrt(rateTimes[i]),
                                                  todaysDiscounts[i+1]*tau).value();
                        priceMinus[i]=BlackCalculator(displacedPayoffs[i], fwdMinus[i],
                                                      volatilities[i]*sqrt(rateTimes[i]),
                                                      discMinus[i+1]*tau).value();
                    }

                    for (Size i=0; i<product.numberOfProducts(); ++i) {
                        Real numDelta = (pricePlus[i]-priceMinus[i])/(2.0*forwardBump);
                        Real numGamma = (pricePlus[i]-2*price0[i]+priceMinus[i])/(forwardBump*forwardBump);
                        if (printReport_) {
                            BOOST_MESSAGE(io::ordinal(i+1) << " caplet: "
                                          << "value = " << price0[i] << ", "
                                          << "delta = " << numDelta << ", "
                                          << "gamma = " << numGamma);
                            BOOST_MESSAGE(io::ordinal(i+1) << " caplet: "
                                          << "value = " << values[i]
                                          << " +- " << errors[i]
                                          << " (" << (values[i]-price0[i])/errors[i] << " s.e.), "
                                          << "delta = " << deltas[i]
                                          << " +- " << deltaErrors[i]
                                          << " (" << (deltas[i]-numDelta)/deltaErrors[i] << " s.e.), "
                                          << "gamma = " << gammas[i]
                                          << " +- " << gammaErrors[i]
                                          << " (" << (gammas[i]-numGamma)/gammaErrors[i] << " s.e.), "
                                          << "vega = " << vegas[i]
                                          << " +- " << vegaErrors[i]);
                        }
                    }
                }
            }
        }
    }
}






//--------------------- Volatility tests ---------------------

void MarketModelTest::testAbcdVolatilityIntegration() {

    BOOST_MESSAGE("Testing Abcd-volatility integration...");

    setup();

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

    setup();

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

    setup();
    
    AbcdCalibration instVol(std::vector<Time>(rateTimes.begin(), rateTimes.end()-1), blackVols);
    Real a0 = instVol.a();
    Real b0 = instVol.b();
    Real c0 = instVol.c();
    Real d0 = instVol.d();
    Real error0 = instVol.error();

    instVol.compute();

    EndCriteria::Type ec = instVol.endCriteria();
    Real a1 = instVol.a();
    Real b1 = instVol.b();
    Real c1 = instVol.c();
    Real d1 = instVol.d();
    Real error1 = instVol.error();

    if (error1>=error0)
        BOOST_FAIL("Parameters:" <<
            "\na:     " << a0 << " ---> " << a1 <<
            "\nb:     " << b0 << " ---> " << b1 <<
            "\nc:     " << c0 << " ---> " << c1 <<
            "\nd:     " << d0 << " ---> " << d1 <<
            "\nerror: " << error0 << " ---> " << error1);
    
    Abcd abcd(a1, b1, c1, d1);
    std::vector<Real> k = instVol.k(std::vector<Time>(rateTimes.begin(), rateTimes.end()-1), blackVols);
    Real tol = 3.0e-4;
    for (Size i=0; i<blackVols.size(); i++) {
        if (std::abs(k[i]-1.0)>tol) {
            Real modelVol =
                abcd.volatility(0.0, rateTimes[i], rateTimes[i]);
            BOOST_FAIL("\n EndCriteria = " << ec <<
                       "\n Fixing Time = " << rateTimes[i] <<
                       "\n MktVol      = " << io::rate(blackVols[i]) <<
                       "\n ModVol      = " << io::rate(modelVol) <<
                       "\n k           = " << k[i] <<
                       "\n error       = " << std::abs(k[i]-1.0) <<
                       "\n tol         = " << tol);
        }
    }

}

//--------------------- Other tests ---------------------

void MarketModelTest::testDriftCalculator() {

    // Test full factor drift equivalence between compute() and
    // computeReduced()

    BOOST_MESSAGE("Testing drift calculation...");

    setup();

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
        bool logNormal = true;
        boost::shared_ptr<MarketModel> marketModel =
            makeMarketModel(logNormal, evolution, factors, marketModels[k]);
        std::vector<Rate> displacements = marketModel->displacements();
        for (Size j=0; j<numberOfSteps; ++j) {     // loop over steps
            const Matrix& A = marketModel->pseudoRoot(j);
            //BOOST_MESSAGE(io::ordinal(j) << " pseudoroot:\n" << A);
            Size inf = std::max(0,static_cast<Integer>(alive[j]));
            for (Size h=inf; h<numeraires.size(); ++h) {     // loop over numeraires
                LMMDriftCalculator driftcalculator(A, displacements, rateTaus,
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

    BOOST_MESSAGE("Testing isInSubset...");

    setup();

    Size dim = 100;
    std::vector<Time> set, subset;
    for (Size i=0; i<dim; i++) set.push_back(i*1.0);
    for (Size i=0; i<dim; i++) subset.push_back(dim+i*1.0);
    std::vector<bool> result = isInSubset(set, subset);
    if (printReport_) {
        for (Size i=0; i<dim; i++) {
            BOOST_MESSAGE("\n" << io::ordinal(i) << ":" <<
                          " set[" << i << "] =  " << set[i] <<
                          ", subset[" << i << "] =  " << subset[i] <<
                          ", result[" << i << "] =  " << result[i]);
        }
    }
}

// --- Call the desired tests
test_suite* MarketModelTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Market-model tests");

    suite->add(BOOST_TEST_CASE(&MarketModelTest::testOneStepForwardsAndOptionlets));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testOneStepNormalForwardsAndOptionlets));

    suite->add(BOOST_TEST_CASE(&MarketModelTest::testAllMultiStepProducts));

    //// just one of the tests below is run in order to reduce running times...
    //// uncomment as much as you prefer...
    //suite->add(BOOST_TEST_CASE(&MarketModelTest::testCallableSwapNaif));
    //suite->add(BOOST_TEST_CASE(&MarketModelTest::testCallableSwapLS));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testCallableSwapAnderson));

    suite->add(BOOST_TEST_CASE(&MarketModelTest::testGreeks));

    suite->add(BOOST_TEST_CASE(&MarketModelTest::testAbcdVolatilityIntegration));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testAbcdVolatilityCompare));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testAbcdVolatilityFit));

    suite->add(BOOST_TEST_CASE(&MarketModelTest::testDriftCalculator));
    suite->add(BOOST_TEST_CASE(&MarketModelTest::testIsInSubset));

    return suite;
}
