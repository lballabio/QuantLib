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


#ifndef quantlib_constantloss_latentmodel_hpp
#define quantlib_constantloss_latentmodel_hpp

#include <ql/experimental/credit/defaultprobabilitylatentmodel.hpp>
// take the loss model to a different file and avoid this inclusion
#include <ql/experimental/credit/defaultlossmodel.hpp>

namespace QuantLib {

    /*! Constant deterministic loss amount default latent model. Integrable 
        implementation.
    */
    /* \todo: 
        Several options: Refer to a set of individual RR models, have quotes to
        RRs (registered)
    */
    template <class copulaPolicy>
    class ConstantLossLatentmodel : public DefaultLatentModel<copulaPolicy> {
    private:
        const std::vector<Real> recoveries_;
        typedef typename copulaPolicy::initTraits initTraits;
    public:
        ConstantLossLatentmodel(
            const std::vector<std::vector<Real> >& factorWeights,
            const std::vector<Real>& recoveries,
            LatentModelIntegrationType::LatentModelIntegrationType integralType,
            const initTraits& ini = initTraits()            
            ) 
        : DefaultLatentModel<copulaPolicy>(factorWeights, integralType, ini),
          recoveries_(recoveries) {

              QL_REQUIRE(recoveries.size() == factorWeights.size(), 
                "Incompatible factors and recovery sizes.");
        }

        ConstantLossLatentmodel(
            const Handle<Quote>& mktCorrel,
            const std::vector<Real>& recoveries,
            LatentModelIntegrationType::LatentModelIntegrationType integralType,
            Size nVariables,
            const initTraits& ini = initTraits()            
            ) 
        : DefaultLatentModel<copulaPolicy>(mktCorrel, nVariables,
                                           integralType, ini),
          recoveries_(recoveries) {
            // actually one could define the other and get rid of the variable 
            // here and in other similar models
            QL_REQUIRE(recoveries.size() == nVariables, 
                "Incompatible model and recovery sizes.");
        }

        Real conditionalRecovery(const Date& d, Size iName, 
                                 const std::vector<Real>& mktFactors) const {
            return recoveries_[iName];
        }

        Real conditionalRecovery(Probability uncondDefP, Size iName, 
                                 const std::vector<Real>& mktFactors) const {
            return recoveries_[iName];
        }

        Real conditionalRecoveryInvP(Real invUncondDefP, Size iName, 
                                 const std::vector<Real>& mktFactors) const {
            return recoveries_[iName];
        }

        Real conditionalRecovery(Real latentVarSample, 
            Size iName, const Date& d) const {
            return recoveries_[iName];
        }

        const std::vector<Real>& recoveries() const {
            return recoveries_;
        }

        // this is really an interface to rr models even if not imposed. Default
        // loss models do have an interface for this one. Enforced only through
        // duck typing.
        Real expectedRecovery(const Date& d, Size iName, 
            const DefaultProbKey& defKeys) const {
            return recoveries_[iName];
        }
    };

    typedef ConstantLossLatentmodel<GaussianCopulaPolicy> 
        GaussianConstantLossLM;
    typedef ConstantLossLatentmodel<TCopulaPolicy> TConstantLossLM;


    /*! ConstantLossLatentModel interface for loss models. 
    While it does not provide distribution type losses (e.g. expected tranche 
    losses) because it lacks an integration algorithm it serves to allow 
    pricing of digital type products like NTDs.

    Alternatively fuse with the aboves class.
    */
    template <class copulaPolicy>
    class ConstantLossModel : 
        public virtual ConstantLossLatentmodel<copulaPolicy>, 
        public virtual DefaultLossModel 
    {
    public:
        ConstantLossModel(
            const std::vector<std::vector<Real> >& factorWeights,
            const std::vector<Real>& recoveries,
            LatentModelIntegrationType::LatentModelIntegrationType integralType,
            const typename copulaPolicy::initTraits& ini = 
                copulaPolicy::initTraits()) 
        : ConstantLossLatentmodel<copulaPolicy>(factorWeights, recoveries, 
            integralType, ini) {}

        ConstantLossModel(
            const Handle<Quote>& mktCorrel,
            const std::vector<Real>& recoveries,
            LatentModelIntegrationType::LatentModelIntegrationType integralType,
            Size nVariables,
            const typename copulaPolicy::initTraits& ini = 
                copulaPolicy::initTraits()) 
        : ConstantLossLatentmodel<copulaPolicy>(mktCorrel, recoveries, 
            integralType, nVariables,ini) {}

    protected:
        //Disposable<std::vector<Probability> > probsBeingNthEvent(
        //    Size n, const Date& d) const {
        //    return 
        //      ConstantLossLatentmodel<copulaPolicy>::probsBeingNthEvent(n, d);
        //}
        Real defaultCorrelation(const Date& d, Size iName, 
            Size jName) const {
            return 
              ConstantLossLatentmodel<copulaPolicy>::defaultCorrelation(d, 
                iName, jName);
        }
        Probability probAtLeastNEvents(Size n, const Date& d) const {
            return 
              ConstantLossLatentmodel<copulaPolicy>::probAtLeastNEvents(n, d);
        }
        Real expectedRecovery(const Date& d, Size iName, 
            const DefaultProbKey& k) const {
                return 
                    ConstantLossLatentmodel<copulaPolicy>::expectedRecovery(d, 
                        iName, k);
        }
    private:
        virtual void resetModel() {
            // update the default latent model we derive from
            DefaultLatentModel<copulaPolicy>::resetBasket(
                DefaultLossModel::basket_.currentLink());// forces interface
        }

    };

}

#endif
