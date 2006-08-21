/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/MarketModels/PseudoRoots/calibratedmarketmodel.hpp>

namespace QuantLib {

    CalibratedMarketModel::CalibratedMarketModel(
        const boost::shared_ptr<LmVolatilityModel>& volModel,
        const boost::shared_ptr<LmCorrelationModel>& corrModel,
        const EvolutionDescription& evolution,
        const Size numberOfFactors,
        const std::vector<Real>& initialRates,
        const std::vector<Real>& displacements)
    : CalibratedModel(volModel->params().size()+corrModel->params().size()),
      covarProxy_(new LfmCovarianceProxy(volModel, corrModel)),
      rateTimes_(evolution.rateTimes()),
      evolutionTimes_(evolution.evolutionTimes()),
      numberOfFactors_(numberOfFactors),
      initialRates_(initialRates),
      displacements_(displacements),
      pseudoRoots_(evolution.evolutionTimes().size())
    {
        const Size i=volModel->params().size();
        std::copy(volModel->params().begin(), volModel->params().end(),
                  arguments_.begin());
        std::copy(corrModel->params().begin(), corrModel->params().end(),
                  arguments_.begin()+i);

        for (Size k=0; k<evolutionTimes_.size(); ++k) {
            pseudoRoots_[k]=covarProxy_->diffusion(evolutionTimes_[k]);
        }
    }

    void CalibratedMarketModel::setParams(const Array& params)
    {
        CalibratedModel::setParams(params);

        const Size k=covarProxy_->volatilityModel()->params().size();
        covarProxy_->volatilityModel()->setParams(
            std::vector<Parameter>(arguments_.begin(), arguments_.begin()+k));
        covarProxy_->correlationModel()->setParams(
            std::vector<Parameter>(arguments_.begin()+k, arguments_.end()));

    }

}
