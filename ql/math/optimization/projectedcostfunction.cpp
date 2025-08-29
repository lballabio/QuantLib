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
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/math/optimization/projectedcostfunction.hpp>

namespace QuantLib {

    ProjectedCostFunction::ProjectedCostFunction(
                const CostFunction& costFunction,
                const Array& parameterValues,
                const std::vector<bool>& fixParameters)
        : Projection(parameterValues, fixParameters), costFunction_(costFunction) {}

    ProjectedCostFunction::ProjectedCostFunction(
                const CostFunction& costFunction,
                const Projection& projection)
        : Projection(projection), costFunction_(costFunction) {}

    Real ProjectedCostFunction::value(const Array& freeParameters) const {
        mapFreeParameters(freeParameters);
        return costFunction_.value(actualParameters_);
    }

    Array ProjectedCostFunction::values(const Array& freeParameters) const{
        mapFreeParameters(freeParameters);
        return costFunction_.values(actualParameters_);
    }

}
