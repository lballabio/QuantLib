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
#include <ql/experimental/finitedifferences/fdmmesher.hpp>
#include <ql/experimental/finitedifferences/fdmlinearoplayout.hpp>
#include <ql/experimental/finitedifferences/fdmvppstepcondition.hpp>
#include <ql/experimental/finitedifferences/fdmlinearopiterator.hpp>
#include <ql/experimental/finitedifferences/fdminnervaluecalculator.hpp>

#include <boost/bind.hpp>

namespace QuantLib {
    FdmVPPStepCondition::FdmVPPStepCondition(
        Real heatRate,
        Real pMin, Real pMax,
        Size tMinUp, Size tMinDown,
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
      startUpFuel_     (startUpFuel),
      startUpFixCost_  (startUpFixCost),
      carbonPrice_     (carbonPrice),
      stateDirection_  (stateDirection),
      mesher_          (mesher),
      gasPrice_        (gasPrice),
      sparkSpreadPrice_(sparkSpreadPrice),
      stateEvolveFcts_ (2*tMinUp + tMinDown) {

        QL_REQUIRE(tMinUp_ > 1,   "minimum up time must be greater than one");
        QL_REQUIRE(tMinDown_ > 1, "minimum down time must be greater than one");

        for (Size i=0; i < 2*tMinUp; ++i) {
            if (i < tMinUp) {
                stateEvolveFcts_[i] = boost::function<Real (Real)>(
                    boost::bind(&FdmVPPStepCondition::evolveAtPMin,this, _1));
            }
            else {
                stateEvolveFcts_[i] = boost::function<Real (Real)>(
                    boost::bind(&FdmVPPStepCondition::evolveAtPMax,this, _1));
            }
        }
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
                a = changeState(gasPrice, x, t);
            }
        }
    }

    Real FdmVPPStepCondition::evolve(
                               const FdmLinearOpIterator& iter, Time t) const {

        const Size state = iter.coordinates()[stateDirection_];

        if (stateEvolveFcts_[state].empty()) {
            return 0;
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

        for (Size i=0; i < tMinUp_-1; ++i) {
            retVal[i] = retVal[tMinUp_ + i]
                      = std::max(state[i+1], state[tMinUp_ + i+1]);
        }

        retVal[tMinUp_-1] = retVal[2*tMinUp_-1]
                          = std::max(state[2*tMinUp_],
                            std::max(state[tMinUp_-1], state[2*tMinUp_-1]));

        for (Size i=0; i < tMinDown_-1; ++i) {
            retVal[2*tMinUp_ + i] = state[2*tMinUp_ + i+1];
        }

        retVal.back() = std::max(state.back(),
            std::max(state.front(), state[tMinUp_]) - startUpCost);

        return retVal;
    }

    Real FdmVPPStepCondition::evolveAtPMin(Real sparkSpread) const {
        return pMin_*(sparkSpread - heatRate_*carbonPrice_);
    }

    Real FdmVPPStepCondition::evolveAtPMax(Real sparkSpread) const {
        return pMax_*(sparkSpread - heatRate_*carbonPrice_);
    }
}
