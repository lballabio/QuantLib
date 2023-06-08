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

/*! \file fdmvppstepcondition.cpp
*/

#include <ql/experimental/finitedifferences/fdmvppstepcondition.hpp>
#include <ql/math/array.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopiterator.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <utility>

namespace QuantLib {
    FdmVPPStepCondition::FdmVPPStepCondition(
        const FdmVPPStepConditionParams& params,
        Size nStates,
        const FdmVPPStepConditionMesher& mesh,
        ext::shared_ptr<FdmInnerValueCalculator> gasPrice,
        ext::shared_ptr<FdmInnerValueCalculator> sparkSpreadPrice)
    : heatRate_(params.heatRate), pMin_(params.pMin), pMax_(params.pMax), tMinUp_(params.tMinUp),
      tMinDown_(params.tMinDown), startUpFuel_(params.startUpFuel),
      startUpFixCost_(params.startUpFixCost), fuelCostAddon_(params.fuelCostAddon),
      stateDirection_(mesh.stateDirection), nStates_(nStates), mesher_(mesh.mesher),
      gasPrice_(std::move(gasPrice)), sparkSpreadPrice_(std::move(sparkSpreadPrice)),
      stateEvolveFcts_(nStates_) {

        QL_REQUIRE(nStates_ == mesher_->layout()->dim()[stateDirection_],
                   "mesher does not fit to vpp arguments");

        for (Size i=0; i < nStates_; ++i) {
            const Size j = i % (2*tMinUp_ + tMinDown_);

            if (j < tMinUp_) {
                stateEvolveFcts_[i] = [&](Real x){ return evolveAtPMin(x); };
            }
            else if (j < 2*tMinUp_){
                stateEvolveFcts_[i] = [&](Real x) { return evolveAtPMax(x); };
            }
        }
    }


    Size FdmVPPStepCondition::nStates() const {
        return nStates_;
    }


    void FdmVPPStepCondition::applyTo(Array& a, Time t) const {
        const Size nStates = mesher_->layout()->dim()[stateDirection_];

        for (const auto& iter : *mesher_->layout()) {
            a[iter.index()] += evolve(iter, t);
        }

        for (const auto& iter : *mesher_->layout()) {
            if (iter.coordinates()[stateDirection_] == 0U) {

                Array x(nStates);
                for (Size i=0; i < nStates; ++i) {
                    x[i] = a[mesher_->layout()->neighbourhood(iter, stateDirection_, i)];
                }

                const Real gasPrice = gasPrice_->innerValue(iter, t);
                x = changeState(gasPrice, x, t);
                for (Size i=0; i < nStates; ++i) {
                    a[mesher_->layout()->neighbourhood(iter, stateDirection_, i)] = x[i];
                }
            }
        }
    }

    Real FdmVPPStepCondition::evolve(
        const FdmLinearOpIterator& iter, Time t) const {

        const Size state = iter.coordinates()[stateDirection_];

        if (!(stateEvolveFcts_[state])) {
            return 0.0;
        }
        else {
            const Real sparkSpread = sparkSpreadPrice_->innerValue(iter, t);
            return stateEvolveFcts_[state](sparkSpread);
        }
    }


    Real FdmVPPStepCondition::evolveAtPMin(Real sparkSpread) const {
        return pMin_*(sparkSpread - heatRate_*fuelCostAddon_);
    }

    Real FdmVPPStepCondition::evolveAtPMax(Real sparkSpread) const {
        return pMax_*(sparkSpread - heatRate_*fuelCostAddon_);
    }
}
