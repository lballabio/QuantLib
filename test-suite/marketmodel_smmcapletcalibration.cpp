/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Marco Bianchetti
 Copyright (C) 2007 Cristina Duminuco

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

#include "marketmodel_smmcapletcalibration.hpp"
#include "utilities.hpp"
#include <ql/marketmodels/models/swapfromfracorrelationstructure.hpp>
#include <ql/marketmodels/models/piecewiseconstantabcdvariance.hpp>
#include <ql/marketmodels/models/capletcoterminalcalibration.hpp>
#include <ql/marketmodels/products/multistep/multistepcoterminalswaps.hpp>
#include <ql/marketmodels/products/multistep/multistepcoterminalswaptions.hpp>
#include <ql/marketmodels/products/multistep/multistepswap.hpp>
#include <ql/marketmodels/products/multiproductcomposite.hpp>
#include <ql/marketmodels/accountingengine.hpp>
#include <ql/marketmodels/utilities.hpp>
#include <ql/marketmodels/evolvers/coterminalswapratepcevolver.hpp>
#include <ql/marketmodels/evolvers/forwardratepcevolver.hpp>
#include <ql/marketmodels/models/correlations.hpp>
#include <ql/marketmodels/models/expcorrflatvol.hpp>
#include <ql/marketmodels/models/expcorrabcdvol.hpp>
#include <ql/marketmodels/browniangenerators/mtbrowniangenerator.hpp>
#include <ql/marketmodels/browniangenerators/sobolbrowniangenerator.hpp>
#include <ql/marketmodels/swapforwardmappings.hpp>
#include <ql/marketmodels/curvestates/coterminalswapcurvestate.hpp>
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
#include <ql/math/convergencestatistics.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/optimization/simplex.hpp>
#include <ql/math/sequencestatistics.hpp>
#include <sstream>

#if defined(BOOST_MSVC)
#include <float.h>
//namespace { unsigned int u = _controlfp(_EM_INEXACT, _MCW_EM); }
#endif

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(MarketModelTest_smm)

#define BEGIN(x) (x+0)
#define END(x) (x+LENGTH(x))

Date todaysDate_, startDate_, endDate_;
std::vector<Time> rateTimes_;
std::vector<Real> accruals_;
Calendar calendar_;
DayCounter dayCounter_;
std::vector<Rate> todaysForwards_, todaysSwaps_;
std::vector<Real> coterminalAnnuity_;
Spread displacement_;
std::vector<DiscountFactor> todaysDiscounts_;
std::vector<Volatility> swaptionDisplacedVols_, swaptionVols_;
std::vector<Volatility> capletDisplacedVols_, capletVols_;
Real a_, b_, c_, d_;
Real longTermCorrelation_, beta_;
Size measureOffset_;
unsigned long seed_;
Size paths_, trainingPaths_;
bool printReport_ = true;

