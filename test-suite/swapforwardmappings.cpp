/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2007 Ferdinando Ametrano
Copyright (C) 2006 François du Vignaud

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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/models/marketmodels/swapforwardmappings.hpp>
#include <ql/models/marketmodels/correlations/timehomogeneousforwardcorrelation.hpp>
#include <ql/models/marketmodels/curvestates/lmmcurvestate.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/models/marketmodels/evolvers/lognormalfwdratepc.hpp>
#include <ql/models/marketmodels/models/flatvol.hpp>
#include <ql/models/marketmodels/correlations/expcorrelations.hpp>
#include <ql/models/marketmodels/browniangenerators/sobolbrowniangenerator.hpp>
#include <ql/models/marketmodels/products/multistep/multistepcoterminalswaptions.hpp>
#include <ql/models/marketmodels/accountingengine.hpp>
#include <ql/models/marketmodels/models/cotswaptofwdadapter.hpp>
#include <ql/models/marketmodels/curvestates/coterminalswapcurvestate.hpp>
#include <ql/time/schedule.hpp>
#include <ql/time/daycounters/simpledaycounter.hpp>
#include <ql/math/statistics/sequencestatistics.hpp>
#include <ql/pricingengines/blackcalculator.hpp>

#include <ql/models/marketmodels/products/multistep/multistepswaption.hpp>

#if defined(BOOST_MSVC)
#include <float.h>
//namespace { unsigned int u = _controlfp(_EM_INEXACT, _MCW_EM); }
#endif

using namespace QuantLib;
using namespace boost::unit_test_framework;

using std::fabs;
using std::sqrt;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(SwapForwardMappingsTests)

namespace {

    class MarketModelData{
    public:
        MarketModelData();
        const std::vector<Time>& rateTimes(){return rateTimes_;}
        const std::vector<Rate>& forwards(){return forwards_;}
        const std::vector<Volatility>& volatilities(){return volatilities_;}
        const std::vector<Rate>& displacements(){return displacements_;}
        const std::vector<DiscountFactor>& discountFactors(){return discountFactors_;}
        Size nbRates() const { return nbRates_; }

      private:
        std::vector<Time> rateTimes_, accruals_;
        std::vector<Rate> forwards_;
        std::vector<Spread> displacements_;
        std::vector<Volatility> volatilities_;
        std::vector<DiscountFactor> discountFactors_;
        Size nbRates_;
    };

    MarketModelData::MarketModelData(){
        // Times
        Calendar calendar = NullCalendar();
        Date todaysDate = Settings::instance().evaluationDate();
        Date endDate = todaysDate + 9*Years; // change back
        Schedule dates(todaysDate, endDate, Period(Semiannual),
            calendar, Following, Following, DateGeneration::Backward, false);
        nbRates_ = dates.size()-2;
        rateTimes_ = std::vector<Time>(nbRates_+1);
        //paymentTimes_ = std::vector<Time>(rateTimes_.size()-1);
        accruals_ = std::vector<Time>(nbRates_);
        DayCounter dayCounter = SimpleDayCounter();
        for (Size i=1; i<nbRates_+2; ++i)
            rateTimes_[i-1] = dayCounter.yearFraction(todaysDate, dates[i]);

        displacements_ = std::vector<Rate>(nbRates_, .0);

        forwards_ = std::vector<Rate>(nbRates_);
        discountFactors_ = std::vector<Rate>(nbRates_+1);
        discountFactors_[0] = 1.0; // .95; fdv1-> WHY ???????
        for (Size i=0; i<nbRates_; ++i){
            forwards_[i] = 0.03 + 0.0010*i;
            accruals_[i] = rateTimes_[i+1] - rateTimes_[i];
            discountFactors_[i+1] = discountFactors_[i]
            /(1+forwards_[i]*accruals_[i]);
        }
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
            /*
            0.2,
            0.2,
            0.2,
            0.2,
            0.2,
            0.2,
            0.2,
            0.2,
            0.2,
            0.2,
            0.2,
            0.2,
            0.2,
            0.2,
            0.2,
            0.2,
            0.2,
            0.2,
            0.2,
            0.2
            */

        };
        volatilities_ = std::vector<Volatility>(nbRates_);
        for (Size i = 0; i < volatilities_.size(); ++i)
            volatilities_[i] =   mktVols[i];//.0;
    }

    ext::shared_ptr<SequenceStatisticsInc>
    simulate(const std::vector<Real>& todaysDiscounts,
             const ext::shared_ptr<MarketModelEvolver>& evolver,
             const MarketModelMultiProduct& product) {
        Size paths_;
#ifdef _DEBUG
        paths_ = 127;// //
#else
        paths_ = 32767; //262144-1; // //; // 2^15-1
#endif

        Size initialNumeraire = evolver->numeraires().front();
        Real initialNumeraireValue = todaysDiscounts[initialNumeraire];

        AccountingEngine engine(evolver, product, initialNumeraireValue);
        ext::shared_ptr<SequenceStatisticsInc> stats(new
            SequenceStatisticsInc(product.numberOfProducts()));
        engine.multiplePathValues(*stats, paths_);
        return stats;
    }

