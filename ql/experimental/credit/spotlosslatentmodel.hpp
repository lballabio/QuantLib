#ifndef quantlib_spotlosslatentmodel_hpp
#define quantlib_spotlosslatentmodel_hpp

#include <ql/experimental/credit/defaultprobabilitylatentmodel.hpp>

namespace QuantLib {

    /* Integrable random spot recovery rate latent variable portfolio model.

    */
    template <class copulaPolicy>
    class SpotRecoveryLatentModel : public DefaultLatentModel<copulaPolicy> {// ----- not deriving from a RR model........, DefaultLatentModel<CP>::size() will return an incorrect value.
    private:
        const std::vector<Real> recoveries_;
        const Real modelA_;
        //! \products of default and recoveries factors, see literature. ('covariances')
        std::vector<Real> crossIdiosyncFctrs_;
        LatentModel<copulaPolicy> recoveryCrossSection_;
    public:
        SpotRecoveryLatentModel(
            const std::vector<std::vector<Real> >& factorWeights,
            const std::vector<Real>& recoveries,
            Real modelA,
            LatentModelIntegrationType::LatentModelIntegrationType integralType,
            const typename copulaPolicy::initTraits& ini = 
                copulaPolicy::initTraits()
            ) 
        : recoveries_(recoveries), 
          modelA_(modelA),
          recoveryCrossSection_(std::vector<std::vector<Real> > (factorWeights.begin() + factorWeights.size()/2, factorWeights.end()), ini), //<- splitting duplicates machinery, gets default items for free
          // NEED TO PASS ONLY "HALF" OF THE FACTORS, default cross section should be presented first!!! 
          DefaultLatentModel<copulaPolicy>(std::vector<std::vector<Real> > (factorWeights.begin(), factorWeights.begin() + factorWeights.size()/2), integralType, ini) {
    //...      DefaultProbLM<copula>(pool, betas, quadOrder) {
            // but I passing a pool and not a basket
            //QL_REQUIRE(recoveries_.size() == basket->remainingNames().size(), "");
QL_REQUIRE(factorWeights.size() % 2 == 0, "Number of recovery variables must be equal to number of default variables");
/* NO BASKET YET!!!!!!!!!!!!!!!! DO THIS IN SETUPBASKET!!!!  ***************
                    // These tests apply if the betas matrix size is of the order of the true model: Ps+RRs
                    QL_REQUIRE(basket_->size() == recoveries.size(),
                        "Incompatible pool and recoveries sizes.");
                    QL_REQUIRE(basket_->size() + recoveries.size() == factorWeights.size(),
                        "Incompatible number of betas with model dimensions.");
                    //QL_REQUIRE(model_A >= 0., 
                    //    "Recovery latent model volatility factor out of bounds");
*****************************************************************/

    Size modelSize = factorWeights.size()/2; // will have to match the basket size when asigned
QL_REQUIRE(recoveries.size() == modelSize , "Number of recoveries does not match number of defaultable entities.");

            // reminder: first betas are default, last ones are recovery 
            for(Size iName=0; iName<modelSize; iName++) /// USE STL AT THIS LEVEL TOO..
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
     //this is only formally true                   factorWeights_[iName + pool->size()][iB] * factorWeights_[iName + pool->size()][iB];
                        factorWeights[iName + modelSize][iB] * factorWeights[iName + modelSize][iB];
                crossIdiosyncFctrs_.push_back(cumul);
            }

          }

//overwrite to return true size:
        Size size()const {return 2 * DefaultLatentModel<copulaPolicy>::size();}

            //! Expected (it is volatile) conditional (to the latent factor) spot recovery rate. It is also conditional to default taking place.
            //  corresponds to a multifactor generalization of the model in eq. 44 on p.15 of 'Extension of Spot Recovery Model for Gaussian Copula' Hui Li. 2009  Only remember that \rho_l Z there is here (multiple betas) \sum_k \beta_{ik}^l Z_k and that \rho_d \rho_l there is here: \sum_k \beta_{ik}^d \beta_{ik}^l (d,l corresponds to first and last set of betas) 

        // THIS ONE  NEED TO HAVE ALL THE CORREL/BETAS FACTORS REVISED, THINK THE SQRTS ARE WRONG...
        Real conditionalRecovery(const Date& d, Size iName, 
                                 const std::vector<Real>& mktFactors) const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
          //  QL_REQUIRE(mktFactors.size() == this->size(), 
            QL_REQUIRE(mktFactors.size() == this->numFactors(), 
                "Realization of market factors and latent model size do not match");///////?????s this test still valid???
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

