#ifndef quantlib_spotlosslatentmodel_hpp
#define quantlib_spotlosslatentmodel_hpp

#include <ql/experimental/credit/defaultprobabilitylatentmodel.hpp>

namespace QuantLib {

    /* Integrable random spot recovery rate latent variable portfolio model.

    */
    template <class copulaPolicy>
    class SpotRecoveryLatentModel : public LatentModel<copulaPolicy> {
    public:
        // resolve LM interface:
        using LatentModel<copulaPolicy>::factorWeights;
        using LatentModel<copulaPolicy>::inverseCumulativeY;
        using LatentModel<copulaPolicy>::cumulativeY;
        using LatentModel<copulaPolicy>::latentVarValue;
        using LatentModel<copulaPolicy>::integratedExpectedValue;
   ///////////////     using LatentModel<copulaPolicy>::conditionalDefaultProbabilityInvP;
    private:
        const std::vector<Real> recoveries_;
        const Real modelA_;
        //! \products of default and recoveries factors, see literature. ('covariances')
        std::vector<Real> crossIdiosyncFctrs_;
   //     LatentModel<copulaPolicy> recoveryCrossSection_;
        mutable Size numNames_;
        mutable boost::shared_ptr<Basket> basket_;
        boost::shared_ptr<LMIntegration> integration_;
    protected:
        //! access to integration:
        const boost::shared_ptr<LMIntegration>& 
            integration() const { return integration_; }
    public:
        SpotRecoveryLatentModel(
            const std::vector<std::vector<Real> >& factorWeights,
            const std::vector<Real>& recoveries,
            Real modelA,
            LatentModelIntegrationType::LatentModelIntegrationType integralType,
            const typename copulaPolicy::initTraits& ini = 
                copulaPolicy::initTraits()
            ) 
        : LatentModel<copulaPolicy>(factorWeights, ini),
          integration_(LatentModel<copulaPolicy>::IntegrationFactory::
            createLMIntegration(factorWeights[0].size(), integralType)),
          recoveries_(recoveries), 
          modelA_(modelA),
          numNames_(factorWeights.size()/2)
        {
            QL_REQUIRE(factorWeights.size() % 2 == 0, 
                "Number of recovery variables must be equal to number of default variables");
            QL_REQUIRE(recoveries.size() == numNames_ , 
                "Number of recoveries does not match number of defaultable entities.");

            // reminder: first betas are default, last ones are recovery 
            for(Size iName=0; iName<numNames_; iName++) /// USE STL AT THIS LEVEL TOO..
                /* Corresponds to: (k denotes factor, i denotes modelled variable -default and recoveries))
                    \sum_k a^2_{i,k} a^2_{N+i,k}
                */
                // not sure I should call it cross systemic instead...

                /* NOT WORKING AS IS
                crossIdiosyncFctrs_.push_back( // back inserter??????
                    std::inner_product(betas_[iName].begin(), betas_[iName].end(), 
                    betas_[iName + pool->size()].begin(), 0.,
                    boost::lambda::_1*boost::lambda::_1*boost::lambda::_2*boost::lambda::_2,
                    std::plus<Real>()));
            */
            {
                Real cumul = 0.;
                for(Size iB=0; iB<factorWeights[iName].size(); iB++)// actually this size is unique
                    cumul += factorWeights[iName][iB] * factorWeights[iName][iB] * 
                        factorWeights[iName + numNames_][iB] * factorWeights[iName + numNames_][iB];
                crossIdiosyncFctrs_.push_back(cumul);
            }

        }

        void resetBasket(const boost::shared_ptr<Basket> basket) const {
            basket_ = basket;
            // in the future change 'size' to 'liveSize'
            QL_REQUIRE(basket_->size() == numNames_, 
                "Incompatible new basket and model sizes.");
        }






        Probability conditionalDefaultProbability(const Date& date, Size iName,
            const std::vector<Real>& mktFactors) const 
        {
            const boost::shared_ptr<Pool>& pool = basket_->pool();
            Probability pDefUncond =
                pool->get(pool->names()[iName]).
                defaultProbability(basket_->defaultKeys()[iName])
                  ->defaultProbability(date);
            return conditionalDefaultProbability(pDefUncond, iName, mktFactors);
        }

        Probability conditionalDefaultProbability(Probability prob, Size iName,
            const std::vector<Real>& mktFactors) const 
        {
            // we can be called from the outside (from an integrable loss model)
            //   but we are called often at integration points. This or
            //   consider a list of friends.
        #if defined(QL_EXTRA_SAFETY_CHECKS)
            QL_REQUIRE(basket_, "No portfolio basket set.");
        #endif
            /*Avoid redundant call to minimum value inversion (might be \infty),
            and this independently of the copula function.
            */
            if (prob < 1.e-10) return 0.;// use library macro...
            return conditionalDefaultProbabilityInvP(
                inverseCumulativeY(prob, iName), iName, mktFactors);
        }

