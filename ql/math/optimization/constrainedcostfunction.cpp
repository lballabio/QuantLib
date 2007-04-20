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
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/math/optimization/constrainedcostfunction.hpp>

namespace QuantLib {

    ConstrainedCostFunction::ConstrainedCostFunction(
                const CostFunction& costFunction,
                const Array& x,
                const std::vector<bool>& argAreFixed)
    :numberOfFreeParameters_(0),
     arg_(x), newArg_(x), 
     argAreFixed_(argAreFixed),
     costFunction_(costFunction){
         QL_REQUIRE(arg_.size()==argAreFixed_.size(),"arg_.size()!=argAreFixed_.size()");

         for (Size i=0; i<argAreFixed_.size(); i++)
            if(!argAreFixed_[i]) numberOfFreeParameters_++;
         QL_REQUIRE(numberOfFreeParameters_>0,"numberOfFreeParameters==0");
    } 

    void ConstrainedCostFunction::resetNewArg(const Array& x) const {
        QL_REQUIRE(x.size()==numberOfFreeParameters_,"x.size()!=numberOfFreeParameters");
        Size i = 0;
        for (Size j=0; j<newArg_.size(); j++)
            if(!argAreFixed_[j]) newArg_[j] = x[i++];
    }
    
    Real ConstrainedCostFunction::value(const Array& x) const {
        resetNewArg(x);
        return costFunction_.value(newArg_);
    }
    
    Disposable<Array> ConstrainedCostFunction::values(const Array& x) const{
        resetNewArg(x);
        return costFunction_.values(newArg_);
    }

    Disposable<Array> ConstrainedCostFunction::project(const Array& x) const{
        QL_REQUIRE(x.size()==argAreFixed_.size(),"x.size()!=argAreFixed_.size()");
        Array projectedx(numberOfFreeParameters_);
        Size i = 0;
        for (Size j=0; j<argAreFixed_.size(); j++)
            if(!argAreFixed_[j]) projectedx[i++] = x[j];

        return projectedx;
    }

    Disposable<Array> ConstrainedCostFunction::include(const Array& projectedx) const{
        QL_REQUIRE(projectedx.size()==numberOfFreeParameters_,
                   "projectedx.size()!=numberOfFreeParameters");
        Array y(arg_);
        Size i = 0;
        for (Size j=0; j<y.size(); j++)
            if(!argAreFixed_[j]) y[j] = projectedx[i++];
        return y;
    }

    
}
