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

#ifndef quantlib_spotlosslatentmodel_hpp
#define quantlib_spotlosslatentmodel_hpp

#include <ql/experimental/credit/defaultprobabilitylatentmodel.hpp>

namespace QuantLib {

    /*! \brief Random spot recovery rate latent variable portfolio model.\par
    See: \par
    <b>A Spot Stochastic Recovery Extension of the Gaussian Copula</b> N.Bennani
         and J.Maetz, MPRA July 2009 \par
    <b>Extension of Spot Recovery model for Gaussian Copula</b> H.Li, October
        2009,  MPRA \par
    The model is adpated here for a multifactor set up and a generic copula so 
    it can be used for pricing in single factor mode or for risk metrics in its
    multifactor version.\par
    \todo Rewrite this model: the distribution of the spot recovery given
    default could be given as a functional of rr_i with the market factors and
    the rest of methods depend on this. That would offer a family of models.
    \todo Implement eq. 45 to have the EL(t) and be able to integrate the model
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
    private:
        const std::vector<Real> recoveries_;
        const Real modelA_;
        // products of default and recoveries factors, see refs ('covariances')
        std::vector<Real> crossIdiosyncFctrs_;
        mutable Size numNames_;
        mutable ext::shared_ptr<Basket> basket_;
        ext::shared_ptr<LMIntegration> integration_;
    protected:
        //! access to integration:
      const ext::shared_ptr<LMIntegration>& integration() const override { return integration_; }

    private:
        typedef typename copulaPolicy::initTraits initTraits;
    public:
        SpotRecoveryLatentModel(
            const std::vector<std::vector<Real> >& factorWeights,
            const std::vector<Real>& recoveries,
            Real modelA,
            LatentModelIntegrationType::LatentModelIntegrationType integralType,
            const initTraits& ini = initTraits()
            );

        void resetBasket(const ext::shared_ptr<Basket>& basket) const;
        Probability conditionalDefaultProbability(const Date& date, Size iName,
            const std::vector<Real>& mktFactors) const;
        Probability conditionalDefaultProbability(Probability prob, Size iName,
            const std::vector<Real>& mktFactors) const;
        Probability conditionalDefaultProbabilityInvP(Real invCumYProb, 
            Size iName, 
            const std::vector<Real>& m) const;
        /*! Expected conditional spot recovery rate. Conditional on a set of 
        systemic factors and default returns the integrated attainable recovery 
        values. \par
        Corresponds to a multifactor generalization of the model in eq. 44 
        on p.15 of <b>Extension of Spot Recovery Model for Gaussian Copula</b> 
        Hui Li. 2009  Only remember that \f$\rho_l Z \f$ there is here 
        (multiple betas): 
        \f$ \sum_k \beta_{ik}^l Z_k \f$ and that \f$ \rho_d \rho_l \f$ there is
        here: 
        \f$ \sum_k \beta_{ik}^d \beta_{ik}^l \f$ \par
        (d,l corresponds to first and last set of betas) 
        */
        Real expCondRecovery/*conditionalRecovery*/(const Date& d, Size iName, 
                                 const std::vector<Real>& mktFactors) const;
        Real expCondRecoveryP(Real uncondDefP, Size iName, 
                                 const std::vector<Real>& mktFactors) const;
        Real expCondRecoveryInvPinvRR(Real invUncondDefP, Real invUncondRR,
            Size iName, const std::vector<Real>& mktFactors) const;
        /*! Implements equation 42 on p.14 (second).
            Remember that for this call to make sense the sample used must be 
            one leading to a default. Theres no check on this. This member
            typically to be used within a simulation.
        */
        Real conditionalRecovery(Real latentVarSample, Size iName, 
            const Date& d) const;
        /*! Due to the way the latent model is splitted in two parts, we call 
        the base class for the default sample and the LM owned here for the RR 
        model sample. This sample only makes sense if it led to a default.
        @param allFactors All sampled factors, default and RR valiables.
        @param iName The index of the name for which we want the RR sample

        \todo Write vector version for all names' RRs 
        */
        Real latentRRVarValue(const std::vector<Real>& allFactors, 
            Size iName) const;
        Real conditionalExpLossRR(const Date& d, Size iName, 
            const std::vector<Real>& mktFactors) const;
        Real conditionalExpLossRRInv(Real invP, Real invRR, Size iName, 
            const std::vector<Real>& mktFactors) const;
        /*! Single name expected loss.\par 
        The main reason of this method is for the testing of this model. The 
        model is coherent in that it preserves the single name expected loss
        and thus is coherent with the single name CDS market when used in the
        pricing context. i.e. it should match: \f$pdef_i(d) \times RR_i \f$
        */
        Real expectedLoss(const Date& d, Size iName) const;
    };


    typedef SpotRecoveryLatentModel<GaussianCopulaPolicy> GaussianSpotLossLM;
    typedef SpotRecoveryLatentModel<TCopulaPolicy> TSpotLossLM;


    // ------------------------------------------------------------------------

    template <class CP>
    inline void
    SpotRecoveryLatentModel<CP>::resetBasket(const ext::shared_ptr<Basket>& basket) const {
        basket_ = basket;
        // in the future change 'size' to 'liveSize'
        QL_REQUIRE(basket_->size() == numNames_, 
            "Incompatible new basket and model sizes.");
    }

    template<class CP>
    inline Probability 
        SpotRecoveryLatentModel<CP>::conditionalDefaultProbability(
        const Date& date, 
        Size iName, const std::vector<Real>& mktFactors) const 
    {
        const ext::shared_ptr<Pool>& pool = basket_->pool();
        Probability pDefUncond =
            pool->get(pool->names()[iName]).
            defaultProbability(basket_->defaultKeys()[iName])
              ->defaultProbability(date);
        return conditionalDefaultProbability(pDefUncond, iName, mktFactors);
    }

    template<class CP>
    inline Probability 
        SpotRecoveryLatentModel<CP>::conditionalDefaultProbability(
        Probability prob, 
        Size iName, const std::vector<Real>& mktFactors) const 
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

    template<class CP>
    inline Probability 
        SpotRecoveryLatentModel<CP>::conditionalDefaultProbabilityInvP(
        Real invCumYProb, 
        Size iName, 
        const std::vector<Real>& m) const 
    {
        Real sumMs = 
            std::inner_product(this->factorWeights_[iName].begin(), 
                               this->factorWeights_[iName].end(), m.begin(), 0.);
        Real res = this->cumulativeZ((invCumYProb - sumMs) / 
                this->idiosyncFctrs_[iName] );
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE (res >= 0. && res <= 1.,
                    "conditional probability " << res << "out of range");
        #endif
    
        return res;
    }

    template<class CP>
    inline Real 
        SpotRecoveryLatentModel<CP>::expCondRecovery(const Date& d, 
        Size iName,
        const std::vector<Real>& mktFactors) const 
    {
    #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(mktFactors.size() == this->numFactors(), 
        "Realization of market factors and latent model size do not match");
    #endif
        const ext::shared_ptr<Pool>& pool = basket_->pool();
        Probability pDefUncond =
            pool->get(pool->names()[iName]).
            defaultProbability(basket_->defaultKeys()[iName])
              ->defaultProbability(d);

        return expCondRecoveryP(pDefUncond, iName, mktFactors);
    }

    template<class CP>
    inline Real SpotRecoveryLatentModel<CP>::expCondRecoveryP(
        Real uncondDefP, Size iName, const std::vector<Real>& mktFactors) const 
    {
        return expCondRecoveryInvPinvRR(
            inverseCumulativeY(uncondDefP, iName), 
            inverseCumulativeY(recoveries_[iName], iName + numNames_),
            iName, mktFactors);
    }

    template<class CP>
    Real SpotRecoveryLatentModel<CP>::expCondRecoveryInvPinvRR(
        Real invUncondDefP, 
        Real invUncondRR, 
        Size iName, 
        const std::vector<Real>& mktFactors) const 
    {
        const std::vector<std::vector<Real> >& fctrs_ = factorWeights();
        //Size iRR = iName + basket_->size();// should be live pool
        const Real sumMs =
          std::inner_product(fctrs_[iName].begin(), fctrs_[iName].end(), 
              mktFactors.begin(), 0.);
        const Real sumBetaLoss = 
          std::inner_product(fctrs_[iName + numNames_].begin(),
              fctrs_[iName + numNames_].end(),
              fctrs_[iName + numNames_].begin(), 
              0.);
        return this->cumulativeZ((sumMs + std::sqrt(1.-crossIdiosyncFctrs_[iName])
                 * std::sqrt(1.+modelA_*modelA_) * 
                   invUncondRR
            - std::sqrt(crossIdiosyncFctrs_[iName]) * 
                invUncondDefP
                )
            / std::sqrt(1.- sumBetaLoss + modelA_*modelA_ * 
                (1.-crossIdiosyncFctrs_[iName])) );
    }

    template<class CP>
    Real SpotRecoveryLatentModel<CP>::conditionalRecovery(Real latentVarSample,
        Size iName, const Date& d) const 
    {
        const ext::shared_ptr<Pool>& pool = basket_->pool();

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
                (modelA_ * std::sqrt(1.-crossIdiosyncFctrs_[iName]))
            // cache the sqrts
            // cache this factor.
            +std::sqrt(1.+ 1./(modelA_*modelA_)) * 
                inverseCumulativeY(recoveries_[iName], iRecovery) 
            , iRecovery);
    }

    template<class CP>
    inline Real SpotRecoveryLatentModel<CP>::latentRRVarValue(
        const std::vector<Real>& allFactors, 
        Size iName) const 
    {
        Size iRecovery = iName + numNames_;// should be live pool
        return latentVarValue(allFactors, iRecovery);
    }

    template<class CP>
    inline Real SpotRecoveryLatentModel<CP>::conditionalExpLossRR(const Date& d,
        Size iName, 
        const std::vector<Real>& mktFactors) const 
    {
        const ext::shared_ptr<Pool>& pool = basket_->pool();
        Probability pDefUncond =
            pool->get(pool->names()[iName]).
            defaultProbability(basket_->defaultKeys()[iName])
              ->defaultProbability(d);

        Real invP = inverseCumulativeY(pDefUncond, iName);
        Real invRR = inverseCumulativeY(recoveries_[iName], iName + numNames_);

        return conditionalExpLossRRInv(invP, invRR, iName, mktFactors);
    }

    template<class CP>
    inline Real SpotRecoveryLatentModel<CP>::conditionalExpLossRRInv(
        Real invP, 
        Real invRR,
        Size iName, 
        const std::vector<Real>& mktFactors) const 
    {
        return conditionalDefaultProbabilityInvP(invP, iName, mktFactors)
            * (1.-this->conditionalRecoveryInvPinvRR(invP, invRR, iName, mktFactors));
    }

    template<class CP>
    inline Real SpotRecoveryLatentModel<CP>::expectedLoss(const Date& d, 
        Size iName) const 
    {
        const ext::shared_ptr<Pool>& pool = basket_->pool();
        Probability pDefUncond =
            pool->get(pool->names()[iName]).
            defaultProbability(basket_->defaultKeys()[iName])
              ->defaultProbability(d);

        Real invP = inverseCumulativeY(pDefUncond, iName);
        Real invRR = inverseCumulativeY(recoveries_[iName], iName + numNames_);

        return integratedExpectedValue(
            [&](const std::vector<Real>& v){
                return conditionalExpLossRRInv(invP, invRR, iName, v);
            });
    }

    template<class CP>
    SpotRecoveryLatentModel<CP>::SpotRecoveryLatentModel(
        const std::vector<std::vector<Real> >& factorWeights,
        const std::vector<Real>& recoveries,
        Real modelA,
        LatentModelIntegrationType::LatentModelIntegrationType integralType,
        const typename CP::initTraits& ini
        ) 
    : LatentModel<CP>(factorWeights, ini),
      recoveries_(recoveries), 
      modelA_(modelA),
      numNames_(factorWeights.size()/2),
      integration_(LatentModel<CP>::IntegrationFactory::
        createLMIntegration(factorWeights[0].size(), integralType))
    {
        QL_REQUIRE(factorWeights.size() % 2 == 0, 
         "Number of RR variables must be equal to number of default variables");
        QL_REQUIRE(recoveries.size() == numNames_ , 
         "Number of recoveries does not match number of defaultable entities.");

        // reminder: first betas are default, last ones are recovery 
        for(Size iName=0; iName<numNames_; iName++) /// USE STL
            /* Corresponds to: (k denotes factor, i denotes modelled 
                variable -default and recoveries))
                \sum_k a^2_{i,k} a^2_{N+i,k}
            */
        {
            Real cumul = 0.;
            for(Size iB=0; iB<factorWeights[iName].size(); iB++)
                // actually this size is unique
                cumul += factorWeights[iName][iB] * 
                    factorWeights[iName][iB] * 
                    factorWeights[iName + numNames_][iB] * 
                    factorWeights[iName + numNames_][iB];
            crossIdiosyncFctrs_.push_back(cumul);
        }

    }


}

#endif
