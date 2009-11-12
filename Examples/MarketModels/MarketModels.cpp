/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*!
Copyright (C) 2009 Mark Joshi

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
#  include <ql/auto_link.hpp>
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
#include <ql/math/functional.hpp>
#include <ql/math/optimization/simplex.hpp>
#include <ql/quotes/simplequote.hpp>
#include <sstream>

#include <ql/models/marketmodels/products/pathwise/pathwiseproductcaplet.hpp>
#include <ql/models/marketmodels/products/pathwise/pathwiseproductswaption.hpp>

#include <ql/models/marketmodels/pathwiseaccountingengine.hpp>
#include <ql/models/marketmodels/pathwisegreeks/ratepseudorootjacobian.hpp>
#include <ql/models/marketmodels/pathwisegreeks/swaptionpseudojacobian.hpp>

#include <ql/models/marketmodels/models/pseudorootfacade.hpp>

#include <ql/models/marketmodels/pathwisegreeks/bumpinstrumentjacobian.hpp>

#include <iostream>
#include <ctime>

using namespace QuantLib;

int main()
{

    Size numberRates = 10;
    Real accrual = 0.5;
    Real firstTime = 0.5;


    std::vector<Real> rateTimes(numberRates+1);
    for (Size i=0; i < rateTimes.size(); ++i)
        rateTimes[i] = firstTime + i*accrual;

    std::vector<Real> paymentTimes(numberRates);
    std::vector<Real> accruals(numberRates,accrual);
    for (Size i=0; i < paymentTimes.size(); ++i)
        paymentTimes[i] = firstTime + (i+1)*accrual;




    Real fixedRate = 0.05;

    // 0. a payer swap
    MultiStepSwap payerSwap(rateTimes, accruals, accruals, paymentTimes,
        fixedRate, true);

    // 1. the equivalent receiver swap
    MultiStepSwap receiverSwap(rateTimes, accruals, accruals, paymentTimes,
        fixedRate, false);

    //exercise schedule, we can exercise on any rate time except the last one
    std::vector<Rate> exerciseTimes(rateTimes);
    exerciseTimes.pop_back();

    // naive exercise strategy, exercise above a trigger level 
    std::vector<Rate> swapTriggers(exerciseTimes.size(), fixedRate);
    SwapRateTrigger naifStrategy(rateTimes, swapTriggers, exerciseTimes);

    // Longstaff-Schwartz exercise strategy
    std::vector<std::vector<NodeData> > collectedData;
    std::vector<std::vector<Real> > basisCoefficients;

    // control that does nothing, need it because some control is expected 
    NothingExerciseValue control(rateTimes);

    SwapBasisSystem basisSystem(rateTimes,exerciseTimes);

    // rebate that does nothing, need it because some rebate is expected
    // when you break a swap nothing happens.
    NothingExerciseValue nullRebate(rateTimes);

    CallSpecifiedMultiProduct dummyProduct =
        CallSpecifiedMultiProduct(receiverSwap, naifStrategy,
        ExerciseAdapter(nullRebate));

    EvolutionDescription evolution = dummyProduct.evolution();


    // parameters for models


    Size seed = 12332; // for Sobol generator
    Size trainingPaths = 8192;
    Size paths = 65536;

    // set up a calibration, this would typically be done by using a calibrator



    Real rateLevel =0.05;

    Real initialNumeraireValue = 0.95;

    Real volLevel = 0.1;
    Real longTermCorr = 0.5;
    Real beta = 0.2;
    Real gamma = 1.0;
    Size numberOfFactors = 5;

    Spread displacementLevel =0.02;

    // set up vectors 
    std::vector<Rate> initialRates(numberRates,rateLevel);
    std::vector<Volatility> volatilities(numberRates, volLevel);
    std::vector<Spread> displacements(numberRates, displacementLevel);

    ExponentialForwardCorrelation correlations(
        rateTimes,volLevel, beta,gamma);


    FlatVol  calibration(
        volatilities,
        boost::shared_ptr<PiecewiseConstantCorrelation>(new  ExponentialForwardCorrelation(correlations)),
        evolution,
        numberOfFactors,
        initialRates,
        displacements);

    // we use a factory since their is data that will only be known later 
    SobolBrownianGeneratorFactory generatorFactory(
        SobolBrownianGenerator::Diagonal, seed);

    std::vector<Size> numeraires( moneyMarketMeasure(evolution));

    // the evolver will actually evolve the rates 
    LogNormalFwdRatePc  evolver(const boost::shared_ptr<MarketModel>(new FlatVol(calibration)),
        generatorFactory,
        numeraires   // numeraires for each step
        );

    boost::shared_ptr<MarketModelEvolver> evolverPtr(new LogNormalFwdRatePc(evolver));

    int t1= clock();

    // gather data before computing exercise strategy
    collectNodeData(evolver,
        receiverSwap, 
        basisSystem, 
        nullRebate,
        control, 
        trainingPaths,
        collectedData);

    int t2 = clock();


    // calculate the exercise strategy's coefficients
    genericLongstaffSchwartzRegression(collectedData,
        basisCoefficients);


    // turn the coefficients into an exercise strategy 
    LongstaffSchwartzExerciseStrategy exerciseStrategy(
        basisSystem, basisCoefficients,
        evolution, numeraires,
        nullRebate, control);

    //  bermudan swaption to enter into the payer swap
    CallSpecifiedMultiProduct bermudanProduct =
        CallSpecifiedMultiProduct(
        MultiStepNothing(evolution),
        exerciseStrategy, payerSwap);

    //  callable receiver swap
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

    AccountingEngine accounter(evolverPtr,
        Clone<MarketModelMultiProduct>(allProducts),
        initialNumeraireValue);

    SequenceStatisticsInc stats;

    accounter.multiplePathValues (stats,paths);

    int t3 = clock();

    std::vector<Real> means(stats.mean());

    for (Size i=0; i < means.size(); ++i)
        std::cout << means[i] << "\n";

    std::cout << " time to build strategy " << (t2-t1)/static_cast<Real>(CLOCKS_PER_SEC);
    std::cout << " time to price " << (t3-t2)/static_cast<Real>(CLOCKS_PER_SEC);
    char c;
    std::cin >> c;

    /*

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

    std::valarray<bool> isExerciseTime =
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
    */
}