//    MultiStepCoterminalSwaptions makeMultiStepCoterminalSwaptions(
//        const std::vector<Time>& rateTimes, Real strike ){
//            std::vector<Time> paymentTimes(rateTimes.begin(), rateTimes.end()-1);
//            std::vector<ext::shared_ptr<StrikedTypePayoff> > payoffs(paymentTimes.size());
//            for (auto& payoff : payoffs) {
//                payoff = ext::shared_ptr<StrikedTypePayoff>(
//                    new PlainVanillaPayoff(Option::Call, strike));
//            }
//            return MultiStepCoterminalSwaptions (rateTimes,
//                paymentTimes, payoffs);
//
//    }

}

BOOST_AUTO_TEST_CASE(testForwardSwapJacobians)
{
    {
        BOOST_TEST_MESSAGE("Testing forward-rate coinitial-swap Jacobian...");
        MarketModelData marketData;
        const std::vector<Time>& rateTimes = marketData.rateTimes();
        const std::vector<Rate>& forwards = marketData.forwards();
        const Size nbRates = marketData.nbRates();
        LMMCurveState lmmCurveState(rateTimes);
        lmmCurveState.setOnForwardRates(forwards);

        Real bumpSize = 1e-8;

        std::vector<Rate> bumpedForwards(forwards);

        Matrix coinitialJacobian(nbRates,nbRates);

        for (Size i=0; i < nbRates; ++i)
            for (Size j=0; j < nbRates; ++j)
            {
                bumpedForwards = forwards;
                bumpedForwards[j]+= bumpSize;
                lmmCurveState.setOnForwardRates(bumpedForwards);
                Real upRate = lmmCurveState.cmSwapRate(0,i+1);
                bumpedForwards[j]-= 2.0*bumpSize;
                lmmCurveState.setOnForwardRates(bumpedForwards);
                Real downRate = lmmCurveState.cmSwapRate(0,i+1);
                Real deriv = (upRate-downRate)/(2.0*bumpSize);
                coinitialJacobian[i][j] = deriv;

            }

        Matrix modelJacobian(SwapForwardMappings::coinitialSwapForwardJacobian(lmmCurveState));

        Real errorTolerance = 1e-5;


        for (Size i=0; i < nbRates; ++i)
            for (Size j=0; j < nbRates; ++j)
                if( fabs(modelJacobian[i][j]-coinitialJacobian[i][j]) > errorTolerance)
                {
                    BOOST_TEST_MESSAGE("rate " << i
                                       << ", sensitivity "  <<  j
                                       << ", formula value " << modelJacobian[i][j]
                                       << " bumping value " << coinitialJacobian[i][j]
                                       <<  "\n");

                    BOOST_ERROR("test failed");
                }
    }

    {

        BOOST_TEST_MESSAGE("Testing forward-rate constant-maturity swap Jacobian...");
        MarketModelData marketData;
        const std::vector<Time>& rateTimes = marketData.rateTimes();
        const std::vector<Rate>& forwards = marketData.forwards();
        const Size nbRates = marketData.nbRates();
        LMMCurveState lmmCurveState(rateTimes);
        lmmCurveState.setOnForwardRates(forwards);

        Real bumpSize = 1e-8;

        for( Size spanningForwards = 1; spanningForwards < nbRates; ++spanningForwards)
        {

            std::vector<Rate> bumpedForwards(forwards);

            Matrix cmsJacobian(nbRates,nbRates);

            for (Size i=0; i < nbRates; ++i)
                for (Size j=0; j < nbRates; ++j)
                {
                    bumpedForwards = forwards;
                    bumpedForwards[j]+= bumpSize;
                    lmmCurveState.setOnForwardRates(bumpedForwards);
                    Real upRate = lmmCurveState.cmSwapRate(i,spanningForwards);
                    bumpedForwards[j]-= 2.0*bumpSize;
                    lmmCurveState.setOnForwardRates(bumpedForwards);
                    Real downRate = lmmCurveState.cmSwapRate(i,spanningForwards);
                    Real deriv = (upRate-downRate)/(2.0*bumpSize);
                    cmsJacobian[i][j] = deriv;

                }

            Matrix modelJacobian(SwapForwardMappings::cmSwapForwardJacobian(lmmCurveState, spanningForwards));

            Real errorTolerance = 1e-5;


            for (Size i=0; i < nbRates; ++i)
                for (Size j=0; j < nbRates; ++j)
                    if( fabs(modelJacobian[i][j]-cmsJacobian[i][j]) > errorTolerance)
                    {
                        BOOST_TEST_MESSAGE(
                                           "rate " << i
                                           << ", sensitivity "  <<  j
                                           << ", formula value " << modelJacobian[i][j]
                                           << " bumping value " << cmsJacobian[i][j]
                                           <<  "\n");

                        BOOST_ERROR("test failed");

                    }
        }

    }
}

