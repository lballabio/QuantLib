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

#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/methods/finitedifferences/finitedifferencemodel.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/methods/finitedifferences/solvers/fdm1dimsolver.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmsnapshotcondition.hpp>

namespace QuantLib {

    Fdm1DimSolver::Fdm1DimSolver(
                             const FdmSolverDesc& solverDesc,
                             const FdmSchemeDesc& schemeDesc,
                             const boost::shared_ptr<FdmLinearOpComposite>& op)
    : solverDesc_(solverDesc),
      schemeDesc_(schemeDesc),
      op_(op),
      thetaCondition_(new FdmSnapshotCondition(
        0.99*std::min(1.0/365.0,
           solverDesc.condition->stoppingTimes().empty()
                    ? solverDesc.maturity
                    : solverDesc.condition->stoppingTimes().front()))),
      conditions_(FdmStepConditionComposite::joinConditions(thetaCondition_,
                                                         solverDesc.condition)),
      x_            (solverDesc.mesher->layout()->size()),
      initialValues_(solverDesc.mesher->layout()->size()),
      resultValues_ (solverDesc.mesher->layout()->size()) {

        const boost::shared_ptr<FdmMesher> mesher = solverDesc.mesher;
        const boost::shared_ptr<FdmLinearOpLayout> layout = mesher->layout();

        const FdmLinearOpIterator endIter = layout->end();
        for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
             ++iter) {
            initialValues_[iter.index()]
                 = solverDesc_.calculator->avgInnerValue(iter,
                                                         solverDesc.maturity);
            x_[iter.index()] = mesher->location(iter, 0);
        }
    }


    void Fdm1DimSolver::performCalculations() const {
        Array rhs(initialValues_.size());
        std::copy(initialValues_.begin(), initialValues_.end(), rhs.begin());

        FdmBackwardSolver(op_, solverDesc_.bcSet, conditions_, schemeDesc_)
            .rollback(rhs, solverDesc_.maturity, 0.0,
                      solverDesc_.timeSteps, solverDesc_.dampingSteps);

        std::copy(rhs.begin(), rhs.end(), resultValues_.begin());
        interpolation_ = boost::shared_ptr<CubicInterpolation>(new
            MonotonicCubicNaturalSpline(x_.begin(), x_.end(),
                                        resultValues_.begin()));
    }

    Real Fdm1DimSolver::interpolateAt(Real x) const {
        calculate();
        return interpolation_->operator()(x);
    }

    Real Fdm1DimSolver::thetaAt(Real x) const {
        QL_REQUIRE(conditions_->stoppingTimes().front() > 0.0,
                   "stopping time at zero-> can't calculate theta");

        calculate();
        Array thetaValues(resultValues_.size());

        const Array& rhs = thetaCondition_->getValues();
        std::copy(rhs.begin(), rhs.end(), thetaValues.begin());

        Real temp = MonotonicCubicNaturalSpline(
            x_.begin(), x_.end(), thetaValues.begin())(x);
        return ( temp - interpolateAt(x) ) / thetaCondition_->getTime();
    }


    Real Fdm1DimSolver::derivativeX(Real x) const {
        calculate();
        return interpolation_->derivative(x);
    }

    Real Fdm1DimSolver::derivativeXX(Real x) const {
        calculate();
        return interpolation_->secondDerivative(x);
    }
}
