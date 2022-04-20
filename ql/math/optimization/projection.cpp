/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2013 Peter Caspers

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

#include <ql/math/optimization/projection.hpp>
#include <utility>

namespace QuantLib {

    Projection::Projection(const Array& parameterValues, std::vector<bool> fixParameters)
    : numberOfFreeParameters_(0), fixedParameters_(parameterValues),
      actualParameters_(parameterValues), fixParameters_(std::move(fixParameters)) {

        if (fixParameters_.empty())
            fixParameters_ =
                std::vector<bool>(actualParameters_.size(), false);

        QL_REQUIRE(fixedParameters_.size() == fixParameters_.size(),
                   "fixedParameters_.size()!=parametersFreedoms_.size()");
        for (auto&& fixParameter : fixParameters_)
            if (!fixParameter)
                numberOfFreeParameters_++;
        QL_REQUIRE(numberOfFreeParameters_ > 0, "numberOfFreeParameters==0");
    }

    void Projection::mapFreeParameters(const Array &parameterValues) const {

        QL_REQUIRE(parameterValues.size() == numberOfFreeParameters_,
                   "parameterValues.size()!=numberOfFreeParameters");
        Size i = 0;
        for (Size j = 0; j < actualParameters_.size(); j++)
            if (!fixParameters_[j])
                actualParameters_[j] = parameterValues[i++];

    }

    Array Projection::project(const Array &parameters) const {

        QL_REQUIRE(parameters.size() == fixParameters_.size(),
                   "parameters.size()!=parametersFreedoms_.size()");
        Array projectedParameters(numberOfFreeParameters_);
        Size i = 0;
        for (Size j = 0; j < fixParameters_.size(); j++)
            if (!fixParameters_[j])
                projectedParameters[i++] = parameters[j];
        return projectedParameters;

    }

    Array Projection::include(const Array &projectedParameters) const {

        QL_REQUIRE(projectedParameters.size() == numberOfFreeParameters_,
                   "projectedParameters.size()!=numberOfFreeParameters");
        Array y(fixedParameters_);
        Size i = 0;
        for (Size j = 0; j < y.size(); j++)
            if (!fixParameters_[j])
                y[j] = projectedParameters[i++];
        return y;

    }
}