        Probability conditionalDefaultProbabilityInvP(Real invCumYProb, 
            Size iName, 
            const std::vector<Real>& m) const {
            Real sumMs = 
                std::inner_product(factorWeights_[iName].begin(), 
                    factorWeights_[iName].end(), m.begin(), 0.);
            Real res = cumulativeZ((invCumYProb - sumMs) / 
                    idiosyncFctrs_[iName] );
            #if defined(QL_EXTRA_SAFETY_CHECKS)
            QL_REQUIRE (res >= 0. && res <= 1.,
                        "conditional probability " << res << "out of range");
            #endif
        
            return res;
        }






















        /*! Expected (it is volatile) conditional (to the latent factor) 
        spot recovery rate. It is also conditional to default taking place.
        Corresponds to a multifactor generalization of the model in eq. 44 
        on p.15 of 'Extension of Spot Recovery Model for Gaussian Copula' 
        Hui Li. 2009  Only remember that \rho_l Z there is here (multiple betas)
        \sum_k \beta_{ik}^l Z_k and that \rho_d \rho_l there is here: 
        \sum_k \beta_{ik}^d \beta_{ik}^l 
        (d,l corresponds to first and last set of betas) 
        */
        // THIS ONE  NEED TO HAVE ALL THE CORREL/BETAS FACTORS REVISED, THINK THE SQRTS ARE WRONG...
        Real conditionalRecovery(const Date& d, Size iName, 
                                 const std::vector<Real>& mktFactors) const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
            QL_REQUIRE(mktFactors.size() == numFactors(), 
            "Realization of market factors and latent model size do not match");
        #endif
            const boost::shared_ptr<Pool>& pool = basket_->pool();
            Probability pDefUncond =
                pool->get(pool->names()[iName]).
                defaultProbability(basket_->defaultKeys()[iName])
                  ->defaultProbability(d);

            return conditionalRecoveryP(pDefUncond, iName, mktFactors);
        }

        Real conditionalRecoveryP(Real uncondDefP, Size iName, 
                                 const std::vector<Real>& mktFactors) const {//////////NEEDS ANOTHER VERSION USING THE P-INVERSE
///.........reviewing here...................

            const std::vector<std::vector<Real> >& fctrs_ = factorWeights();

            Size iRecovery = iName + basket_->size();// should be live pool

            const Real sumMs =
              std::inner_product(
                  fctrs_[iName /*+ basket_->size()*/].begin(),///betas_[iName].begin(), 
 //                 fctrs_[iName /*+ basket_->size()*/].end(), 
                  fctrs_[iName /*+ basket_->size()*/].end(), 
//?????                  fctrs_[iName /*+ basket_->size()*/].begin() + numNames_, 
                  mktFactors.begin() /*+ pool_->size()*/,
                  0.);
    // CACHE THESE ONES FOR EACH NAME?
 //           const Real sumBetaLoss = 
 //             std::inner_product(
 //                 fctrs_[iName/* + basket_->size()*/].begin(),
 ////                 fctrs_[iName /*+ basket_->size()*/].end(),
 //                 fctrs_[iName /*+ basket_->size()*/].begin() + numNames_,
 //                 fctrs_[iName /*+ basket_->size()*/].begin(), 
 //                 0.);
            const Real sumBetaLoss = 
              std::inner_product(
                  fctrs_[iName + numNames_].begin(),
                  fctrs_[iName + numNames_].end(),
                  fctrs_[iName + numNames_].begin(), 
                  0.);

            return cumulativeZ((sumMs + std::sqrt(1.-crossIdiosyncFctrs_[iName])
                     * std::sqrt(1.+modelA_*modelA_) * 
                       inverseCumulativeY(recoveries_[iName], iRecovery)
                - std::sqrt(crossIdiosyncFctrs_[iName]) * 
                    inverseCumulativeY(uncondDefP, iName))
                / std::sqrt(1.- sumBetaLoss + modelA_*modelA_ * 
                    (1.-crossIdiosyncFctrs_[iName])) );
        }
//---------------------------------------------------------------------------------------------------------------------------------------------------------------- TO MONTECARLO
        /*! Implements equation 42 on p.14 (second 
            Remember that for this call to make sense the sample used must be 
            one leading to a default.
        */
        Real conditionalRecovery(Real latentVarSample, Size iName, 
            const Date& d) const 
        {
            const boost::shared_ptr<Pool>& pool = basket_->pool();

            // retrieve the default probability for this name
            const Handle<DefaultProbabilityTermStructure>& dfts = 
                pool->get(basket_->names()[iName]).defaultProbability(
                    basket_->defaultKeys()[iName]);
            const Probability pdef = dfts->defaultProbability(d, true);
            // before asking for -\infty
            if (pdef < 1.e-10) return 0.;

            Size iRecovery = iName + numNames_;// should be live pool
            return cumulativeY(
                (latentVarSample - std::sqrt(crossIdiosyncFctrs_[iName]) 
                    * inverseCumulativeY(pdef, iName)) / 
                    (modelA_ * std::sqrt(1.-crossIdiosyncFctrs_[iName]))// cache the sqrts
                // cache this factor.
                +(1.+ 1./(modelA_*modelA_)) * 
                    inverseCumulativeY(recoveries_[iName], iRecovery) 
                , iRecovery);
        }
    ////ADD INTEGRATION OVER THE LATENT FACTOR TO OBTAIN THE PROB DENSITY OF A NAMES RECOVERY 
//----------------------------------------------------------------------------------------------------------------------------------------------------------------

