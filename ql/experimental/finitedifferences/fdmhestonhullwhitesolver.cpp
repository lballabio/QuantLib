/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Klaus Spanderen

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
#include <ql/experimental/finitedifferences/fdmhestonhullwhitesolver.hpp>
#include <ql/experimental/finitedifferences/fdmstepconditioncomposite.hpp>
#include <ql/experimental/finitedifferences/fdmmesher.hpp>
#include <ql/experimental/finitedifferences/fdmsnapshotcondition.hpp>
#include <ql/experimental/finitedifferences/fdmquantohelper.hpp>
#include <ql/experimental/finitedifferences/fdmhestonhullwhiteop.hpp>
#include <ql/experimental/finitedifferences/fdminnervaluecalculator.hpp>


namespace QuantLib {

    FdmHestonHullWhiteSolver::FdmHestonHullWhiteSolver(
        const Handle<HestonProcess>& hestonProcess,
        const Handle<HullWhiteProcess>& hwProcess,
        Rate corrEquityShortRate,
        const boost::shared_ptr<FdmMesher>& mesher,
        const FdmBoundaryConditionSet& bcSet,
        const boost::shared_ptr<FdmStepConditionComposite> & condition,
        const boost::shared_ptr<FdmInnerValueCalculator>& calculator,
        Time maturity,
        Size timeSteps,
        Size dampingSteps,
        const FdmSchemeDesc& schemeDesc)
    : hestonProcess_(hestonProcess),
      hwProcess_(hwProcess),
      corrEquityShortRate_(corrEquityShortRate),
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
      resultValues_(mesher->layout()->dim()[2], 
                    Matrix(mesher->layout()->dim()[1], 
                           mesher->layout()->dim()[0])),
      interpolation_(mesher->layout()->dim()[2]) {

        registerWith(hestonProcess);
        registerWith(hwProcess);
        
        x_.reserve(mesher->layout()->dim()[0]);
        v_.reserve(mesher->layout()->dim()[1]);
        r_.reserve(mesher->layout()->dim()[2]);

        const boost::shared_ptr<FdmLinearOpLayout> layout = mesher->layout();
        const FdmLinearOpIterator endIter = layout->end();
        for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
             ++iter) {
            initialValues_[iter.index()] = calculator->avgInnerValue(iter);


            if (!iter.coordinates()[1] && !iter.coordinates()[2]) {
                x_.push_back(mesher->location(iter, 0));
            }
            if (!iter.coordinates()[0] && !iter.coordinates()[2]) {
                v_.push_back(mesher->location(iter, 1));
            }
            if (!iter.coordinates()[0] && !iter.coordinates()[1]) {
                r_.push_back(mesher->location(iter, 2));
            }
        }
    }

    void FdmHestonHullWhiteSolver::performCalculations() const {
        boost::shared_ptr<FdmLinearOpComposite> map(
            new FdmHestonHullWhiteOp(mesher_, 
                                     hestonProcess_.currentLink(), 
                                     hwProcess_.currentLink(), 
                                     corrEquityShortRate_));

        Array rhs(initialValues_.size());
        std::copy(initialValues_.begin(), initialValues_.end(), rhs.begin());

        FdmBackwardSolver(map, bcSet_, condition_, schemeDesc_)
                   .rollback(rhs, maturity_, 0.0, timeSteps_, dampingSteps_);

        for (Size i=0; i < r_.size(); ++i) {
            std::copy(rhs.begin()+i    *v_.size()*x_.size(), 
                      rhs.begin()+(i+1)*v_.size()*x_.size(),
                      resultValues_[i].begin());

            interpolation_[i] = boost::shared_ptr<BicubicSpline> (
                new BicubicSpline(x_.begin(), x_.end(),
                                  v_.begin(), v_.end(),
                                  resultValues_[i]));
        }
    }

    Real FdmHestonHullWhiteSolver::valueAt(Real s, Real v, Rate r) const {
        
        calculate();
        
        Array y(r_.size());
        const Real x = std::log(s);
        for (Size i=0; i < r_.size(); ++i) {
            y[i] = interpolation_[i]->operator()(x, v);
        }
        return MonotonicCubicNaturalSpline(r_.begin(), r_.end(), y.begin())(r);
    }

    Real FdmHestonHullWhiteSolver::deltaAt(Real s, Real v, Rate r, Real eps) 
    const {
        return (valueAt(s+eps, v, r) - valueAt(s-eps, v, r))/(2*eps);
    }

    Real FdmHestonHullWhiteSolver::gammaAt(Real s, Real v, Rate r, Real eps) 
    const {
        return (valueAt(s+eps, v, r)+valueAt(s-eps, v,r )
                -2*valueAt(s, v, r))/(eps*eps);
    }

    Real FdmHestonHullWhiteSolver::thetaAt(Real s, Real v, Rate r) const {
        QL_REQUIRE(condition_->stoppingTimes().front() > 0.0,
                   "stopping time at zero-> can't calculate theta");

        calculate();

        const Array& rhs = thetaCondition_->getValues();
        std::vector<Matrix> thetaValues(r_.size(), Matrix(v_.size(),x_.size()));
        for (Size i=0; i < r_.size(); ++i) {
            std::copy(rhs.begin()+i    *v_.size()*x_.size(), 
                      rhs.begin()+(i+1)*v_.size()*x_.size(),
                      thetaValues[i].begin());
        }

        Array y(r_.size());
        const Real x = std::log(s);
        for (Size i=0; i < r_.size(); ++i) {
            y[i] = BicubicSpline(x_.begin(), x_.end(),
                                 v_.begin(), v_.end(), thetaValues[i])(x, v);
        }
                
        return (MonotonicCubicNaturalSpline(r_.begin(), r_.end(), y.begin())(r)
                - valueAt(s, v, r)) / thetaCondition_->getTime();
    }
}