//BOOST_AUTO_TEST_CASE(testForwardCoterminalMappings) {
//
//    BOOST_TEST_MESSAGE("Testing forward-rate coterminal-swap mappings...");
//    MarketModelData marketData;
//    const std::vector<Time>& rateTimes = marketData.rateTimes();
//    const std::vector<Rate>& forwards = marketData.forwards();
//    const Size nbRates = marketData.nbRates();
//    LMMCurveState lmmCurveState(rateTimes);
//    lmmCurveState.setOnForwardRates(forwards);
//
//    const Real longTermCorr=0.5;
//    const Real beta = .2;
//    Real strike = .03;
//    MultiStepCoterminalSwaptions product
//        = makeMultiStepCoterminalSwaptions(rateTimes, strike);
//
//    const EvolutionDescription& evolution = product.evolution();
//    const Size numberOfFactors = nbRates;
//    Spread displacement = marketData.displacements().front();
//    Matrix jacobian =
//        SwapForwardMappings::coterminalSwapZedMatrix(
//        lmmCurveState, displacement);
//
//    Matrix correlations = exponentialCorrelations(evolution.rateTimes(),
//        longTermCorr,
//        beta);
//    ext::shared_ptr<PiecewiseConstantCorrelation> corr(new
//        TimeHomogeneousForwardCorrelation(correlations,
//        rateTimes));
//    ext::shared_ptr<MarketModel> smmMarketModel(new
//        FlatVol(marketData.volatilities(),
//        corr,
//        evolution,
//        numberOfFactors,
//        lmmCurveState.coterminalSwapRates(),
//        marketData.displacements()));
//
//    ext::shared_ptr<MarketModel>
//        lmmMarketModel(new CotSwapToFwdAdapter(smmMarketModel));
//
//    SobolBrownianGeneratorFactory generatorFactory(SobolBrownianGenerator::Diagonal);
//    std::vector<Size> numeraires(nbRates,
//        nbRates);
//    ext::shared_ptr<MarketModelEvolver> evolver(new LogNormalFwdRatePc
//        (lmmMarketModel, generatorFactory, numeraires));
//
//    ext::shared_ptr<SequenceStatisticsInc> stats =
//        simulate(marketData.discountFactors(), evolver, product);
//    std::vector<Real> results = stats->mean();
//    std::vector<Real> errors = stats->errorEstimate();
//
//    const std::vector<DiscountFactor>& todaysDiscounts = marketData.discountFactors();
//    const std::vector<Rate>& todaysCoterminalSwapRates = lmmCurveState.coterminalSwapRates();
//    for (Size i=0; i<nbRates; ++i) {
//        const Matrix& cotSwapsCovariance = smmMarketModel->totalCovariance(i);
//        //Matrix cotSwapsCovariance= jacobian * forwardsCovariance * transpose(jacobian);
//        //Time expiry = rateTimes[i];
//        ext::shared_ptr<PlainVanillaPayoff> payoff(
//            new PlainVanillaPayoff(Option::Call, strike+displacement));
//        //const std::vector<Time>&  taus = lmmCurveState.rateTaus();
//        Real expectedSwaption = BlackCalculator(payoff,
//            todaysCoterminalSwapRates[i]+displacement,
//            std::sqrt(cotSwapsCovariance[i][i]),
//            lmmCurveState.coterminalSwapAnnuity(i,i) *
//            todaysDiscounts[i]).value();
//        if (fabs(expectedSwaption-results[i]) > 0.0001)
//            BOOST_ERROR(
//            "expected\t" << expectedSwaption <<
//            "\tLMM\t" << results[i]
//        << "\tstdev:\t" << errors[i] <<
//            "\t" <<std::fabs(results[i]- expectedSwaption)/errors[i]);
//    }
//}

