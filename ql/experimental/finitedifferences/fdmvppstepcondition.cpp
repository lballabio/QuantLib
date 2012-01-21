/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen

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

/*! \file fdmvppstatetransitionmatrix.cpp
*/

#include <ql/math/array.hpp>
#include <ql/math/functional.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/experimental/finitedifferences/fdmvppstepcondition.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopiterator.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>

#include <boost/bind.hpp>

namespace QuantLib {
    FdmVPPStepCondition::FdmVPPStepCondition(
        Real heatRate,
        Real pMin, Real pMax,
        Size tMinUp, Size tMinDown,
        Size nStarts,
        Real startUpFuel, Real startUpFixCost,
        Real carbonPrice,
        Size stateDirection,
        const boost::shared_ptr<FdmMesher>& mesher,
        const boost::shared_ptr<FdmInnerValueCalculator>& gasPrice,
        const boost::shared_ptr<FdmInnerValueCalculator>& sparkSpreadPrice)
    : heatRate_        (heatRate),
      pMin_            (pMin),
      pMax_            (pMax),
      tMinUp_          (tMinUp),
      tMinDown_        (tMinDown),
      nStarts_         (nStarts),
      startUpFuel_     (startUpFuel),
      startUpFixCost_  (startUpFixCost),
      carbonPrice_     (carbonPrice),
      stateDirection_  (stateDirection),
      mesher_          (mesher),
      gasPrice_        (gasPrice),
      sparkSpreadPrice_(sparkSpreadPrice),
      nStates_         ((2*tMinUp + tMinDown)
                          *((nStarts == Null<Size>()) ? 1 : nStarts+1)),
      stateEvolveFcts_ (nStates_) {

        QL_REQUIRE(tMinUp_ > 1,   "minimum up time must be greater than one");
        QL_REQUIRE(tMinDown_ > 1, "minimum down time must be greater than one");
        QL_REQUIRE(stateEvolveFcts_.size() ==
                   mesher_->layout()->dim()[stateDirection_],
                   "mesher does not fit to vpp arguments");

        for (Size i=0; i < nStates_; ++i) {
            const Size j = i % (2*tMinUp + tMinDown);

            if (j < tMinUp) {
                stateEvolveFcts_[i] = boost::function<Real (Real)>(
                    boost::bind(&FdmVPPStepCondition::evolveAtPMin,this, _1));
            }
            else if (j < 2*tMinUp){
                stateEvolveFcts_[i] = boost::function<Real (Real)>(
                    boost::bind(&FdmVPPStepCondition::evolveAtPMax,this, _1));
            }
        }
    }


    Size FdmVPPStepCondition::nStates() const {
        return nStates_;
    }


    void FdmVPPStepCondition::applyTo(Array& a, Time t) const {
        boost::shared_ptr<FdmLinearOpLayout> layout = mesher_->layout();

        const Size nStates = layout->dim()[stateDirection_];
        const FdmLinearOpIterator endIter = layout->end();

        for (FdmLinearOpIterator iter=layout->begin();iter != endIter; ++iter) {
            a[iter.index()] += evolve(iter, t);
        }

        for (FdmLinearOpIterator iter=layout->begin();iter != endIter; ++iter) {
            if (!iter.coordinates()[stateDirection_]) {

                Array x(nStates);
                for (Size i=0; i < nStates; ++i) {
                    x[i] = a[layout->neighbourhood(iter, stateDirection_, i)];
                }

                const Real gasPrice = gasPrice_->innerValue(iter, t);
                x = changeState(gasPrice, x, t);
                for (Size i=0; i < nStates; ++i) {
                    a[layout->neighbourhood(iter, stateDirection_, i)] = x[i];
                }
            }
        }
    }

    Real FdmVPPStepCondition::evolve(
                               const FdmLinearOpIterator& iter, Time t) const {

        const Size state = iter.coordinates()[stateDirection_];

        if (stateEvolveFcts_[state].empty()) {
            return 0.0;
        }
        else {
            const Real sparkSpread = sparkSpreadPrice_->innerValue(iter, t);
            return stateEvolveFcts_[state](sparkSpread);
        }
    }

    Disposable<Array> FdmVPPStepCondition::changeState(
                        const Real gasPrice, const Array& state, Time t) const {
        const Real startUpCost
                = startUpFixCost_ + (gasPrice + carbonPrice_)*startUpFuel_;

        Array retVal(state.size());
        const Size sss = 2*tMinUp_ + tMinDown_;
        const Size n = sss * (nStarts_ == Null<Size>() ? 1 : nStarts_+1);

        for (Size i=0; i < n; ++i) {
            const Size j = i % sss;

            if (j < tMinUp_-1) {
                retVal[i] = std::max(state[i+1], state[tMinUp_+i+1]);
            }
            else if (j == tMinUp_-1) {
                retVal[i] = std::max(state[i+tMinUp_+1],
                                     std::max(state[i], state[i+tMinUp_]));
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

    Real FdmVPPStepCondition::evolveAtPMin(Real sparkSpread) const {
        return pMin_*(sparkSpread - heatRate_*carbonPrice_);
    }

    Real FdmVPPStepCondition::evolveAtPMax(Real sparkSpread) const {
        return pMax_*(sparkSpread - heatRate_*carbonPrice_);
    }
}
