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

#include <ql/qldefines.hpp>
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#  include <ql/auto_link.hpp>
#endif
#include <ql/models/marketmodels/marketmodel.hpp>
#include <ql/models/marketmodels/accountingengine.hpp>
#include <ql/models/marketmodels/pathwiseaccountingengine.hpp>
#include <ql/models/marketmodels/products/multiproductcomposite.hpp>
#include <ql/models/marketmodels/products/multistep/multistepswap.hpp>
#include <ql/models/marketmodels/products/multistep/callspecifiedmultiproduct.hpp>
#include <ql/models/marketmodels/products/multistep/exerciseadapter.hpp>
#include <ql/models/marketmodels/products/multistep/multistepnothing.hpp>
#include <ql/models/marketmodels/products/multistep/multistepinversefloater.hpp>
#include <ql/models/marketmodels/products/pathwise/pathwiseproductswap.hpp>
#include <ql/models/marketmodels/products/pathwise/pathwiseproductinversefloater.hpp>
#include <ql/models/marketmodels/products/pathwise/pathwiseproductcallspecified.hpp>
#include <ql/models/marketmodels/models/flatvol.hpp>
#include <ql/models/marketmodels/callability/swapratetrigger.hpp>
#include <ql/models/marketmodels/callability/swapbasissystem.hpp>
#include <ql/models/marketmodels/callability/swapforwardbasissystem.hpp>
#include <ql/models/marketmodels/callability/nothingexercisevalue.hpp>
#include <ql/models/marketmodels/callability/collectnodedata.hpp>
#include <ql/models/marketmodels/callability/lsstrategy.hpp>
#include <ql/models/marketmodels/callability/upperboundengine.hpp>
#include <ql/models/marketmodels/correlations/expcorrelations.hpp>
#include <ql/models/marketmodels/browniangenerators/mtbrowniangenerator.hpp>
#include <ql/models/marketmodels/browniangenerators/sobolbrowniangenerator.hpp>
#include <ql/models/marketmodels/evolvers/lognormalfwdratepc.hpp>
#include <ql/models/marketmodels/evolvers/lognormalfwdrateeuler.hpp>
#include <ql/models/marketmodels/pathwisegreeks/bumpinstrumentjacobian.hpp>
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
#include <sstream>
#include <iostream>
#include <ctime>

using namespace QuantLib;

std::vector<std::vector<Matrix>>
theVegaBumps(bool factorwiseBumping, const ext::shared_ptr<MarketModel>& marketModel, bool doCaps) {
    Real multiplierCutOff = 50.0;
    Real projectionTolerance = 1E-4;
    Size numberRates= marketModel->numberOfRates();

    std::vector<VolatilityBumpInstrumentJacobian::Cap> caps;

    if (doCaps)
    {

        Rate capStrike = marketModel->initialRates()[0];

        for (Size i=0; i< numberRates-1; i=i+1)
        {
            VolatilityBumpInstrumentJacobian::Cap nextCap;
            nextCap.startIndex_ = i;
            nextCap.endIndex_ = i+1;
            nextCap.strike_ = capStrike;
            caps.push_back(nextCap);
        }


    }



    std::vector<VolatilityBumpInstrumentJacobian::Swaption> swaptions(numberRates);

    for (Size i=0; i < numberRates; ++i)
    {
        swaptions[i].startIndex_ = i;
        swaptions[i].endIndex_ = numberRates;

    }

    VegaBumpCollection possibleBumps(marketModel,
        factorwiseBumping);

    OrthogonalizedBumpFinder  bumpFinder(possibleBumps,
        swaptions,
        caps,
        multiplierCutOff, // if vector length grows by more than this discard
        projectionTolerance);      // if vector projection before scaling less than this discard

    std::vector<std::vector<Matrix>> theBumps;

    bumpFinder.GetVegaBumps(theBumps);

    return theBumps;

}