void setup() {

    // Times
    calendar_ = NullCalendar();
    todaysDate_ = Settings::instance().evaluationDate();
    //startDate = todaysDate + 5*Years;
    endDate_ = todaysDate_ + 66*Months;
    Schedule dates(todaysDate_, endDate_, Period(Semiannual),
                   calendar_, Following, Following, true, false);
    rateTimes_ = std::vector<Time>(dates.size()-1);
    accruals_ = std::vector<Real>(rateTimes_.size()-1);
    dayCounter_ = SimpleDayCounter();
    for (Size i=1; i<dates.size(); ++i)
        rateTimes_[i-1] = dayCounter_.yearFraction(todaysDate_, dates[i]);
    for (Size i=1; i<rateTimes_.size(); ++i)
        accruals_[i-1] = rateTimes_[i] - rateTimes_[i-1];

    // Rates & displacement
    todaysForwards_ = std::vector<Rate>(accruals_.size());
    displacement_ = 0.02;
    for (Size i=0; i<todaysForwards_.size(); ++i) {
        todaysForwards_[i] = 0.03 + 0.0010*i;
        todaysForwards_[i] = 0.03;
    }
    LMMCurveState curveState_lmm(rateTimes_);
    curveState_lmm.setOnForwardRates(todaysForwards_);
    todaysSwaps_ = curveState_lmm.coterminalSwapRates();

    // Discounts
    todaysDiscounts_ = std::vector<DiscountFactor>(rateTimes_.size());
    todaysDiscounts_[0] = 0.95;
    for (Size i=1; i<rateTimes_.size(); ++i)
        todaysDiscounts_[i] = todaysDiscounts_[i-1] /
            (1.0+todaysForwards_[i-1]*accruals_[i-1]);

    //// Swaption Volatilities
    //Volatility mktSwaptionVols[] = {
    //                        0.15541283,
    //                        0.18719678,
    //                        0.20890740,
    //                        0.22318179,
    //                        0.23212717,
    //                        0.23731450,
    //                        0.23988649,
    //                        0.24066384,
    //                        0.24023111,
    //                        0.23900189,
    //                        0.23726699,
    //                        0.23522952,
    //                        0.23303022,
    //                        0.23076564,
    //                        0.22850101,
    //                        0.22627951,
    //                        0.22412881,
    //                        0.22206569,
    //                        0.22009939
    //};

    //a = -0.0597;
    //b =  0.1677;
    //c =  0.5403;
    //d =  0.1710;

    a_ = 0.0;
    b_ = 0.17;
    c_ = 1.0;
    d_ = 0.10;

    Volatility mktCapletVols[] = {
                            0.1640,
                            0.1740,
                            0.1840,
                            0.1940,
                            0.1840,
                            0.1740,
                            0.1640,
                            0.1540,
                            0.1440,
                            0.1340 // not used??
    };

    //swaptionDisplacedVols = std::vector<Volatility>(todaysSwaps.size());
    //swaptionVols = std::vector<Volatility>(todaysSwaps.size());
    //capletDisplacedVols = std::vector<Volatility>(todaysSwaps.size());
    capletVols_.resize(todaysSwaps_.size());
    for (Size i=0; i<todaysSwaps_.size(); i++) {
    //    swaptionDisplacedVols[i] = todaysSwaps[i]*mktSwaptionVols[i]/
    //                              (todaysSwaps[i]+displacement);
    //    swaptionVols[i]= mktSwaptionVols[i];
    //    capletDisplacedVols[i] = todaysForwards[i]*mktCapletVols[i]/
    //                            (todaysForwards[i]+displacement);
        capletVols_[i]= mktCapletVols[i];
    }

    // Cap/Floor Correlation
    longTermCorrelation_ = 0.5;
    beta_ = 0.2;
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
    Real initialNumeraireValue = todaysDiscounts_[initialNumeraire];

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
                                        MarketModelType marketModelType,
                                        Spread rateBump = 0.0,
                                        Volatility volBump = 0.0) {

    std::vector<Time> fixingTimes(evolution.rateTimes());
    fixingTimes.pop_back();
    boost::shared_ptr<LmVolatilityModel> volModel(new
        LmExtLinearExponentialVolModel(fixingTimes, 0.5, 0.6, 0.1, 0.1));
    boost::shared_ptr<LmCorrelationModel> corrModel(new
        LmLinearExponentialCorrelationModel(evolution.numberOfRates(),
                                            longTermCorrelation_, beta_));
    std::vector<Rate> bumpedRates(todaysForwards_.size());
    LMMCurveState curveState_lmm(rateTimes_);
    curveState_lmm.setOnForwardRates(todaysForwards_);
    std::vector<Rate> usedRates = curveState_lmm.coterminalSwapRates();
    std::transform(usedRates.begin(), usedRates.end(),
                   bumpedRates.begin(),
                   std::bind1st(std::plus<Rate>(), rateBump));

    std::vector<Volatility> bumpedVols(swaptionDisplacedVols_.size());
    std::transform(swaptionDisplacedVols_.begin(), swaptionDisplacedVols_.end(),
                   bumpedVols.begin(),
                   std::bind1st(std::plus<Rate>(), volBump));
    Matrix correlations = exponentialCorrelations(evolution.rateTimes(),
                                                  longTermCorrelation_,
                                                  beta_);
    switch (marketModelType) {
        case ExponentialCorrelationFlatVolatility:
            return boost::shared_ptr<MarketModel>(new
                ExpCorrFlatVol(bumpedVols,
                               correlations,
                               evolution,
                               numberOfFactors,
                               bumpedRates,
                               std::vector<Spread>(bumpedRates.size(),
                               displacement_)));
        case ExponentialCorrelationAbcdVolatility:
            return boost::shared_ptr<MarketModel>(new
                ExpCorrAbcdVol(0.0,0.0,1.0,1.0,
                               bumpedVols,
                               correlations,
                               evolution,
                               numberOfFactors,
                               bumpedRates,
                               std::vector<Spread>(bumpedRates.size(),
                               displacement_)));
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

enum EvolverType { Ipc, Pc , NormalPc};

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
        case Pc:
            return boost::shared_ptr<MarketModelEvolver>(new
                CoterminalSwapRatePcEvolver(marketModel, generatorFactory,
                                            numeraires,
                                            initialStep));
        default:
            QL_FAIL("unknown CoterminalSwapMarketModelEvolver type");
    }
}

