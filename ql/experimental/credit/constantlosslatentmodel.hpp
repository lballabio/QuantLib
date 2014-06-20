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
    public:
        ConstantLossLatentmodel(
            const std::vector<std::vector<Real> >& factorWeights,
            const std::vector<Real>& recoveries,
            LatentModelIntegrationType::LatentModelIntegrationType integralType,
            const typename copulaPolicy::initTraits& ini = 
                copulaPolicy::initTraits()            
            ) 
        : recoveries_(recoveries), 
          DefaultLatentModel<copulaPolicy>(factorWeights, integralType, ini) { }

        ConstantLossLatentmodel(
            const Handle<Quote>& mktCorrel,
            const std::vector<Real>& recoveries,
            LatentModelIntegrationType::LatentModelIntegrationType integralType,
            Size nVariables,
            const typename copulaPolicy::initTraits& ini = 
                copulaPolicy::initTraits()            
            ) 
        : recoveries_(recoveries), 
          DefaultLatentModel<copulaPolicy>(mktCorrel, nVariables,
            integralType, ini) { }

        Real conditionalRecovery(const Date& d, Size iName, 
                                 const std::vector<Real>& mktFactors) const {
            return recoveries_[iName];
        }

        Real conditionalRecovery(Real uncondDefP, Size iName, 
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

}

#endif