BOOST_AUTO_TEST_CASE(testSwaptionImpliedVolatility)
{

    BOOST_TEST_MESSAGE("Testing implied swaption vol in LMM using HW approximation...");
    MarketModelData marketData;
    const std::vector<Time>& rateTimes = marketData.rateTimes();
    const std::vector<Rate>& forwards = marketData.forwards();
    const Size nbRates = marketData.nbRates();
    LMMCurveState lmmCurveState(rateTimes);
    lmmCurveState.setOnForwardRates(forwards);

    const Real longTermCorr=0.5;
    const Real beta = .2;
    Real strike = .03;

    for (Size startIndex = 1; startIndex+2 < nbRates; startIndex = startIndex+5)
    {
        
        Size endIndex = nbRates-2;

        ext::shared_ptr<StrikedTypePayoff> payoff(new   
            PlainVanillaPayoff(Option::Call, strike));
        MultiStepSwaption product(rateTimes, startIndex, endIndex,payoff );

        const EvolutionDescription& evolution = product.evolution();
        const Size numberOfFactors = nbRates;
        Spread displacement = marketData.displacements().front();
        Matrix jacobian =
            SwapForwardMappings::coterminalSwapZedMatrix(
            lmmCurveState, displacement);

        Matrix correlations = exponentialCorrelations(evolution.rateTimes(),
            longTermCorr,
            beta);
        ext::shared_ptr<PiecewiseConstantCorrelation> corr(new
            TimeHomogeneousForwardCorrelation(correlations,
            rateTimes));
        ext::shared_ptr<MarketModel> lmmMarketModel(new
            FlatVol(marketData.volatilities(),
            corr,
            evolution,
            numberOfFactors,
            lmmCurveState.forwardRates(),
            marketData.displacements()));


        SobolBrownianGeneratorFactory generatorFactory(SobolBrownianGenerator::Diagonal);
        std::vector<Size> numeraires(nbRates,
            nbRates);
        ext::shared_ptr<MarketModelEvolver> evolver(new LogNormalFwdRatePc
            (lmmMarketModel, generatorFactory, numeraires));

        ext::shared_ptr<SequenceStatisticsInc> stats =
            simulate(marketData.discountFactors(), evolver, product);
        std::vector<Real> results = stats->mean();
        std::vector<Real> errors = stats->errorEstimate();


        Real estimatedImpliedVol = SwapForwardMappings::swaptionImpliedVolatility(*lmmMarketModel,startIndex,endIndex);

        Real swapRate = lmmCurveState.cmSwapRate(startIndex,endIndex-startIndex);
        Real swapAnnuity = lmmCurveState.cmSwapAnnuity(startIndex,startIndex,endIndex-startIndex)*marketData.discountFactors()[startIndex];

        ext::shared_ptr<PlainVanillaPayoff> payoffDis( new PlainVanillaPayoff(Option::Call, strike+displacement));

        Real expectedSwaption = BlackCalculator(payoffDis,
            swapRate+displacement, estimatedImpliedVol *sqrt(rateTimes[startIndex]),
            swapAnnuity).value();

        Real error = expectedSwaption - results[0];
        Real errorInSds = error/errors[0];
        if (fabs(errorInSds) > 3.5 )
            BOOST_ERROR(
            "expected\t" << expectedSwaption <<
            "\tLMM\t" << results[0]
        << "\tstdev:\t" << errors[0] <<
            "\t" <<errorInSds);
    }

}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