        /* Due to the way the latent model is splitted in two parts, we call the
        base class for the default sample and the LM owned here for the RR model
        sample. This sample only makes sense if it led to a default.
        @param allFactors All sampled factors, default and RR valiables.
        @param iVar The index of the name for which we want the RR sample

        \todo Write vector version for all names' RRs 
        */
        Real latentRRVarValue(const std::vector<Real>& allFactors, 
            Size iName) const 
        {
            Size iRecovery = iName + numNames_;// should be live pool

            return latentVarValue(allFactors, iRecovery);


/*
            // remove the idiosyncratic variables corresponding to the default
            std::vector<Real> sample(allFactors.begin(), 
                allFactors.begin() + DefaultLatentModel<copulaPolicy>::numFactors());
            sample.insert(sample.end(), allFactors.begin() + 
                DefaultLatentModel<copulaPolicy>::numTotalFactors(), allFactors.end());
            // might get rid of the copy above if I rewrite this one here....
            return recoveryCrossSection_.latentVarValue(sample, iRRVar);
            */
        }

        // wrong name, its no loss
        Real expectedLossRR(const Date& d, Size iName) const {/////////THESE SHOULD BE CONST MEMBER FUNCTIONS!!!!!!!!!!!!!!
            const boost::shared_ptr<Pool>& pool = basket_->pool();

            Probability pDefUncond =
                pool->get(pool->names()[iName]).
                defaultProbability(basket_->defaultKeys()[iName])
                  ->defaultProbability(d);

            return integratedExpectedValue(
                boost::function<Real (const std::vector<Real>& v1)>(
                          boost::bind(
                          &SpotRecoveryLatentModel<copulaPolicy>::conditionalRecoveryP,
                          this,
                          pDefUncond,
                          iName,
                          _1)
                         ));
        }

        ////// -----------------IMPLEMENT EXPECTED RECOVERY -----------------------------------

/* NEED TO ADD DEFAULT TO ADD THIS METHOD
        Real conditionalExpLossRR(const Date& d, Size iName, 
            const std::vector<Real>& mktFactors) const 
        {////WRITE VERSION TAKING P-INV AND NOT THE DATE TO INTEGRATE BELOW
            const boost::shared_ptr<Pool>& pool = basket_->pool();
            Probability pDefUncond =
                pool->get(pool->names()[iName]).
                defaultProbability(basket_->defaultKeys()[iName])
                  ->defaultProbability(d);

            Real invP = inverseCumulativeY(pDefUncond, iName);

            return conditionalDefaultProbabilityInvP(invP, iName, mktFactors)
                * conditionalRecoveryP(pDefUncond, iName, mktFactors);//CHANGE TO VERSION USING THE P-INVERSE
        }
*/
        Real expectedLoss(const Date& d, Size iName) const {
            return integratedExpectedValue(
                boost::function<Real (const std::vector<Real>& v1)>(
                          boost::bind(
                          &SpotRecoveryLatentModel<copulaPolicy>::conditionalExpLossRR,
                          this,
                          d, //// USE THIS----->>>>  pDefUncond,
                          iName,
                          _1)
                         ));

        }

    };

    typedef SpotRecoveryLatentModel<GaussianCopulaPolicy> GaussianSpotLossLM;
    typedef SpotRecoveryLatentModel<TCopulaPolicy> TSpotLossLM;

}

#endif