QL_END_TEST_LOCALS(MarketModelTest_smm)

void MarketModelSmmCapletCalibrationTest::testFunction() {

    BOOST_MESSAGE("Run coterminal swap + caplet calibration function ...");
    QL_TEST_BEGIN
    QL_TEST_SETUP

    Size numberOfFactors = 3;
    Size numberOfRates = todaysForwards_.size();

    EvolutionDescription evolution(rateTimes_);
    Size numberOfSteps = evolution.numberOfSteps();

    LMMCurveState cs(rateTimes_);
    cs.setOnForwardRates(todaysForwards_);
    Matrix correlations = exponentialCorrelations(evolution.rateTimes(),
                                                  longTermCorrelation_,
                                                  beta_);
    SwapFromFRACorrelationStructure corr(correlations,
                                         cs,
                                         displacement_,
                                         evolution,
                                         numberOfFactors);

    std::vector<boost::shared_ptr<PiecewiseConstantVariance> >
                                    swapVariances(numberOfRates);
    for (Size i=0; i<numberOfRates; ++i) {
        swapVariances[i] = boost::shared_ptr<PiecewiseConstantVariance>(new
            PiecewiseConstantAbcdVariance(a_, b_, c_, d_,
                                          i, rateTimes_));
    }

    std::vector<Real> alpha(numberOfRates, 0.0);
    std::vector<Matrix> swaptionPseudoRoots;
    bool result = capletCoterminalCalibration(evolution,
                                              corr,
                                              swapVariances,
                                              capletVols_,
                                              cs,
                                              displacement_,
                                              alpha,
                                              true,
                                              swaptionPseudoRoots);

    if (!result)
        BOOST_ERROR("calibration failed");

    Real error, tolerance = 1e-6;
    Matrix swapTerminalCovariance(numberOfRates, numberOfRates, 0.0);
    for (Size i=0; i<numberOfRates; ++i) {
        Volatility expSwaptionVol = swapVariances[i]->totalVolatility(i);
        swapTerminalCovariance += swaptionPseudoRoots[i] * transpose(swaptionPseudoRoots[i]);
        Volatility swaptionVol = std::sqrt(swapTerminalCovariance[i][i]/rateTimes_[i]);
        error = std::fabs(swaptionVol-expSwaptionVol);
        if (error>tolerance)
            BOOST_FAIL("\n failed to reproduce "
                       << io::ordinal(i) << " swaption vol:"
                       "\n expected:  " << expSwaptionVol <<
                       "\n realized:  " << swaptionVol <<
                       "\n error:     " << error <<
                       "\n tolerance: " << tolerance);
    }

    //Array modelCapletDisplacedVols(numberOfRates, 0.0);
    //for (Size j=0; j<numberOfRates; ++j) {
    //    for (Size i=0; i<numberOfSteps; ++i) {
    //        for (Size k=0; k<numberOfFactors; ++k) {
    //            Real stdDev = capletPseudoRoots[i][j][k];
    //            modelCapletDisplacedVols[j] += stdDev*stdDev;
    //        }
    //    }
    //    modelCapletDisplacedVols[j] = std::sqrt(modelCapletDisplacedVols[j]/rateTimes[j]);
    //}
    //BOOST_MESSAGE("" << modelCapletDisplacedVols);

    QL_TEST_END
}

