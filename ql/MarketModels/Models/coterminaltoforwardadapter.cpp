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

#include <ql/MarketModels/Models/coterminaltoforwardadapter.hpp>
#include <ql/MarketModels/CurveStates/coterminalswapcurvestate.hpp>
#include <ql/MarketModels/swapforwardmappings.hpp>
#include <ql/Utilities/dataformatters.hpp>


namespace QuantLib
{
    CoterminalToForwardAdapter::CoterminalToForwardAdapter(const boost::shared_ptr<MarketModel>& fwdModel)
    : coterminalModel_(fwdModel),
      numberOfFactors_(fwdModel->numberOfFactors()),
      numberOfRates_(fwdModel->numberOfRates()),
      numberOfSteps_(fwdModel->numberOfSteps()),
      pseudoRoots_(numberOfSteps_, Matrix(numberOfRates_, numberOfFactors_)),
      covariance_(numberOfSteps_, Matrix(numberOfRates_, numberOfRates_)),
      totalCovariance_(numberOfSteps_, Matrix(numberOfRates_, numberOfRates_))
    {

        const std::vector<Spread>& displacements =
            coterminalModel_->displacements();
        for (Size i = 1; i<displacements.size(); ++i) {
            QL_REQUIRE(displacements[i]==displacements[0],
                       io::ordinal(i) << " displacement (" <<
                       displacements[i] << ") not equal to the previous ones"
                       " (" << displacements[0] << ")");
        }

        const std::vector<Time>& rateTimes =
            coterminalModel_->evolution().rateTimes();
        CoterminalSwapCurveState cs(rateTimes);
        const std::vector<Rate>& initialCoterminalSwapRates =
            coterminalModel_->initialRates();
        cs.setOnCoterminalSwapRates(initialCoterminalSwapRates);
        initialRates_ = cs.forwardRates();

        Matrix zedMatrix = SwapForwardMappings::coterminalSwapZedMatrix(
            cs, displacements[0]);
        Matrix invertedZedMatrix = zedMatrix; //FIXME

        for (Size k = 0; k<numberOfSteps_; ++k) {
            pseudoRoots_[k]=invertedZedMatrix*coterminalModel_->pseudoRoot(k);
            // FIXME
            covariance_[k]=pseudoRoots_[k]*transpose(pseudoRoots_[k]);
            totalCovariance_[k] = covariance_[k];
            if (k>0)
                totalCovariance_[k] += totalCovariance_[k-1];
        }

    }

}

