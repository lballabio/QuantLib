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

#include <ql/processes/blackscholesprocess.hpp>
#include <ql/math/interpolations/bicubicsplineinterpolation.hpp>
#include <ql/methods/finitedifferences/finitedifferencemodel.hpp>
#include <ql/experimental/finitedifferences/fdmmesher.hpp>
#include <ql/experimental/finitedifferences/fdminnervaluecalculator.hpp>
#include <ql/experimental/finitedifferences/fdmdirichletboundary.hpp>
#include <ql/experimental/finitedifferences/fdmblackscholesop.hpp>
#include <ql/experimental/finitedifferences/fdm2dblackscholesop.hpp>
#include <ql/experimental/finitedifferences/fdm2dblackscholessolver.hpp>
#include <ql/experimental/finitedifferences/secondordermixedderivativeop.hpp>
#include <ql/experimental/finitedifferences/fdmstepconditioncomposite.hpp>
#include <ql/experimental/finitedifferences/fdmsnapshotcondition.hpp>


namespace QuantLib {

    Fdm2dBlackScholesSolver::Fdm2dBlackScholesSolver(
        const Handle<GeneralizedBlackScholesProcess>& p1,
        const Handle<GeneralizedBlackScholesProcess>& p2,
        const Real correlation,
        const boost::shared_ptr<FdmMesher>& mesher,
        const FdmBoundaryConditionSet & bcSet,
        const boost::shared_ptr<FdmStepConditionComposite> & condition,
        const boost::shared_ptr<FdmInnerValueCalculator>& calculator,
        Time maturity,
        Size timeSteps,
        Size dampingSteps,
        const FdmSchemeDesc& schemeDesc)
    : p1_(p1),
      p2_(p2),
      correlation_(correlation),
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
      initialValues_(mesher->layout()->size()),
      resultValues_(mesher->layout()->dim()[1], mesher->layout()->dim()[0]) {

        registerWith(p1_);
        registerWith(p2_);

        x_.reserve(mesher->layout()->dim()[0]);
        y_.reserve(mesher->layout()->dim()[1]);

        const boost::shared_ptr<FdmLinearOpLayout> layout = mesher->layout();
        const FdmLinearOpIterator endIter = layout->end();
        for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
             ++iter) {
            initialValues_[iter.index()] = calculator->avgInnerValue(iter);

            if (!iter.coordinates()[1]) {
                x_.push_back(mesher->location(iter, 0));
            }
            if (!iter.coordinates()[0]) {
                y_.push_back(mesher->location(iter, 1));
            }
        }
    }

        
    void Fdm2dBlackScholesSolver::performCalculations() const {
        
        boost::shared_ptr<Fdm2dBlackScholesOp> map(
                new Fdm2dBlackScholesOp(mesher_, 
                                        p1_.currentLink(), 
                                        p2_.currentLink(), 
                                        correlation_, maturity_));

        Array rhs(initialValues_.size());
        std::copy(initialValues_.begin(), initialValues_.end(), rhs.begin());

        FdmBackwardSolver(map, bcSet_, condition_, schemeDesc_)
                      .rollback(rhs, maturity_, 0.0, timeSteps_, dampingSteps_);

        std::copy(rhs.begin(), rhs.end(), resultValues_.begin());
        interpolation_ = boost::shared_ptr<BicubicSpline> (
            new BicubicSpline(x_.begin(), x_.end(),
                              y_.begin(), y_.end(),
                              resultValues_));
    }

    Real Fdm2dBlackScholesSolver::valueAt(Real u, Real v) const {
        calculate();
        const Real x = std::log(u);
        const Real y = std::log(v);

        return interpolation_->operator()(x, y);
    }
    
    Real Fdm2dBlackScholesSolver::thetaAt(Real u, Real v) const {
        QL_REQUIRE(condition_->stoppingTimes().front() > 0.0,
                   "stopping time at zero-> can't calculate theta");

        calculate();
        Matrix thetaValues(resultValues_.rows(), resultValues_.columns());

        const Array& rhs = thetaCondition_->getValues();
        std::copy(rhs.begin(), rhs.end(), thetaValues.begin());

        const Real x = std::log(u);
        const Real y = std::log(v);

        return (BicubicSpline(x_.begin(), x_.end(), y_.begin(), y_.end(),
                              thetaValues)(x, y) - valueAt(u, v))
              / thetaCondition_->getTime();
    }


    Real Fdm2dBlackScholesSolver::deltaXat(Real u, Real v) const {
        calculate();

        const Real x = std::log(u);
        const Real y = std::log(v);

        return interpolation_->derivativeX(x, y)/u;
    }

    Real Fdm2dBlackScholesSolver::deltaYat(Real u, Real v) const {
        calculate();

        const Real x = std::log(u);
        const Real y = std::log(v);

        return interpolation_->derivativeY(x, y)/v;
    }

    Real Fdm2dBlackScholesSolver::gammaXat(Real u, Real v) const {
        calculate();
        
        const Real x = std::log(u);
        const Real y = std::log(v);
        
        return (interpolation_->secondDerivativeX(x, y)
                -interpolation_->derivativeX(x, y))/(u*u);
    }

    Real Fdm2dBlackScholesSolver::gammaYat(Real u, Real v) const {
        calculate();
        
        const Real x = std::log(u);
        const Real y = std::log(v);
        
        return (interpolation_->secondDerivativeY(x, y)
                -interpolation_->derivativeY(x, y))/(v*v);
    }
}