int Bermudan()
{

    Size numberRates =20;
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
    std::vector<Real> strikes(numberRates,fixedRate);
    Real receive = -1.0;

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
    std::vector<std::vector<NodeData>> collectedData;
    std::vector<std::vector<Real>> basisCoefficients;

    // control that does nothing, need it because some control is expected
    NothingExerciseValue control(rateTimes);

//    SwapForwardBasisSystem basisSystem(rateTimes,exerciseTimes);
    SwapBasisSystem basisSystem(rateTimes,exerciseTimes);



    // rebate that does nothing, need it because some rebate is expected
    // when you break a swap nothing happens.
    NothingExerciseValue nullRebate(rateTimes);

    CallSpecifiedMultiProduct dummyProduct =
        CallSpecifiedMultiProduct(receiverSwap, naifStrategy,
        ExerciseAdapter(nullRebate));

    const EvolutionDescription& evolution = dummyProduct.evolution();


    // parameters for models


    Size seed = 12332; // for Sobol generator
    Size trainingPaths = 65536;
    Size paths = 16384;
    Size vegaPaths = 16384*64;

    std::cout << "training paths, " << trainingPaths << "\n";
    std::cout << "paths, " << paths << "\n";
    std::cout << "vega Paths, " << vegaPaths << "\n";
#ifdef _DEBUG
   trainingPaths = 512;
  paths = 1024;
  vegaPaths = 1024;
#endif


    // set up a calibration, this would typically be done by using a calibrator



    Real rateLevel =0.05;


    Real initialNumeraireValue = 0.95;

    Real volLevel = 0.11;
    Real beta = 0.2;
    Real gamma = 1.0;
    Size numberOfFactors = std::min<Size>(5,numberRates);

    Spread displacementLevel =0.02;

    // set up vectors
    std::vector<Rate> initialRates(numberRates,rateLevel);
    std::vector<Volatility> volatilities(numberRates, volLevel);
    std::vector<Spread> displacements(numberRates, displacementLevel);

    ExponentialForwardCorrelation correlations(
        rateTimes,volLevel, beta,gamma);




    FlatVol  calibration(
        volatilities,
        ext::make_shared<ExponentialForwardCorrelation>(correlations),
        evolution,
        numberOfFactors,
        initialRates,
        displacements);

    auto marketModel = ext::make_shared<FlatVol>(calibration);

    // we use a factory since there is data that will only be known later
    SobolBrownianGeneratorFactory generatorFactory(
        SobolBrownianGenerator::Diagonal, seed);

    std::vector<Size> numeraires( moneyMarketMeasure(evolution));

    // the evolver will actually evolve the rates
    LogNormalFwdRatePc  evolver(marketModel,
        generatorFactory,
        numeraires   // numeraires for each step
        );

    auto evolverPtr = ext::make_shared<LogNormalFwdRatePc>(evolver);

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

    for (Real mean : means)
        std::cout << mean << "\n";

    std::cout << " time to build strategy, " << (t2-t1)/static_cast<Real>(CLOCKS_PER_SEC)<< ", seconds.\n";
    std::cout << " time to price, " << (t3-t2)/static_cast<Real>(CLOCKS_PER_SEC)<< ", seconds.\n";

    // vegas

    // do it twice once with factorwise bumping, once without
    Size pathsToDoVegas = vegaPaths;

    for (Size i=0; i < 4; ++i)
    {

        bool allowFactorwiseBumping = i % 2 > 0 ;

        bool doCaps = i / 2 > 0 ;





        LogNormalFwdRateEuler evolverEuler(marketModel,
            generatorFactory,
            numeraires
            ) ;

        MarketModelPathwiseSwap receiverPathwiseSwap(  rateTimes,
            accruals,
            strikes,
            receive);
        Clone<MarketModelPathwiseMultiProduct> receiverPathwiseSwapPtr(receiverPathwiseSwap.clone());

        //  callable receiver swap
        CallSpecifiedPathwiseMultiProduct callableProductPathwise(receiverPathwiseSwapPtr,
            exerciseStrategy);

        Clone<MarketModelPathwiseMultiProduct> callableProductPathwisePtr(callableProductPathwise.clone());


        std::vector<std::vector<Matrix>> theBumps(theVegaBumps(allowFactorwiseBumping,
            marketModel,
            doCaps));

        PathwiseVegasOuterAccountingEngine
            accountingEngineVegas(ext::make_shared<LogNormalFwdRateEuler>(evolverEuler),
            callableProductPathwisePtr,
            marketModel,
            theBumps,
            initialNumeraireValue);

        std::vector<Real> values,errors;

        accountingEngineVegas.multiplePathValues(values,errors,pathsToDoVegas);


        std::cout << "vega output \n";
        std::cout << " factorwise bumping " << allowFactorwiseBumping << "\n";
        std::cout << " doCaps " << doCaps << "\n";



        Size r=0;

        std::cout << " price estimate, " << values[r++] << "\n";

        for (Size i=0; i < numberRates; ++i, ++r)
            std::cout << " Delta, " << i << ", " << values[r] << ", " << errors[r] << "\n";

        Real totalVega = 0.0;

        for (; r < values.size(); ++r)
        {
            std::cout << " vega, " << r - 1 -  numberRates<< ", " << values[r] << " ," << errors[r] << "\n";
            totalVega +=  values[r];
        }

        std::cout << " total Vega, " << totalVega << "\n";
    }

    // upper bound

    MTBrownianGeneratorFactory uFactory(seed+142);

    auto upperEvolver = ext::make_shared<LogNormalFwdRatePc>(ext::make_shared<FlatVol>(calibration),
            uFactory,
            numeraires   // numeraires for each step
            );

    std::vector<ext::shared_ptr<MarketModelEvolver>> innerEvolvers;

    std::valarray<bool> isExerciseTime =   isInSubset(evolution.evolutionTimes(),    exerciseStrategy.exerciseTimes());

    for (Size s=0; s < isExerciseTime.size(); ++s)
    {
        if (isExerciseTime[s])
        {
            MTBrownianGeneratorFactory iFactory(seed+s);
            auto e = ext::make_shared<LogNormalFwdRatePc>(ext::make_shared<FlatVol>(calibration),
                    uFactory,
                    numeraires,  // numeraires for each step
                    s);

            innerEvolvers.push_back(e);
        }
    }



    UpperBoundEngine uEngine(upperEvolver,  // does outer paths
                             innerEvolvers, // for sub-simulations that do continuation values
                             receiverSwap,
                             nullRebate,
                             receiverSwap,
                             nullRebate,
                             exerciseStrategy,
                             initialNumeraireValue);

    Statistics uStats;
    Size innerPaths = 255;
    Size outerPaths =256;

    int t4 = clock();

    uEngine.multiplePathValues(uStats,outerPaths,innerPaths);
    Real upperBound = uStats.mean();
    Real upperSE = uStats.errorEstimate();

    int t5=clock();

    std::cout << " Upper - lower is, " << upperBound << ", with standard error " << upperSE << "\n";
    std::cout << " time to compute upper bound is,  " << (t5-t4)/static_cast<Real>(CLOCKS_PER_SEC) << ", seconds.\n";

    return 0;
}

