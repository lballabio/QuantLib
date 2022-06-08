/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Jose Aparicio

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
#include <ql/experimental/credit/gaussianlhplossmodel.hpp>
#include <ql/experimental/credit/constantlosslatentmodel.hpp>
#include <ql/experimental/credit/binomiallossmodel.hpp>
#include <ql/experimental/credit/randomdefaultlatentmodel.hpp>
#include <ql/experimental/credit/randomlosslatentmodel.hpp>
#include <ql/experimental/credit/spotlosslatentmodel.hpp>
#include <ql/experimental/credit/basecorrelationlossmodel.hpp>
#include <ql/termstructures/credit/flathazardrate.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/currencies/europe.hpp>
#include <iostream>
#include <iomanip>
#include <string>

using namespace std;
using namespace QuantLib;

#if defined(QL_ENABLE_SESSIONS)
namespace QuantLib {
    ThreadKey sessionId() { return {}; }
}
#endif

int main(int, char* []) {

    try {

        std::cout << std::endl;

        Calendar calendar = TARGET();
        Date todaysDate(19, March, 2014);
        // must be a business day
        todaysDate = calendar.adjust(todaysDate);

        Settings::instance().evaluationDate() = todaysDate;


        /* --------------------------------------------------------------
                        SET UP BASKET PORTFOLIO
        -------------------------------------------------------------- */
        // build curves and issuers into a basket of ten names
        std::vector<Real> hazardRates = {
            0.001, 0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09
        };
        std::vector<std::string> names;
        for(Size i=0; i<hazardRates.size(); i++)
            names.push_back(std::string("Acme") + std::to_string(i));
        std::vector<Handle<DefaultProbabilityTermStructure> > defTS;
        for (Real& hazardRate : hazardRates) {
            defTS.emplace_back(
                ext::make_shared<FlatHazardRate>(0, TARGET(), hazardRate, Actual365Fixed()));
            defTS.back()->enableExtrapolation();
        }
        std::vector<Issuer> issuers;
        for(Size i=0; i<hazardRates.size(); i++) {
            std::vector<QuantLib::Issuer::key_curve_pair> curves(1, 
                std::make_pair(NorthAmericaCorpDefaultKey(
                    EURCurrency(), QuantLib::SeniorSec,
                    Period(), 1. // amount threshold
                    ), defTS[i]));
            issuers.emplace_back(curves);
        }

        ext::shared_ptr<Pool> thePool = ext::make_shared<Pool>();
        for(Size i=0; i<hazardRates.size(); i++)
            thePool->add(names[i], issuers[i], NorthAmericaCorpDefaultKey(
                    EURCurrency(), QuantLib::SeniorSec, Period(), 1.));

        std::vector<DefaultProbKey> defaultKeys(hazardRates.size(), 
            NorthAmericaCorpDefaultKey(EURCurrency(), SeniorSec, Period(), 1.));
        ext::shared_ptr<Basket> theBskt = ext::make_shared<Basket>(
            todaysDate, 
            names, std::vector<Real>(hazardRates.size(), 100.), thePool,
         //   0.0, 0.78);
            0.03, .06);

        /* --------------------------------------------------------------
                        SET UP DEFAULT LOSS MODELS
        -------------------------------------------------------------- */

        std::vector<Real> recoveries(hazardRates.size(), 0.4);

        Date calcDate(TARGET().advance(Settings::instance().evaluationDate(), 
            Period(60, Months)));
        Real factorValue = 0.05;
        std::vector<std::vector<Real> > fctrsWeights(hazardRates.size(), 
            std::vector<Real>(1, std::sqrt(factorValue)));

        // --- LHP model --------------------------
        #ifndef QL_PATCH_SOLARIS
        ext::shared_ptr<DefaultLossModel> lmGLHP(
            ext::make_shared<GaussianLHPLossModel>(
                fctrsWeights[0][0] * fctrsWeights[0][0], recoveries));
        theBskt->setLossModel(lmGLHP);

        std::cout << "GLHP Expected 10-Yr Losses: "  << std::endl;
        std::cout << theBskt->expectedTrancheLoss(calcDate) << std::endl;

        // --- G Binomial model --------------------
        ext::shared_ptr<GaussianConstantLossLM> ktLossLM(
            ext::make_shared<GaussianConstantLossLM>(fctrsWeights, 
            recoveries, LatentModelIntegrationType::GaussianQuadrature, 
            GaussianCopulaPolicy::initTraits()));
        ext::shared_ptr<DefaultLossModel> lmBinomial(
            ext::make_shared<GaussianBinomialLossModel>(ktLossLM));
        theBskt->setLossModel(lmBinomial);

        std::cout << "Gaussian Binomial Expected 10-Yr Losses: "  << std::endl;
        std::cout << theBskt->expectedTrancheLoss(calcDate) << std::endl;

        #endif

        // --- T Binomial model --------------------
        TCopulaPolicy::initTraits initT;
        initT.tOrders = std::vector<Integer>(2, 3);
        ext::shared_ptr<TConstantLossLM> ktTLossLM(
            ext::make_shared<TConstantLossLM>(fctrsWeights, 
            recoveries, 
            //LatentModelIntegrationType::GaussianQuadrature,
              LatentModelIntegrationType::Trapezoid,
            initT));
        ext::shared_ptr<DefaultLossModel> lmTBinomial(
            ext::make_shared<TBinomialLossModel>(ktTLossLM));
        theBskt->setLossModel(lmTBinomial);

        std::cout << "T Binomial Expected 10-Yr Losses: "  << std::endl;
        std::cout << theBskt->expectedTrancheLoss(calcDate) << std::endl;

        // --- G Inhomogeneous model ---------------
        Size numSimulations = 100000;
        #ifndef QL_PATCH_SOLARIS
        ext::shared_ptr<GaussianConstantLossLM> gLM(
            ext::make_shared<GaussianConstantLossLM>(fctrsWeights, 
            recoveries,
            LatentModelIntegrationType::GaussianQuadrature,
            // g++ requires this when using make_shared
            GaussianCopulaPolicy::initTraits()));

        Size numBuckets = 100;
        ext::shared_ptr<DefaultLossModel> inhomogeneousLM(
            ext::make_shared<IHGaussPoolLossModel>(gLM, numBuckets));
        theBskt->setLossModel(inhomogeneousLM);

        std::cout << "G Inhomogeneous Expected 10-Yr Losses: "  << std::endl;
        std::cout << theBskt->expectedTrancheLoss(calcDate) << std::endl;

        // --- G Random model ---------------------
        // Gaussian random joint default model:
        // Size numCoresUsed = 4;
        // Sobol, many cores
        ext::shared_ptr<DefaultLossModel> rdlmG(
            ext::make_shared<RandomDefaultLM<GaussianCopulaPolicy, 
            RandomSequenceGenerator<
                BoxMullerGaussianRng<MersenneTwisterUniformRng> > > >(gLM, 
                    recoveries, numSimulations, 1.e-6, 2863311530UL));
        //ext::shared_ptr<DefaultLossModel> rdlmG(
        //    ext::make_shared<RandomDefaultLM<GaussianCopulaPolicy> >(gLM, 
        //        recoveries, numSimulations, 1.e-6, 2863311530));
        theBskt->setLossModel(rdlmG);

        std::cout << "Random G Expected 10-Yr Losses: "  << std::endl;
        std::cout << theBskt->expectedTrancheLoss(calcDate) << std::endl;
        #endif

        // --- StudentT Random model ---------------------
        // Sobol, many cores
        ext::shared_ptr<DefaultLossModel> rdlmT(
            ext::make_shared<RandomDefaultLM<TCopulaPolicy, 
            RandomSequenceGenerator<
                PolarStudentTRng<MersenneTwisterUniformRng> > > >(ktTLossLM, 
                    recoveries, numSimulations, 1.e-6, 2863311530UL));
        //ext::shared_ptr<DefaultLossModel> rdlmT(
        //    ext::make_shared<RandomDefaultLM<TCopulaPolicy> >(ktTLossLM, 
        //        recoveries, numSimulations, 1.e-6, 2863311530));
        theBskt->setLossModel(rdlmT);

        std::cout << "Random T Expected 10-Yr Losses: "  << std::endl;
        std::cout << theBskt->expectedTrancheLoss(calcDate) << std::endl;


        // Spot Loss latent model: 
        #ifndef QL_PATCH_SOLARIS
        std::vector<std::vector<Real> > fctrsWeightsRR(2 * hazardRates.size(), 
            std::vector<Real>(1, std::sqrt(factorValue)));
        Real modelA = 2.2;
        ext::shared_ptr<GaussianSpotLossLM> sptLG(new GaussianSpotLossLM(
            fctrsWeightsRR, recoveries, modelA,
            LatentModelIntegrationType::GaussianQuadrature,
            GaussianCopulaPolicy::initTraits()));
        ext::shared_ptr<TSpotLossLM> sptLT(new TSpotLossLM(fctrsWeightsRR, 
            recoveries, modelA,
            LatentModelIntegrationType::GaussianQuadrature, initT));


        // --- G Random Loss model ---------------------
        // Gaussian random joint default model:
        // Sobol, many cores
        ext::shared_ptr<DefaultLossModel> rdLlmG(
            ext::make_shared<RandomLossLM<GaussianCopulaPolicy> >(sptLG, 
                numSimulations, 1.e-6, 2863311530UL));
        theBskt->setLossModel(rdLlmG);

        std::cout << "Random Loss G Expected 10-Yr Losses: "  << std::endl;
        std::cout << theBskt->expectedTrancheLoss(calcDate) << std::endl;

        // --- T Random Loss model ---------------------
        // Gaussian random joint default model:
        // Sobol, many cores
        ext::shared_ptr<DefaultLossModel> rdLlmT(
            ext::make_shared<RandomLossLM<TCopulaPolicy> >(sptLT, 
                numSimulations, 1.e-6, 2863311530UL));
        theBskt->setLossModel(rdLlmT);

        std::cout << "Random Loss T Expected 10-Yr Losses: "  << std::endl;
        std::cout << theBskt->expectedTrancheLoss(calcDate) << std::endl;

        // Base Correlation model set up to test coherence with base LHP model
        std::vector<Period> bcTenors;
        bcTenors.emplace_back(1, Years);
        bcTenors.emplace_back(5, Years);
        std::vector<Real> bcLossPercentages;
        bcLossPercentages.push_back(0.03);
        bcLossPercentages.push_back(0.12);
        std::vector<std::vector<Handle<Quote> > > correls;
        // 
        std::vector<Handle<Quote> > corr1Y;
        // 3%
        corr1Y.emplace_back(
            ext::shared_ptr<Quote>(new SimpleQuote(fctrsWeights[0][0] * fctrsWeights[0][0])));
        // 12%
        corr1Y.emplace_back(
            ext::shared_ptr<Quote>(new SimpleQuote(fctrsWeights[0][0] * fctrsWeights[0][0])));
        correls.push_back(corr1Y);
        std::vector<Handle<Quote> > corr2Y;
        // 3%
        corr2Y.emplace_back(
            ext::shared_ptr<Quote>(new SimpleQuote(fctrsWeights[0][0] * fctrsWeights[0][0])));
        // 12%
        corr2Y.emplace_back(
            ext::shared_ptr<Quote>(new SimpleQuote(fctrsWeights[0][0] * fctrsWeights[0][0])));
        correls.push_back(corr2Y);
        ext::shared_ptr<BaseCorrelationTermStructure<BilinearInterpolation> > 
          correlSurface(
            new BaseCorrelationTermStructure<BilinearInterpolation>(
                // first one would do, all should be the same.
                defTS[0]->settlementDays(),
                defTS[0]->calendar(),
                Unadjusted,
                bcTenors,
                bcLossPercentages,
                correls,
                Actual365Fixed()
            )
        );
        Handle<BaseCorrelationTermStructure<BilinearInterpolation> > 
            correlHandle(correlSurface);
        ext::shared_ptr<DefaultLossModel> bcLMG_LHP_Bilin(
            ext::make_shared<GaussianLHPFlatBCLM>(correlHandle, recoveries,
                GaussianCopulaPolicy::initTraits()));

        theBskt->setLossModel(bcLMG_LHP_Bilin);

        std::cout << "Base Correlation GLHP Expected 10-Yr Losses: "  
            << std::endl;
        std::cout << theBskt->expectedTrancheLoss(calcDate) << std::endl;
        #endif


        return 0;
    } catch (exception& e) {
        cerr << e.what() << endl;
        return 1;
    } catch (...) {
        cerr << "unknown error" << endl;
        return 1;
    }
}

