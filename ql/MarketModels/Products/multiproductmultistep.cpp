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

#include <ql/MarketModels/Products/multiproductmultistep.hpp>

namespace QuantLib {

    MultiProductMultiStep::MultiProductMultiStep(
            const std::vector<Time>& rateTimes)
    : rateTimes_(rateTimes) {}

    EvolutionDescription MultiProductMultiStep::suggestedEvolution() const {
        std::vector<Time> evolutionTimes(rateTimes_.size()-1);
        std::vector<Size> numeraires(evolutionTimes.size());
        for (Size i=0; i<evolutionTimes.size(); ++i) {
            evolutionTimes[i]=rateTimes_[i];
            // MoneyMarketPlus(1)
            numeraires[i]=i+1;
        }

        std::vector<std::pair<Size,Size> > relevanceRates(
            evolutionTimes.size());
        for (Size i=0; i<evolutionTimes.size(); ++i)
            relevanceRates[i] = std::make_pair(i, i+1);

        return EvolutionDescription(rateTimes_, evolutionTimes,
                                    numeraires, relevanceRates);
    }
}
