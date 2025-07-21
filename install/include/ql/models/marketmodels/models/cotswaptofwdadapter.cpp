/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Mark Joshi
 Copyright (C) 2007 StatPro Italia srl

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

#include <ql/models/marketmodels/models/cotswaptofwdadapter.hpp>
#include <ql/models/marketmodels/curvestates/coterminalswapcurvestate.hpp>
#include <ql/models/marketmodels/swapforwardmappings.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/utilities/dataformatters.hpp>

namespace QuantLib {

    CotSwapToFwdAdapter::CotSwapToFwdAdapter(
                               const ext::shared_ptr<MarketModel>& ctModel)
    : coterminalModel_(ctModel),
      numberOfFactors_(ctModel->numberOfFactors()),
      numberOfRates_(ctModel->numberOfRates()),
      numberOfSteps_(ctModel->numberOfSteps()),
      pseudoRoots_(numberOfSteps_, Matrix(numberOfRates_, numberOfFactors_))
    {

        const std::vector<Spread>& displacements =
            coterminalModel_->displacements();
        for (Size i = 1; i<displacements.size(); ++i) {
            QL_REQUIRE(displacements[i]==displacements[0],
                       io::ordinal(i+1) << " displacement (" <<
                       displacements[i] << ") not equal to the previous ones"
                       " (" << displacements[0] << ")");
        }

        const std::vector<Time>& rateTimes =
            coterminalModel_->evolution().rateTimes();
        // we must ensure we step through all rateTimes
        const std::vector<Time>& evolutionTimes =
            coterminalModel_->evolution().evolutionTimes();
        for (Size i = 0;
             i<rateTimes.size() && rateTimes[i]<=evolutionTimes.back(); ++i) {
            QL_REQUIRE(std::find(evolutionTimes.begin(), evolutionTimes.end(),
                                 rateTimes[i])!=evolutionTimes.end(),
                                 "skipping " << io::ordinal(i+1) << " rate time");
        }

        CoterminalSwapCurveState cs(rateTimes);
        const std::vector<Rate>& initialCoterminalSwapRates =
            coterminalModel_->initialRates();
        cs.setOnCoterminalSwapRates(initialCoterminalSwapRates);
        initialRates_ = cs.forwardRates();

        Matrix zedMatrix = SwapForwardMappings::coterminalSwapZedMatrix(
            cs, displacements[0]);
        Matrix invertedZedMatrix = inverse(zedMatrix);

        const std::vector<Size>& alive =
            coterminalModel_->evolution().firstAliveRate();
        for (Size k = 0; k<numberOfSteps_; ++k) {
            pseudoRoots_[k]=invertedZedMatrix*coterminalModel_->pseudoRoot(k);
            for (Size i=0; i<alive[k]; ++i)
                std::fill(pseudoRoots_[k].row_begin(i),
                          pseudoRoots_[k].row_end(i),
                          0.0);
        }
    }


    CotSwapToFwdAdapterFactory::CotSwapToFwdAdapterFactory(
             const ext::shared_ptr<MarketModelFactory>& coterminalFactory)
    : coterminalFactory_(coterminalFactory) {
        registerWith(coterminalFactory);
    }

    ext::shared_ptr<MarketModel>
    CotSwapToFwdAdapterFactory::create(
                                        const EvolutionDescription& evolution,
                                        Size numberOfFactors) const {
        ext::shared_ptr<MarketModel> coterminalModel =
            coterminalFactory_->create(evolution,numberOfFactors);
        return ext::shared_ptr<MarketModel>(
                             new CotSwapToFwdAdapter(coterminalModel));
    }

    void CotSwapToFwdAdapterFactory::update() {
        notifyObservers();
    }

}

