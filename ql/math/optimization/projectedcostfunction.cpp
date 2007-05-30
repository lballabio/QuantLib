/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2007 Giorgio Facchinetti

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

#include <ql/math/optimization/projectedcostfunction.hpp>

namespace QuantLib {

    ProjectedCostFunction::ProjectedCostFunction(
                const CostFunction& costFunction,
                const Array& parametersValues,
                const std::vector<bool>& parametersFreedoms)
    :numberOfFreeParameters_(0),
     fixedParameters_(parametersValues), 
     actualParameters_(parametersValues), 
     parametersFreedoms_(parametersFreedoms),
     costFunction_(costFunction){
         QL_REQUIRE(fixedParameters_.size()==parametersFreedoms_.size(),
             "fixedParameters_.size()!=parametersFreedoms_.size()");

         for (Size i=0; i<parametersFreedoms_.size(); i++)
            if(!parametersFreedoms_[i]) numberOfFreeParameters_++;
         QL_REQUIRE(numberOfFreeParameters_>0,"numberOfFreeParameters==0");
    } 

    void ProjectedCostFunction::mapFreeParameters
                                    (const Array& parametersValues) const {
        QL_REQUIRE(parametersValues.size()==numberOfFreeParameters_,
                   "parametersValues.size()!=numberOfFreeParameters");
        Size i = 0;
        for (Size j=0; j<actualParameters_.size(); j++)
            if(!parametersFreedoms_[j]) 
                actualParameters_[j] = parametersValues[i++];
    }
    
    Real ProjectedCostFunction::value(const Array& freeParameters) const {
        mapFreeParameters(freeParameters);
        return costFunction_.value(actualParameters_);
    }
    
    Disposable<Array> ProjectedCostFunction::values(
                                        const Array& freeParameters) const{
        mapFreeParameters(freeParameters);
        return costFunction_.values(actualParameters_);
    }

    Disposable<Array> ProjectedCostFunction::project
                                            (const Array& parameters) const{
        QL_REQUIRE(parameters.size()==parametersFreedoms_.size(),
            "parameters.size()!=parametersFreedoms_.size()");
        Array projectedParameters(numberOfFreeParameters_);
        Size i = 0;
        for (Size j=0; j<parametersFreedoms_.size(); j++)
            if(!parametersFreedoms_[j]) 
                projectedParameters[i++] = parameters[j];
        return projectedParameters;
    }

    Disposable<Array> ProjectedCostFunction::include(
                                    const Array& projectedParameters) const{
        QL_REQUIRE(projectedParameters.size()==numberOfFreeParameters_,
                   "projectedParameters.size()!=numberOfFreeParameters");
        Array y(fixedParameters_);
        Size i = 0;
        for (Size j=0; j<y.size(); j++)
            if(!parametersFreedoms_[j]) 
                y[j] = projectedParameters[i++];
        return y;
    }

    
}
