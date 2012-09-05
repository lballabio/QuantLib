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
#include <ql/math/interpolations/bicubicsplineinterpolation.hpp>
#include <ql/methods/finitedifferences/finitedifferencemodel.hpp>
#include <ql/methods/finitedifferences/solvers/fdm3dimsolver.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmsnapshotcondition.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>


namespace QuantLib {

    Fdm3DimSolver::Fdm3DimSolver(
                        const FdmSolverDesc& solverDesc,
                        const FdmSchemeDesc& schemeDesc,
                        const boost::shared_ptr<FdmLinearOpComposite>& op)
    : solverDesc_(solverDesc),
      schemeDesc_(schemeDesc),
      op_(op),
      thetaCondition_(new FdmSnapshotCondition(
        0.99*std::min(1.0/365.0,
                solverDesc.condition->stoppingTimes().empty()
                ? solverDesc.maturity :
                  solverDesc.condition->stoppingTimes().front()))),
      conditions_(FdmStepConditionComposite::joinConditions(thetaCondition_,
                                                         solverDesc.condition)),
      initialValues_(solverDesc.mesher->layout()->size()),
      resultValues_ (solverDesc.mesher->layout()->dim()[2],
                     Matrix(solverDesc.mesher->layout()->dim()[1],
                            solverDesc.mesher->layout()->dim()[0])),
      interpolation_(solverDesc.mesher->layout()->dim()[2]) {

        const boost::shared_ptr<FdmMesher> mesher = solverDesc.mesher;
        const boost::shared_ptr<FdmLinearOpLayout> layout = mesher->layout();

        x_.reserve(layout->dim()[0]);
        y_.reserve(layout->dim()[1]);
        z_.reserve(layout->dim()[2]);

        const FdmLinearOpIterator endIter = layout->end();
        for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
             ++iter) {
            initialValues_[iter.index()]
               = solverDesc.calculator->avgInnerValue(iter,
                                                      solverDesc.maturity);


            if (!iter.coordinates()[1] && !iter.coordinates()[2]) {
                x_.push_back(mesher->location(iter, 0));
            }
            if (!iter.coordinates()[0] && !iter.coordinates()[2]) {
                y_.push_back(mesher->location(iter, 1));
            }
            if (!iter.coordinates()[0] && !iter.coordinates()[1]) {
                z_.push_back(mesher->location(iter, 2));
            }
        }
    }

    void Fdm3DimSolver::performCalculations() const {
        Array rhs(initialValues_.size());
        std::copy(initialValues_.begin(), initialValues_.end(), rhs.begin());

        FdmBackwardSolver(op_, solverDesc_.bcSet, conditions_, schemeDesc_)
             .rollback(rhs, solverDesc_.maturity, 0.0,
                       solverDesc_.timeSteps, solverDesc_.dampingSteps);

        for (Size i=0; i < z_.size(); ++i) {
            std::copy(rhs.begin()+i    *y_.size()*x_.size(),
                      rhs.begin()+(i+1)*y_.size()*x_.size(),
                      resultValues_[i].begin());

            interpolation_[i] = boost::shared_ptr<BicubicSpline> (
                new BicubicSpline(x_.begin(), x_.end(),
                                  y_.begin(), y_.end(),
                                  resultValues_[i]));
        }
    }

    Real Fdm3DimSolver::interpolateAt(Real x, Real y, Rate z) const {
        calculate();

        Array zArray(z_.size());
        for (Size i=0; i < z_.size(); ++i) {
            zArray[i] = interpolation_[i]->operator()(x, y);
        }
        return MonotonicCubicNaturalSpline(z_.begin(), z_.end(),
                                           zArray.begin())(z);
    }

    Real Fdm3DimSolver::thetaAt(Real x, Real y, Rate z) const {
        QL_REQUIRE(conditions_->stoppingTimes().front() > 0.0,
                   "stopping time at zero-> can't calculate theta");
        calculate();

        const Array& rhs = thetaCondition_->getValues();
        std::vector<Matrix> thetaValues(z_.size(), Matrix(y_.size(),x_.size()));
        for (Size i=0; i < z_.size(); ++i) {
            std::copy(rhs.begin()+i    *y_.size()*x_.size(),
                      rhs.begin()+(i+1)*y_.size()*x_.size(),
                      thetaValues[i].begin());
        }

        Array zArray(z_.size());
        for (Size i=0; i < z_.size(); ++i) {
            zArray[i] = BicubicSpline(x_.begin(),x_.end(),
                                      y_.begin(),y_.end(), thetaValues[i])(x,y);
        }

        return (MonotonicCubicNaturalSpline(z_.begin(), z_.end(),
                                            zArray.begin())(z)
                - interpolateAt(x, y, z)) / thetaCondition_->getTime();
    }
}