            const Real sumMs =
              std::inner_product(
                  fctrs_[iName /*+ basket_->size()*/].begin(),///betas_[iName].begin(), 
                  fctrs_[iName /*+ basket_->size()*/].end(), 
                  mktFactors.begin() /*+ pool_->size()*/,
                  0.);
            const Real sumBetaLoss = 
              std::inner_product(
                  fctrs_[iName/* + basket_->size()*/].begin(),
                  fctrs_[iName /*+ basket_->size()*/].end(),
                  fctrs_[iName /*+ basket_->size()*/].begin(), 
                  0.);
          ////  return cumulativeRR_((sumMs
            return cumulativeZ((sumMs
                + std::sqrt(1.-crossIdiosyncFctrs_[iName]) * 

                //still I dont like, works here but its missing concepts, not clean:
    //////////                std::sqrt(1.+modelA_*modelA_) * inverseCumulativeRR_(recoveries_[iName]) 
                    std::sqrt(1.+modelA_*modelA_) * inverseCumulativeY(recoveries_[iName], iName/* + basket_->size()*/)  // I just added the pool->size() stride, it was missing, is the RR inve
    ////////            - std::sqrt(crossIdiosyncFctrs_[iName])  * inverseCumulative_(pdef)) 
                - std::sqrt(crossIdiosyncFctrs_[iName])  * inverseCumulativeY(uncondDefP, iName)) // revise with eqs I  might have screwed it up
                / std::sqrt(1.- sumBetaLoss + modelA_*modelA_ * (1.-crossIdiosyncFctrs_[iName])) );
        }
//---------------------------------------------------------------------------------------------------------------------------------------------------------------- TO MONTECARLO
        /*! Implements equation 42 on p.14 (second 
            Remember that for this call to make sense the sample used must be one leading to a default.
        */
        Real conditionalRecovery(Real latentVarSample, Size iName, const Date& d) const {
const boost::shared_ptr<Pool>& pool = basket_->pool();

            // retrieve the default probability for this name
            const Handle<DefaultProbabilityTermStructure>& dfts = 
                pool->get(basket_->names()[iName]).defaultProbability(basket_->defaultKeys()[iName]);
            const Probability pdef = dfts->defaultProbability(d, true);

            ////Probability pdef = 
            ////    pool_->get(pool_->names()[iName]).
            ////      defaultProbability(pool_->defaultKeys()[iName])->defaultProbability(d, true);
            Size iRecovery = iName + basket_->size();// should be live pool
        ///////////////////    return jointCopula::cumulativeRR_(
            return cumulativeY(
        //        (latentVarSample - std::sqrt(crossIdiosyncFctrs_[iName]) * inverseCumulative_(pdef)) / 
                (latentVarSample - std::sqrt(crossIdiosyncFctrs_[iName]) 
                    * inverseCumulativeY(pdef, iName)) / 
                    (modelA_ * std::sqrt(1.-crossIdiosyncFctrs_[iName]))// cache the sqrts
                // cache this factor.
        //////        +(1.+ 1./(modelA_*modelA_)) * jointCopula::inverseCumulativeRR_(recoveries_[iName])
                +(1.+ 1./(modelA_*modelA_)) * inverseCumulativeY(recoveries_[iName], iRecovery) 
                , iRecovery
                );
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
            Size iVar) const {
            // remove the idiosyncratic variables corresponding to the default
            std::vector<Real> sample(allFactors.begin(), 
                allFactors.begin() + DefaultLatentModel<copulaPolicy>::numFactors());
            sample.insert(sample.end(), allFactors.begin() + 
                DefaultLatentModel<copulaPolicy>::numTotalFactors(), allFactors.end());
            // might get rid of the copy above if I rewrite this one here....
            return recoveryCrossSection_.latentVarValue(sample, iVar);
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

        Real conditionalExpLossRR(const Date& d, Size iName, const std::vector<Real>& mktFactors) const {////WRITE VERSION TAKING P-INV AND NOT THE DATE TO INTEGRATE BELOW
                        const boost::shared_ptr<Pool>& pool = basket_->pool();
            Probability pDefUncond =
                pool->get(pool->names()[iName]).
                defaultProbability(basket_->defaultKeys()[iName])
                  ->defaultProbability(d);
Real invP = inverseCumulativeY(pDefUncond, iName);
return conditionalDefaultProbabilityInvP(invP, iName, mktFactors)
    * conditionalRecoveryP(pDefUncond, iName, mktFactors);//CHANGE TO VERSION USING THE P-INVERSE
        }
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
