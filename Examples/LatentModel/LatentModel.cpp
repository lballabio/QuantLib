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
#include <ql/experimental/credit/randomdefaultlatentmodel.hpp>
#include <ql/termstructures/credit/flathazardrate.hpp>
#include <ql/currencies/europe.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <string>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace QuantLib;

/* This sample code shows basic usage of a Latent variable model.
   The data and correlation problem presented is the same as in:
     'Modelling Dependent Defaults: Asset Correlations Are Not Enough!'
     Frey R., A. J. McNeil and M. A. Nyfeler RiskLab publications March 2001
*/
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
        // build curves and issuers into a basket of three names
        std::vector<Real> hazardRates(3, -std::log(1.-0.01));
        std::vector<std::string> names;
        for(Size i=0; i<hazardRates.size(); i++)
            names.push_back(std::string("Acme") + std::to_string(i));
        std::vector<Handle<DefaultProbabilityTermStructure> > defTS;
        defTS.reserve(hazardRates.size());
        for (double& hazardRate : hazardRates)
            defTS.emplace_back(
                ext::make_shared<FlatHazardRate>(0, TARGET(), hazardRate, Actual365Fixed()));
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
        // Recoveries are irrelevant in this example but must be given as the 
        //   lib stands.
        std::vector<ext::shared_ptr<RecoveryRateModel> > rrModels(
            hazardRates.size(), ext::make_shared<ConstantRecoveryModel>(
            ConstantRecoveryModel(0.5, SeniorSec)));
        ext::shared_ptr<Basket> theBskt = ext::make_shared<Basket>(
            todaysDate, names, std::vector<Real>(hazardRates.size(), 100.), 
            thePool);
        /* --------------------------------------------------------------
                        SET UP JOINT DEFAULT EVENT LATENT MODELS
        -------------------------------------------------------------- */
        // Latent model factors, corresponds to the first entry in Table1 of the
        //   publication mentioned. It is a single factor model
        std::vector<std::vector<Real> > fctrsWeights(hazardRates.size(), 
            std::vector<Real>(1, std::sqrt(0.1)));
        // --- Default Latent models -------------------------------------
        #ifndef QL_PATCH_SOLARIS
        // Gaussian integrable joint default model:
        ext::shared_ptr<GaussianDefProbLM> lmG(new 
            GaussianDefProbLM(fctrsWeights, 
            LatentModelIntegrationType::GaussianQuadrature,
			GaussianCopulaPolicy::initTraits() // otherwise gcc screams
			));
        #endif
        // Define StudentT copula
        // this is as far as we can be from the Gaussian, 2 T_3 factors:
        std::vector<Integer> ordersT(2, 3);
        TCopulaPolicy::initTraits iniT;
        iniT.tOrders = ordersT;
        // StudentT integrable joint default model:
        ext::shared_ptr<TDefProbLM> lmT(new TDefProbLM(fctrsWeights, 
            // LatentModelIntegrationType::GaussianQuadrature,
            LatentModelIntegrationType::Trapezoid,
            iniT));

        // --- Default Loss models ----------------------------------------
        // Gaussian random joint default model:
        Size numSimulations = 100000;
        // Size numCoresUsed = 4;
        #ifndef QL_PATCH_SOLARIS
        // Sobol, many cores
        ext::shared_ptr<DefaultLossModel> rdlmG(
            ext::make_shared<RandomDefaultLM<GaussianCopulaPolicy> >(lmG, 
                std::vector<Real>(), numSimulations, 1.e-6, 2863311530UL));
        #endif
        // StudentT random joint default model:
        ext::shared_ptr<DefaultLossModel> rdlmT(
            ext::make_shared<RandomDefaultLM<TCopulaPolicy> >(lmT, 
            std::vector<Real>(), numSimulations, 1.e-6, 2863311530UL));

        /* --------------------------------------------------------------
                        DUMP SOME RESULTS
        -------------------------------------------------------------- */
        /* Default correlations in a T copula should be below those of the 
        gaussian for the same factors.
        The calculations on the MC show dispersion on both copulas (thats
        ok) and too large values with very large dispersions on the T case.
        Computations are ok, within the dispersion, for the gaussian; compare
        with the direct integration in both cases.
        However the T does converge to the gaussian value for large value of
        the parameters.
        */
        Date calcDate(TARGET().advance(Settings::instance().evaluationDate(), 
            Period(120, Months)));
        std::vector<Probability> probEventsTLatent, probEventsTRandLoss;
        #ifndef QL_PATCH_SOLARIS
        std::vector<Probability> probEventsGLatent, probEventsGRandLoss;
        #endif
        //
        lmT->resetBasket(theBskt);
        for(Size numEvts=0; numEvts <=theBskt->size(); numEvts++) {
            probEventsTLatent.push_back(lmT->probAtLeastNEvents(numEvts, 
                calcDate));
         }
        //
        theBskt->setLossModel(rdlmT);
        for(Size numEvts=0; numEvts <=theBskt->size(); numEvts++) {
            probEventsTRandLoss.push_back(theBskt->probAtLeastNEvents(numEvts, 
                calcDate));
         }
        //
        #ifndef QL_PATCH_SOLARIS
        lmG->resetBasket(theBskt);
        for(Size numEvts=0; numEvts <=theBskt->size(); numEvts++) {
            probEventsGLatent.push_back(lmG->probAtLeastNEvents(numEvts, 
                calcDate));
         }
        //
        theBskt->setLossModel(rdlmG);
        for(Size numEvts=0; numEvts <=theBskt->size(); numEvts++) {
            probEventsGRandLoss.push_back(theBskt->probAtLeastNEvents(numEvts, 
                calcDate));
         }
        #endif

        Date correlDate = TARGET().advance(
            Settings::instance().evaluationDate(), Period(12, Months));
        std::vector<std::vector<Real> > correlsGlm, correlsTlm, correlsGrand, 
            correlsTrand;
        //
        lmT->resetBasket(theBskt);
        for(Size iName1=0; iName1 <theBskt->size(); iName1++) {
            std::vector<Real> tmp;
            for(Size iName2=0; iName2 <theBskt->size(); iName2++)
                tmp.push_back(lmT->defaultCorrelation(correlDate, 
                    iName1, iName2));
            correlsTlm.push_back(tmp);
        }
        //
        theBskt->setLossModel(rdlmT);
        for(Size iName1=0; iName1 <theBskt->size(); iName1++) {
            std::vector<Real> tmp;
            for(Size iName2=0; iName2 <theBskt->size(); iName2++)
                tmp.push_back(theBskt->defaultCorrelation(correlDate, 
                    iName1, iName2));
            correlsTrand.push_back(tmp);
        }
        #ifndef QL_PATCH_SOLARIS
        //
        lmG->resetBasket(theBskt);
        for(Size iName1=0; iName1 <theBskt->size(); iName1++) {
            std::vector<Real> tmp;
            for(Size iName2=0; iName2 <theBskt->size(); iName2++)
                tmp.push_back(lmG->defaultCorrelation(correlDate, 
                    iName1, iName2));
            correlsGlm.push_back(tmp);
        }
        //
        theBskt->setLossModel(rdlmG);
        for(Size iName1=0; iName1 <theBskt->size(); iName1++) {
            std::vector<Real> tmp;
            for(Size iName2=0; iName2 <theBskt->size(); iName2++)
                tmp.push_back(theBskt->defaultCorrelation(correlDate, 
                    iName1, iName2));
            correlsGrand.push_back(tmp);
        }
        #endif


        std::cout << 
            " Gaussian versus T prob of extreme event (random and integrable)-" 
            << std::endl;
        for(Size numEvts=0; numEvts <=theBskt->size(); numEvts++) {
            std::cout << "-Prob of " << numEvts << " events... " <<
                #ifndef QL_PATCH_SOLARIS
                probEventsGLatent[numEvts] << " ** " <<
                #else
                "n/a" << " ** " <<
                #endif
                probEventsTLatent[numEvts] << " ** " << 
                #ifndef QL_PATCH_SOLARIS
                probEventsGRandLoss[numEvts]<< " ** " <<
                #else
                "n/a" << " ** " <<
                #endif
                probEventsTRandLoss[numEvts] 
            << std::endl;
        }

        cout << endl;
        cout << "-- Default correlations G,T,GRand,TRand--" << endl;
        cout << "-----------------------------------------" << endl;
        for(Size iName1=0; iName1 <theBskt->size(); iName1++) {
            for(Size iName2=0; iName2 <theBskt->size(); iName2++)
                cout << 
                #ifndef QL_PATCH_SOLARIS
                    correlsGlm[iName1][iName2] << " , ";
                #else
                    "n/a" << " , ";
                #endif
            cout << endl;
        }
        cout << endl;
        for(Size iName1=0; iName1 <theBskt->size(); iName1++) {
            for(Size iName2=0; iName2 <theBskt->size(); iName2++)
                cout << 
                    correlsTlm[iName1][iName2] << " , ";
            ;
                cout << endl;
        }
        cout << endl;
        for(Size iName1=0; iName1 <theBskt->size(); iName1++) {
            for(Size iName2=0; iName2 <theBskt->size(); iName2++)
                cout << 
                #ifndef QL_PATCH_SOLARIS
                    correlsGrand[iName1][iName2] << " , ";
                #else
                    "n/a" << " , ";
                #endif
            cout << endl;
        }
        cout << endl;
        for(Size iName1=0; iName1 <theBskt->size(); iName1++) {
            for(Size iName2=0; iName2 <theBskt->size(); iName2++)
                cout << 
                    correlsTrand[iName1][iName2] << " , ";
            ;
                cout << endl;
        }

        return 0;
    } catch (exception& e) {
        cerr << e.what() << endl;
        return 1;
    } catch (...) {
        cerr << "unknown error" << endl;
        return 1;
    }
}

