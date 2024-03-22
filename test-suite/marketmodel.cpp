/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2006 Ferdinando Ametrano
Copyright (C) 2006 Marco Bianchetti
Copyright (C) 2006 Cristina Duminuco
Copyright (C) 2006 StatPro Italia srl
Copyright (C) 2008 Mark Joshi
Copyright (C) 2012 Peter Caspers

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

#include "preconditions.hpp"
#include "toplevelfixture.hpp"
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
#include <ql/models/marketmodels/evolvers/lognormalfwdrateballand.hpp>
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
#include <ql/models/marketmodels/products/multistep/multistepperiodcapletswaptions.hpp>
#include <ql/models/marketmodels/products/multistep/multistepforwards.hpp>
#include <ql/models/marketmodels/products/multistep/multistepnothing.hpp>
#include <ql/models/marketmodels/products/multistep/multistepoptionlets.hpp>
#include <ql/models/marketmodels/products/multistep/multistepswap.hpp>
#include <ql/models/marketmodels/products/onestep/onestepforwards.hpp>
#include <ql/models/marketmodels/products/onestep/onestepoptionlets.hpp>
#include <ql/models/marketmodels/forwardforwardmappings.hpp>
#include <ql/models/marketmodels/proxygreekengine.hpp>
#include <ql/models/marketmodels/swapforwardmappings.hpp>
#include <ql/models/marketmodels/models/fwdperiodadapter.hpp>
#include <ql/models/marketmodels/models/fwdtocotswapadapter.hpp>
#include <ql/models/marketmodels/models/cotswaptofwdadapter.hpp>
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
#include <ql/termstructures/volatility/abcd.hpp>
#include <ql/termstructures/volatility/abcdcalibration.hpp>
#include <ql/math/optimization/simplex.hpp>
#include <ql/quotes/simplequote.hpp>

#include <ql/models/marketmodels/products/pathwise/pathwiseproductcaplet.hpp>
#include <ql/models/marketmodels/products/pathwise/pathwiseproductswaption.hpp>

#include <ql/models/marketmodels/pathwiseaccountingengine.hpp>
#include <ql/models/marketmodels/pathwisegreeks/ratepseudorootjacobian.hpp>
#include <ql/models/marketmodels/pathwisegreeks/swaptionpseudojacobian.hpp>

#include <ql/models/marketmodels/models/pseudorootfacade.hpp>

#include <ql/models/marketmodels/pathwisegreeks/bumpinstrumentjacobian.hpp>

#include <ql/models/marketmodels/evolvers/volprocesses/squarerootandersen.hpp>

#include <ql/models/marketmodels/evolvers/svddfwdratepc.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>

#include <ql/models/marketmodels/products/multistep/multistepinversefloater.hpp>
#include <ql/models/marketmodels/products/pathwise/pathwiseproductinversefloater.hpp>
#include <ql/models/marketmodels/products/multistep/multisteppathwisewrapper.hpp>

#include <cmath>
#include <sstream>

using namespace QuantLib;
using namespace boost::unit_test_framework;

using std::fabs;
using std::sqrt;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(MarketModelTests)

enum MarketModelType { ExponentialCorrelationFlatVolatility,
                       ExponentialCorrelationAbcdVolatility/*, CalibratedMM*/
};

Date todaysDate, startDate, endDate;
Schedule dates;
std::vector<Time> rateTimes, paymentTimes;
std::vector<Real> accruals;
Calendar calendar;
DayCounter dayCounter;
std::vector<Rate> todaysForwards, todaysCoterminalSwapRates;
Rate meanForward;
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
    explicit SubProductExpectedValues(std::string descr) : description(std::move(descr)) {}
    std::string description;
    std::vector<Real> values;
    bool testBias = false;
    Real errorThreshold;
};

