/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008, 2009 Ralph Schreyer
 Copyright (C) 2008, 2009 Klaus Spanderen

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

#include <ql/processes/hestonprocess.hpp>
#include <ql/methods/finitedifferences/finitedifferencemodel.hpp>
#include <ql/experimental/finitedifferences/fdminnervaluecalculator.hpp>
#include <ql/experimental/finitedifferences/fdmdirichletboundary.hpp>
#include <ql/experimental/finitedifferences/fdmhestonsolver.hpp>
#include <ql/experimental/finitedifferences/fdmstepconditioncomposite.hpp>
#include <ql/experimental/finitedifferences/fdmmesher.hpp>
#include <ql/experimental/finitedifferences/fdmsnapshotcondition.hpp>
#include <ql/experimental/finitedifferences/fdmhestonop.hpp>
#include <ql/math/interpolations/bicubicsplineinterpolation.hpp>


namespace QuantLib {

    FdmHestonSolver::FdmHestonSolver(
        const Handle<HestonProcess>& process,
        const boost::shared_ptr<FdmMesher>& mesher,
        const FdmBoundaryConditionSet& bcSet,
        const boost::shared_ptr<FdmStepConditionComposite> & condition,
        const boost::shared_ptr<FdmInnerValueCalculator>& calculator,
        Time maturity,
        Size timeSteps,
        Size dampingSteps,
        const FdmSchemeDesc& schemeDesc,
        const Handle<FdmQuantoHelper>& quantoHelper)
    : process_(process),
      mesher_(mesher),
      bcSet_(bcSet),
      thetaCondition_(new FdmSnapshotCondition(
        0.99*std::min(1.0/365.0,
                      condition->stoppingTimes().empty() ? maturity :
                                 condition->stoppingTimes().front()))),
      condition_(FdmStepConditionComposite::joinConditions(thetaCondition_, 
                                                           condition)),
      maturity_(maturity),
      timeSteps_(timeSteps),
      dampingSteps_(dampingSteps),
      schemeDesc_(schemeDesc),
      quantoHelper_(quantoHelper),
      initialValues_(mesher->layout()->size()),
      resultValues_(mesher->layout()->dim()[1], mesher->layout()->dim()[0]) {
        registerWith(process_);
        registerWith(quantoHelper_);

        x_.reserve(mesher->layout()->dim()[0]);
        v_.reserve(mesher->layout()->dim()[1]);

        const boost::shared_ptr<FdmLinearOpLayout> layout = mesher->layout();
        const FdmLinearOpIterator endIter = layout->end();
        for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
             ++iter) {
            initialValues_[iter.index()] = calculator->avgInnerValue(iter);

            if (!iter.coordinates()[1]) {
                x_.push_back(mesher->location(iter, 0));
            }
            if (!iter.coordinates()[0]) {
                v_.push_back(mesher->location(iter, 1));
            }
        }
    }

    void FdmHestonSolver::backwardSolve(
                       boost::shared_ptr<FdmLinearOpComposite>& map) const {

        Array rhs(initialValues_.size());
        std::copy(initialValues_.begin(), initialValues_.end(), rhs.begin());

        FdmBackwardSolver(map, bcSet_, condition_, schemeDesc_)
            .rollback(rhs, maturity_, 0.0, timeSteps_, dampingSteps_);

        std::copy(rhs.begin(), rhs.end(), resultValues_.begin());
        interpolation_ = boost::shared_ptr<BicubicSpline> (
            new BicubicSpline(x_.begin(), x_.end(),
                              v_.begin(), v_.end(),
                              resultValues_));
    }
        
    void FdmHestonSolver::performCalculations() const {
        boost::shared_ptr<FdmLinearOpComposite> map(
                new FdmHestonOp(
                        mesher_, process_.currentLink(),
                        (!quantoHelper_.empty()) ? quantoHelper_.currentLink()
                                     : boost::shared_ptr<FdmQuantoHelper>()));
        this->backwardSolve(map);
    }

    Real FdmHestonSolver::valueAt(Real s, Real v) const {
        calculate();
        const Real x = std::log(s);
        return interpolation_->operator()(x, v);
    }

    Real FdmHestonSolver::deltaAt(Real s, Real v) const {
        calculate();
        const Real x = std::log(s);
        return interpolation_->derivativeX(x, v)/s;
    }

    Real FdmHestonSolver::gammaAt(Real s, Real v) const {
        calculate();
        const Real x = std::log(s);
        return (interpolation_->secondDerivativeX(x, v)
                -interpolation_->derivativeX(x, v))/(s*s);
    }

    Real FdmHestonSolver::meanVarianceDeltaAt(Real s, Real v) const {
        calculate();
        const Real x = std::log(s);
        const Real alpha = process_->rho()*process_->sigma()/s;
        return deltaAt(s, v) + alpha*interpolation_->derivativeY(x, v);
    }

    Real FdmHestonSolver::meanVarianceGammaAt(Real s, Real v) const {
        calculate();
        const Real x = std::log(s);
        const Real alpha = process_->rho()*process_->sigma()/s;
        return gammaAt(s, v)
                +  interpolation_->secondDerivativeY(x, v)*alpha*alpha
                +2*interpolation_->derivativeXY(x, v)*alpha/s;
    }

    Real FdmHestonSolver::thetaAt(Real s, Real v) const {
        QL_REQUIRE(condition_->stoppingTimes().front() > 0.0,
                   "stopping time at zero-> can't calculate theta");

        calculate();
        Matrix thetaValues(resultValues_.rows(), resultValues_.columns());

        const Array& rhs = thetaCondition_->getValues();
        std::copy(rhs.begin(), rhs.end(), thetaValues.begin());

        return (BicubicSpline(x_.begin(), x_.end(), v_.begin(), v_.end(),
                              thetaValues)(std::log(s), v) - valueAt(s, v))
              / thetaCondition_->getTime();
    }
}
