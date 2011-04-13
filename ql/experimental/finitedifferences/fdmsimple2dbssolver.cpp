/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ralph Schreyer

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

/*!
 * \file fdmsimple2dbssolver.cpp
*/

#include <ql/methods/finitedifferences/finitedifferencemodel.hpp>
#include <ql/math/interpolations/bicubicsplineinterpolation.hpp>
#include <ql/experimental/finitedifferences/fdmbackwardsolver.hpp>
#include <ql/experimental/finitedifferences/fdmblackscholesop.hpp>
#include <ql/experimental/finitedifferences/fdmsimple2dbssolver.hpp>
#include <ql/experimental/finitedifferences/fdminnervaluecalculator.hpp>
#include <ql/experimental/finitedifferences/fdmmesher.hpp>
#include <ql/experimental/finitedifferences/fdmstepconditioncomposite.hpp>
#include <ql/experimental/finitedifferences/fdmsnapshotcondition.hpp>

namespace QuantLib {

    FdmSimple2dBSSolver::FdmSimple2dBSSolver(
        const Handle<GeneralizedBlackScholesProcess>& process,
        const boost::shared_ptr<FdmMesher>& mesher,
        const FdmSimple2dBSSolver::BoundaryConditionSet& bcSet,
        const boost::shared_ptr<FdmStepConditionComposite> & condition,
        const boost::shared_ptr<FdmInnerValueCalculator>& calculator,
        Real strike,
        Time maturity,
        Size timeSteps,
        const FdmSchemeDesc& schemeDesc)
    : process_(process),
      mesher_(mesher),
      bcSet_(bcSet),
      thetaCondition_(new FdmSnapshotCondition(
        0.99*std::min(1.0/365.0,
                      condition->stoppingTimes().empty() ? maturity :
                                 condition->stoppingTimes().front()))),
      condition_(FdmStepConditionComposite::joinConditions(thetaCondition_, 
                                                           condition)),
      strike_(strike),
      maturity_(maturity),
      timeSteps_(timeSteps),
      schemeDesc_(schemeDesc),
      initialValues_(mesher->layout()->size()),
      resultValues_(mesher->layout()->dim()[1], mesher->layout()->dim()[0]) {
        registerWith(process_);

        x_.reserve(mesher->layout()->dim()[0]);
        a_.reserve(mesher->layout()->dim()[1]);

        const boost::shared_ptr<FdmLinearOpLayout> layout = mesher->layout();
        const FdmLinearOpIterator endIter = layout->end();
        for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
             ++iter) {
            initialValues_[iter.index()] = calculator->avgInnerValue(iter);
            if (!iter.coordinates()[1]) {
                x_.push_back(mesher->location(iter, 0));
            }
            if (!iter.coordinates()[0]) {
                a_.push_back(mesher->location(iter, 1));
            }
        }
    }

    void FdmSimple2dBSSolver::performCalculations() const {
        boost::shared_ptr<FdmBlackScholesOp> map(new FdmBlackScholesOp(
                mesher_, process_.currentLink(), strike_));

        Array rhs(initialValues_.size());
        std::copy(initialValues_.begin(), initialValues_.end(), rhs.begin());

        FdmBackwardSolver(map, bcSet_, condition_, schemeDesc_)
                               .rollback(rhs, maturity_, 0.0, timeSteps_, 0);

        for (Size j=0; j < a_.size(); ++j)
            std::copy(rhs.begin()+j*x_.size(), rhs.begin()+(j+1)*x_.size(),
                      resultValues_.row_begin(j));

        interpolation_ = boost::shared_ptr<BicubicSpline> (
            new BicubicSpline(x_.begin(), x_.end(),
                              a_.begin(), a_.end(),
                              resultValues_));
    }

    Real FdmSimple2dBSSolver::valueAt(Real s, Real a) const {
        calculate();
        return interpolation_->operator()(std::log(s), std::log(a));
    }

    Real FdmSimple2dBSSolver::deltaAt(Real s, Real a, Real eps) const {
        return (valueAt(s+eps, a) - valueAt(s-eps, a))/(2*eps);
    }

    Real FdmSimple2dBSSolver::gammaAt(Real s, Real a, Real eps) const {
        return (valueAt(s+eps, a)+valueAt(s-eps, a)-2*valueAt(s,a))/(eps*eps);
    }

    Real FdmSimple2dBSSolver::thetaAt(Real s, Real a) const {
        QL_REQUIRE(condition_->stoppingTimes().front() > 0.0,
                   "stopping time at zero-> can't calculate theta");

        calculate();
        Matrix thetaValues(resultValues_.rows(), resultValues_.columns());

        const Array& rhs = thetaCondition_->getValues();
        for (Size j=0; j < a_.size(); ++j)
            std::copy(rhs.begin()+j*x_.size(), rhs.begin()+(j+1)*x_.size(),
                      thetaValues.row_begin(j));

        return (BicubicSpline(x_.begin(), x_.end(), a_.begin(), a_.end(),
                              thetaValues)(std::log(s), std::log(a)) - valueAt(s, a))
              / thetaCondition_->getTime();
    }
}