int InverseFloater(Real rateLevel)
{

    Size numberRates =20;
    Real accrual = 0.5;
    Real firstTime = 0.5;

    Real strike =0.15;
    Real fixedMultiplier = 2.0;
    Real floatingSpread =0.0;
    bool payer = true;


    std::vector<Real> rateTimes(numberRates+1);
    for (Size i=0; i < rateTimes.size(); ++i)
        rateTimes[i] = firstTime + i*accrual;

    std::vector<Real> paymentTimes(numberRates);
    std::vector<Real> accruals(numberRates,accrual);
    std::vector<Real> fixedStrikes(numberRates,strike);
    std::vector<Real> floatingSpreads(numberRates,floatingSpread);
    std::vector<Real> fixedMultipliers(numberRates,fixedMultiplier);

    for (Size i=0; i < paymentTimes.size(); ++i)
        paymentTimes[i] = firstTime + (i+1)*accrual;

  MultiStepInverseFloater inverseFloater(
                                                        rateTimes,
                                                        accruals,
                                                         accruals,
                                                        fixedStrikes,
                                                        fixedMultipliers,
                                                        floatingSpreads,
                                                         paymentTimes,
                                                         payer);




    //exercise schedule, we can exercise on any rate time except the last one
    std::vector<Rate> exerciseTimes(rateTimes);
    exerciseTimes.pop_back();

    // naive exercise strategy, exercise above a trigger level
    Real trigger =0.05;
    std::vector<Rate> swapTriggers(exerciseTimes.size(), trigger);
    SwapRateTrigger naifStrategy(rateTimes, swapTriggers, exerciseTimes);

    // Longstaff-Schwartz exercise strategy
    std::vector<std::vector<NodeData>> collectedData;
    std::vector<std::vector<Real>> basisCoefficients;

    // control that does nothing, need it because some control is expected
    NothingExerciseValue control(rateTimes);

   SwapForwardBasisSystem basisSystem(rateTimes,exerciseTimes);
//    SwapBasisSystem basisSystem(rateTimes,exerciseTimes);



    // rebate that does nothing, need it because some rebate is expected
    // when you break a swap nothing happens.
    NothingExerciseValue nullRebate(rateTimes);

    CallSpecifiedMultiProduct dummyProduct =
        CallSpecifiedMultiProduct(inverseFloater, naifStrategy,
        ExerciseAdapter(nullRebate));

    const EvolutionDescription& evolution = dummyProduct.evolution();


    // parameters for models


    Size seed = 12332; // for Sobol generator
    Size trainingPaths = 65536;
    Size paths = 65536;
    Size vegaPaths =16384;

#ifdef _DEBUG
   trainingPaths = 8192;
  paths = 8192;
  vegaPaths = 1024;
#endif


    std::cout <<  " inverse floater \n";
    std::cout << " fixed strikes :  "  << strike << "\n";
    std::cout << " number rates :  " << numberRates << "\n";

    std::cout << "training paths, " << trainingPaths << "\n";
    std::cout << "paths, " << paths << "\n";
    std::cout << "vega Paths, " << vegaPaths << "\n";


    // set up a calibration, this would typically be done by using a calibrator



    //Real rateLevel =0.08;

    std::cout << " rate level " <<  rateLevel << "\n";

    Real initialNumeraireValue = 0.95;

    Real volLevel = 0.11;
    Real beta = 0.2;
    Real gamma = 1.0;
    Size numberOfFactors = std::min<Size>(5,numberRates);

    Spread displacementLevel =0.02;

    // set up vectors
    std::vector<Rate> initialRates(numberRates,rateLevel);
    std::vector<Volatility> volatilities(numberRates, volLevel);
    std::vector<Spread> displacements(numberRates, displacementLevel);

    ExponentialForwardCorrelation correlations(
        rateTimes,volLevel, beta,gamma);




    FlatVol  calibration(
        volatilities,
        ext::make_shared<ExponentialForwardCorrelation>(correlations),
        evolution,
        numberOfFactors,
        initialRates,
        displacements);

    auto marketModel = ext::make_shared<FlatVol>(calibration);

    // we use a factory since there is data that will only be known later
    SobolBrownianGeneratorFactory generatorFactory(
        SobolBrownianGenerator::Diagonal, seed);

    std::vector<Size> numeraires( moneyMarketMeasure(evolution));

    // the evolver will actually evolve the rates
    LogNormalFwdRatePc  evolver(marketModel,
        generatorFactory,
        numeraires   // numeraires for each step
        );

    auto evolverPtr = ext::make_shared<LogNormalFwdRatePc>(evolver);

    int t1= clock();

    // gather data before computing exercise strategy
    collectNodeData(evolver,
        inverseFloater,
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


    //  callable receiver swap
    CallSpecifiedMultiProduct callableProduct =
        CallSpecifiedMultiProduct(
        inverseFloater, exerciseStrategy,
        ExerciseAdapter(nullRebate));

     MultiProductComposite allProducts;
    allProducts.add(inverseFloater);
    allProducts.add(callableProduct);
    allProducts.finalize();


    AccountingEngine accounter(evolverPtr,
        Clone<MarketModelMultiProduct>(allProducts),
        initialNumeraireValue);

    SequenceStatisticsInc stats;

    accounter.multiplePathValues (stats,paths);

    int t3 = clock();

    std::vector<Real> means(stats.mean());

    for (Real mean : means)
        std::cout << mean << "\n";

    std::cout << " time to build strategy, " << (t2-t1)/static_cast<Real>(CLOCKS_PER_SEC)<< ", seconds.\n";
    std::cout << " time to price, " << (t3-t2)/static_cast<Real>(CLOCKS_PER_SEC)<< ", seconds.\n";

    // vegas

    // do it twice once with factorwise bumping, once without
    Size pathsToDoVegas = vegaPaths;

    for (Size i=0; i < 4; ++i)
    {

        bool allowFactorwiseBumping = i % 2 > 0 ;

        bool doCaps = i / 2 > 0 ;


        LogNormalFwdRateEuler evolverEuler(marketModel,
            generatorFactory,
            numeraires
            ) ;

        MarketModelPathwiseInverseFloater pathwiseInverseFloater(
                                                         rateTimes,
                                                         accruals,
                                                         accruals,
                                                         fixedStrikes,
                                                         fixedMultipliers,
                                                         floatingSpreads,
                                                         paymentTimes,
                                                         payer);

        Clone<MarketModelPathwiseMultiProduct> pathwiseInverseFloaterPtr(pathwiseInverseFloater.clone());

        //  callable inverse floater
        CallSpecifiedPathwiseMultiProduct callableProductPathwise(pathwiseInverseFloaterPtr,
                                                                                                                                               exerciseStrategy);

        Clone<MarketModelPathwiseMultiProduct> callableProductPathwisePtr(callableProductPathwise.clone());


        std::vector<std::vector<Matrix>> theBumps(theVegaBumps(allowFactorwiseBumping,
            marketModel,
            doCaps));

        PathwiseVegasOuterAccountingEngine
            accountingEngineVegas(ext::make_shared<LogNormalFwdRateEuler>(evolverEuler),
   //         pathwiseInverseFloaterPtr,
            callableProductPathwisePtr,
            marketModel,
            theBumps,
            initialNumeraireValue);

        std::vector<Real> values,errors;

        accountingEngineVegas.multiplePathValues(values,errors,pathsToDoVegas);


        std::cout << "vega output \n";
        std::cout << " factorwise bumping " << allowFactorwiseBumping << "\n";
        std::cout << " doCaps " << doCaps << "\n";



        Size r=0;

        std::cout << " price estimate, " << values[r++] << "\n";

        for (Size i=0; i < numberRates; ++i, ++r)
            std::cout << " Delta, " << i << ", " << values[r] << ", " << errors[r] << "\n";

        Real totalVega = 0.0;

        for (; r < values.size(); ++r)
        {
            std::cout << " vega, " << r - 1 -  numberRates<< ", " << values[r] << " ," << errors[r] << "\n";
            totalVega +=  values[r];
        }

        std::cout << " total Vega, " << totalVega << "\n";
    }

    // upper bound

    MTBrownianGeneratorFactory uFactory(seed+142);


    auto upperEvolver = ext::make_shared<LogNormalFwdRatePc>(ext::make_shared<FlatVol>(calibration),
            uFactory,
            numeraires   // numeraires for each step
            );

    std::vector<ext::shared_ptr<MarketModelEvolver>> innerEvolvers;

    std::valarray<bool> isExerciseTime =   isInSubset(evolution.evolutionTimes(),    exerciseStrategy.exerciseTimes());

    for (Size s=0; s < isExerciseTime.size(); ++s)
    {
        if (isExerciseTime[s])
        {
            MTBrownianGeneratorFactory iFactory(seed+s);
            auto e = ext::make_shared<LogNormalFwdRatePc>(ext::make_shared<FlatVol>(calibration),
                    uFactory,
                    numeraires ,  // numeraires for each step
                    s);

            innerEvolvers.push_back(e);
        }
    }



    UpperBoundEngine uEngine(upperEvolver,  // does outer paths
                             innerEvolvers, // for sub-simulations that do continuation values
                             inverseFloater,
                             nullRebate,
                             inverseFloater,
                             nullRebate,
                             exerciseStrategy,
                             initialNumeraireValue);

    Statistics uStats;
    Size innerPaths = 255;
    Size outerPaths =256;

    int t4 = clock();

    uEngine.multiplePathValues(uStats,outerPaths,innerPaths);
    Real upperBound = uStats.mean();
    Real upperSE = uStats.errorEstimate();

    int t5=clock();

    std::cout << " Upper - lower is, " << upperBound << ", with standard error " << upperSE << "\n";
    std::cout << " time to compute upper bound is,  " << (t5-t4)/static_cast<Real>(CLOCKS_PER_SEC) << ", seconds.\n";


    return 0;

}

int main()
{
    try {
        for (Size i=5; i < 10; ++i)
            InverseFloater(i/100.0);

        return 0;
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 1;
    }
}