//void MarketModelTest::testMultiStepForwardsAndOptionlets() {
//
//    BOOST_MESSAGE("Repricing multi-step forwards and optionlets "
//                  "in a Swap market model...");
//
//    QL_TEST_SETUP
//
//    std::vector<Rate> forwardStrikes(todaysForwards.size());
//    std::vector<boost::shared_ptr<Payoff> > optionletPayoffs(todaysForwards.size());
//    std::vector<boost::shared_ptr<StrikedTypePayoff> >
//        displacedPayoffs(todaysForwards.size());
//
//    for (Size i=0; i<todaysForwards.size(); ++i) {
//        forwardStrikes[i] = todaysForwards[i] + 0.01;
//        optionletPayoffs[i] = boost::shared_ptr<Payoff>(new
//            PlainVanillaPayoff(Option::Call, todaysForwards[i]));
//        displacedPayoffs[i] = boost::shared_ptr<StrikedTypePayoff>(new
//            PlainVanillaPayoff(Option::Call, todaysForwards[i]+displacement));
//    }
//
//    MultiStepForwards forwards(rateTimes, accruals,
//                               paymentTimes, forwardStrikes);
//    MultiStepOptionlets optionlets(rateTimes, accruals,
//                                   paymentTimes, optionletPayoffs);
//
//    MultiProductComposite product;
//    product.add(forwards);
//    product.add(optionlets);
//    product.finalize();
//
//    EvolutionDescription evolution = product.evolution();
//
//    MarketModelType marketModels[] = {
//        // CalibratedMM,
//        ExponentialCorrelationFlatVolatility,
//        ExponentialCorrelationAbcdVolatility };
//    for (Size j=0; j<LENGTH(marketModels); j++) {
//
//        Size testedFactors[] = { 4, 8,
//                                 todaysForwards.size()};
//        for (Size m=0; m<LENGTH(testedFactors); ++m) {
//            Size factors = testedFactors[m];
//
//            // Composite's ProductSuggested is the Terminal one
//            MeasureType measures[] = { // ProductSuggested,
//                                        Terminal,
//                                        MoneyMarketPlus,
//                                        MoneyMarket};
//        for (Size k=0; k<LENGTH(measures); k++) {
//                std::vector<Size> numeraires = makeMeasure(product, measures[k]);
//
//                bool logNormal = true;
//                boost::shared_ptr<MarketModel> marketModel =
//                    makeMarketModel(logNormal, evolution, factors, marketModels[j]);
//
//
//                EvolverType evolvers[] = { Pc, Ipc };
//                boost::shared_ptr<MarketModelEvolver> evolver;
//                Size stop =
//                    isInTerminalMeasure(evolution, numeraires) ? 0 : 1;
//                for (Size i=0; i<LENGTH(evolvers)-stop; i++) {
//
//                    for (Size n=0; n<1; n++) {
//                        //MTBrownianGeneratorFactory generatorFactory(seed_);
//                        SobolBrownianGeneratorFactory generatorFactory(
//                            SobolBrownianGenerator::Diagonal);
//
//                        evolver = makeMarketModelEvolver(marketModel,
//                                                         numeraires,
//                                                         generatorFactory,
//                                                         evolvers[i]);
//                        std::ostringstream config;
//                        config <<
//                            marketModelTypeToString(marketModels[j]) << ", " <<
//                            factors << (factors>1 ? (factors==todaysForwards.size() ? " (full) factors, " : " factors, ") : " factor,") <<
//                            measureTypeToString(measures[k]) << ", " <<
//                            evolverTypeToString(evolvers[i]) << ", " <<
//                            "MT BGF";
//                        if (printReport_)
//                            BOOST_MESSAGE("    " << config.str());
//
//                        boost::shared_ptr<SequenceStatistics> stats =
//                            simulate(evolver, product);
//                        checkForwardsAndOptionlets(*stats,
//                                                   forwardStrikes,
//                                                   displacedPayoffs,
//                                                   config.str());
//                    }
//                }
//            }
//        }
//    }
//}




// --- Call the desired tests
test_suite* MarketModelSmmCapletCalibrationTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("SMM Caplet calibration test");

    suite->add(BOOST_TEST_CASE(&MarketModelSmmCapletCalibrationTest::testFunction));

    return suite;
}
