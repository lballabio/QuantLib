/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Mark Joshi

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

#include <ql/MarketModels/Models/forwardtocoterminaladapter.hpp>
#include <ql/MarketModels/CurveStates/lmmcurvestate.hpp>
#include <ql/MarketModels/swapforwardmappings.hpp>
#include <ql/Utilities/dataformatters.hpp>


namespace QuantLib
{
    ForwardToCoterminalAdapter::ForwardToCoterminalAdapter(const boost::shared_ptr<MarketModel>& fwdModel)
    : fwdModel_(fwdModel),
      numberOfFactors_(fwdModel->numberOfFactors()),
      numberOfRates_(fwdModel->numberOfRates()),
      numberOfSteps_(fwdModel->numberOfSteps()),
      pseudoRoots_(numberOfSteps_, Matrix(numberOfRates_, numberOfFactors_)),
      covariance_(numberOfSteps_, Matrix(numberOfRates_, numberOfRates_)),
      totalCovariance_(numberOfSteps_, Matrix(numberOfRates_, numberOfRates_))
    {

        const std::vector<Spread>& displacements =
            fwdModel_->displacements();
        for (Size i = 1; i<displacements.size(); ++i) {
            QL_REQUIRE(displacements[i]==displacements[0],
                       io::ordinal(i) << " displacement (" <<
                       displacements[i] << ") not equal to the previous ones"
                       " (" << displacements[0] << ")");
        }

        const std::vector<Time>& rateTimes =
            fwdModel_->evolution().rateTimes();
        const std::vector<Time>& evolutionTimes =
            fwdModel_->evolution().evolutionTimes();
        for (Size i = 0; i<rateTimes.size() && rateTimes[i]<=evolutionTimes.back(); ++i) {
            QL_REQUIRE(std::find(evolutionTimes.begin(), evolutionTimes.end(),
                                 rateTimes[i])!=evolutionTimes.end(),
                                 "skipping " << io::ordinal(i) << " rate time");
        }

        LMMCurveState cs(rateTimes);
        const std::vector<Rate>& initialFwdRates = fwdModel_->initialRates();
        cs.setOnForwardRates(initialFwdRates);
        initialRates_ = cs.coterminalSwapRates();

        Matrix zMatrix = SwapForwardMappings::coterminalSwapZedMatrix(
            cs, displacements[0]);
        const std::vector<Size>& alive = fwdModel_->evolution().firstAliveRate();

        for (Size k = 0; k<numberOfSteps_; ++k) {
            pseudoRoots_[k]=zMatrix*fwdModel_->pseudoRoot(k);
            for (Size i=0; i<alive[k]; ++i)
                std::fill(pseudoRoots_[k].row_begin(i),
                          pseudoRoots_[k].row_end(i),
                          0.0);
            // FIXME
            covariance_[k]=pseudoRoots_[k]*transpose(pseudoRoots_[k]);
            totalCovariance_[k] = covariance_[k];
            if (k>0)
                totalCovariance_[k] += totalCovariance_[k-1];
        }

    }

}

