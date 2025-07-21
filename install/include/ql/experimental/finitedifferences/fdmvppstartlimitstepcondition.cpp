/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011, 2012 Klaus Spanderen

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

/*! \file fdmvppstartlimitstepcondition.cpp
*/

#include <algorithm>
#include <ql/math/array.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/experimental/finitedifferences/fdmvppstartlimitstepcondition.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopiterator.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>


namespace QuantLib {

    FdmVPPStartLimitStepCondition::FdmVPPStartLimitStepCondition(
        const FdmVPPStepConditionParams& params,
        Size nStarts,
        const FdmVPPStepConditionMesher& mesh,
        const ext::shared_ptr<FdmInnerValueCalculator>& gasPrice,
        const ext::shared_ptr<FdmInnerValueCalculator>& sparkSpreadPrice)
    : FdmVPPStepCondition(params,
                          nStates(params.tMinUp, params.tMinDown, nStarts),
                          mesh, gasPrice, sparkSpreadPrice),
      nStarts_(nStarts) {
        QL_REQUIRE(tMinUp_ > 0,   "minimum up time must be greater than one");
        QL_REQUIRE(tMinDown_ > 0, "minimum down time must be greater than one");
    }

    Array FdmVPPStartLimitStepCondition::changeState(
        const Real gasPrice, const Array& state, Time) const {

        const Real startUpCost
                = startUpFixCost_ + (gasPrice + fuelCostAddon_)*startUpFuel_;

        Array retVal(state.size());
        const Size sss = 2*tMinUp_ + tMinDown_;

        for (Size i=0; i < nStates_; ++i) {
            const Size j = i % sss;

            if (j < tMinUp_-1) {
                retVal[i] = std::max(state[i+1], state[tMinUp_+i+1]);
            }
            else if (j == tMinUp_-1) {
                retVal[i] = std::max({state[i+tMinUp_+1], state[i], state[i+tMinUp_]});
            }
            else if (j < 2*tMinUp_) {
                retVal[i] = retVal[i-tMinUp_];
            }
            else if (j <  2*tMinUp_+tMinDown_-1) {
                retVal[i] = state[i+1];
            }
            else if (nStarts_ == Null<Size>()) {
                retVal[i] = std::max(state[i],
                    std::max(state.front(), state[tMinUp_]) - startUpCost);

            }
            else if (i >= sss) {
                retVal[i] = std::max(state[i],
                    std::max(state[i+1-2*sss], state[i+1-2*sss+tMinUp_])
                            - startUpCost);
            }
            else {
                retVal[i] = state[i];
            }
        }

        return retVal;
    }

    Size FdmVPPStartLimitStepCondition::nStates(Size tMinUp, Size tMinDown,
                                                Size nStarts) {
        return (2*tMinUp+tMinDown)*((nStarts == Null<Size>())? 1 : nStarts+1);
    }

    Real FdmVPPStartLimitStepCondition::maxValue(const Array& states) const {
        return *std::max_element(states.begin(), states.end());
    }

}