void setup() {

    // Times
    calendar = NullCalendar();
    todaysDate = Settings::instance().evaluationDate();
    //startDate = todaysDate + 5*Years;
    endDate = todaysDate + 5*Years;
    dates =Schedule(todaysDate, endDate, Period(Semiannual),
                    calendar, Following, Following,
                    DateGeneration::Backward, false);
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
    meanForward=0.0;

    for (Size i=0; i<todaysForwards.size(); ++i)
        {
            todaysForwards[i] = 0.03 + 0.0010*i;
            meanForward+= todaysForwards[i];
        }
    meanForward /= todaysForwards.size();

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
    normalVols = std::vector<Volatility>(todaysForwards.size());
    for (Size i=0; i<std::min(LENGTH(mktVols),todaysForwards.size()); i++) {
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

ext::shared_ptr<SequenceStatisticsInc>
simulate(const ext::shared_ptr<MarketModelEvolver>& evolver,
         const MarketModelMultiProduct& product) {
    Size initialNumeraire = evolver->numeraires().front();
    Real initialNumeraireValue = todaysDiscounts[initialNumeraire];

    AccountingEngine engine(evolver, product, initialNumeraireValue);
    ext::shared_ptr<SequenceStatisticsInc> stats(
            new SequenceStatisticsInc(product.numberOfProducts()));
    engine.multiplePathValues(*stats, paths_);
    return stats;
}


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

ext::shared_ptr<MarketModel> makeMarketModel(bool logNormal,
                                             const EvolutionDescription& evolution,
                                             Size numberOfFactors,
                                             MarketModelType marketModelType,
                                             Spread forwardBump = 0.0,
                                             Volatility volBump = 0.0) {

    std::vector<Time> fixingTimes(evolution.rateTimes());
    fixingTimes.pop_back();
    ext::shared_ptr<LmVolatilityModel> volModel(new
                LmExtLinearExponentialVolModel(fixingTimes,0.5, 0.6, 0.1, 0.1));
    ext::shared_ptr<LmCorrelationModel> corrModel(
                new LmLinearExponentialCorrelationModel(evolution.numberOfRates(),
                longTermCorrelation, beta));

    std::vector<Rate> bumpedForwards(todaysForwards.size());
    std::transform(todaysForwards.begin(), todaysForwards.end(),
                   bumpedForwards.begin(),
                   [=](Rate r){ return r + forwardBump; });

    std::vector<Volatility> bumpedVols(volatilities.size());
    if (logNormal)
        std::transform(volatilities.begin(), volatilities.end(),
                       bumpedVols.begin(),
                       [=](Volatility v){ return v + volBump; });
    else
        std::transform(normalVols.begin(), normalVols.end(),
                       bumpedVols.begin(),
                       [=](Volatility v){ return v + volBump; });

    Matrix correlations = exponentialCorrelations(evolution.rateTimes(),
                                                  longTermCorrelation,
                                                  beta);
    ext::shared_ptr<PiecewiseConstantCorrelation> corr(new
                TimeHomogeneousForwardCorrelation(correlations,
                evolution.rateTimes()));
    switch (marketModelType) {
      case ExponentialCorrelationFlatVolatility:
        return ext::shared_ptr<MarketModel>(new
                FlatVol(bumpedVols,
                corr,
                evolution,
                numberOfFactors,
                bumpedForwards,
                std::vector<Spread>(bumpedForwards.size(), displacement)));
      case ExponentialCorrelationAbcdVolatility:
        return ext::shared_ptr<MarketModel>(new
                AbcdVol(0.0,0.0,1.0,1.0,
                bumpedVols,
                corr,
                evolution,
                numberOfFactors,
                bumpedForwards,
                std::vector<Spread>(bumpedForwards.size(), displacement)));
        //case CalibratedMM:
        //    return ext::shared_ptr<MarketModel>(new
        //        CalibratedMarketModel(volModel, corrModel,
        //                              evolution,
        //                              numberOfFactors,
        //                              bumpedForwards,
        //                              displacement));
      default:
        QL_FAIL("unknown MarketModel type");
    }
}

enum MeasureType { ProductSuggested, Terminal,
                   MoneyMarket, MoneyMarketPlus };

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
                              MeasureType measureType) {
    std::vector<Size> result;
    const EvolutionDescription& evolution(product.evolution());
    switch (measureType) {
      case ProductSuggested:
        result = product.suggestedNumeraires();
        break;
      case Terminal:
        result = terminalMeasure(evolution);
        if (!isInTerminalMeasure(evolution, result)) {
            BOOST_ERROR("\nfailure in verifying Terminal measure:\n"
                        << to_stream(result));
        }
        break;
      case MoneyMarket:
        result = moneyMarketMeasure(evolution);
        if (!isInMoneyMarketMeasure(evolution, result)) {
            BOOST_ERROR("\nfailure in verifying MoneyMarket measure:\n"
                        << to_stream(result));
        }
        break;
      case MoneyMarketPlus:
        result = moneyMarketPlusMeasure(evolution, measureOffset_);
        if (!isInMoneyMarketPlusMeasure(evolution, result, measureOffset_)) {
            BOOST_ERROR("\nfailure in verifying MoneyMarketPlus(" <<
                        measureOffset_ << ") measure:\n" << to_stream(result));
        }
        break;
      default:
        QL_FAIL("unknown measure type");
    }
    checkCompatibility(evolution, result);
    if (printReport_) {
        BOOST_TEST_MESSAGE("    " << measureTypeToString(measureType) << ": " << to_stream(result));
    }
    return result;
}

enum EvolverType { Ipc, Balland, Pc, NormalPc};

std::string evolverTypeToString(EvolverType type) {
    switch (type) {
      case Ipc:
        return "iterative predictor corrector";
      case Balland:
        return "Balland predictor corrector";
      case Pc:
        return "predictor corrector";
      case NormalPc:
        return "predictor corrector for normal case";
      default:
        QL_FAIL("unknown MarketModelEvolver type");
    }
}

ext::shared_ptr<MarketModelEvolver> makeMarketModelEvolver(const ext::shared_ptr<MarketModel>& marketModel,
                                                           const std::vector<Size>& numeraires,
                                                           const BrownianGeneratorFactory& generatorFactory,
                                                           EvolverType evolverType,
                                                           Size initialStep = 0) {
    switch (evolverType) {
      case Ipc:
        return ext::shared_ptr<MarketModelEvolver>(
                  new LogNormalFwdRateIpc(marketModel, generatorFactory,
                  numeraires, initialStep));
      case Balland:
        return ext::shared_ptr<MarketModelEvolver>(
                  new LogNormalFwdRateBalland(marketModel, generatorFactory,
                  numeraires, initialStep));
      case Pc:
        return ext::shared_ptr<MarketModelEvolver>(
                  new LogNormalFwdRatePc(marketModel, generatorFactory,
                  numeraires, initialStep));
      case NormalPc:
        return ext::shared_ptr<MarketModelEvolver>(
                  new NormalFwdRatePc(marketModel, generatorFactory,
                  numeraires, initialStep));
      default:
        QL_FAIL("unknown MarketModelEvolver type");
    }
}

void checkMultiProductCompositeResults(const SequenceStatisticsInc& stats,
                                       const std::vector<SubProductExpectedValues>& subProductExpectedValues,
                                       const std::string& config) {

    std::vector<Real> results = stats.mean();
    std::vector<Real> errors = stats.errorEstimate();

    // size check
    Size nbOfResults = 0;
    for (const auto& subProductExpectedValue : subProductExpectedValues) {
        for (Size j = 0; j < subProductExpectedValue.values.size(); ++j)
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
        for (Real value : subProductExpectedValue->values) {
            Real stdDev =
                (results[currentResultIndex] - value) / errors[currentResultIndex];
            stdDevs.push_back(stdDev);
            maxError = std::max(maxError, stdDev);
            minError = std::min(minError, stdDev);
            ++currentResultIndex;
        }
        bool isBiased = minError > 0.0 || maxError < 0.0;
        if (printReport_
            || (subProductExpectedValue->testBias && isBiased)
            || std::max(-minError, maxError) > errorThreshold) {
            BOOST_TEST_MESSAGE(config);
            currentResultIndex = 0;
            for (Size j=0; j<subProductExpectedValue->values.size(); ++j) {
                BOOST_TEST_MESSAGE(io::ordinal(j+1)
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


void checkForwardsAndOptionlets(const SequenceStatisticsInc& stats,
                                const std::vector<Rate>& forwardStrikes,
                                const std::vector<ext::shared_ptr<StrikedTypePayoff> >& displacedPayoffs,
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
        BOOST_TEST_MESSAGE(config);
        Size i;
        for (i=0; i<N; ++i) {
            BOOST_TEST_MESSAGE(io::ordinal(i+1) << " forward: "
                               << io::rate(results[i])
                               << "\t" << io::rate(expectedForwards[i])
                               << "\t" << io::rate(errors[i])
                               << "; discrepancy = "
                               << forwardStdDevs[i]
                               << "\n");
        }
        for (i=0; i<N; ++i) {
            BOOST_TEST_MESSAGE(io::ordinal(i+1) << "\t"
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



void checkNormalForwardsAndOptionlets(const SequenceStatisticsInc& stats,
                                      const std::vector<Rate>& forwardStrikes,
                                      const std::vector<ext::shared_ptr<PlainVanillaPayoff> >& displacedPayoffs,
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
        BOOST_TEST_MESSAGE(config);
        Size i;
        for (i=0; i<N; ++i) {
            BOOST_TEST_MESSAGE(io::ordinal(i+1) << " forward: "
                               << io::rate(results[i])
                               << " +- " << io::rate(errors[i])
                               << "; expected: " << io::rate(expectedForwards[i])
                               << "; discrepancy = "
                               << forwardStdDevs[i]
                               << " standard errors");
        }
        for (i=0; i<N; ++i) {
            BOOST_TEST_MESSAGE(io::ordinal(i+1) << " caplet: "
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



void checkCallableSwap(const SequenceStatisticsInc& stats,
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
        BOOST_TEST_MESSAGE(config);  // detailed error info below
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
        BOOST_TEST_MESSAGE(std::setprecision(2) <<
                           "    payer swap:        " << io::rate(payerNPV) << " +/- " << io::rate(stats.errorEstimate()[0]) <<
                           "\n    receiver swap:     " << io::rate(receiverNPV) << " +/- " << io::rate(stats.errorEstimate()[1]) <<
                           "\n    bermudan:          " << io::rate(bermudanNPV) << " +/- " << io::rate(stats.errorEstimate()[2]) <<
                           "\n    receiver+bermudan: " << io::rate(receiverNPV+bermudanNPV) <<
                           "\n    callable:          " << io::rate(callableNPV) << " +/- " << io::rate(stats.errorEstimate()[3]));
    }
}


BOOST_AUTO_TEST_CASE(testOneStepForwardsAndOptionlets) {

    BOOST_TEST_MESSAGE("Testing exact repricing of "
                       "one-step forwards and optionlets "
                       "in a lognormal forward rate market model...");

    setup();

    std::vector<Rate> forwardStrikes(todaysForwards.size());
    std::vector<ext::shared_ptr<Payoff> > optionletPayoffs(todaysForwards.size());
    std::vector<ext::shared_ptr<StrikedTypePayoff> >
        displacedPayoffs(todaysForwards.size());
    for (Size i=0; i<todaysForwards.size(); ++i) {
        forwardStrikes[i] = todaysForwards[i] + 0.01;
        optionletPayoffs[i] = ext::shared_ptr<Payoff>(new
            PlainVanillaPayoff(Option::Call, todaysForwards[i]));
        displacedPayoffs[i] = ext::shared_ptr<StrikedTypePayoff>(new
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
    for (auto& j : marketModels) {

        // one step must be always full factors
        Size testedFactors[] = {todaysForwards.size()};
        for (unsigned long factors : testedFactors) {
            // for one step product ProductSuggested is equal to Terminal
            // for one step product MoneyMarketPlus is equal to Terminal
            MeasureType measures[] = {MoneyMarket, Terminal};
            for (auto& measure : measures) {
                std::vector<Size> numeraires = makeMeasure(product, measure);

                bool logNormal = true;
                ext::shared_ptr<MarketModel> marketModel =
                    makeMarketModel(logNormal, evolution, factors, j);

                EvolverType evolvers[] = {Pc, Balland, Ipc};
                ext::shared_ptr<MarketModelEvolver> evolver;
                Size stop = isInTerminalMeasure(evolution, numeraires) ? 0 : 1;
                for (Size i = 0; i < LENGTH(evolvers) - stop; i++) {

                    for (Size n = 0; n < 1; n++) {
                        MTBrownianGeneratorFactory generatorFactory(seed_);
                        // SobolBrownianGeneratorFactory generatorFactory(
                        //    SobolBrownianGenerator::Diagonal, seed_);

                        evolver = makeMarketModelEvolver(marketModel, numeraires, generatorFactory,
                                                         evolvers[i]);
                        std::ostringstream config;
                        config << marketModelTypeToString(j) << ", " << factors
                               << (factors > 1 ?
                                       (factors == todaysForwards.size() ? " (full) factors, " :
                                                                           " factors, ") :
                                       " factor,")
                               << measureTypeToString(measure) << ", "
                               << evolverTypeToString(evolvers[i]) << ", "
                               << "MT BGF";
                        if (printReport_)
                            BOOST_TEST_MESSAGE("    " << config.str());

                        ext::shared_ptr<SequenceStatisticsInc> stats = simulate(evolver, product);
                        checkForwardsAndOptionlets(*stats, forwardStrikes, displacedPayoffs,
                                                   config.str());
                    }
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testOneStepNormalForwardsAndOptionlets) {

    BOOST_TEST_MESSAGE("Testing exact repricing of "
                       "one-step forwards and optionlets "
                       "in a normal forward rate market model...");

    setup();

    std::vector<Rate> forwardStrikes(todaysForwards.size());
    std::vector<ext::shared_ptr<Payoff> > optionletPayoffs(todaysForwards.size());
    std::vector<ext::shared_ptr<PlainVanillaPayoff> >
        displacedPayoffs(todaysForwards.size());
    for (Size i=0; i<todaysForwards.size(); ++i) {
        forwardStrikes[i] = todaysForwards[i] + 0.01;
        optionletPayoffs[i] = ext::shared_ptr<Payoff>(new
            PlainVanillaPayoff(Option::Call, todaysForwards[i]));
        displacedPayoffs[i] = ext::make_shared<PlainVanillaPayoff>(Option::Call, todaysForwards[i]+displacement);
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
    for (auto& j : marketModels) {

        // one step must be always full factors
        Size testedFactors[] = {todaysForwards.size()};
        for (unsigned long factors : testedFactors) {
            // for one step product ProductSuggested is equal to Terminal
            // for one step product MoneyMarketPlus is equal to Terminal
            MeasureType measures[] = {MoneyMarket, Terminal};
            for (auto& measure : measures) {
                std::vector<Size> numeraires = makeMeasure(product, measure);

                bool logNormal = false;
                ext::shared_ptr<MarketModel> marketModel =
                    makeMarketModel(logNormal, evolution, factors, j);

                EvolverType evolvers[] = {NormalPc};
                ext::shared_ptr<MarketModelEvolver> evolver;
                Size stop = isInTerminalMeasure(evolution, numeraires) ? 0 : 1;
                for (Size i = 0; i < LENGTH(evolvers) - stop; i++) {

                    for (Size n = 0; n < 1; n++) {
                        MTBrownianGeneratorFactory generatorFactory(seed_);
                        // SobolBrownianGeneratorFactory generatorFactory(
                        //    SobolBrownianGenerator::Diagonal, seed_);

                        evolver = makeMarketModelEvolver(marketModel, numeraires, generatorFactory,
                                                         evolvers[i]);
                        std::ostringstream config;
                        config << marketModelTypeToString(j) << ", " << factors
                               << (factors > 1 ?
                                       (factors == todaysForwards.size() ? " (full) factors, " :
                                                                           " factors, ") :
                                       " factor,")
                               << measureTypeToString(measure) << ", "
                               << evolverTypeToString(evolvers[i]) << ", "
                               << "MT BGF";
                        if (printReport_)
                            BOOST_TEST_MESSAGE("    " << config.str());

                        ext::shared_ptr<SequenceStatisticsInc> stats = simulate(evolver, product);
                        checkNormalForwardsAndOptionlets(*stats, forwardStrikes, displacedPayoffs,
                                                         config.str());
                    }
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testInverseFloater) {

    BOOST_TEST_MESSAGE("Testing exact repricing of "
                       "inverse floater "
                       "in forward rate market model...");

    setup();


    std::vector<Real> fixedStrikes(accruals.size(), 0.1);
    std::vector<Real> fixedMultipliers(accruals.size(), 2.0);
    std::vector<Real> floatingSpreads(accruals.size(), 0.002);
    std::vector<Real> fixedAccruals(accruals);
    std::vector<Real> floatingAccruals(accruals);

    bool payer = true;

    MultiStepInverseFloater product(rateTimes,
                                    fixedAccruals,
                                    floatingAccruals,
                                    fixedStrikes,
                                    fixedMultipliers, 
                                    floatingSpreads,
                                    paymentTimes,
                                    payer);

    MarketModelPathwiseInverseFloater productPathwise(rateTimes,
                                                      fixedAccruals,
                                                      floatingAccruals,
                                                      fixedStrikes,
                                                      fixedMultipliers, 
                                                      floatingSpreads,
                                                      paymentTimes,
                                                      payer);

    MultiProductPathwiseWrapper productWrapped(productPathwise);

    MultiProductComposite productComposite;
    productComposite.add(product);
    productComposite.add(productWrapped);
    productComposite.finalize();

    
    EvolutionDescription evolution = productComposite.evolution();

    MarketModelType marketModels[] = {
        // CalibratedMM,
        ExponentialCorrelationFlatVolatility,
        ExponentialCorrelationAbcdVolatility };
    for (auto& j : marketModels) {

        Size testedFactors[] = {std::min<Size>(todaysForwards.size(), 3)};
        for (unsigned long factors : testedFactors) {
            MeasureType measures[] = {MoneyMarket};
            for (auto& measure : measures) {
                std::vector<Size> numeraires = makeMeasure(product, measure);

                bool logNormal = false;
                ext::shared_ptr<MarketModel> marketModel =
                    makeMarketModel(logNormal, evolution, factors, j);

                EvolverType evolvers[] = {Pc};
                ext::shared_ptr<MarketModelEvolver> evolver;

                for (auto& i : evolvers) {


                    MTBrownianGeneratorFactory generatorFactory(seed_);
                    // SobolBrownianGeneratorFactory generatorFactory(
                    //    SobolBrownianGenerator::Diagonal, seed_);

                    evolver = makeMarketModelEvolver(marketModel, numeraires, generatorFactory, i);
                    std::ostringstream config;
                    config << marketModelTypeToString(j) << ", " << factors
                           << (factors > 1 ?
                                   (factors == todaysForwards.size() ? " (full) factors, " :
                                                                       " factors, ") :
                                   " factor,")
                           << measureTypeToString(measure) << ", " << evolverTypeToString(i) << ", "
                           << "MT BGF";
                    if (printReport_)
                        BOOST_TEST_MESSAGE("    " << config.str());

                    ext::shared_ptr<SequenceStatisticsInc> stats =
                        simulate(evolver, productComposite);

                    std::vector<Real> modelVolatilities(accruals.size());
                    for (Size i = 0; i < accruals.size(); ++i)
                        modelVolatilities[i] = sqrt(marketModel->totalCovariance(i)[i][i]);


                    Real truePrice = 0.0;

                    for (Size i = 0; i < accruals.size(); ++i) {
                        Real floatingCouponPV = floatingAccruals[i] *
                                                (todaysForwards[i] + floatingSpreads[i]) *
                                                todaysDiscounts[i + 1];
                        Real inverseCouponPV =
                            2 * fixedAccruals[i] * todaysDiscounts[i + 1] *
                            blackFormula(Option::Put, fixedStrikes[i] / 2.0, todaysForwards[i],
                                         modelVolatilities[i]);

                        truePrice += floatingCouponPV - inverseCouponPV;
                    }


                    Real priceError = stats->mean()[0] - truePrice;
                    Real priceSD = stats->errorEstimate()[0];

                    Real errorInSds = priceError / priceSD;
                    if (fabs(errorInSds) > 4.0)
                        BOOST_FAIL("Inverse floater product has price error equal to "
                                   << errorInSds << " sds . Price " << truePrice << " MC price "
                                   << stats->mean()[0] << " \n");

                    Real numericalTolerance = 1E-12;

                    if (fabs(stats->mean()[0] - stats->mean()[1]) > numericalTolerance)
                        BOOST_FAIL(
                            "Inverse floater and wrapper pathwise inverse floater do not agree:"
                            << stats->mean()[0] << "  " << stats->mean()[1]);


                } // evolvers
            }     // measures
        }         // factors
    }
}

void testMultiProductComposite(const MarketModelMultiProduct& product,
                               const std::vector<SubProductExpectedValues>& subProductExpectedValues,
                               const std::string& testDescription) {

    BOOST_TEST_MESSAGE("Testing exact repricing of "
                       << testDescription
                       << "in a lognormal forward rate market model...");

    setup();

    const EvolutionDescription& evolution = product.evolution();

    MarketModelType marketModels[] = {
        // CalibratedMM,
        ExponentialCorrelationFlatVolatility,
        ExponentialCorrelationAbcdVolatility };
    for (auto& j : marketModels) {

        Size testedFactors[] = {4, 8, todaysForwards.size()};
        for (unsigned long factors : testedFactors) {
            // Composite's ProductSuggested is the Terminal one
            MeasureType measures[] = {// ProductSuggested,
                Terminal, MoneyMarketPlus,
                MoneyMarket};
            for (auto& measure : measures) {
                std::vector<Size> numeraires =
                    makeMeasure(product, measure);

                bool logNormal = true;
                ext::shared_ptr<MarketModel> marketModel =
                    makeMarketModel(logNormal, evolution, factors,
                                    j);


                EvolverType evolvers[] = {Pc, Balland, Ipc};
                ext::shared_ptr<MarketModelEvolver> evolver;
                Size stop =
                    isInTerminalMeasure(evolution, numeraires) ? 0 :
                    1;
                for (Size i = 0; i < LENGTH(evolvers) - stop; i++) {

                    for (Size n = 0; n < 1; n++) {
                        // MTBrownianGeneratorFactory
                        // generatorFactory(seed_);
                        SobolBrownianGeneratorFactory
                            generatorFactory(SobolBrownianGenerator::Diagonal,
                                             seed_);

                        evolver = makeMarketModelEvolver(marketModel, numeraires,
                                                         generatorFactory, evolvers[i]);
                        std::ostringstream config;
                        config
                            << marketModelTypeToString(j) << ", "
                            << factors
                            << (factors > 1 ?
                                (factors ==
                                 todaysForwards.size() ?
                                 " (full) factors, " :
                                 " factors, ") :
                                " factor,")
                            << measureTypeToString(measure) << ", "
                            << evolverTypeToString(evolvers[i])
                            << ", "
                            << "MT BGF";
                        if (printReport_)
                            BOOST_TEST_MESSAGE("    "
                                               << config.str());

                        ext::shared_ptr<SequenceStatisticsInc>
                            stats = simulate(evolver, product);
                        checkMultiProductCompositeResults(*stats, subProductExpectedValues,
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
    subProductExpectedValues.emplace_back("Forward");
    subProductExpectedValues.back().errorThreshold = 2.50;
    for (Size i=0; i<todaysForwards.size(); ++i) {
        subProductExpectedValues.back().values.push_back((todaysForwards[i]-forwardStrikes[i])
                                                         *accruals[i]*todaysDiscounts[i+1]);
    }
}

void addOptionLets(MultiProductComposite& product,
                   std::vector<SubProductExpectedValues>& subProductExpectedValues) {

    // create the products...
    std::vector<ext::shared_ptr<Payoff> > optionletPayoffs(todaysForwards.size());
    std::vector<ext::shared_ptr<StrikedTypePayoff> >
        displacedPayoffs(todaysForwards.size());

    for (Size i=0; i<todaysForwards.size(); ++i) {
        optionletPayoffs[i] = ext::shared_ptr<Payoff>(new
                                                      PlainVanillaPayoff(Option::Call, todaysForwards[i]));
        //CashOrNothingPayoff(Option::Call, todaysForwards[i], 0.01));
        displacedPayoffs[i] = ext::shared_ptr<StrikedTypePayoff>(new
                                                                 PlainVanillaPayoff(Option::Call, todaysForwards[i]+displacement));
        //CashOrNothingPayoff(Option::Call, todaysForwards[i]+displacement, 0.01));
    }

    MultiStepOptionlets optionlets(rateTimes, accruals,
                                   paymentTimes, optionletPayoffs);
    product.add(optionlets);

    // computing and storing expected values
    subProductExpectedValues.emplace_back("Caplet");
    subProductExpectedValues.back().errorThreshold = 2.50;
    for (Size i=0; i<todaysForwards.size(); ++i) {
        subProductExpectedValues.back().values.push_back(BlackCalculator(displacedPayoffs[i],
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
    subProductExpectedValues.emplace_back("coinitial swap");
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

    std::vector<ext::shared_ptr<StrikedTypePayoff> > payoffs(todaysForwards.size());
    for (Size i = 0; i < payoffs.size(); ++i)
        payoffs[i] = ext::shared_ptr<StrikedTypePayoff>(new
                                                        PlainVanillaPayoff(Option::Call, todaysForwards[i]));

    MultiStepCoterminalSwaptions swaptions(rateTimes,
                                           rateTimes, payoffs);
    product.add(swaps);
    product.add(swaptions);

    subProductExpectedValues.emplace_back("coterminal swap");
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
    subProductExpectedValues.emplace_back(
                                          "coterminal swaption");
    subProductExpectedValues.back().testBias = false;
    subProductExpectedValues.back().errorThreshold = 2.32;
    const Spread displacement = 0;
    Matrix jacobian =
        SwapForwardMappings::coterminalSwapZedMatrix(curveState, displacement);
    bool logNormal = true;

    EvolutionDescription evolution = productClone.evolution();
    Size factors = todaysForwards.size();
    MarketModelType marketModelType = ExponentialCorrelationFlatVolatility;
    ext::shared_ptr<MarketModel> marketModel =
        makeMarketModel(logNormal, evolution, factors, marketModelType);
    for (Size i=0; i<todaysForwards.size(); ++i) {
        const Matrix& forwardsCovariance = marketModel->totalCovariance(i);
        Matrix cotSwapsCovariance =
            jacobian * forwardsCovariance * transpose(jacobian);
        ext::shared_ptr<PlainVanillaPayoff> payoff(new PlainVanillaPayoff(Option::Call, todaysForwards[i]+displacement));

        Real expectedSwaption =
            BlackCalculator(payoff,
                            todaysCoterminalSwapRates[i]+displacement,
                            std::sqrt(cotSwapsCovariance[i][i]),
                            curveState.coterminalSwapAnnuity(0,i) *
                            todaysDiscounts[0]).value();
        subProductExpectedValues.back().values.push_back(expectedSwaption);
    }
}

BOOST_AUTO_TEST_CASE(testAllMultiStepProducts, *precondition(if_speed(Slow))) {
    std::string testDescription = "all multi-step products ";

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

BOOST_AUTO_TEST_CASE(testPeriodAdapter) {

    BOOST_TEST_MESSAGE("Testing period-adaptation routines in LIBOR market model...");

    setup();
    LMMCurveState cs(rateTimes);
    cs.setOnForwardRates(todaysForwards);

    Size period=2;
    Size offset =0;

    LMMCurveState bigRateCS(
        ForwardForwardMappings::RestrictCurveState(cs,
        period,
        offset
        ));

    std::vector<Time> swaptionPaymentTimes(bigRateCS.rateTimes());
    swaptionPaymentTimes.pop_back();

    std::vector<Time> capletPaymentTimes(swaptionPaymentTimes);


    Size numberBigRates = bigRateCS.numberOfRates();

    std::vector<ext::shared_ptr<StrikedTypePayoff> > optionletPayoffs(numberBigRates);
    std::vector<ext::shared_ptr<StrikedTypePayoff> > swaptionPayoffs(numberBigRates);
    std::vector<ext::shared_ptr<StrikedTypePayoff> > displacedOptionletPayoffs(numberBigRates);
    std::vector<ext::shared_ptr<StrikedTypePayoff> > displacedSwaptionPayoffs(numberBigRates);

    for (Size i=0; i<numberBigRates; ++i)
    {
        optionletPayoffs[i] = ext::shared_ptr<StrikedTypePayoff>(new
            PlainVanillaPayoff(Option::Call, bigRateCS.forwardRate(i)));
        swaptionPayoffs[i] = ext::shared_ptr<StrikedTypePayoff>(new
            PlainVanillaPayoff(Option::Call, bigRateCS.coterminalSwapRate(i)));
        displacedOptionletPayoffs[i] = ext::shared_ptr<StrikedTypePayoff>(new
            PlainVanillaPayoff(Option::Call, bigRateCS.forwardRate(i)+displacement));
        displacedSwaptionPayoffs[i] = ext::shared_ptr<StrikedTypePayoff>(new
            PlainVanillaPayoff(Option::Call, bigRateCS.coterminalSwapRate(i)+displacement));

    }

    MultiStepPeriodCapletSwaptions theProduct(rateTimes,
        capletPaymentTimes,
        swaptionPaymentTimes,
        optionletPayoffs,
        swaptionPayoffs,
        period,
        offset);

    const EvolutionDescription& evolution(theProduct.evolution());

    bool logNormal = true;
    Size factors = 5;

    ext::shared_ptr<MarketModel> originalModel =
        makeMarketModel(logNormal,
        evolution,
        factors,
        ExponentialCorrelationAbcdVolatility);

    std::vector<Spread> newDisplacements;

    ext::shared_ptr<MarketModel> adaptedforwardModel(new FwdPeriodAdapter(originalModel,
        period,
        offset,
        newDisplacements));

    ext::shared_ptr<MarketModel> adaptedSwapModel(new
        FwdToCotSwapAdapter(adaptedforwardModel));

    Matrix finalForwardCovariances(adaptedforwardModel->totalCovariance(adaptedforwardModel->numberOfSteps()-1));
    Matrix finalSwapCovariances(adaptedSwapModel->totalCovariance(adaptedSwapModel->numberOfSteps()-1));

    std::vector<Volatility> adaptedForwardSds(adaptedforwardModel->numberOfRates());
    std::vector<Volatility> adaptedSwapSds(adaptedSwapModel->numberOfRates());
    std::vector<Real> approxCapletPrices(adaptedforwardModel->numberOfRates());
    std::vector<Real> approxSwaptionPrices(adaptedSwapModel->numberOfRates());

    for (Size j=0; j < adaptedSwapModel->numberOfRates(); ++j)
    {
        adaptedForwardSds[j] = sqrt(finalForwardCovariances[j][j]);
        adaptedSwapSds[j] = sqrt(finalSwapCovariances[j][j]);

        Real capletAnnuity = todaysDiscounts[0]*bigRateCS.discountRatio(j+1,0)
            *bigRateCS.rateTaus()[j];

        approxCapletPrices[j] = BlackCalculator(displacedOptionletPayoffs[j],
            bigRateCS.forwardRate(j)+displacement,
            adaptedForwardSds[j],
            capletAnnuity).value();

        Real swaptionAnnuity = todaysDiscounts[0]
        *bigRateCS.coterminalSwapAnnuity(0,j);

        approxSwaptionPrices[j] = BlackCalculator(displacedSwaptionPayoffs[j],
            bigRateCS.coterminalSwapRate(j)+displacement,
            adaptedSwapSds[j],
            swaptionAnnuity).value();
    }
    SobolBrownianGeneratorFactory generatorFactory(
        SobolBrownianGenerator::Diagonal, seed_);



    ext::shared_ptr<MarketModelEvolver> evolver = makeMarketModelEvolver(originalModel,
        theProduct.suggestedNumeraires(),
        generatorFactory,
        Pc);

    ext::shared_ptr<SequenceStatisticsInc> stats =
        simulate(evolver, theProduct);

    std::vector<Real> results = stats->mean();
    std::vector<Real> errors = stats->errorEstimate();

    std::vector<Real> capletErrorsInSds(numberBigRates);
    std::vector<Real> swaptionErrorsInSds(numberBigRates);

    if (2*numberBigRates != results.size())
        BOOST_ERROR("mismatch between the size of the result and the \
                    number of results");

    for (Size i=0; i < numberBigRates; ++i)
    {
        capletErrorsInSds[i]= (results[i]-approxCapletPrices[i])/errors[i];
        swaptionErrorsInSds[i]= (results[i+numberBigRates]-approxSwaptionPrices[i])/errors[i+numberBigRates];
    }

    Real capletTolerance = 4;
    Real swaptionTolerance = 4;


    for (Size i=0; i < numberBigRates; ++i) {
        if (fabs(capletErrorsInSds[i]) > capletTolerance) {
            BOOST_FAIL(io::ordinal(i+1) << "caplet , approx price " <<
                approxCapletPrices[i] <<
                ", \t simulation price " << results[i] <<
                ", \t error in sds " << capletErrorsInSds[i]);
        }
    }
    for (Size i=0; i < numberBigRates; ++i) {
        if (fabs(swaptionErrorsInSds[i]) > swaptionTolerance) {
            BOOST_FAIL(io::ordinal(i+1) << "swaption, approx price " <<
                approxSwaptionPrices[i] <<
                ", \t simulation price " << results[i+numberBigRates] <<
                ", \t error in sds " << swaptionErrorsInSds[i]);
        }
    }
}

BOOST_AUTO_TEST_CASE(testCallableSwapNaif, *precondition(if_speed(Slow))) {

    BOOST_TEST_MESSAGE("Pricing callable swap with naif exercise strategy in a LIBOR market model...");

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

    const EvolutionDescription& evolution = dummyProduct.evolution();

    MarketModelType marketModels[] = {
        // CalibratedMM,
        ExponentialCorrelationFlatVolatility,
        ExponentialCorrelationAbcdVolatility };
    for (auto& j : marketModels) {

        Size testedFactors[] = {4, // 8,
                                todaysForwards.size()};
        for (unsigned long factors : testedFactors) {
            // Composite's ProductSuggested is the Terminal one
            MeasureType measures[] = {
                // ProductSuggested,
                MoneyMarketPlus
                // MoneyMarket,
                // Terminal
            };
            for (auto& measure : measures) {
                std::vector<Size> numeraires = makeMeasure(dummyProduct, measure);

                bool logNormal = true;
                ext::shared_ptr<MarketModel> marketModel =
                    makeMarketModel(logNormal, evolution, factors, j);


                EvolverType evolvers[] = {Pc, Balland, Ipc};
                ext::shared_ptr<MarketModelEvolver> evolver;
                Size stop = isInTerminalMeasure(evolution, numeraires) ? 0 : 1;
                for (Size i = 0; i < LENGTH(evolvers) - stop; i++) {

                    for (Size n = 0; n < 1; n++) {
                        // MTBrownianGeneratorFactory generatorFactory(seed_);
                        SobolBrownianGeneratorFactory generatorFactory(
                            SobolBrownianGenerator::Diagonal, seed_);

                        evolver = makeMarketModelEvolver(marketModel, numeraires, generatorFactory,
                                                         evolvers[i]);
                        std::ostringstream config;
                        config << marketModelTypeToString(j) << ", " << factors
                               << (factors > 1 ?
                                       (factors == todaysForwards.size() ? " (full) factors, " :
                                                                           " factors, ") :
                                       " factor,")
                               << measureTypeToString(measure) << ", "
                               << evolverTypeToString(evolvers[i]) << ", "
                               << "MT BGF";
                        if (printReport_)
                            BOOST_TEST_MESSAGE("    " << config.str());

                        // use the naif strategy

                        // 2. bermudan swaption to enter into the payer swap
                        CallSpecifiedMultiProduct bermudanProduct = CallSpecifiedMultiProduct(
                            MultiStepNothing(evolution), naifStrategy, payerSwap);

                        // 3. callable receiver swap
                        CallSpecifiedMultiProduct callableProduct = CallSpecifiedMultiProduct(
                            receiverSwap, naifStrategy, ExerciseAdapter(nullRebate));

                        // lower bound: evolve all 4 products togheter
                        MultiProductComposite allProducts;
                        allProducts.add(payerSwap);
                        allProducts.add(receiverSwap);
                        allProducts.add(bermudanProduct);
                        allProducts.add(callableProduct);
                        allProducts.finalize();

                        ext::shared_ptr<SequenceStatisticsInc> stats =
                            simulate(evolver, allProducts);
                        checkCallableSwap(*stats, config.str());


                        // upper bound

                        // MTBrownianGeneratorFactory uFactory(seed_+142);
                        SobolBrownianGeneratorFactory uFactory(SobolBrownianGenerator::Diagonal,
                                                               seed_ + 142);
                        evolver =
                            makeMarketModelEvolver(marketModel, numeraires, uFactory, evolvers[i]);

                        std::vector<ext::shared_ptr<MarketModelEvolver> > innerEvolvers;

                        std::valarray<bool> isExerciseTime =
                            isInSubset(evolution.evolutionTimes(), naifStrategy.exerciseTimes());
                        for (Size s = 0; s < isExerciseTime.size(); ++s) {
                            if (isExerciseTime[s]) {
                                MTBrownianGeneratorFactory iFactory(seed_ + s);
                                ext::shared_ptr<MarketModelEvolver> e = makeMarketModelEvolver(
                                    marketModel, numeraires, iFactory, evolvers[i], s);
                                innerEvolvers.push_back(e);
                            }
                        }

                        Size initialNumeraire = evolver->numeraires().front();
                        Real initialNumeraireValue = todaysDiscounts[initialNumeraire];

                        UpperBoundEngine uEngine(evolver, innerEvolvers, receiverSwap, nullRebate,
                                                 receiverSwap, nullRebate, naifStrategy,
                                                 initialNumeraireValue);
                        Statistics uStats;
                        uEngine.multiplePathValues(uStats, 255, 256);
                        Real delta = uStats.mean();
                        Real deltaError = uStats.errorEstimate();
                        if (printReport_)
                            BOOST_TEST_MESSAGE("    upper bound delta: " << io::rate(delta)
                                                                         << " +- "
                                                                         << io::rate(deltaError));
                    }
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testCallableSwapLS, *precondition(if_speed(Slow))) {

    BOOST_TEST_MESSAGE("Pricing callable swap with Longstaff-Schwartz exercise strategy in a LIBOR market model...");

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

    const EvolutionDescription& evolution = dummyProduct.evolution();

    MarketModelType marketModels[] = {
        // CalibratedMM,
        ExponentialCorrelationFlatVolatility,
        ExponentialCorrelationAbcdVolatility };
    for (auto& j : marketModels) {

        Size testedFactors[] = {4, // 8,
                                todaysForwards.size()};
        for (unsigned long factors : testedFactors) {
            // Composite's ProductSuggested is the Terminal one
            MeasureType measures[] = {
                // ProductSuggested,
                // MoneyMarketPlus,
                MoneyMarket
                // Terminal
            };
            for (auto& measure : measures) {
                std::vector<Size> numeraires = makeMeasure(dummyProduct, measure);

                bool logNormal = true;
                ext::shared_ptr<MarketModel> marketModel =
                    makeMarketModel(logNormal, evolution, factors, j);


                EvolverType evolvers[] = {Pc, Balland, Ipc};
                ext::shared_ptr<MarketModelEvolver> evolver;
                Size stop = isInTerminalMeasure(evolution, numeraires) ? 0 : 1;
                for (Size i = 0; i < LENGTH(evolvers) - stop; i++) {

                    for (Size n = 0; n < 1; n++) {
                        // MTBrownianGeneratorFactory generatorFactory(seed_);
                        SobolBrownianGeneratorFactory generatorFactory(
                            SobolBrownianGenerator::Diagonal, seed_);

                        evolver = makeMarketModelEvolver(marketModel, numeraires, generatorFactory,
                                                         evolvers[i]);
                        std::ostringstream config;
                        config << marketModelTypeToString(j) << ", " << factors
                               << (factors > 1 ?
                                       (factors == todaysForwards.size() ? " (full) factors, " :
                                                                           " factors, ") :
                                       " factor,")
                               << measureTypeToString(measure) << ", "
                               << evolverTypeToString(evolvers[i]) << ", "
                               << "MT BGF";
                        if (printReport_)
                            BOOST_TEST_MESSAGE("    " << config.str());

                        // calculate the exercise strategy
                        collectNodeData(*evolver, receiverSwap, basisSystem, nullRebate, control,
                                        trainingPaths_, collectedData);
                        genericLongstaffSchwartzRegression(collectedData, basisCoefficients);
                        LongstaffSchwartzExerciseStrategy exerciseStrategy(
                            basisSystem, basisCoefficients, evolution, numeraires, nullRebate,
                            control);

                        // 2. bermudan swaption to enter into the payer swap
                        CallSpecifiedMultiProduct bermudanProduct = CallSpecifiedMultiProduct(
                            MultiStepNothing(evolution), exerciseStrategy, payerSwap);

                        // 3. callable receiver swap
                        CallSpecifiedMultiProduct callableProduct = CallSpecifiedMultiProduct(
                            receiverSwap, exerciseStrategy, ExerciseAdapter(nullRebate));

                        // lower bound: evolve all 4 products togheter
                        MultiProductComposite allProducts;
                        allProducts.add(payerSwap);
                        allProducts.add(receiverSwap);
                        allProducts.add(bermudanProduct);
                        allProducts.add(callableProduct);
                        allProducts.finalize();

                        ext::shared_ptr<SequenceStatisticsInc> stats =
                            simulate(evolver, allProducts);
                        checkCallableSwap(*stats, config.str());


                        // upper bound

                        // MTBrownianGeneratorFactory uFactory(seed_+142);
                        SobolBrownianGeneratorFactory uFactory(SobolBrownianGenerator::Diagonal,
                                                               seed_ + 142);
                        evolver =
                            makeMarketModelEvolver(marketModel, numeraires, uFactory, evolvers[i]);

                        std::vector<ext::shared_ptr<MarketModelEvolver> > innerEvolvers;

                        std::valarray<bool> isExerciseTime = isInSubset(
                            evolution.evolutionTimes(), exerciseStrategy.exerciseTimes());
                        for (Size s = 0; s < isExerciseTime.size(); ++s) {
                            if (isExerciseTime[s]) {
                                MTBrownianGeneratorFactory iFactory(seed_ + s);
                                ext::shared_ptr<MarketModelEvolver> e = makeMarketModelEvolver(
                                    marketModel, numeraires, iFactory, evolvers[i], s);
                                innerEvolvers.push_back(e);
                            }
                        }

                        Size initialNumeraire = evolver->numeraires().front();
                        Real initialNumeraireValue = todaysDiscounts[initialNumeraire];

                        UpperBoundEngine uEngine(evolver, innerEvolvers, receiverSwap, nullRebate,
                                                 receiverSwap, nullRebate, exerciseStrategy,
                                                 initialNumeraireValue);
                        Statistics uStats;
                        uEngine.multiplePathValues(uStats, 255, 256);
                        Real delta = uStats.mean();
                        Real deltaError = uStats.errorEstimate();
                        if (printReport_)
                            BOOST_TEST_MESSAGE("    upper bound delta: " << io::rate(delta)
                                                                         << " +- "
                                                                         << io::rate(deltaError));
                    }
                }
            }
        }
    }
}

BOOST_AUTO_TEST_SUITE(CallableSwapAnderson, *precondition(if_speed(Slow)))

template <MarketModelType mmtype, Size factors>
struct slice {
    static const MarketModelType marketModelType = mmtype;
    static const Size testedFactor = factors;
};

using slices = boost::mpl::vector<
    slice<ExponentialCorrelationFlatVolatility, 4>,
    slice<ExponentialCorrelationFlatVolatility, 8>,
    slice<ExponentialCorrelationFlatVolatility, 0>,
    slice<ExponentialCorrelationAbcdVolatility, 4>,
    slice<ExponentialCorrelationAbcdVolatility, 8>,
    slice<ExponentialCorrelationAbcdVolatility, 0>
>;

BOOST_AUTO_TEST_CASE_TEMPLATE(testCallableSwapAnderson, T, slices) {

    setup();

    MarketModelType marketModelType = T::marketModelType;
    Size testedFactor = T::testedFactor != 0 ? T::testedFactor : todaysForwards.size();

    BOOST_TEST_MESSAGE("Pricing callable swap with Anderson exercise "
                       "strategy in a LIBOR market model for test factor "
                        << testedFactor << " and model type "
                        << marketModelTypeToString(marketModelType)
                        << "...");

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

    const EvolutionDescription& evolution = dummyProduct.evolution();

    Size factors = testedFactor;

    // Composite's ProductSuggested is the Terminal one
    MeasureType measures[] = { // ProductSuggested,
        // MoneyMarketPlus,
        // MoneyMarket,
        Terminal
    };
    for (auto& measure : measures) {
        std::vector<Size> numeraires = makeMeasure(dummyProduct, measure);
        bool logNormal = true;
        ext::shared_ptr<MarketModel> marketModel =
            makeMarketModel(logNormal, evolution, factors, marketModelType);
        EvolverType evolvers[] = { Pc, Balland, Ipc };
        ext::shared_ptr<MarketModelEvolver> evolver;
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
                config << marketModelTypeToString(marketModelType) << ", " << factors
                       << (factors > 1 ? (factors == todaysForwards.size() ? " (full) factors, " :
                                                                             " factors, ") :
                                         " factor,")
                       << measureTypeToString(measure) << ", " << evolverTypeToString(evolvers[i])
                       << ", "
                       << "MT BGF";
                if (printReport_)
                    BOOST_TEST_MESSAGE("    " << config.str());
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
                    BOOST_TEST_MESSAGE("    initial estimate:  " << io::rate(firstPassValue));
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
                ext::shared_ptr<SequenceStatisticsInc> stats =
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
                std::vector<ext::shared_ptr<MarketModelEvolver> >
                    innerEvolvers;
                std::valarray<bool> isExerciseTime =
                    isInSubset(evolution.evolutionTimes(),
                               exerciseStrategy.exerciseTimes());
                for (Size s=0; s < isExerciseTime.size(); ++s) {
                    if (isExerciseTime[s]) {
                        MTBrownianGeneratorFactory iFactory(seed_+s);
                        ext::shared_ptr<MarketModelEvolver> e =
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
                    BOOST_TEST_MESSAGE("    upper bound delta: " << io::rate(delta) << " +- " << io::rate(deltaError));

            }
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(testGreeks, *precondition(if_speed(Fast))) {

    BOOST_TEST_MESSAGE("Testing caplet greeks in a lognormal forward rate market model using partial proxy simulation...");

    setup();

    std::vector<ext::shared_ptr<Payoff> > payoffs(todaysForwards.size());
    std::vector<ext::shared_ptr<StrikedTypePayoff> >
        displacedPayoffs(todaysForwards.size());
    for (Size i=0; i<todaysForwards.size(); ++i) {
        payoffs[i] = ext::shared_ptr<Payoff>(new
            //PlainVanillaPayoff(Option::Call, todaysForwards[i]));
            CashOrNothingPayoff(Option::Call, todaysForwards[i], 0.01));
        displacedPayoffs[i] = ext::shared_ptr<StrikedTypePayoff>(new
            //PlainVanillaPayoff(Option::Call, todaysForwards[i]+displacement));
            CashOrNothingPayoff(Option::Call, todaysForwards[i]+displacement, 0.01));
    }

    MultiStepOptionlets product(rateTimes, accruals,
        paymentTimes, payoffs);

    const EvolutionDescription& evolution = product.evolution();

    MarketModelType marketModels[] = {
        // CalibratedMM,
        // ExponentialCorrelationFlatVolatility,
        ExponentialCorrelationAbcdVolatility };
    for (auto& j : marketModels) {

        Size testedFactors[] = {4, 8, todaysForwards.size()};
        for (unsigned long factors : testedFactors) {
            MeasureType measures[] = {
                // MoneyMarketPlus,
                MoneyMarket //,
                // Terminal
            };
            for (auto& measure : measures) {
                std::vector<Size> numeraires = makeMeasure(product, measure);

                for (Size n = 0; n < 1; n++) {
                    // MTBrownianGeneratorFactory generatorFactory(seed_);
                    SobolBrownianGeneratorFactory generatorFactory(SobolBrownianGenerator::Diagonal,
                                                                   seed_);

                    bool logNormal = true;
                    ext::shared_ptr<MarketModel> marketModel =
                        makeMarketModel(logNormal, evolution, factors, j);

                    ext::shared_ptr<MarketModelEvolver> evolver(
                        new LogNormalFwdRateEuler(marketModel, generatorFactory, numeraires));
                    SequenceStatisticsInc stats(product.numberOfProducts());


                    std::vector<Size> startIndexOfConstraint;
                    std::vector<Size> endIndexOfConstraint;

                    for (Size i = 0; i < evolution.evolutionTimes().size(); ++i) {
                        startIndexOfConstraint.push_back(i);
                        endIndexOfConstraint.push_back(i + 1);
                    }


                    std::vector<std::vector<ext::shared_ptr<ConstrainedEvolver> > >
                        constrainedEvolvers;
                    std::vector<std::vector<std::vector<Real> > > diffWeights;
                    std::vector<std::vector<SequenceStatisticsInc> > greekStats;

                    std::vector<ext::shared_ptr<ConstrainedEvolver> > deltaGammaEvolvers;
                    std::vector<std::vector<Real> > deltaGammaWeights(2, std::vector<Real>(3));
                    std::vector<SequenceStatisticsInc> deltaGammaStats(2, stats);


                    Spread forwardBump = 1.0e-6;
                    marketModel = makeMarketModel(logNormal, evolution, factors, j, -forwardBump);
                    deltaGammaEvolvers.push_back(
                        ext::shared_ptr<ConstrainedEvolver>(new LogNormalFwdRateEulerConstrained(
                            marketModel, generatorFactory, numeraires)));
                    deltaGammaEvolvers.back()->setConstraintType(startIndexOfConstraint,
                                                                 endIndexOfConstraint);
                    marketModel = makeMarketModel(logNormal, evolution, factors, j, forwardBump);
                    deltaGammaEvolvers.push_back(
                        ext::shared_ptr<ConstrainedEvolver>(new LogNormalFwdRateEulerConstrained(
                            marketModel, generatorFactory, numeraires)));
                    deltaGammaEvolvers.back()->setConstraintType(startIndexOfConstraint,
                                                                 endIndexOfConstraint);

                    deltaGammaWeights[0][0] = 0.0;
                    deltaGammaWeights[0][1] = -1.0 / (2.0 * forwardBump);
                    deltaGammaWeights[0][2] = 1.0 / (2.0 * forwardBump);

                    deltaGammaWeights[1][0] = -2.0 / (forwardBump * forwardBump);
                    deltaGammaWeights[1][1] = 1.0 / (forwardBump * forwardBump);
                    deltaGammaWeights[1][2] = 1.0 / (forwardBump * forwardBump);


                    std::vector<ext::shared_ptr<ConstrainedEvolver> > vegaEvolvers;
                    std::vector<std::vector<Real> > vegaWeights(1, std::vector<Real>(3));
                    std::vector<SequenceStatisticsInc> vegaStats(1, stats);

                    Volatility volBump = 1.0e-4;
                    marketModel = makeMarketModel(logNormal, evolution, factors, j, 0.0, -volBump);
                    vegaEvolvers.push_back(
                        ext::shared_ptr<ConstrainedEvolver>(new LogNormalFwdRateEulerConstrained(
                            marketModel, generatorFactory, numeraires)));
                    vegaEvolvers.back()->setConstraintType(startIndexOfConstraint,
                                                           endIndexOfConstraint);
                    marketModel = makeMarketModel(logNormal, evolution, factors, j, 0.0, volBump);
                    vegaEvolvers.push_back(
                        ext::shared_ptr<ConstrainedEvolver>(new LogNormalFwdRateEulerConstrained(
                            marketModel, generatorFactory, numeraires)));
                    vegaEvolvers.back()->setConstraintType(startIndexOfConstraint,
                                                           endIndexOfConstraint);

                    vegaWeights[0][0] = 0.0;
                    vegaWeights[0][1] = -1.0 / (2.0 * volBump);
                    vegaWeights[0][2] = 1.0 / (2.0 * volBump);


                    constrainedEvolvers.push_back(deltaGammaEvolvers);
                    diffWeights.push_back(deltaGammaWeights);
                    greekStats.push_back(deltaGammaStats);

                    constrainedEvolvers.push_back(vegaEvolvers);
                    diffWeights.push_back(vegaWeights);
                    greekStats.push_back(vegaStats);

                    std::ostringstream config;
                    config << marketModelTypeToString(j) << ", " << factors
                           << (factors > 1 ?
                                   (factors == todaysForwards.size() ? " (full) factors, " :
                                                                       " factors, ") :
                                   " factor,")
                           << measureTypeToString(measure) << ", "
                           << "MT BGF";
                    if (printReport_)
                        BOOST_TEST_MESSAGE("    " << config.str());

                    Size initialNumeraire = evolver->numeraires().front();
                    Real initialNumeraireValue = todaysDiscounts[initialNumeraire];

                    ProxyGreekEngine engine(evolver, constrainedEvolvers, diffWeights,
                                            startIndexOfConstraint, endIndexOfConstraint, product,
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

                    std::vector<DiscountFactor> discPlus(todaysForwards.size() + 1,
                                                         todaysDiscounts[0]);
                    std::vector<DiscountFactor> discMinus(todaysForwards.size() + 1,
                                                          todaysDiscounts[0]);
                    std::vector<Rate> fwdPlus(todaysForwards.size());
                    std::vector<Rate> fwdMinus(todaysForwards.size());
                    std::vector<Rate> pricePlus(todaysForwards.size());
                    std::vector<Rate> price0(todaysForwards.size());
                    std::vector<Rate> priceMinus(todaysForwards.size());
                    for (Size i = 0; i < todaysForwards.size(); ++i) {
                        Time tau = rateTimes[i + 1] - rateTimes[i];
                        fwdPlus[i] = todaysForwards[i] + forwardBump;
                        fwdMinus[i] = todaysForwards[i] - forwardBump;
                        discPlus[i + 1] = discPlus[i] / (1.0 + fwdPlus[i] * tau);
                        discMinus[i + 1] = discMinus[i] / (1.0 + fwdMinus[i] * tau);
                        pricePlus[i] = BlackCalculator(displacedPayoffs[i], fwdPlus[i],
                                                       volatilities[i] * sqrt(rateTimes[i]),
                                                       discPlus[i + 1] * tau)
                                           .value();
                        price0[i] = BlackCalculator(displacedPayoffs[i], todaysForwards[i],
                                                    volatilities[i] * sqrt(rateTimes[i]),
                                                    todaysDiscounts[i + 1] * tau)
                                        .value();
                        priceMinus[i] = BlackCalculator(displacedPayoffs[i], fwdMinus[i],
                                                        volatilities[i] * sqrt(rateTimes[i]),
                                                        discMinus[i + 1] * tau)
                                            .value();
                    }

                    for (Size i = 0; i < product.numberOfProducts(); ++i) {
                        Real numDelta = (pricePlus[i] - priceMinus[i]) / (2.0 * forwardBump);
                        Real numGamma = (pricePlus[i] - 2 * price0[i] + priceMinus[i]) /
                                        (forwardBump * forwardBump);
                        if (printReport_) {
                            BOOST_TEST_MESSAGE(io::ordinal(i + 1) << " caplet: "
                                                                  << "value = " << price0[i] << ", "
                                                                  << "delta = " << numDelta << ", "
                                                                  << "gamma = " << numGamma);
                            BOOST_TEST_MESSAGE(
                                io::ordinal(i + 1)
                                << " caplet: "
                                << "value = " << values[i] << " +- " << errors[i] << " ("
                                << (values[i] - price0[i]) / errors[i] << " s.e.), "
                                << "delta = " << deltas[i] << " +- " << deltaErrors[i] << " ("
                                << (deltas[i] - numDelta) / deltaErrors[i] << " s.e.), "
                                << "gamma = " << gammas[i] << " +- " << gammaErrors[i] << " ("
                                << (gammas[i] - numGamma) / gammaErrors[i] << " s.e.), "
                                << "vega = " << vegas[i] << " +- " << vegaErrors[i]);
                        }
                    }
                }
            }
        }
    }
}

// pathwise deltas

BOOST_AUTO_TEST_CASE(testPathwiseGreeks) {

    BOOST_TEST_MESSAGE("Testing caplet deltas in a lognormal forward rate market model using pathwise method...");

    setup();


    std::vector<ext::shared_ptr<Payoff> > payoffs(todaysForwards.size());
    std::vector<ext::shared_ptr<StrikedTypePayoff> >
        displacedPayoffs(todaysForwards.size());
    for (Size i=0; i<todaysForwards.size(); ++i) {
        payoffs[i] = ext::shared_ptr<Payoff>(new
            PlainVanillaPayoff(Option::Call, todaysForwards[i]));
        //CashOrNothingPayoff(Option::Call, todaysForwards[i], 0.01));
        displacedPayoffs[i] = ext::shared_ptr<StrikedTypePayoff>(new
            PlainVanillaPayoff(Option::Call, todaysForwards[i]+displacement));
        //CashOrNothingPayoff(Option::Call, todaysForwards[i]+displacement, 0.01));
    }

    for (Size whichProduct=0; whichProduct<2; ++whichProduct)
    {
        MarketModelPathwiseMultiDeflatedCaplet product1(rateTimes, accruals,
            paymentTimes, todaysForwards);

        MarketModelPathwiseMultiCaplet product2(rateTimes, accruals,
            paymentTimes, todaysForwards);

        Clone<MarketModelPathwiseMultiProduct> product;

        if (whichProduct == 0)
            product = product2;
        else
            product = product1;


        MultiStepOptionlets productDummy(rateTimes, accruals,
            paymentTimes, payoffs);



        EvolutionDescription evolution = product->evolution();

        MarketModelType marketModels[] = {
            // CalibratedMM,
            // ExponentialCorrelationFlatVolatility,
            ExponentialCorrelationAbcdVolatility };

        for (auto& j : marketModels) {

            Size testedFactors[] = {
                2
                //, 4, 8, todaysForwards.size()
            };

            for (unsigned long factors : testedFactors) {
                MeasureType measures[] = {MoneyMarket};

                for (auto& measure : measures) {
                    std::vector<Size> numeraires = makeMeasure(productDummy, measure);

                    for (Size n = 0; n < 1; n++) {
                        MTBrownianGeneratorFactory generatorFactory(seed_);

                        bool logNormal = true;
                        ext::shared_ptr<MarketModel> marketModel =
                            makeMarketModel(logNormal, evolution, factors, j);

                        LogNormalFwdRateEuler evolver(marketModel, generatorFactory, numeraires);
                        SequenceStatisticsInc stats(product->numberOfProducts() *
                                                    (todaysForwards.size() + 1));


                        Spread forwardBump = 1.0e-6;

                        std::ostringstream config;
                        config << marketModelTypeToString(j) << ", " << factors
                               << (factors > 1 ?
                                       (factors == todaysForwards.size() ? " (full) factors, " :
                                                                           " factors, ") :
                                       " factor,")
                               << measureTypeToString(measure) << ", "
                               << "MT BGF";
                        if (printReport_)
                            BOOST_TEST_MESSAGE("    " << config.str());

                        Size initialNumeraire = evolver.numeraires().front();
                        Real initialNumeraireValue = todaysDiscounts[initialNumeraire];


                        {

                            PathwiseAccountingEngine accountingengine(
                                ext::make_shared<LogNormalFwdRateEuler>(
                                    evolver), // method relies heavily on LMM Euler
                                *product,
                                marketModel, // we need pseudo-roots and displacements
                                initialNumeraireValue);


                            accountingengine.multiplePathValues(stats, paths_);
                        }


                        std::vector<Real> valuesAndDeltas = stats.mean();
                        std::vector<Real> errors = stats.errorEstimate();

                        std::vector<Real> prices(product->numberOfProducts());
                        std::vector<Real> priceErrors(product->numberOfProducts());

                        Matrix deltas(product->numberOfProducts(), todaysForwards.size());
                        Matrix deltasErrors(product->numberOfProducts(), todaysForwards.size());
                        std::vector<Real> modelPrices(product->numberOfProducts());


                        for (Size i = 0; i < product->numberOfProducts(); ++i) {
                            prices[i] = valuesAndDeltas[i];

                            priceErrors[i] = errors[i];

                            modelPrices[i] = BlackCalculator(displacedPayoffs[i], todaysForwards[i],
                                                             volatilities[i] * sqrt(rateTimes[i]),
                                                             todaysDiscounts[i + 1] *
                                                                 (rateTimes[i + 1] - rateTimes[i]))
                                                 .value();


                            for (Size j = 0; j < todaysForwards.size(); ++j) {
                                deltas[i][j] =
                                    valuesAndDeltas[(i + 1) * product->numberOfProducts() + j];
                                deltasErrors[i][j] =
                                    errors[(i + 1) * product->numberOfProducts() + j];
                            }
                        }

                        Matrix modelDeltas(product->numberOfProducts(), todaysForwards.size());


                        std::vector<DiscountFactor> discPlus(todaysForwards.size() + 1,
                                                             todaysDiscounts[0]);
                        std::vector<DiscountFactor> discMinus(todaysForwards.size() + 1,
                                                              todaysDiscounts[0]);
                        std::vector<Rate> fwdPlus(todaysForwards.size());
                        std::vector<Rate> fwdMinus(todaysForwards.size());


                        for (Size i = 0; i < todaysForwards.size(); ++i) {
                            for (Size j = 0; j < todaysForwards.size(); ++j) {
                                if (i != j) {
                                    fwdPlus[j] = todaysForwards[j];
                                    fwdMinus[j] = todaysForwards[j];

                                } else {
                                    fwdPlus[j] = todaysForwards[j] + forwardBump;
                                    fwdMinus[j] = todaysForwards[j] - forwardBump;
                                }

                                Time tau = rateTimes[j + 1] - rateTimes[j];
                                discPlus[j + 1] = discPlus[j] / (1.0 + fwdPlus[j] * tau);
                                discMinus[j + 1] = discMinus[j] / (1.0 + fwdMinus[j] * tau);
                            }

                            for (Size k = 0; k < product->numberOfProducts(); ++k) {
                                Real tau = rateTimes[k + 1] - rateTimes[k];
                                Real priceUp = BlackCalculator(displacedPayoffs[k], fwdPlus[k],
                                                               volatilities[k] * sqrt(rateTimes[k]),
                                                               discPlus[k + 1] * tau)
                                                   .value();
                                Real priceDown =
                                    BlackCalculator(displacedPayoffs[k], fwdMinus[k],
                                                    volatilities[k] * sqrt(rateTimes[k]),
                                                    discMinus[k + 1] * tau)
                                        .value();

                                modelDeltas[k][i] = (priceUp - priceDown) / (2 * forwardBump);
                            }
                        }


                        Integer numberErrors = 0;

                        for (Size i = 0; i < product->numberOfProducts(); ++i) {

                            Real thisPrice = prices[i];
                            Real thisModelPrice = modelPrices[i];
                            Real priceErrorInSds = ((thisPrice - thisModelPrice) / priceErrors[i]);

                            Real errorTheshold = 3.5;

                            if (fabs(priceErrorInSds) > errorTheshold) {
                                BOOST_TEST_MESSAGE("Caplet "
                                                   << i << " price " << prices[i] << " model price "
                                                   << modelPrices[i]
                                                   << "   Standard error: " << priceErrors[i]
                                                   << " errors in sds: " << priceErrorInSds);

                                ++numberErrors;
                            }

                            Real threshold = 1e-10;

                            for (Size j = 0; j < todaysForwards.size(); ++j) {
                                Real delta = deltas[i][j];
                                Real modelDelta = modelDeltas[i][j];

                                Real deltaErrorInSds = 100;

                                if (deltasErrors[i][j] > 0.0)
                                    deltaErrorInSds = ((delta - modelDelta) / deltasErrors[i][j]);
                                else if (fabs(modelDelta - delta) <
                                         threshold) // to cope with zero over zero
                                    deltaErrorInSds = 0.0;

                                if (fabs(deltaErrorInSds) > errorTheshold) {

                                    BOOST_TEST_MESSAGE("Caplet "
                                                       << i << " delta " << j << "has value "
                                                       << deltas[i][j] << " model value "
                                                       << modelDeltas[i][j] << "   Standard error: "
                                                       << deltasErrors[i][j]
                                                       << " errors in sds: " << deltaErrorInSds);

                                    ++numberErrors;
                                }
                            }
                        }

                        if (numberErrors > 0)
                            BOOST_FAIL("Pathwise greeks test has " << numberErrors << "\n");
                    }
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testPathwiseVegas, *precondition(if_speed(Fast))) {

    BOOST_TEST_MESSAGE(
        "Testing pathwise vegas in a lognormal forward rate market model...");

    setup();


    std::vector<ext::shared_ptr<Payoff> > payoffs(todaysForwards.size());
    std::vector<ext::shared_ptr<StrikedTypePayoff> >
        displacedPayoffs(todaysForwards.size());
    for (Size i=0; i<todaysForwards.size(); ++i) {
        payoffs[i] = ext::shared_ptr<Payoff>(new
            PlainVanillaPayoff(Option::Call, todaysForwards[i]));
        //CashOrNothingPayoff(Option::Call, todaysForwards[i], 0.01));
        displacedPayoffs[i] = ext::shared_ptr<StrikedTypePayoff>(new
            PlainVanillaPayoff(Option::Call, todaysForwards[i]+displacement));
        //CashOrNothingPayoff(Option::Call, todaysForwards[i]+displacement, 0.01));
    }


    MultiStepOptionlets product(rateTimes, accruals,
        paymentTimes, payoffs);

    MarketModelPathwiseMultiCaplet caplets(rateTimes, accruals,
        paymentTimes, todaysForwards);


    MarketModelPathwiseMultiDeflatedCaplet capletsDeflated(rateTimes, accruals,
        paymentTimes, todaysForwards);

    LMMCurveState cs(rateTimes);
    cs.setOnForwardRates(todaysForwards);




    EvolutionDescription evolution = product.evolution();
    Size steps = evolution.numberOfSteps();
    Size numberRates = evolution.numberOfRates();

    Real bumpSizeNumericalDifferentiation = 1E-6;
    Real vegaBumpSize = 1e-2;
    Size pathsToDo =10; // for the numerical differentiation test we are requiring equality on each path so this is actually quite strict
    Size pathsToDoSimulation = paths_;
    Size bumpIncrement = 1 + evolution.numberOfSteps()/3;
    Real numericalBumpSizeForSwaptionPseudo =1E-7;

    Real multiplier = 50; // how many times the bump size squared, the numerical differentation is allowed to differ by
    // printReport_ = true;
    Real maxError =0.0;
    Size numberSwaptionPseudoFailures =0;
    Size numberCapPseudoFailures = 0;
    Size numberCapImpVolFailures = 0;
    Size numberCapVolPseudoFailures =0;
    Real swaptionPseudoTolerance = 1e-8;
    Real impVolTolerance = 1e-5;
    Real capStrike = meanForward;
    Real initialNumeraireValue =0.95;


    MarketModelType marketModels[] =
    {
        // CalibratedMM,
        // ExponentialCorrelationFlatVolatility,
        ExponentialCorrelationAbcdVolatility
    };
    /////////////////////////////////// test derivative of swaption implied vol with respect to pseudo-root elements

    for (auto& j : marketModels) {

        Size testedFactors[] = { std::min<Size>(3UL,todaysForwards.size())
            //    todaysForwards.size()
            //, 4, 8,
        };


        for (unsigned long factors : testedFactors) {
            bool logNormal = true;

            ext::shared_ptr<MarketModel> marketModel =
                makeMarketModel(logNormal, evolution, factors, j);

            Size startIndex = std::min<Size>(1,evolution.numberOfRates()-2) ;
            Size endIndex = evolution.numberOfRates()-1;

            SwaptionPseudoDerivative derivative(marketModel,
                startIndex,
                endIndex);

            std::vector<Matrix> pseudoRoots;
            for (Size k=0; k < marketModel->numberOfSteps(); ++k)
                pseudoRoots.push_back( marketModel->pseudoRoot(k));

         // test that the derivative of swaption implied vols to the pseudo-root elements are correct, finite differencing versus analytic value

            for (Size step=0; step < evolution.numberOfSteps(); ++ step)
            {
                for (Size l=0; l < evolution.numberOfRates(); ++l)
                    for (Size f=0; f < factors; ++f)
                    {

                        // change one pseudo root element in the calibration by adding a bump to it 

                        pseudoRoots[step][l][f] += numericalBumpSizeForSwaptionPseudo;


                        // create new market model with the pseudo root bumped

                        PseudoRootFacade bumpedUp(pseudoRoots,rateTimes,marketModel->initialRates(),marketModel->displacements());


                        // compute the implied vol of the swaption with the bumped pseudo roots 

                        Real upImpVol = SwapForwardMappings::swaptionImpliedVolatility(bumpedUp,
                            startIndex,
                            endIndex);


                        // undo the bump

                        pseudoRoots[step][l][f] -= numericalBumpSizeForSwaptionPseudo;


                        // bump down

                        pseudoRoots[step][l][f] -= numericalBumpSizeForSwaptionPseudo;


                        // create facade for the bumped down pseudo roots

                        PseudoRootFacade bumpedDown(pseudoRoots,rateTimes,marketModel->initialRates(),marketModel->displacements());

                       // compute the implied vol of the swaption with the bumped down pseudo roots 

                        Real downImpVol = SwapForwardMappings::swaptionImpliedVolatility(bumpedDown,
                            startIndex,
                            endIndex);

                        // undo bumping

                        pseudoRoots[step][l][f] += numericalBumpSizeForSwaptionPseudo;

                        // use symmetric finite differencing to compute the change in the swaptions implied vol for changes in this pseudo-root element

                        Real volDeriv = (upImpVol-downImpVol)/(2.0*numericalBumpSizeForSwaptionPseudo);

                        Real modelVal = derivative.volatilityDerivative(step)[l][f];

                        Real error = volDeriv - modelVal;

                        if (fabs(error) > swaptionPseudoTolerance)
                            ++numberSwaptionPseudoFailures;



                    }

            }

            if (numberSwaptionPseudoFailures >0)
                BOOST_ERROR("swaption pseudo test failed " << numberSwaptionPseudoFailures << " times" );
        }
    }

    /////////////////////////////////////

    for (auto& j : marketModels) {

        Size testedFactors[] = { std::min<Size>(3UL,todaysForwards.size())
            //    todaysForwards.size()
            //, 4, 8,
                                                          };


        for (unsigned long factors : testedFactors) {
            bool logNormal = true;

            ext::shared_ptr<MarketModel> marketModel =
                makeMarketModel(logNormal, evolution, factors, j);

            for (Size startIndex = 1; startIndex < evolution.numberOfRates()-1; ++startIndex)
                for (Size endIndex = startIndex+1; endIndex < evolution.numberOfRates(); ++endIndex)
                {

                    CapPseudoDerivative derivative(marketModel,
                        capStrike,
                        startIndex,
                        endIndex, initialNumeraireValue);

                    std::vector<Matrix> pseudoRoots;
                    for (Size k=0; k < marketModel->numberOfSteps(); ++k)
                        pseudoRoots.push_back( marketModel->pseudoRoot(k));

                    // test cap price derivatives with respect to pseudo-root elements

                    for (Size step=0; step < evolution.numberOfSteps(); ++ step)
                    {
                        for (Size l=0; l < evolution.numberOfRates(); ++l)
                            for (Size f=0; f < factors; ++f)
                            {

                                // similar to swaption pseudo derivative test but with prices not implied vols

                                pseudoRoots[step][l][f] += numericalBumpSizeForSwaptionPseudo;

                                PseudoRootFacade bumpedUp(pseudoRoots,rateTimes,marketModel->initialRates(),marketModel->displacements());

                                // get total covariances of rates with bumped up pseudo-roots , we really only need the variances
                                Matrix totalCovUp(bumpedUp.totalCovariance( marketModel->numberOfSteps()-1));


                                pseudoRoots[step][l][f] -= numericalBumpSizeForSwaptionPseudo;

                                pseudoRoots[step][l][f] -= numericalBumpSizeForSwaptionPseudo;

                                PseudoRootFacade bumpedDown(pseudoRoots,rateTimes,marketModel->initialRates(),marketModel->displacements());

                                // get total covariances of rates with bumped down pseudo-roots , we really only need the variances
                                Matrix totalCovDown(bumpedDown.totalCovariance( marketModel->numberOfSteps()-1));


                                pseudoRoots[step][l][f] += numericalBumpSizeForSwaptionPseudo;


                                // we have to loop through all the caplets underlying the cap to get the price

                                Real priceDeriv=0.0;
                                for (Size k=startIndex; k < endIndex; ++k)
                                {
                                    Real upSd = sqrt(totalCovUp[k][k]);
                                    Real downSd = sqrt(totalCovDown[k][k]);

                                    Real annuity =  todaysDiscounts[k+1]* marketModel->evolution().rateTaus()[k];
                                    Real forward = todaysForwards[k];


                                    Real upPrice = blackFormula(Option::Call,
                                        capStrike,
                                        forward,
                                        upSd,
                                        annuity,
                                        marketModel->displacements()[k]);


                                    Real downPrice = blackFormula(Option::Call,
                                        capStrike,
                                        forward,
                                        downSd,
                                        annuity,
                                        marketModel->displacements()[k]);


                                    priceDeriv += (upPrice-downPrice)/(2.0*numericalBumpSizeForSwaptionPseudo);

                                }

                                Real modelVal = derivative.priceDerivative(step)[l][f];

                                Real error = priceDeriv - modelVal;

                                if (fabs(error) > swaptionPseudoTolerance)
                                    ++numberCapPseudoFailures;



                            }

                    }

                    // test the implied vol of the cap, each underlying caplet has a different implied vol and the cap's is different again

                    Real impVol = derivative.impliedVolatility();

                    Matrix totalCov(marketModel->totalCovariance(evolution.numberOfSteps()-1 ) );
                    Real priceConstVol =0.0;
                    Real priceVarVol =0.0;

                    for (Size m= startIndex; m < endIndex; ++m)
                    {
                        Real annuity = todaysDiscounts[m+1]* marketModel->evolution().rateTaus()[m];
                        Real expiry = rateTimes[m];
                        Real forward = todaysForwards[m];

                        priceConstVol += blackFormula(Option::Call,
                            capStrike,
                            forward,
                            impVol*sqrt(expiry),
                            annuity,
                            marketModel->displacements()[m]);

                        priceVarVol += blackFormula(Option::Call,
                            capStrike,
                            forward,
                            sqrt(totalCov[m][m]),
                            annuity,
                            marketModel->displacements()[m]);

                    }

                    if (fabs(priceVarVol - priceConstVol) > impVolTolerance)
                        ++numberCapImpVolFailures;


                }

            if (numberCapPseudoFailures >0)
                BOOST_ERROR("cap pseudo test failed for prices "
                            << numberCapPseudoFailures << " times" );

            if (numberCapImpVolFailures >0)
                BOOST_ERROR("cap pseudo test failed for implied vols "
                            << numberCapImpVolFailures << " times" );
        }

        // we have tested the price derivative and the implied vol function, now the derivative of the cap implied vols
        // with respect to pseudo-root elements  

        // since we have already tested the imp vol function we use it here


        for (unsigned long factors : testedFactors) {
            bool logNormal = true;

            ext::shared_ptr<MarketModel> marketModel =
                makeMarketModel(logNormal, evolution, factors, j);

            for (Size startIndex = 1; startIndex < evolution.numberOfRates()-1; ++startIndex)
                for (Size endIndex = startIndex+1; endIndex < evolution.numberOfRates(); ++endIndex)
                {

                    CapPseudoDerivative derivative(marketModel,
                        capStrike,
                        startIndex,
                        endIndex,initialNumeraireValue);

                    std::vector<Matrix> pseudoRoots;
                    for (Size k=0; k < marketModel->numberOfSteps(); ++k)
                        pseudoRoots.push_back( marketModel->pseudoRoot(k));


                    for (Size step=0; step < evolution.numberOfSteps(); ++ step)
                    {
                        for (Size l=0; l < evolution.numberOfRates(); ++l)
                            for (Size f=0; f < factors; ++f)
                            {
                                pseudoRoots[step][l][f] += numericalBumpSizeForSwaptionPseudo;

                                PseudoRootFacade bumpedUp(pseudoRoots,rateTimes,marketModel->initialRates(),marketModel->displacements());

                                CapPseudoDerivative upDerivative(ext::shared_ptr<MarketModel>(new PseudoRootFacade(bumpedUp)),
                                    capStrike,
                                    startIndex,
                                    endIndex,initialNumeraireValue);

                                Real volUp = upDerivative.impliedVolatility();




                                pseudoRoots[step][l][f] -= numericalBumpSizeForSwaptionPseudo;

                                pseudoRoots[step][l][f] -= numericalBumpSizeForSwaptionPseudo;

                                PseudoRootFacade bumpedDown(pseudoRoots,rateTimes,marketModel->initialRates(),marketModel->displacements());

                                CapPseudoDerivative downDerivative(ext::shared_ptr<MarketModel>(new PseudoRootFacade(bumpedDown)),
                                    capStrike,
                                    startIndex,
                                    endIndex,initialNumeraireValue);


                                Real volDown = downDerivative.impliedVolatility();




                                pseudoRoots[step][l][f] += numericalBumpSizeForSwaptionPseudo;


                                Real volDeriv = (volUp-volDown)/(2.0*numericalBumpSizeForSwaptionPseudo);

                                Real modelVal = derivative.volatilityDerivative(step)[l][f];

                                Real error = volDeriv - modelVal;

                                if (fabs(error) > impVolTolerance*10)
                                    ++numberCapVolPseudoFailures;



                            }



                    }



                }

            if (numberCapVolPseudoFailures >0)
                BOOST_ERROR("cap pseudo test failed for implied vols "
                            << numberCapVolPseudoFailures << " times" );
        }
    }


    /////////////////////////////////////

    for (Size j=0; j<LENGTH(marketModels); j++)
    {

        Size testedFactors[] = { 
                                                                std::min<Size>(1UL,todaysForwards.size())
            //    todaysForwards.size()
            //, 4, 8,
                                                        
                                                            };


        for (unsigned long factors : testedFactors) {
            Size factorsToTest =
                std::min<Size>(2, factors); // doing all possible vegas is combinatorially explosive


            MeasureType measures[] = {
                                                                               MoneyMarket
                                                                       };

            std::vector<Matrix> pseudoBumps;
            std::vector<Matrix> pseudoBumpsDown;

            for (Size k=0; k < evolution.numberOfRates(); ++k)
            {
                for (Size f=0; f < factors; ++f)
                {
                    Matrix modelBump(evolution.numberOfRates(), factors,0.0);
                    modelBump[k][f] =bumpSizeNumericalDifferentiation;
                    pseudoBumps.push_back(modelBump);
                    modelBump[k][f] =-bumpSizeNumericalDifferentiation;
                    pseudoBumpsDown.push_back(modelBump);
                }
            }

            std::vector<std::vector<Matrix> > vegaBumps;

            Matrix modelBump(evolution.numberOfRates(), factors,0.0);


            for (Size l = 0; l < evolution.numberOfSteps(); ++l)
            {
                vegaBumps.emplace_back();
                for (Size k=0; k < evolution.numberOfRates(); k=k+bumpIncrement)
                {
                    for (Size f=0; f < factorsToTest; ++f)
                    {

                        for (Size m=0; m < evolution.numberOfSteps(); ++m)
                        {
                            if (l ==m && k >= l)
                                modelBump[k][f] = vegaBumpSize;

                            vegaBumps[l].push_back(modelBump);

                            modelBump[k][f] =0.0;
                        }
                    }
                }

            }


            for (auto& measure : measures) {

                std::vector<Size> numeraires = makeMeasure(product, measure);

                std::vector<RatePseudoRootJacobian> testees;
                std::vector<RatePseudoRootJacobianAllElements> testees2;

                std::vector<RatePseudoRootJacobianNumerical> testers;
                std::vector<RatePseudoRootJacobianNumerical> testersDown;


                MTBrownianGeneratorFactory generatorFactory(seed_);

                bool logNormal = true;
                ext::shared_ptr<MarketModel> marketModel =
                    makeMarketModel(logNormal, evolution, factors,
                    marketModels[j]);

                for (Size l=0; l < evolution.numberOfSteps(); ++l)
                {
                    const Matrix& pseudoRoot = marketModel->pseudoRoot(l);
                    testees.emplace_back(pseudoRoot, evolution.firstAliveRate()[l], numeraires[l],
                                         evolution.rateTaus(), pseudoBumps,
                                         marketModel->displacements());

                    testees2.emplace_back(pseudoRoot, evolution.firstAliveRate()[l], numeraires[l],
                                          evolution.rateTaus(), marketModel->displacements());


                    testers.emplace_back(pseudoRoot, evolution.firstAliveRate()[l], numeraires[l],
                                         evolution.rateTaus(), pseudoBumps,
                                         marketModel->displacements());
                    testersDown.emplace_back(pseudoRoot, evolution.firstAliveRate()[l],
                                             numeraires[l], evolution.rateTaus(), pseudoBumpsDown,
                                             marketModel->displacements());
                }




                ext::shared_ptr<BrownianGenerator> generator(generatorFactory.create(factors,
                    steps));
                LogNormalFwdRateEuler evolver(marketModel,
                    generatorFactory,
                    numeraires);


                std::vector<Real> oldRates(evolution.numberOfRates());
                std::vector<Real> newRates(evolution.numberOfRates());
                std::vector<Real> gaussians(factors);

                std::vector<Size> numberCashFlowsThisStep(product.numberOfProducts());

                std::vector<std::vector<MarketModelMultiProduct::CashFlow> > cashFlowsGenerated(product.numberOfProducts());

                for (Size i=0; i < product.numberOfProducts(); ++i)
                    cashFlowsGenerated[i].resize(product.maxNumberOfCashFlowsPerProductPerStep());

                Matrix B(pseudoBumps.size(),evolution.numberOfRates());
                Matrix B2(pseudoBumps.size(),evolution.numberOfRates());
                Matrix B3(pseudoBumps.size(),evolution.numberOfRates());
                Matrix B4(pseudoBumps.size(),evolution.numberOfRates());

                std::vector<Matrix> globalB;
                {
                    Matrix modelB(evolution.numberOfRates(), factors);
                    for (Size i=0; i < steps; ++i)
                        globalB.push_back(modelB);
                }

                std::vector<Real> oneStepDFs(evolution.numberOfRates()+1);
                oneStepDFs[0] = 1.0;


                Size numberFailures=0;
                Size numberFailures2=0;

                for (Size l=0; l < pathsToDo; ++l)
                {
                    evolver.startNewPath();
                    product.reset();
                    generator->nextPath();

                    bool done;
                    newRates = marketModel->initialRates();
                    Size currentStep =0;

                    do
                    {
                        oldRates = newRates;


                        evolver.advanceStep();
                        done = product.nextTimeStep(evolver.currentState(),
                            numberCashFlowsThisStep,
                            cashFlowsGenerated);

                        newRates = evolver.currentState().forwardRates();

                        for (Size i=1; i <= evolution.numberOfRates(); ++i)
                            oneStepDFs[i] = 1.0/(1+oldRates[i-1]*evolution.rateTaus()[i-1]);


                        generator->nextStep(gaussians);

                        testees[currentStep].getBumps(oldRates, oneStepDFs, newRates, gaussians, B);
                        testees2[currentStep].getBumps(oldRates, oneStepDFs, newRates, gaussians, globalB);
                

                        testers[currentStep].getBumps(oldRates, oneStepDFs, newRates, gaussians, B2);
                        testersDown[currentStep].getBumps(oldRates, oneStepDFs, newRates, gaussians, B3);

                        // now do make out put of allElements class into same form 

                        for (Size i1 =0; i1 < pseudoBumps.size(); ++i1)
                        {
                            Size j1=0;

                            for (; j1 < evolution.firstAliveRate()[i1]; ++j1)
                            {
                                B4[i1][j1]=0.0;
                            }
                            for (; j1 < numberRates; ++j1)
                            {
                                Real sum =0.0;

                                for (Size k1=evolution.firstAliveRate()[i1]; k1 < numberRates; ++k1)
                                    for (Size f1=0; f1 < factors; ++f1)
                                        sum += pseudoBumps[i1][k1][f1]*globalB[j1][k1][f1];

                                B4[i1][j1] =sum;

                            }
                        }



                        for (Size j=0; j < B.rows(); ++j)
                            for (Size k=0; k < B.columns(); ++k)
                            {
                                Real analytic = B[j][k]/bumpSizeNumericalDifferentiation;
                                Real analytic2 = B4[j][k]/bumpSizeNumericalDifferentiation;
                                Real numerical = (B2[j][k]-B3[j][k])/(2*bumpSizeNumericalDifferentiation);
                                Real errorSize = (analytic - numerical)/ ( bumpSizeNumericalDifferentiation*bumpSizeNumericalDifferentiation);
                                Real errorSize2 = (analytic2 - numerical)/ ( bumpSizeNumericalDifferentiation*bumpSizeNumericalDifferentiation);

                                maxError = std::max(maxError,fabs(errorSize));

                                if ( fabs( errorSize  ) > multiplier  )
                                {
                                    ++numberFailures;
                                    if (printReport_)
                                        BOOST_TEST_MESSAGE("path " << l << " step "
                                        << currentStep << " j " << j
                                        << " k " << k << " B " << B[j][k] << "  B2 " << B2[j][k]);

                                }

                                if ( fabs( errorSize2  ) > multiplier  )
                                {
                                    ++numberFailures2;
                                    if (printReport_)
                                        BOOST_TEST_MESSAGE("path " << l << " step "
                                        << currentStep << " j " << j
                                        << " k " << k << " B4 " << B4[j][k] << "  B2 " << B2[j][k]);

                                }

                            }
                        ++currentStep;
                    }
                    while (!done);

                }

                if (numberFailures >0)
                    BOOST_FAIL("Pathwise rate pseudoroot jacobian test fails : " << numberFailures <<"\n");

                
                if (numberFailures2 >0)
                    BOOST_FAIL("Pathwise rate pseudoroot jacobian all elements test fails : " << numberFailures2 <<"\n");
            } // end of k loop over measures


            // the quick test done now do a simulation test for the vegas for caplets

            Size numberDeflatedErrors =0;
            Size numberUndeflatedErrors =0;
            Real biggestError=0.0;


            for (Size deflate =0; deflate <2; ++deflate)
            {
                Clone<MarketModelPathwiseMultiProduct> productToUse;

                if (deflate ==0)
                    productToUse = caplets;
                else
                    productToUse = capletsDeflated;

                for (auto& measure : measures) {

                    std::vector<Size> numeraires = makeMeasure(product, measure);

                    MTBrownianGeneratorFactory generatorFactory(seed_);

                    bool logNormal = true;
                    ext::shared_ptr<MarketModel> marketModel =
                        makeMarketModel(logNormal, evolution, factors,
                        marketModels[j]);

                    LogNormalFwdRateEuler evolver(marketModel,
                        generatorFactory,
                        numeraires);

                    //      SequenceStatistics stats(product.numberOfProducts()*(todaysForwards.size()+1+vegaBumps[0].size()));


                    std::ostringstream config;
                    config << marketModelTypeToString(marketModels[j]) << ", " << factors
                           << (factors > 1 ?
                                   (factors == todaysForwards.size() ? " (full) factors, " :
                                                                       " factors, ") :
                                   " factor,")
                           << measureTypeToString(measure) << ", "
                           << "MT BGF";
                    if (printReport_)
                        BOOST_TEST_MESSAGE("    " << config.str());

                    Size initialNumeraire = evolver.numeraires().front();
                    Real initialNumeraireValue =
                        todaysDiscounts[initialNumeraire];

                    std::vector<Real> values;

                    std::vector<Real> errors;

                    {

                        PathwiseVegasAccountingEngine accountingengine(ext::make_shared<LogNormalFwdRateEuler>(evolver), // method relies heavily on LMM Euler
                            productToUse,
                            marketModel, // we need pseudo-roots and displacements
                            vegaBumps,
                            initialNumeraireValue);

                        accountingengine.multiplePathValues(values,errors,pathsToDoSimulation);
                    }

                    // we have computed the vegas now we have to test them against the analytic values

                    // extract into easier format




                    Matrix vegasMatrix(caplets.numberOfProducts(), vegaBumps[0].size());
                    Matrix standardErrors(vegasMatrix);
                    Matrix deltasMatrix(caplets.numberOfProducts(), numberRates);
                    Matrix deltasErrors(deltasMatrix);
                    std::vector<Real> prices(caplets.numberOfProducts());
                    std::vector<Real> priceErrors(caplets.numberOfProducts());

                    Size entriesPerProduct = 1+numberRates+vegaBumps[0].size();

                    for (Size i=0; i < caplets.numberOfProducts(); ++i)
                    {
                        prices[i] =  values[i*entriesPerProduct];
                        priceErrors[i] = errors[i*entriesPerProduct];

                        for (Size j=0; j < vegaBumps[0].size(); ++j)
                        {
                            vegasMatrix[i][j] = values[i*entriesPerProduct + numberRates+1 + j];
                            standardErrors[i][j] = errors[i*entriesPerProduct + numberRates+1 + j];
                        }
                        for (Size j=0; j < numberRates; ++j)
                        {
                            deltasMatrix[i][j] = values[i*entriesPerProduct +1 + j];
                            deltasErrors[i][j] = errors[i*entriesPerProduct +1 + j];
                        }
                    }



                    // first get the terminal vols

                    Matrix totalCovariance(marketModel->totalCovariance(marketModel->numberOfSteps()-1));


                    std::vector<Real> truePrices(caplets.numberOfProducts());

                    for (Size r =0; r < truePrices.size(); ++r)
                    {
                        truePrices[r] = BlackCalculator(displacedPayoffs[r], todaysForwards[r], sqrt(totalCovariance[r][r]),
                            todaysDiscounts[r+1]*(rateTimes[r+1]-rateTimes[r])).value();
                    }


                    for (Size b =0; b < vegaBumps[0].size(); ++b)
                    {


                        std::vector<Real> bumpedPrices(truePrices.size());
                        std::vector<Real> variances(truePrices.size(),0.0);
                        std::vector<Real> vegas(truePrices.size());


                        for (Size step = 0; step < marketModel->numberOfSteps(); ++step)
                        {
                            Matrix pseudoRoot( marketModel->pseudoRoot(step));
                            pseudoRoot += vegaBumps[step][b];

                            for (Size rate=step; rate<marketModel->numberOfRates(); ++rate)
                            {
                                Real variance = 0.0;
                                for (Size f=0; f < marketModel->numberOfFactors(); ++f)
                                    variance+= pseudoRoot[rate][f]* pseudoRoot[rate][f];

                                variances[rate]+=variance;
                            }
                        }

                        for (Size r =0; r < truePrices.size(); ++r)
                        {

                            bumpedPrices[r] = BlackCalculator(displacedPayoffs[r], todaysForwards[r], sqrt(variances[r]),
                                todaysDiscounts[r+1]*(rateTimes[r+1]-rateTimes[r])).value();

                            vegas[r] = bumpedPrices[r] - truePrices[r];

                        }


                        for (Size s=0; s  < truePrices.size(); ++s)
                        {
                            Real mcVega = vegasMatrix[s][b];
                            Real analyticVega = vegas[s];
                            Real thisError =  mcVega - analyticVega;
                            Real thisSE = standardErrors[s][b];

                            if (fabs(thisError) >  0.0)
                            {
                                Real errorInSEs = thisError/thisSE;
                                biggestError = std::max(fabs(errorInSEs),biggestError);

                                if (fabs(errorInSEs) > 4.5)
                                {
                                    if (deflate==0)
                                        ++numberUndeflatedErrors;
                                    else
                                        ++numberDeflatedErrors;
                                }
                            }

                        }


                    }



                    // for deltas and prices the pathwise vega engine should agree precisely with the pathwiseaccounting engine
                    // so lets see if it does

                    Clone<MarketModelPathwiseMultiProduct> productToUse2;

                    if (deflate ==0)
                        productToUse2 = caplets;
                    else
                        productToUse2 = capletsDeflated;


                    SequenceStatisticsInc stats(productToUse2->numberOfProducts()*(todaysForwards.size()+1));
                    {
                        PathwiseAccountingEngine accountingengine(ext::make_shared<LogNormalFwdRateEuler>(evolver), // method relies heavily on LMM Euler
                            *productToUse2,
                            marketModel, // we need pseudo-roots and displacements
                            initialNumeraireValue);

                        accountingengine.multiplePathValues(stats,pathsToDoSimulation);
                    }

                    std::vector<Real> valuesAndDeltas2 = stats.mean();
                    std::vector<Real> errors2 = stats.errorEstimate();

                    std::vector<Real> prices2(productToUse2->numberOfProducts());
                    std::vector<Real> priceErrors2(productToUse2->numberOfProducts());

                    Matrix deltas2( productToUse2->numberOfProducts(), todaysForwards.size());
                    Matrix deltasErrors2( productToUse2->numberOfProducts(), todaysForwards.size());
                    std::vector<Real> modelPrices2(productToUse2->numberOfProducts());


                    for (Size i=0; i < productToUse2->numberOfProducts(); ++i)
                    {
                        prices2[i] = valuesAndDeltas2[i];
                        priceErrors2[i] = errors2[i];

                        for (Size j=0; j <  todaysForwards.size(); ++j)
                        {
                            deltas2[i][j] = valuesAndDeltas2[(i+1)*productToUse2->numberOfProducts()+j];
                            deltasErrors2[i][j]  = errors2[(i+1)* productToUse2->numberOfProducts()+j];
                        }
                    }

                    for (Size i=0; i < productToUse2->numberOfProducts(); ++i)
                    {

                        Real priceDiff = prices2[i] - prices[i];

                        if (fabs(priceDiff) > 5*priceErrors2[i])  // two sets of standard error
                            BOOST_FAIL("pathwise accounting engine and pathwise vegas accounting engine not in perfect agreement for price.\n product " << i << ",  vega computed price: " << prices[j] << " previous price " << prices2[j] << ", deflate " << deflate << "\n" );

                        for (Size j=0; j <  todaysForwards.size(); ++j)
                        {
                            Real error = deltas2[i][j] - deltasMatrix[i][j];
                            if (fabs(error)> 5* deltasErrors2[i][j] ) // two sets of standard error
                                BOOST_FAIL("pathwise accounting engine and pathwise vegas accounting engine not in perfect agreement for dealts.\n product " << i << ", rate " << j << " vega computed delta: " << deltasMatrix[i][j] << " previous delta " << deltas2[i][j] << "\n" );
                        }
                    }
                } // end of k loop over measures
            } // end of loop over deflation


            if (numberDeflatedErrors+numberUndeflatedErrors >0)
                BOOST_FAIL("Model pathwise vega test for caplets fails : " << numberDeflatedErrors <<" deflated errors and " <<numberUndeflatedErrors <<  " undeflated errors , biggest error in SEs is " << biggestError << "\n");


            {
                //  now do a simulation test for the vegas for caps

                std::vector<VolatilityBumpInstrumentJacobian::Cap> caps;

                Rate capStrike = todaysForwards[0];

                for (Size i=0; i +2 < numberRates; i=i+3)
                {
                    VolatilityBumpInstrumentJacobian::Cap nextCap;
                    //            nextCap.startIndex_ = i;
                    //            nextCap.endIndex_ = i+3;
                    //             nextCap.strike_ = capStrike;
                    //             caps.push_back(nextCap);

                    //        nextCap.startIndex_ = i+1;
                    //      nextCap.endIndex_ = i+3;
                    //    nextCap.strike_ = capStrike;
                    //  caps.push_back(nextCap);

                    nextCap.startIndex_ = i+2;
                    nextCap.endIndex_ = i+3;
                    nextCap.strike_ = capStrike;
                    caps.push_back(nextCap);

                }

                std::vector<std::pair<Size,Size> > startsAndEnds(caps.size());

                for (Size r=0; r < caps.size(); ++r)
                {
                    startsAndEnds[r].first = caps[r].startIndex_;
                    startsAndEnds[r].second = caps[r].endIndex_;
                }

                MarketModelPathwiseMultiDeflatedCap capsDeflated(
                    rateTimes,
                    accruals,
                    paymentTimes,
                    capStrike,
                    startsAndEnds);

                for (auto& measure : measures) {

                    std::vector<Size> numeraires = makeMeasure(product, measure);

                    MTBrownianGeneratorFactory generatorFactory(seed_);
                    MTBrownianGeneratorFactory generatorFactory2(seed_);

                    bool logNormal = true;
                    ext::shared_ptr<MarketModel> marketModel =
                        makeMarketModel(logNormal, evolution, factors,
                        marketModels[j]);

                    LogNormalFwdRateEuler evolver(marketModel,
                        generatorFactory,
                        numeraires);

                     LogNormalFwdRateEuler evolver2(marketModel,
                        generatorFactory2,
                        numeraires);

                    //      SequenceStatistics stats(product.numberOfProducts()*(todaysForwards.size()+1+vegaBumps[0].size()));


                    std::ostringstream config;
                    config << marketModelTypeToString(marketModels[j]) << ", " << factors
                           << (factors > 1 ?
                                   (factors == todaysForwards.size() ? " (full) factors, " :
                                                                       " factors, ") :
                                   " factor,")
                           << measureTypeToString(measure) << ", "
                           << "MT BGF";
                    if (printReport_)
                        BOOST_TEST_MESSAGE("    " << config.str());

                    Size initialNumeraire = evolver.numeraires().front();
                    Real initialNumeraireValue =
                        todaysDiscounts[initialNumeraire];

                    std::vector<Real> values;
                    std::vector<Real> errors;

                    std::vector<Real> values2;
                    std::vector<Real> errors2;


                    {

                        PathwiseVegasOuterAccountingEngine accountingengine(ext::make_shared<LogNormalFwdRateEuler>(evolver2), // method relies heavily on LMM Euler
                            capsDeflated,
                            marketModel, // we need pseudo-roots and displacements
                            vegaBumps,
                            initialNumeraireValue);

                        accountingengine.multiplePathValues(values2,errors2,pathsToDoSimulation);
                    }

                    {

                        PathwiseVegasAccountingEngine accountingengine(ext::make_shared<LogNormalFwdRateEuler>(evolver), // method relies heavily on LMM Euler
                            capsDeflated,
                            marketModel, // we need pseudo-roots and displacements
                            vegaBumps,
                            initialNumeraireValue);

                        accountingengine.multiplePathValues(values,errors,pathsToDoSimulation);
                    }

                    // first test to see that the two implementation give the same results

                    {
                        Real tol = 1E-8;

                        Size numberMeanFailures =0;

                        for (Size i=0; i <values.size(); ++i)
                            if (fabs(values[i]-values2[i]) > tol)
                                ++numberMeanFailures;

                              if (numberMeanFailures >0)
                                  BOOST_FAIL("Comparison of Pathwise vegas accounting engine and PathwiseVegasOuterAccountingEngine yields discrepancies:" 
                                                                 << numberMeanFailures 
                                                                 << "  out of " 
                                                                 << values.size() );

                    }

                    // we have computed the vegas now we have to test them against the analytic values

                    // extract into easier format




                    Matrix vegasMatrix(capsDeflated.numberOfProducts(), vegaBumps[0].size());
                    Matrix standardErrors(vegasMatrix);
                    Size entriesPerProduct = 1+numberRates+vegaBumps[0].size();

                    for (Size i=0; i < capsDeflated.numberOfProducts(); ++i)
                        for (Size j=0; j < vegaBumps[0].size(); ++j)
                        {
                            vegasMatrix[i][j] = values[i*entriesPerProduct + numberRates+1 + j];
                            standardErrors[i][j] = errors[i*entriesPerProduct + numberRates+1 + j];
                        }


                    // first get the terminal vols

                    Matrix totalCovariance(marketModel->totalCovariance(marketModel->numberOfSteps()-1));

                    std::vector<Real> trueCapletPrices(numberRates);
                    ext::shared_ptr<StrikedTypePayoff> dispayoff( new
                        PlainVanillaPayoff(Option::Call, capStrike+displacement));

                    for (Size r =0; r < trueCapletPrices.size(); ++r)
                        trueCapletPrices[r] = BlackCalculator(dispayoff, todaysForwards[r], sqrt(totalCovariance[r][r]),
                                                              todaysDiscounts[r+1]*(rateTimes[r+1]-rateTimes[r])).value();

                    std::vector<Real> trueCapPrices(capsDeflated.numberOfProducts());
                    std::vector<Real> vegaCaps(capsDeflated.numberOfProducts());


                    for (Size s=0; s < capsDeflated.numberOfProducts(); ++s)
                    {

                        trueCapPrices[s]=0.0;

                        for (Size t= caps[s].startIndex_; t <  caps[s].endIndex_; ++t)
                            trueCapPrices[s] += trueCapletPrices[t];
                    }

                    Size numberErrors =0;


                    for (Size b =0; b < vegaBumps[0].size(); ++b)
                    {

                        std::vector<Real> bumpedCapletPrices(trueCapletPrices.size());
                        //                  std::vector<Real> bumpedCapPrices(trueCapPrices.size());

                        std::vector<Real> variances(trueCapletPrices.size(),0.0);
                        std::vector<Real> vegasCaplets(trueCapletPrices.size());

                        for (Size step = 0; step < marketModel->numberOfSteps(); ++step)
                        {
                            Matrix pseudoRoot( marketModel->pseudoRoot(step));
                            pseudoRoot += vegaBumps[step][b];

                            for (Size rate=step; rate<marketModel->numberOfRates(); ++rate)
                            {
                                Real variance = 0.0;
                                for (Size f=0; f < marketModel->numberOfFactors(); ++f)
                                    variance+= pseudoRoot[rate][f]* pseudoRoot[rate][f];

                                variances[rate]+=variance;
                            }
                        }

                        for (Size r =0; r < trueCapletPrices.size(); ++r)
                        {
                            bumpedCapletPrices[r] = BlackCalculator(dispayoff, todaysForwards[r], sqrt(variances[r]),
                                                                    todaysDiscounts[r+1]*(rateTimes[r+1]-rateTimes[r])).value();

                            vegasCaplets[r] = bumpedCapletPrices[r] - trueCapletPrices[r];
                        }

                        for (Size s=0; s < capsDeflated.numberOfProducts(); ++s)
                        {
                            vegaCaps[s]=0.0;

                            for (Size t= caps[s].startIndex_; t <  caps[s].endIndex_; ++t)
                                vegaCaps[s] += vegasCaplets[t];
                        }

                        for (Size s=0; s  < capsDeflated.numberOfProducts(); ++s)
                        {
                            Real mcVega = vegasMatrix[s][b];
                            Real analyticVega = vegaCaps[s];
                            Real thisError =  mcVega - analyticVega;
                            Real thisSE = standardErrors[s][b];

                            if (fabs(thisError) >  0.0)
                            {
                                Real errorInSEs = fabs(thisError/thisSE);

                                if (errorInSEs > 4.0)
                                    ++numberErrors;
                            }

                        }

                    }


                    if (numberErrors >0)
                        BOOST_FAIL("caps Pathwise vega test fails : " << numberErrors <<"\n");

                } // end of k loop over measures
            }
        }
    }

}

BOOST_AUTO_TEST_CASE(testPathwiseMarketVegas) {

    BOOST_TEST_MESSAGE("Testing pathwise market vegas in a lognormal forward rate market model...");

    setup();

    // specify collection of caps and swaptions and then see if their vegas are correct
    // starting by doing a set of co-terminal swaptions
    LMMCurveState cs(rateTimes);
    cs.setOnForwardRates(todaysForwards);

    std::vector<ext::shared_ptr<Payoff> > payoffs(todaysForwards.size());
    std::vector<ext::shared_ptr<StrikedTypePayoff> >
        displacedPayoffs(todaysForwards.size());
    for (Size i=0; i<todaysForwards.size(); ++i) {
        payoffs[i] = ext::shared_ptr<Payoff>(new
            PlainVanillaPayoff(Option::Call, cs.coterminalSwapRate(i)));

        displacedPayoffs[i] = ext::shared_ptr<StrikedTypePayoff>(new
            PlainVanillaPayoff(Option::Call, cs.coterminalSwapRate(i)+displacement));

    }


    MultiStepOptionlets dummyProduct(rateTimes, accruals,
        paymentTimes, payoffs);

    Real bumpSizeNumericalDifferentiation = 1E-6;

    MarketModelPathwiseCoterminalSwaptionsDeflated swaptionsDeflated(rateTimes, cs.coterminalSwapRates());
    MarketModelPathwiseCoterminalSwaptionsNumericalDeflated swaptionsDeflated2(rateTimes, cs.coterminalSwapRates(),bumpSizeNumericalDifferentiation);


    const EvolutionDescription& evolution = dummyProduct.evolution();
    Size steps = evolution.numberOfSteps();
    Size numberRates = evolution.numberOfRates();


    Size pathsToDo =10; // for the numerical differentiation test we are requiring equality on each path so this is actually quite strict
    Size pathsToDoSimulation = paths_;

    Real multiplier = 50; // how many times the bump size squared, the numerical differentation is allowed to differ by
    Real tolerance = 1E-6;

    // printReport_ = true;
    Real initialNumeraireValue =0.95;

    bool allowFactorwiseBumping = true;
    std::vector<VolatilityBumpInstrumentJacobian::Cap> caps;

    Rate capStrike = todaysForwards[0];

    for (Size i=0; i +2 < numberRates; i=i+3)
    {
        VolatilityBumpInstrumentJacobian::Cap nextCap;
        nextCap.startIndex_ = i;
        nextCap.endIndex_ = i+3;
        nextCap.strike_ = capStrike;
        caps.push_back(nextCap);
    }
    std::vector<std::pair<Size,Size> > startsAndEnds(caps.size());


    for (Size j=0; j < caps.size(); ++j)
    {
        startsAndEnds[j].first = caps[j].startIndex_;
        startsAndEnds[j].second = caps[j].endIndex_;


    }


    MarketModelPathwiseMultiDeflatedCap capsDeflated(
        rateTimes,
        accruals,
        paymentTimes,
        capStrike,
        startsAndEnds);



    std::vector<VolatilityBumpInstrumentJacobian::Swaption> swaptions(numberRates);

    for (Size i=0; i < numberRates; ++i)
    {
        swaptions[i].startIndex_ = i;
        swaptions[i].endIndex_ = numberRates;

    }



    MarketModelType marketModels[] =
    {
        // CalibratedMM,
        // ExponentialCorrelationFlatVolatility,
        ExponentialCorrelationAbcdVolatility
    };
    ///////////////////////////////////
    ///////////////////////////////////
    // test analytically first, it's faster!

    for (auto& j : marketModels) {

        Size testedFactors[] = { std::min<Size>(1UL,todaysForwards.size())
            //    todaysForwards.size()
            //, 4, 8,
        };


        for (unsigned long factors : testedFactors) {
            bool logNormal = true;

            ext::shared_ptr<MarketModel> marketModel =
                makeMarketModel(logNormal, evolution, factors, j);


            // we need to work out our bumps

            VegaBumpCollection possibleBumps(marketModel,
                allowFactorwiseBumping);

            OrthogonalizedBumpFinder  bumpFinder(possibleBumps,
                swaptions,
                caps,
                multiplier, // if vector length grows by more than this discard
                tolerance);      // if vector projection before scaling less than this discard
            std::vector<std::vector<Matrix> > theBumps;

            bumpFinder.GetVegaBumps(theBumps);

            // the bumps is now the bumps required to get a one percent implied vol in each instrumnet
            // indexed by step, instrument, pseudo-root matrix
            // if we dot product with swaption derivatives, we should get a 1% change in imp vol on the diagonal
            // and zero off it
            {
                Matrix swaptionVegasMatrix(swaptionsDeflated.numberOfProducts(), theBumps[0].size());

                for (Size i=0; i < swaptionsDeflated.numberOfProducts(); ++i)
                {
                    SwaptionPseudoDerivative thisPseudoDerivative(marketModel,
                        swaptions[i].startIndex_,
                        swaptions[i].endIndex_);


                    for (Size j=0; j <  theBumps[0].size(); ++j)
                    {
                        swaptionVegasMatrix[i][j] = 0;

                        for (Size k=0; k < steps; ++k)
                            for (Size l=0; l < numberRates; ++l)
                                for (Size m=0; m < factors; ++m)
                                    swaptionVegasMatrix[i][j] += theBumps[k][j][l][m]*thisPseudoDerivative.volatilityDerivative(k)[l][m];
                    }
                }

                Size numberDiagonalFailures = 0;
                Size offDiagonalFailures=0;

                for (Size i=0; i < swaptions.size(); ++i)
                {
                    for (Size j=0; j <  theBumps[0].size(); ++j)
                    {
                        if (i == j)
                        {
                            Real thisError = swaptionVegasMatrix[i][i] - 0.01;

                            if (fabs(thisError) > 1e-8)
                                ++numberDiagonalFailures;
                        }
                        else
                        {
                            Real thisError = swaptionVegasMatrix[i][j];
                            if (fabs(thisError) > 1e-8)
                                ++offDiagonalFailures;
                        }
                    }
                }

                if (numberDiagonalFailures + offDiagonalFailures>0 )
                    BOOST_FAIL("Pathwise market vega analytic test fails for  swaptions : " << offDiagonalFailures <<" off diagonal failures \n "
                    << " and " << numberDiagonalFailures << " on the diagonal." );
            }
            // now do the caps

            Matrix capsVegasMatrix(caps.size(), theBumps[0].size());

            for (Size i=0; i < caps.size(); ++i)
            {
                CapPseudoDerivative thisPseudoDerivative(marketModel,
                    caps[i].strike_,
                    caps[i].startIndex_,
                    caps[i].endIndex_, initialNumeraireValue
                    );


                for (Size j=0; j <  theBumps[0].size(); ++j)
                {
                    capsVegasMatrix[i][j] = 0;

                    for (Size k=0; k < steps; ++k)
                        for (Size l=0; l < numberRates; ++l)
                            for (Size m=0; m < factors; ++m)
                                capsVegasMatrix[i][j] += theBumps[k][j][l][m]*thisPseudoDerivative.volatilityDerivative(k)[l][m];
                }
            }

            Size numberDiagonalFailures = 0;
            Size offDiagonalFailures=0;

            for (Size i=0; i < caps.size(); ++i)
            {
                for (Size j=0; j <  theBumps[0].size(); ++j)
                {
                    if (i +swaptions.size()== j)
                    {
                        Real thisError = capsVegasMatrix[i][j] - 0.01;

                        if (fabs(thisError) > 1e-8)
                            ++numberDiagonalFailures;
                    }
                    else
                    {
                        Real thisError = capsVegasMatrix[i][j];
                        if (fabs(thisError) > 1e-8)
                            ++offDiagonalFailures;
                    }
                }
            }

            if (numberDiagonalFailures + offDiagonalFailures>0 )
                BOOST_FAIL("Pathwise market vega analytic test fails for caps : " << offDiagonalFailures <<" off diagonal failures \n "
                << " and " << numberDiagonalFailures << " on the diagonal." );


        } // end of  for (Size m=0; m<LENGTH(testedFactors); ++m)
    }     // end of   for (Size j=0; j<LENGTH(marketModels); j++)
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // test numerically differentiated swaptions against analytically done ones
    // we require equality on very path so we don't need many paths


    std::vector<Size> numberCashFlowsThisStep1(swaptionsDeflated.numberOfProducts());

    std::vector<std::vector<MarketModelPathwiseMultiProduct::CashFlow> > cashFlowsGenerated1(swaptionsDeflated.numberOfProducts());


    for (Size i=0; i < swaptionsDeflated.numberOfProducts(); ++i)
    {
        cashFlowsGenerated1[i].resize(swaptionsDeflated.maxNumberOfCashFlowsPerProductPerStep());
        for (Size j=0; j < swaptionsDeflated.maxNumberOfCashFlowsPerProductPerStep(); ++j)
            cashFlowsGenerated1[i][j].amount.resize(numberRates+1);
    }

    std::vector<Size> numberCashFlowsThisStep2(numberCashFlowsThisStep1);
    std::vector<std::vector<MarketModelPathwiseMultiProduct::CashFlow> >
        cashFlowsGenerated2(cashFlowsGenerated1);


    for (auto& j : marketModels) {

        Size testedFactors[] = { std::min<Size>(1UL,todaysForwards.size())
            //    todaysForwards.size()
            //, 4, 8,
        };


        for (unsigned long factors : testedFactors) {
            MTBrownianGeneratorFactory generatorFactory(seed_);

            bool logNormal = true;

            ext::shared_ptr<MarketModel> marketModel =
                makeMarketModel(logNormal, evolution, factors, j);

            LogNormalFwdRateEuler evolver1(marketModel,
                generatorFactory,swaptionsDeflated.suggestedNumeraires()
                );

            LogNormalFwdRateEuler evolver2(marketModel,
                generatorFactory,swaptionsDeflated.suggestedNumeraires()
                );

            for (Size p=0; p < pathsToDo; ++p)
            {
                evolver1.startNewPath();
                swaptionsDeflated.reset();
                evolver2.startNewPath();
                swaptionsDeflated2.reset();
                Size step =0;

                bool done;

                do
                {
                    evolver1.advanceStep();
                    done = swaptionsDeflated.nextTimeStep(evolver1.currentState(),
                        numberCashFlowsThisStep1,
                        cashFlowsGenerated1);

                    evolver2.advanceStep();
                    bool done2 = swaptionsDeflated2.nextTimeStep(evolver2.currentState(),
                        numberCashFlowsThisStep2,
                        cashFlowsGenerated2);

                    if (done != done2)
                        BOOST_FAIL("numerical swaptions derivative and swaptions disagree on termination");

                    for (Size prod = 0; prod <  swaptionsDeflated.numberOfProducts(); ++prod)
                    {
                        if (numberCashFlowsThisStep1[prod] != numberCashFlowsThisStep2[prod])
                            BOOST_FAIL("numerical swaptions derivative and swaptions disagree on number of cash flows");

                        for (Size cf =0; cf < numberCashFlowsThisStep1[prod]; ++cf)
                            for (Size rate=0; rate<= numberRates; ++rate)
                                if ( fabs(cashFlowsGenerated1[prod][cf].amount[rate] -  cashFlowsGenerated2[prod][cf].amount[rate]) > tolerance )
                                    BOOST_FAIL("numerical swaptions derivative and swaptions disagree on cash flow size. cf = " << cf <<
                                    "step " << step << ", rate " << rate << ", amount1 " << cashFlowsGenerated1[prod][cf].amount[rate]
                                << " ,amount2 " << cashFlowsGenerated2[prod][cf].amount[rate] << "\n");





                    }

                    ++step;


                }
                while (!done);



            }


        } // end of  for (Size m=0; m<LENGTH(testedFactors); ++m)
    }     // end of   for (Size j=0; j<LENGTH(marketModels); j++)

    /////////////////////////////////////

    // now time for the full simulation test
    // measure vega of each swaption with respect to itself, the other swaptions and the caps
    // should get 0.01 and 0 respectively.
    for (auto& j : marketModels) {

        Size testedFactors[] = { std::min<Size>(1UL,todaysForwards.size())
            //    todaysForwards.size()
            //, 4, 8,
        };


        for (unsigned long factors : testedFactors) {
            MTBrownianGeneratorFactory generatorFactory(seed_);

            bool logNormal = true;

            ext::shared_ptr<MarketModel> marketModel =
                makeMarketModel(logNormal, evolution, factors, j);

            LogNormalFwdRateEuler evolver(marketModel,
                generatorFactory,swaptionsDeflated.suggestedNumeraires()
                );

            Size initialNumeraire = evolver.numeraires().front();
            Real initialNumeraireValue =
                todaysDiscounts[initialNumeraire];


            // we need to work out our bumps

            VegaBumpCollection possibleBumps(marketModel,
                allowFactorwiseBumping);


            OrthogonalizedBumpFinder  bumpFinder(possibleBumps,
                swaptions,
                caps,
                multiplier, // if vector length grows by more than this discard
                tolerance);      // if vector projection before scaling less than this discard
            std::vector<std::vector<Matrix> > theBumps;

            bumpFinder.GetVegaBumps(theBumps);


            std::vector<Real> values;

            std::vector<Real> errors;

            {

                PathwiseVegasAccountingEngine
                    accountingEngine(ext::make_shared<LogNormalFwdRateEuler>(evolver),
                    swaptionsDeflated,
                    marketModel,
                    theBumps,initialNumeraireValue);


                accountingEngine.multiplePathValues(values,errors,pathsToDoSimulation);

            }

            // we now have the simulation vegas, put them in more convenient form


            Matrix vegasMatrix(swaptionsDeflated.numberOfProducts(), theBumps[0].size());
            Matrix standardErrors(vegasMatrix);
            Size entriesPerProduct = 1+numberRates+theBumps[0].size();

            for (Size i=0; i < swaptionsDeflated.numberOfProducts(); ++i)
                for (Size j=0; j < theBumps[0].size(); ++j)
                {
                    vegasMatrix[i][j] = values[i*entriesPerProduct + numberRates+1+j];
                    standardErrors[i][j] = errors[i*entriesPerProduct + numberRates+1 +j];
                }

                // we next get the model vegas for comparison

            std::vector<Real> impliedVols_(swaptions.size());

            for (Size i=0; i < swaptions.size(); ++i)
                impliedVols_[i] = SwapForwardMappings::swaptionImpliedVolatility(*marketModel,
                                                                                 swaptions[i].startIndex_,
                                                                                 swaptions[i].endIndex_);

            std::vector<Real> analyticVegas(swaptions.size());
            for (Size i=0; i < swaptions.size(); ++i)
            {
                Real swapRate = cs.coterminalSwapRates()[i];
                Real annuity =  cs.coterminalSwapAnnuity(0,i)*initialNumeraireValue;
                Real expiry = rateTimes[i];
                Real sd = impliedVols_[i]*sqrt(expiry);
                Real swapDisplacement=0.0;

                Real vega = blackFormulaVolDerivative(swapRate,
                                                      swapRate,
                                                      sd,
                                                      expiry,
                                                      annuity,
                                                      swapDisplacement);

                analyticVegas[i] = vega*0.01; // one percent move

            }

            // diagonal vegas should agree up to standard errors
            // off diagonal vegas should be zero

            Size numberDiagonalFailures = 0;
            Size offDiagonalFailures=0;


            for (Size i=0; i < swaptions.size(); ++i)
            {
                Real thisError = vegasMatrix[i][i] - analyticVegas[i];
                Real thisErrorInSds = thisError /  (standardErrors[i][i]+1E-6); // silly to penalize for tiny standard error

                if (fabs(thisErrorInSds) > 4)
                    ++numberDiagonalFailures;

            }

            for (Size i=0; i < swaptions.size(); ++i)
                for (Size j=0; j < theBumps[0].size(); ++j)
                {
                    if ( i !=j )
                    {
                        Real thisError = vegasMatrix[i][j]; // true value is zero

                        Real thisErrorInSds = thisError /  (standardErrors[i][j]+1E-6);

                        if (fabs(thisErrorInSds) > 3.5)
                            ++offDiagonalFailures;
                    }
                }

            if (offDiagonalFailures + numberDiagonalFailures >0)
                BOOST_FAIL("Pathwise market vega test fails for coterminal swaptions : " << offDiagonalFailures <<" off diagonal failures \n "
                           << " and " << numberDiagonalFailures << " on the diagonal." );


        } // end of  for (Size m=0; m<LENGTH(testedFactors); ++m)
    }     // end of   for (Size j=0; j<LENGTH(marketModels); j++)

    /////////////////////////////////////
    /////////////////////////////////////

    // now time for the full simulation test
    // measure vega of each caps with respect to itself, the swaptions and the other caps
    // should get 0.01, 0 and 0 respectively.
    for (auto& j : marketModels) {

        Size testedFactors[] = { std::min<Size>(2UL,todaysForwards.size())
            //    todaysForwards.size()
            //, 4, 8,
        };


        for (unsigned long factors : testedFactors) {
            MTBrownianGeneratorFactory generatorFactory(seed_);

            bool logNormal = true;

            ext::shared_ptr<MarketModel> marketModel =
                makeMarketModel(logNormal, evolution, factors, j);

            LogNormalFwdRateEuler evolver(marketModel,
                generatorFactory,capsDeflated.suggestedNumeraires()
                );

            Size initialNumeraire = evolver.numeraires().front();
            Real initialNumeraireValue =
                todaysDiscounts[initialNumeraire];


            // we need to work out our bumps

            VegaBumpCollection possibleBumps(marketModel,
                allowFactorwiseBumping);


            OrthogonalizedBumpFinder  bumpFinder(possibleBumps,
                swaptions,
                caps,
                multiplier, // if vector length grows by more than this discard
                tolerance);      // if vector projection before scaling less than this discard
            std::vector<std::vector<Matrix> > theBumps;

            bumpFinder.GetVegaBumps(theBumps);


            std::vector<Real> values;

            std::vector<Real> errors;

            {

                PathwiseVegasAccountingEngine
                    accountingEngine(ext::make_shared<LogNormalFwdRateEuler>(evolver),
                    capsDeflated,
                    marketModel,
                    theBumps,initialNumeraireValue);


                accountingEngine.multiplePathValues(values,errors,pathsToDoSimulation);

            }

            // we now have the simulation vegas, put them in more convenient form


            Matrix vegasMatrix(capsDeflated.numberOfProducts(), theBumps[0].size());
            Matrix standardErrors(vegasMatrix);
            Size entriesPerProduct = 1+numberRates+theBumps[0].size();


            for (Size i=0; i < capsDeflated.numberOfProducts(); ++i)
                for (Size j=0; j < theBumps[0].size(); ++j)
                {
                    vegasMatrix[i][j] = values[i*entriesPerProduct +numberRates+j+1];
                    standardErrors[i][j] = errors[i*entriesPerProduct +numberRates+j+1];
                }

                // we next get the model vegas for comparison

            std::vector<Real> impliedVols_(caps.size());


            std::vector<Real> analyticVegas(caps.size());
            for (Size i=0; i < caps.size(); ++i)
            {

                CapPseudoDerivative capPseudo(marketModel,
                                              caps[i].strike_,
                                              caps[i].startIndex_,
                                              caps[i].endIndex_, initialNumeraireValue);

                impliedVols_[i] = capPseudo.impliedVolatility();

                Real vega=0.0;

                for (Size j= caps[i].startIndex_; j< caps[i].endIndex_; ++j)
                {

                    Real forward  = cs.forwardRates()[j];
                    Real annuity =  cs.discountRatio(j+1,0)*initialNumeraireValue*accruals[j];
                    Real expiry = rateTimes[j];
                    Real sd = impliedVols_[i]*sqrt(expiry);
                    Real displacement=0.0;

                    Real capletVega = blackFormulaVolDerivative(caps[i].strike_,forward,
                                                                sd,
                                                                expiry,
                                                                annuity,
                                                                displacement);

                    vega += capletVega;
                }



                analyticVegas[i] = vega*0.01; // one percent move

            }

            // diagonal vegas should agree up to standard errors
            // off diagonal vegas should be zero

            Size numberDiagonalFailures = 0;
            Size offDiagonalFailures=0;


            for (Size i=0; i < caps.size(); ++i)
            {
                Real thisError = vegasMatrix[i][i+swaptions.size()] - analyticVegas[i];
                Real thisErrorInSds = thisError /  (standardErrors[i][i+swaptions.size()]+1E-6); // silly to penalize for tiny standard error

                if (fabs(thisErrorInSds) > 4)
                {
                    BOOST_TEST_MESSAGE(" MC cap vega: " <<vegasMatrix[i][i+swaptions.size()] << " Analytic cap vega:" << analyticVegas[i] << " Error in sds:" << thisErrorInSds << "\n");
                    ++numberDiagonalFailures;
                }

            }

            for (Size i=0; i < caps.size(); ++i)
                for (Size j=0; j < theBumps[0].size(); ++j)
                {
                    if ( i+swaptions.size() !=j )
                    {
                        Real thisError = vegasMatrix[i][j]; // true value is zero

                        Real thisErrorInSds = thisError /  (standardErrors[i][j]+1E-6);

                        if (fabs(thisErrorInSds) > 3.5)
                            ++offDiagonalFailures;
                    }
                }

            if (offDiagonalFailures + numberDiagonalFailures >0)
                BOOST_FAIL("Pathwise market vega test fails for caps: " << offDiagonalFailures <<" off diagonal failures \n "
                           << " and " << numberDiagonalFailures << " on the diagonal." );


        } // end of  for (Size m=0; m<LENGTH(testedFactors); ++m)
    }     // end of   for (Size j=0; j<LENGTH(marketModels); j++)

    /////////////////////////////////////




}




//--------------------- Volatility tests ---------------------

BOOST_AUTO_TEST_CASE(testAbcdVolatilityIntegration) {

    BOOST_TEST_MESSAGE("Testing Abcd-volatility integration...");

    setup();

    Real a = -0.0597;
    Real b =  0.1677;
    Real c =  0.5403;
    Real d =  0.1710;

    const Size N = 10;
    const Real precision = 1e-04;

    ext::shared_ptr<AbcdFunction> instVol(new AbcdFunction(a,b,c,d));
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
                        BOOST_ERROR("     T1=" << T1 << "," <<
                            "T2=" << T2 << ",\t\t" <<
                            "xMin=" << xMin << "," <<
                            "xMax=" << xMax << ",\t\t" <<
                            "analytical: " << analytical << ",\t" <<
                            "numerical:   " << numerical);
                    }
                    if (T1==T2) {
                        Real variance = instVol->variance(xMin,xMax,T1);
                        if (std::abs(analytical-variance)>1e-14) {
                            BOOST_ERROR("     T1=" << T1 << "," <<
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

BOOST_AUTO_TEST_CASE(testAbcdVolatilityCompare) {

    BOOST_TEST_MESSAGE("Testing different implementations of Abcd-volatility...");

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

    ext::shared_ptr<LmVolatilityModel> lmAbcd(
        new LmExtLinearExponentialVolModel(rateTimes,b,c,d,a));
    ext::shared_ptr<AbcdFunction> abcd(new AbcdFunction(a,b,c,d));
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

BOOST_AUTO_TEST_CASE(testAbcdVolatilityFit) {

    BOOST_TEST_MESSAGE("Testing Abcd-volatility fit...");

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

    AbcdFunction abcd(a1, b1, c1, d1);
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(testStochVolForwardsAndOptionlets) {

    BOOST_TEST_MESSAGE(
        "Testing exact repricing of "
        "forwards and optionlets "
        "in a stochastic vol displaced diffusion forward rate market model...");

    setup();

    std::vector<Rate> forwardStrikes(todaysForwards.size());
    std::vector<ext::shared_ptr<Payoff> > optionletPayoffs(todaysForwards.size());
    /* std::vector<ext::shared_ptr<PlainVanillaPayoff> >
       displacedPayoffs(todaysForwards.size()); */
    for (Size i=0; i<todaysForwards.size(); ++i)
    {
        forwardStrikes[i] = todaysForwards[i] + 0.01;
        optionletPayoffs[i] = ext::shared_ptr<Payoff>(new
            PlainVanillaPayoff(Option::Call, todaysForwards[i]));
        /* displacedPayoffs[i] = ext::shared_ptr<PlainVanillaPayoff>(new
           PlainVanillaPayoff(Option::Call, todaysForwards[i]+displacement)); */
    }

    MultiStepForwards forwards(rateTimes, accruals,
        paymentTimes, forwardStrikes);
    MultiStepOptionlets optionlets(rateTimes, accruals,
        paymentTimes, optionletPayoffs);

    MultiProductComposite product;
    product.add(forwards);
    product.add(optionlets);
    product.finalize();

    EvolutionDescription evolution = product.evolution();

    MarketModelType marketModels[] =
    {
        ExponentialCorrelationFlatVolatility
    };

    Size firstVolatilityFactor = 2;
    Size volatilityFactorStep = 2;

    Real meanLevel=1.0;
    Real reversionSpeed=1.0;

    Real volVar=1;
    Real v0=1.0;
    Size numberSubSteps=8;
    Real w1=0.5;
    Real w2=0.5;
    Real cutPoint = 1.5;

    ext::shared_ptr<MarketModelVolProcess> volProcess(new
                        SquareRootAndersen(meanLevel,
                             reversionSpeed,
                             volVar,
                             v0,
                             evolution.evolutionTimes(),
                             numberSubSteps,
                             w1,
                             w2,
                             cutPoint));

    for (auto& j : marketModels) {

        Size testedFactors[] = {1, 2, todaysForwards.size()};
        for (unsigned long factors : testedFactors) {
            MeasureType measures[] = {MoneyMarket, Terminal};

            for (auto& measure : measures) {
                std::vector<Size> numeraires = makeMeasure(product, measure);

                bool logNormal = true;
                ext::shared_ptr<MarketModel> marketModel =
                    makeMarketModel(logNormal, evolution, factors, j);


                for (Size n=0; n<1; n++)
                {
                    MTBrownianGeneratorFactory generatorFactory(seed_);

                    ext::shared_ptr<MarketModelEvolver> evolver(new SVDDFwdRatePc(marketModel,
                                          generatorFactory,
                                          volProcess,
                                          firstVolatilityFactor,
                                          volatilityFactorStep,
                                          numeraires
                                          ));


                    std::ostringstream config;
                    config << marketModelTypeToString(j) << ", " << factors
                           << (factors > 1 ?
                                   (factors == todaysForwards.size() ? " (full) factors, " :
                                                                       " factors, ") :
                                   " factor,")
                           << measureTypeToString(measure) << ", "
                           << "SVDDFwdRatePc"
                           << ", "
                           << "MT BGF";
                    if (printReport_)
                        BOOST_TEST_MESSAGE("    " << config.str());

                    ext::shared_ptr<SequenceStatisticsInc> stats =
                        simulate(evolver, product);

                    std::vector<Real> results = stats->mean();
                    std::vector<Real> errors = stats->errorEstimate();


                    // check forwards


                       for (Size i=0; i < accruals.size(); ++i)
                       {
                           Real trueValue =  todaysDiscounts[i]- todaysDiscounts[i+1]*(1+ forwardStrikes[i]*accruals[i]);
                           Real error = results[i] - trueValue;
                           Real errorSds = error/ errors[i];

                           if (fabs(errorSds) > 3.5)
                               BOOST_FAIL("error in sds: " << errorSds << " for forward " << i << " in SV LMM test. True value:" << trueValue << ", actual value: " << results[i] << " , standard error " << errors[i]);



                       }

                       for (Size i=0; i < accruals.size(); ++i)
                       {

                              Real volCoeff =  volatilities[i];
//                                  sqrt(marketModel->totalCovariance(i)[i][i]/evolution.evolutionTimes()[i]);
                              Real theta = volCoeff*volCoeff*meanLevel;
                              Real kappa = reversionSpeed;
                              Real sigma = volCoeff*volVar;
                              Real rho = 0.0;
                              Real v1 = v0*volCoeff*volCoeff;

                              ext::shared_ptr<StrikedTypePayoff> payoff(
                                              new PlainVanillaPayoff(Option::Call, forwardStrikes[i]));

                              Real trueValue = AnalyticHestonEngine(
                                  ext::make_shared<HestonModel>(
                                      ext::make_shared<HestonProcess>(
                                          Handle<YieldTermStructure>(flatRate(0.0, dayCounter)),
                                          Handle<YieldTermStructure>(flatRate(0.0, dayCounter)),
                                          Handle<Quote>(ext::make_shared<SimpleQuote>(todaysForwards[i] + displacement)),
                                          v1, kappa, theta, sigma, rho
                                      )
                                  ),
                                  AnalyticHestonEngine::Gatheral,
                                  AnalyticHestonEngine::Integration::gaussLaguerre(128)
                              ).priceVanillaPayoff(
                                  ext::make_shared<PlainVanillaPayoff>(payoff->optionType(), todaysForwards[i] + displacement),
                                  rateTimes[i]
                              );

                              trueValue *= accruals[i] * todaysDiscounts[i + 1];

                              //        trueValue =
                              //                              BlackCalculator(displacedPayoffs[i],
                              //                                                todaysForwards[i]+displacement,
                              //                                             volatilities[i]*std::sqrt(rateTimes[i]),
                              //                                              todaysDiscounts[i+1]*accruals[i]).value();


                              Real error = results[i + accruals.size()] - trueValue;
                              Real errorSds = error / errors[i];

                              if (fabs(errorSds) > 4)
                                  BOOST_FAIL("error in sds: "
                                             << errorSds << " for caplet " << i
                                             << " in SV LMM test. True value:" << trueValue
                                             << ", actual value: " << results[i + accruals.size()]
                                             << " , standard error " << errors[i]);




                       }






                }
            }
        }
    }
}



//--------------------- Other tests ---------------------

BOOST_AUTO_TEST_CASE(testDriftCalculator) {

    // Test full factor drift equivalence between compute() and
    // computeReduced()

    BOOST_TEST_MESSAGE("Testing drift calculation...");

    setup();

    Real tolerance = 1.0e-16;
    Size factors = todaysForwards.size();
    std::vector<Time> evolutionTimes(rateTimes.size()-1);
    std::copy(rateTimes.begin(), rateTimes.end()-1, evolutionTimes.begin());
    EvolutionDescription evolution(rateTimes,evolutionTimes);
    const std::vector<Real>& rateTaus = evolution.rateTaus();
    std::vector<Size> numeraires = moneyMarketPlusMeasure(evolution,
        measureOffset_);
    std::vector<Size> alive = evolution.firstAliveRate();
    Size numberOfSteps = evolutionTimes.size();
    std::vector<Real> drifts(numberOfSteps), driftsReduced(numberOfSteps);
    MarketModelType marketModels[] = {ExponentialCorrelationFlatVolatility,
        ExponentialCorrelationAbcdVolatility};
    for (auto& k : marketModels) { // loop over market models
        bool logNormal = true;
        ext::shared_ptr<MarketModel> marketModel =
            makeMarketModel(logNormal, evolution, factors, k);
        std::vector<Rate> displacements = marketModel->displacements();
        for (Size j=0; j<numberOfSteps; ++j) {     // loop over steps
            const Matrix& A = marketModel->pseudoRoot(j);
            //BOOST_TEST_MESSAGE(io::ordinal(j+1) << " pseudoroot:\n" << A);
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
                        BOOST_ERROR("MarketModel: " << marketModelTypeToString(k) << ", "
                                                    << io::ordinal(j + 1) << " step, "
                                                    << ", " << io::ordinal(h + 1) << " numeraire, "
                                                    << ", " << io::ordinal(i + 1) << " drift, "
                                                    << "\ndrift        =" << drifts[i]
                                                    << "\ndriftReduced =" << driftsReduced[i]
                                                    << "\n       error =" << error
                                                    << "\n   tolerance =" << tolerance);
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testIsInSubset) {

    // Performance test for isInSubset function (temporary)

    BOOST_TEST_MESSAGE("Testing isInSubset function...");

    setup();

    Size dim = 100;
    std::vector<Time> set, subset;
    for (Size i=0; i<dim; i++) set.push_back(i*1.0);
    for (Size i=0; i<dim; i++) subset.push_back(dim+i*1.0);
    std::valarray<bool> result = isInSubset(set, subset);
    if (printReport_) {
        for (Size i=0; i<dim; i++) {
            BOOST_TEST_MESSAGE(io::ordinal(i+1) << ":" <<
                " set[" << i << "] =  " << set[i] <<
                ", subset[" << i << "] =  " << subset[i] <<
                ", result[" << i << "] =  " << result[i]);
        }
    }
}

BOOST_AUTO_TEST_CASE(testAbcdDegenerateCases) {
    BOOST_TEST_MESSAGE("Testing abcd degenerate cases...");

    AbcdFunction f1(0.0,0.0,1.0E-15,1.0);
    AbcdFunction f2(1.0,0.0,1.0E-50,0.0);

    Real cov1 = f1.covariance(0.0,1.0,1.0,1.0);
    if (std::fabs(cov1 - 1.0) > 1E-14
        || std::isnan(cov1) || std::isinf(cov1))
        BOOST_FAIL("(a,b,c,d)=(0,0,0,1): true covariance should be 1.0, "
        << "error is " << std::fabs(cov1 - 1.0));

    Real cov2 = f2.covariance(0.0,1.0,1.0,1.0);
    if (std::fabs(cov2 - 1.0) > 1E-14
        || std::isnan(cov2) || std::isinf(cov2))
        BOOST_FAIL("(a,b,c,d)=(1,0,0,0): true covariance should be 1.0, "
        << "error is " << std::fabs(cov2 - 1.0));
}

BOOST_AUTO_TEST_CASE(testCovariance) {
    BOOST_TEST_MESSAGE("Testing market models covariance...");

    const Size n = 10;

    std::vector<Real> rateTimes;
    std::vector<Real> evolTimes1;
    std::vector<Real> evolTimes2;
    std::vector<Real> evolTimes3;
    std::vector<Real> evolTimes4;
    std::vector<std::vector<Real> > evolTimes;

    for(Size i=1;i<=n;i++) rateTimes.push_back(static_cast<Time>(i));
    evolTimes1.push_back(n-1);
    for(Size i=1;i<=n-1;i++) evolTimes2.push_back(static_cast<Time>(i));
    for(Size i=1;i<=2*n-2;i++) evolTimes3.push_back(0.5*i);
    evolTimes4.push_back(0.3);
    evolTimes4.push_back(1.3);
    evolTimes4.push_back(2.0);
    evolTimes4.push_back(4.5);
    evolTimes4.push_back(8.2);

    evolTimes.push_back(evolTimes1);
    evolTimes.push_back(evolTimes2);
    evolTimes.push_back(evolTimes3);
    evolTimes.push_back(evolTimes4);

    std::vector<std::string> evolNames;
    evolNames.emplace_back("one evolution time");
    evolNames.emplace_back("evolution times on rate fixings");
    evolNames.emplace_back("evolution times on rate fixings and midpoints between fixings");
    evolNames.emplace_back("irregular evolution times");

    std::vector<Real> ks(n-1,1.0);
    std::vector<Real> displ(n-1,0.0);
    std::vector<Real> rates(n-1,0.0);
    std::vector<Real> vols(n-1,1.0);

    Matrix c = exponentialCorrelations(rateTimes,0.5,0.2,1.0,0.0);
    ext::shared_ptr<PiecewiseConstantCorrelation> corr(
                          new TimeHomogeneousForwardCorrelation(c,rateTimes));

    std::vector<std::string> modelNames;
    modelNames.emplace_back("FlatVol");
    modelNames.emplace_back("AbcdVol");

    for(Size k=0;k<modelNames.size();k++) {
        for(Size l=0;l<evolNames.size();l++) {
            EvolutionDescription evolution(rateTimes,evolTimes[l]);
            ext::shared_ptr<MarketModel> model;
            switch(k) {
              case 0:
                model = ext::shared_ptr<MarketModel>(
                            new FlatVol(vols,corr,evolution,n-1,rates,displ));
                break;
              case 1:
                model = ext::shared_ptr<MarketModel>(
                                 new AbcdVol(1.0,0.0,1.0E-50,0.0,ks,
                                             corr,evolution,n-1,rates,displ));
                break;
              default:
                BOOST_FAIL("Unknown model " << modelNames[k]);
            }
            if (model != nullptr) {
                for(Size i=0;i<evolTimes[l].size();i++) {
                    Matrix cov = model->covariance(i);
                    Real dt = evolTimes[l][i] - (i>0 ? evolTimes[l][i-1] : 0.0);
                    for(Size x=0;x<n-1;x++) {
                        for(Size y=0;y<n-1;y++) {
                            if(std::min(rateTimes[x],rateTimes[y])>=evolTimes[l][i]
                               && fabs(cov[x][y]-c[x][y]*dt)>1.0E-14) 
                                BOOST_FAIL("Model " << modelNames[k]
                                           << " with " << evolNames[l]
                                           << ": covariance matrix in step " << i
                                           << ": true value at (" << x << "," << y
                                           << ") is " << c[x][y]*dt
                                           << " actual value is " << cov[x][y]);
                        }
                    }
                }
            }
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
