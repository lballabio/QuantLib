/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/errors.hpp>
#include <ql/models/marketmodels/products/multiproductonestep.hpp>
#include <utility>


namespace QuantLib {

    MultiProductOneStep::MultiProductOneStep(std::vector<Time> rateTimes)
    : rateTimes_(std::move(rateTimes)) {
        QL_REQUIRE(rateTimes_.size()>1,
                   "Rate times must contain at least two values");
        std::vector<Time> evolutionTimes(1, rateTimes_[rateTimes_.size()-2]);
        std::vector<std::pair<Size,Size> > relevanceRates(1);
        relevanceRates[0] =
            std::make_pair<Size,Size>(0, rateTimes_.size()-1);

        evolution_ = EvolutionDescription(rateTimes_, evolutionTimes,
                                          relevanceRates);
    }

    const EvolutionDescription& MultiProductOneStep::evolution() const {
        return evolution_;
    }

    std::vector<Size> MultiProductOneStep::suggestedNumeraires() const {
        // Terminal measure
        return std::vector<Size>(1, rateTimes_.size()-1);
    }

}
