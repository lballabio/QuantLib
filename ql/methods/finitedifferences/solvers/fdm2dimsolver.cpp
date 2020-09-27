/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Klaus Spanderen

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

#include <ql/math/interpolations/bicubicsplineinterpolation.hpp>
#include <ql/methods/finitedifferences/finitedifferencemodel.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/methods/finitedifferences/solvers/fdm2dimsolver.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmsnapshotcondition.hpp>

namespace QuantLib {

    Fdm2DimSolver::Fdm2DimSolver(
                             const FdmSolverDesc& solverDesc,
                             const FdmSchemeDesc& schemeDesc,
                             const ext::shared_ptr<FdmLinearOpComposite>& op)
    : solverDesc_(solverDesc),
      schemeDesc_(schemeDesc),
      op_(op),
      thetaCondition_(ext::make_shared<FdmSnapshotCondition>(
        0.99*std::min(1.0/365.0,
           solverDesc.condition->stoppingTimes().empty()
                    ? solverDesc.maturity
                    : solverDesc.condition->stoppingTimes().front()))),
      conditions_(FdmStepConditionComposite::joinConditions(thetaCondition_,
                                                         solverDesc.condition)),
      initialValues_(solverDesc.mesher->layout()->size()),
      resultValues_ (solverDesc.mesher->layout()->dim()[1],
                     solverDesc.mesher->layout()->dim()[0]) {

        const ext::shared_ptr<FdmMesher> mesher = solverDesc.mesher;
        const ext::shared_ptr<FdmLinearOpLayout> layout = mesher->layout();

        x_.reserve(layout->dim()[0]);
        y_.reserve(layout->dim()[1]);

        const FdmLinearOpIterator endIter = layout->end();
        for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
             ++iter) {
            initialValues_[iter.index()]
                 = solverDesc_.calculator->avgInnerValue(iter,
                                                         solverDesc.maturity);

            if (iter.coordinates()[1] == 0U) {
                x_.push_back(mesher->location(iter, 0));
            }
            if (iter.coordinates()[0] == 0U) {
                y_.push_back(mesher->location(iter, 1));
            }
        }
    }


    void Fdm2DimSolver::performCalculations() const {
        Array rhs(initialValues_.size());
        std::copy(initialValues_.begin(), initialValues_.end(), rhs.begin());

        FdmBackwardSolver(op_, solverDesc_.bcSet, conditions_, schemeDesc_)
            .rollback(rhs, solverDesc_.maturity, 0.0,
                      solverDesc_.timeSteps, solverDesc_.dampingSteps);

        std::copy(rhs.begin(), rhs.end(), resultValues_.begin());
        interpolation_ = ext::make_shared<BicubicSpline>(x_.begin(), x_.end(),
                              y_.begin(), y_.end(),
                              resultValues_);
    }

    Real Fdm2DimSolver::interpolateAt(Real x, Real y) const {
        calculate();
        return (*interpolation_)(x, y);
    }

    Real Fdm2DimSolver::thetaAt(Real x, Real y) const {
        if (conditions_->stoppingTimes().front() == 0.0)
            return Null<Real>();

        calculate();
        Matrix thetaValues(resultValues_.rows(), resultValues_.columns());

        const Array& rhs = thetaCondition_->getValues();
        std::copy(rhs.begin(), rhs.end(), thetaValues.begin());

        return (BicubicSpline(x_.begin(), x_.end(), y_.begin(), y_.end(),
                              thetaValues)(x, y) - interpolateAt(x, y))
              / thetaCondition_->getTime();
    }


    Real Fdm2DimSolver::derivativeX(Real x, Real y) const {
        calculate();
        return interpolation_->derivativeX(x, y);
    }

    Real Fdm2DimSolver::derivativeY(Real x, Real y) const {
        calculate();
        return interpolation_->derivativeY(x, y);
    }

    Real Fdm2DimSolver::derivativeXX(Real x, Real y) const {
        calculate();
        return interpolation_->secondDerivativeX(x, y);
    }

    Real Fdm2DimSolver::derivativeYY(Real x, Real y) const {
        calculate();
        return interpolation_->secondDerivativeY(x, y);
    }

    Real Fdm2DimSolver::derivativeXY(Real x, Real y) const {
        calculate();
        return interpolation_->derivativeXY(x, y);
    }

}
